/***************************boon_in.cpp***************************************
              功能：处理入口
              创建时间：2017-02-05
              创建人：孙振行
              单位：山东博昂信息科技有限公司
              修改时间：
                1. 时间：20171201 17:41 , add by diaoguangqiang
                   修改： 针对交运停车场洗车功能，入口进行权限判断，并减少计次次数
***************************************************************************/
#include "boon_log.h"
#include "common_def.h"
#include "../bcenter_def.h"

#include "boon_in.h"

using namespace BASE;

static char plate_ori[PLATE_LENGTH]; // 原始车牌
static char plate_last[PLATE_LENGTH]; // 上一次车牌
static char plate_final[PLATE_LENGTH]; //最终车牌
static char auth_type[24]; //授权类型
static char open_door[24]; //开闸类型
static char one_way[24]; //同口进出
static char mohu_match[24]; //模糊匹配
static char park_id[256]; //车场id号
static bool auth; //是否授权
static char global_car_type[24]; //车辆类型
static int global_remain_day; //剩余天数
static int global_remain_hour; //剩余小时
static int global_remain_money; //剩余钱数 /元
static int global_remain_jici = 0; // 剩余计次次数
static char global_charge_rule[256]; //收费方案  
static char speak_send[256]; //语音信息
static bool zhuji; //使用主机数据标识
char in_plate_last[PLATE_LENGTH];
long   in_plate_time;
static long plate_last_time = 0;
extern char out_plate_last[PLATE_LENGTH];
extern long out_plate_time;
extern char in_out_flag[24]; // 入口或出口标记
bool in_led_guoqi_flag = false; 
bool in_led_noauth_flag = false; 
bool in_guoqi_flag;//入口车辆过期标志
char in_plate[PLATE_LENGTH];
char in_channel[256];
char in_time[24];
char in_led_ip[24];
bool flagOpenDoor;
extern bool in_fleet;
extern pthread_mutex_t mongo_mutex_channel;
extern pthread_mutex_t mongo_mutex_carinpark;
extern pthread_mutex_t mongo_mutex_car;
extern pthread_mutex_t mongo_mutex_device;
extern char remain_car_time[128];

// 日志
static char log_buf_[LOG_BUF_SIZE] = {0};
// 记录日志
extern void writelog(const char* _buf);
// 是否加入车队模式，不在此赋值, 在main.cpp定义
extern bool g_has_cheduimoshi_;
// 车入场时候持续时间，单位秒, 不在此赋值， 在main.cpp中定义
extern int g_car_in_last_time_;

/**************************************************处理辅机******************************/
static int process_fuji(car_msg *in_msg)  
{
    memset(plate_ori,0,PLATE_LENGTH);
    if(in_msg->num == 1)  //只有一个相机，则返回
    {
        zhuji = true;
        memcpy(plate_ori,in_msg->plate,strlen(in_msg->plate));
        return 0;
    }

    if(strcmp(in_msg->plate,in_msg->plate1) == 0) //两者车牌一致，取主机车牌号
    {
        zhuji = true;
        memcpy(plate_ori,in_msg->plate,strlen(in_msg->plate));
        return 0;
    }

    if(strcmp("无车牌",in_msg->plate) == 0) //主机车牌号为无车牌，则取辅机车牌号
    {
        zhuji = false;
        memcpy(plate_ori,in_msg->plate1,strlen(in_msg->plate1));
        return 0;
    }

    if(strcmp("无车牌",in_msg->plate1) == 0)//辅机车牌号为无车牌，则取主机车牌号
    {
        zhuji = true;
        memcpy(plate_ori,in_msg->plate,strlen(in_msg->plate));
        return 0;
    }

    if(mongodb_flag) //查询mongodb
    {
        if(mongodb_query_car(in_msg->plate) > 0) // 主机车牌号在car表查到，则取主机车牌号
        {
            zhuji = true;
            memcpy(plate_ori,in_msg->plate,strlen(in_msg->plate));
            return 0;
        }
        if(mongodb_query_car(in_msg->plate) < 0)
        {
            return -1;
        }
        if(mongodb_query_car(in_msg->plate1) > 0) //辅机车牌号在car表查到，则取辅机车牌号
        {
            zhuji = false;
            memcpy(plate_ori,in_msg->plate1,strlen(in_msg->plate1));
            return 0;
        }
        if(mongodb_query_car(in_msg->plate1) < 0)
        {
            return -1;
        }
        memcpy(plate_ori,in_msg->plate,strlen(in_msg->plate)); //两个车牌在car表都未查到，则取主机车牌号
    }
    else //查询mysql
    {
        if(mysql_query_car((char*)"入口",in_msg->plate) > 0) // 主机车牌号在car表查到，则取主机车牌号
        {
            zhuji = true;
            memcpy(plate_ori,in_msg->plate,strlen(in_msg->plate));
            return 0;
        }
        if(mysql_query_car((char*)"入口",in_msg->plate1) > 0) //辅机车牌号在car表查到，则取辅机车牌号
        {
            zhuji = false;
            memcpy(plate_ori,in_msg->plate1,strlen(in_msg->plate1));
            return 0;
        }
        zhuji = true;
        memcpy(plate_ori,in_msg->plate,strlen(in_msg->plate)); //两个车牌在car表都未查到，则取主机车牌号
    }
}
/**************************************************end******************************************/
/**************************************************模糊匹配******************************/
/**
 * @brief: 模糊匹配
 *             1. 先查car表；
 *             2.
 * @param in_msg
 * @return
 * @attention: 匹配入口表和出口表 ----> 只匹配出口表，去除入口表
 */
static int process_mohu_match(car_msg *in_msg)  
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 入口 开始模糊匹配 line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    if(zhuji != true)
    {
        memset(in_msg->pcolor,0,strlen(in_msg->pcolor));
        memcpy(in_msg->pcolor,in_msg->pcolor1,strlen(in_msg->pcolor1));
    }

    memset(plate_final,0,PLATE_LENGTH);
    if(mongodb_flag)  //查询mongodb
    {
        if(strcmp("是",mohu_match) == 0) //进行模糊匹配
        {
            if(mongodb_query_car(plate_ori)) //在car表查到该车牌号，就不进行模糊匹配
            {
                fprintf(fp_log,"%s##模糊匹配，在car表查到该车辆，模糊匹配退出\n",time_now);
                memcpy(plate_final,plate_ori,PLATE_LENGTH);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 模糊匹配 在car表找到 plate_ori[%s] line[%d]", __FUNCTION__, plate_ori, __LINE__);
                writelog(log_buf_);

                return 0;
            }

            if(mongodb_mohu_match(in_msg->pcolor,plate_ori,plate_final) < 0) //mongodb的模糊匹配
                return -1;
        }
        else //不进行模糊匹配
        {
            memcpy(plate_final,plate_ori,strlen(plate_ori));
            fprintf(fp_log,"%s##该通道不进行模糊匹配\n",time_now);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] 该通道不进行模糊匹配 plate_ori[%s] line[%d]", __FUNCTION__, plate_ori, __LINE__);
            writelog(log_buf_);
            return 0;
        }
    }
    else  //查询mysql
    {
        if(strcmp("是",mohu_match) == 0)  //进行模糊匹配
        {
            if(mysql_query_car((char*)"入口",plate_ori)) //在car表查到该车牌号，就不进行模糊匹配
            {
                fprintf(fp_log,"%s##模糊匹配，在car表查到该车辆，模糊匹配退出\n",time_now);
                memcpy(plate_final,plate_ori,PLATE_LENGTH);
                return 0;
            }
            mysql_mohu_match((char*)"入口",in_msg->pcolor,plate_ori,plate_final); //mysql的模糊匹配
        }
        else
        {
            memcpy(plate_final,plate_ori,strlen(plate_ori));
            fprintf(fp_log,"%s##该通道不进行模糊匹配\n",time_now);
            return 0;
        }
    }
}
/**************************************************end******************************************/
/*********************************************删除该车牌号的在场记录******************************/
static int delete_car_inpark(char * plate,char *park_id)
{
    if(mongodb_flag)
    {
        mongodb_delete_car_inpark(plate,park_id);
    }
    else
    {
        mysql_delete_car_inpark((char*)"入口",plate,park_id);
    }
}
/**************************************************end******************************************/
/*********************************************处理通道授权****************************************/
static int process_auth(car_msg *in_msg)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] [%s] line[%d]", __FUNCTION__, auth_type, __LINE__);
    writelog(log_buf_);

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    auth = false;
    if(strcmp("按车牌号授权",auth_type) == 0) //按车牌号授权
    {
        if(mongodb_flag) //查询mongodb
        {
            printf("auth1=======================\n");
            if(mongodb_auth_plate(in_msg->channel_id,plate_final,&auth) < 0)
                return -1;
        }
        else
        {
            printf("auth2=======================\n");
            if(mysql_auth_plate((char*)"入口",in_msg->channel_id,plate_final,&auth) < 0)
                return -1;
        }

        if(auth == true)
        {
            fprintf(fp_log,"%s##按车牌号授权，该车辆已授权\n",time_now);
        }
        else
        {
            fprintf(fp_log,"%s##按车牌号授权，该车辆未授权\n",time_now);
        }

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 按车牌号授权 auth[%d] line[%d]", __FUNCTION__, auth, __LINE__);
        writelog(log_buf_);
    }
    else if(strcmp("按车辆类型授权",auth_type) == 0) //按车辆类型授权
    {
        if(mongodb_flag) //查询mongodb
        {
            if(mongodb_auth_type(in_msg->channel_id,plate_final,&auth) < 0)
                return -1;
        }
        else
        {
            if(mysql_auth_type((char*)"入口",in_msg->channel_id,plate_final,&auth) < 0)
                return -1;
        }

        if(auth == true)
        {
            fprintf(fp_log,"%s##按车辆类型授权，该车辆已授权\n",time_now);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] 按车辆类型授权 车牌[%s]已授权 auth[%d] line[%d]", __FUNCTION__, plate_final, auth, __LINE__);
            writelog(log_buf_);
        }
        else
        {
            fprintf(fp_log,"%s##按车辆类型授权，该车辆未授权\n",time_now);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] 按车辆类型授权 车牌[%s]未授权 auth[%d] line[%d]", __FUNCTION__, plate_final, auth, __LINE__);
            writelog(log_buf_);
        }

    }
    else if(strcmp("混合授权",auth_type) == 0) //混合授权
    {
        if(mongodb_flag) //查询mongodb
        {
            if(mongodb_auth_type(in_msg->channel_id,plate_final,&auth) < 0)
                return -1;
            if(mongodb_auth_plate(in_msg->channel_id,plate_final,&auth) < -1)
                return -1;
        }
        else
        {
            if(mysql_auth_type((char*)"入口",in_msg->channel_id,plate_final,&auth) < 0)
                return -1;
            if(mysql_auth_plate((char*)"入口",in_msg->channel_id,plate_final,&auth) < 0)
                return -1;
        }

        if(auth == true)
        {
            fprintf(fp_log,"%s##混合授权，该车辆已授权\n",time_now);
        }
        else
        {
            fprintf(fp_log,"%s##混合授权，该车辆未授权\n",time_now);
        }

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 混合授权 auth[%d] line[%d]", __FUNCTION__, auth, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 未找到通道授权方式 auth_type[%s] line[%d]", __FUNCTION__, auth_type, __LINE__);
        writelog(log_buf_);

        fprintf(fp_log,"%s##未找到通道授权方式\n",time_now);
        return -1;
    }
    return 0;

}
/**************************************************end******************************************/
/*********************************************验证通道授权****************************************/
/**
 * @brief : 验证授权
 *              a. 如果授权过期，直接返回
 *                 如果授权未过期，则继续判断收费规则
 *
 *              b. 拼接语音
 * @param in_msg ：　接收到的消息包
 * @return : -1, 失败
 *            0, 成功
 */
static int validate_auth(car_msg *in_msg)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 入口 验证通道授权 line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);
    bool tmp;

    if(mongodb_flag) //查询mongodb
    {
        if(mongodb_validate_guoqi((char*)"入口",park_id,plate_final,global_car_type,&auth,
                                  &global_remain_day,&global_remain_hour,&global_remain_money,
                                  global_charge_rule) < 0) //验证月租车,储时车，储值车
            return -1;
    }
    else
    {
        if(mysql_validate_guoqi((char*)"入口",park_id,plate_final,global_car_type,&auth,
                                &global_remain_day,&global_remain_hour,&global_remain_money,
                                global_charge_rule) < 0) //验证月租车,储时车，储值车
            return -1;
    }

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 入口 验证通道授权 收费规则 global_charge_rule[%s] line[%d]", __FUNCTION__, global_charge_rule, __LINE__);
    writelog(log_buf_);

    //  下面是验证授权通过后的逻辑处理（即未过期）
    /*********************************************拼语音信息****************************************/
    if(strcmp(global_charge_rule,"免费") == 0 ||
       strcmp(global_charge_rule,"济南2017")==0 ||
       strcmp(global_charge_rule,"白天按小时收费，晚上按次收费") == 0 ||
       strcmp(global_charge_rule,"全天按小时收费") == 0 ) //免费，白天按小时晚上按次，全天按小时
    {
        sprintf(speak_send,"%s",global_car_type); //车牌号 车辆类型
    }
    else if(strcmp(global_charge_rule,"指定时间免费") == 0) //租期车
    {
      
      sprintf(speak_send,"%s%s",global_car_type,remain_car_time); //车牌号 车辆类型  剩余时间
  
        /*
=======
        if(in_guoqi_flag == true) //未过期
        {
           
           sprintf(speak_send,"%s%s",global_car_type,remain_car_time); //车牌号 车辆类型  剩余时间
            
        }
        else //过期
        {
            sprintf(speak_send,"%s已过期",global_car_type); //车牌号 车辆类型  已过期
            in_led_guoqi_flag = true;
        }
>>>>>>> 5e2d066ee81730ea9d8165111ad1a5b85444bf10
*/
    }
    else if(strcmp(global_charge_rule,"储时") == 0) //储时车
    {
        if(in_guoqi_flag == true) //有剩余时间
        {
            sprintf(speak_send,"%s剩余时间%d天%d小时",global_car_type,global_remain_day,global_remain_hour); //车牌号 车辆类型  剩余时间
        }
        else //无剩余时间
        {
            sprintf(speak_send,"%s剩余时间0小时",global_car_type); //车牌号 车辆类型  剩余时间
        }
    }
    else if(strcmp(global_charge_rule,"储值")  == 0)//储值车
    {
        if(in_guoqi_flag == true) //有余额
        {
            sprintf(speak_send,"%s剩余金额%d元",global_car_type,global_remain_money); //车牌号 车辆类型 剩余金额
        }
        else //余额不足
        {
            sprintf(speak_send,"%s剩余金额0元",global_car_type); //车牌号 车辆类型 剩余金额
        }

    }
    else
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到计费规则[%s] line[%d]", __FUNCTION__, global_charge_rule, __LINE__);
        writelog(log_buf_);

        fprintf(fp_log,"%s##车辆类型为%s,未找到对应的收费规则\n",time_now,global_car_type);
        sprintf(speak_send,"%s未授权",global_car_type); //车牌号 车辆类型

        return 2;
    }

    /*********************************************end****************************************/
    tmp =auth;
    if(strcmp(global_car_type,"临时车") == 0)
    {
        fprintf(fp_log,"%s##语音为%s\n",time_now,speak_send);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 临时车 语音内容[%s] line[%d]", __FUNCTION__, speak_send, __LINE__);
        writelog(log_buf_);

        return 0;
    }

    bool chewei_flag;
    mongodb_validate_chewei(&chewei_flag,in_msg->channel_id);
    //chewei_flag = true;

    // 车位数验证开启
    if(chewei_flag)
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 开启车位数验证 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        if(mongodb_flag) //查询mongodb
        {
            if(mongodb_validate_chewei_num(plate_final,&auth) < 0) //验证车位数限制
                return -1;
        }
        else
        {
            if(mysql_validate_chewei_num(plate_final,&auth) < 0) //验证车位数限制
                return -1;
        }

        if(tmp == true && auth == false)  //车位数超限
        {
            sprintf(speak_send,"%s车位数超限",speak_send);
        }
    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 未开启车位数验证 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
    }

    if(auth == true)
        sprintf(speak_send,"%s欢迎光临",speak_send);

    fprintf(fp_log,"%s##语音为%s\n",time_now,speak_send);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 语音内容[%s] return 0! line[%d]", __FUNCTION__, speak_send, __LINE__);
    writelog(log_buf_);

    return 0;

}
/**************************************************end******************************************/
/******************************给bipc发送抬杆信息***********************************************/
static int send_bipc_open_door(car_msg *in_msg)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 准备发送抬杆信息 line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

    char type[24] = {0};

    // 查询车辆类型
    mongodb_query_cartype(in_plate_last,type);

    int retfullfangxing = mongodb_validate_full_fangxing(park_id);
    if(retfullfangxing == 0)
    {
        fprintf(fp_log,"检测到车场设置为车满时禁止进入 type: %s,inplatelast :%s\n",type,in_plate_last);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 检测到车场设置为车满时禁止进入 type[%s] inplatelast[%s] line[%d]", __FUNCTION__, type, in_plate_last, __LINE__);
        writelog(log_buf_);

        return 0;
    }

    fprintf(fp_log,"检测到车场设置为车满时授权车进入 type: %s,inplatelast :%s\n",type,in_plate_last);

    //strcmp(type,"临时车") == 0; // 临时车 0, true, 1, false
    //!strcmp(type, "")                   0, true, 1, false;

    // 授权车放行，且是临时车
    if(retfullfangxing == 1 && (!strcmp(type,"临时车")))
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] !!!车位满 且不是临时车 type[%s] line[%d]", __FUNCTION__, type, __LINE__);
        writelog(log_buf_);

        fprintf(fp_log,"检测到车场设置为车满时授权车进入 type: %s,inplatelast :%s\n",type,in_plate_last);

        return 0;
    }

    flagOpenDoor = true;
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);   //获取当前时间

    Json::Value json_send;
    json_send["cmd"] = Json::Value("open_door");
    json_send["channel_id"] = Json::Value(in_msg->channel_id);
    json_send["in_out"] = Json::Value(in_msg->in_out);

    if(in_fleet == true)
    {
        json_send["flag"] = Json::Value("keep");
    }
    else
    {
        json_send["flag"] = Json::Value("once");
    }

    if(false == g_has_cheduimoshi_){
        json_send["flag"] = Json::Value("once");

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 没有车队模式 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
    }

    std::string send = json_send.toStyledString();

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BIPC);//往相机发送开杆信息PORT_UDP_BCENTER_TO_BIPC  #往led发开杆信息PORT_UDP_BCENTER_TO_BLED
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##send_bipc_open_door_in 发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 向bled发送消息失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BIPC, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##send_bipc_open_door_in 发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BIPC, __LINE__);
        writelog(log_buf_);
    }
    close(sock);
}
/**************************************************end******************************************/
/******************************处理上级停车场***********************************************/
static int process_top_park(car_msg *in_msg)
{
    if(auth == false)
        return 0;

    if(mongodb_flag) //查询mongodb
    {
        if(mongodb_process_top_park(in_msg->channel_id,plate_final,global_charge_rule,global_car_type,in_msg,open_door) < 0)
            return -1;
    }
    else
    {
        if(mysql_process_top_park(in_msg->channel_id,plate_final,global_charge_rule,global_car_type,in_msg,open_door) < 0)
            return -1;
    }

    return 0;
}
/**************************************************end******************************************/
/******************************写数据到入口表和在场表***********************************************/
static int write_in_park(car_msg *in_msg)
{
    bool ret = false;

    if(mongodb_flag) //查询mongodb
    {
        if(mongodb_write_in_park(in_msg,plate_ori,plate_final,park_id,global_car_type,&auth,open_door) < 0){

            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 写失败，  return -1! line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);

            return -1;
        }

        ret = true;
    }
    else
    {
        if(mysql_write_in_park(in_msg,plate_ori,plate_final,park_id,global_car_type,&auth,open_door) < 0) {
            return -1;
        }

        ret = true;
    }

    if(!ret){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 写入数据库失败 plate_ori[%s] plate_final[%s] open_door[%s] line[%d]", __FUNCTION__, plate_ori, plate_final, open_door, __LINE__);
        writelog(log_buf_);
    }

    return 0;

}
/**************************************************end******************************************/
/******************************给bled发送led显示信息和语音信息***********************************************/
static int send_bled(car_msg *in_msg)
{
    char space_count[24] = {0};

    mongodb_query_remain_space_count(space_count);
    fprintf(fp_log,"------车位满---------%s\n",space_count);

    if(atoi(space_count) <= 0)
        sprintf(speak_send,"%s车位满",speak_send);

    if(mongodb_flag) //查询mongodb
    {
        if(mongodb_send_bled(in_msg,(char*)"入口",global_car_type,plate_final,in_msg->channel_id,speak_send,0,0) < 0)
            return -1;
    }
    else
    {

        if(mysql_send_bled((char*)"入口",global_car_type,plate_final,in_msg->channel_id,speak_send) < 0)
            return -1;
    }
}
/**************************************************end******************************************/
/******************************给bgui发送显示信息***********************************************/
static int send_bgui(car_msg *in_msg)
{
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);   //获取当前时间
    char space_count[24];

    memset(space_count,0,24);
    if(mongodb_flag) //查询mongodb
    {
        mongodb_query_space_count(space_count);
    }
    else
    {
        mysql_query_space_count((char*)"入口",space_count);
    }


    Json::Value json_send;
    json_send["cmd"] = Json::Value("carin");

    Json::Value json_content;
    json_content["plate"] = Json::Value(plate_final);
    json_content["type"] = Json::Value(global_car_type);

    //******* begin 过期特殊处理 add 20171221 14:32
    if( (false == in_guoqi_flag) && ((strcmp(global_car_type, "A类车") == 0) || (strcmp(global_car_type, "B类车") == 0) || (strcmp(global_car_type, "C类车") == 0))){
        char car_type_tmp[24] = {0};

        sprintf(car_type_tmp,"%s过期", global_car_type); // 月租车
        json_content["type"] = Json::Value(car_type_tmp);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 入口车辆过期，添加已过期提示 [%s] line[%d]", __FUNCTION__, car_type_tmp,  __LINE__);
        writelog(log_buf_);
    }
    //******* end 过期特殊处理 add 20171221 14:32

    json_content["time"] = Json::Value(in_msg->time);
    json_content["parkcount"] = Json::Value(space_count);

    if(zhuji == true)
    {
        json_content["pcolor"] = Json::Value(in_msg->pcolor);
        json_content["picpath"] = Json::Value(in_msg->path);
    }
    else
    {
        json_content["pcolor"] = Json::Value(in_msg->pcolor1);
        json_content["picpath"] = Json::Value(in_msg->path1);
    }
    json_content["channel_id"] = Json::Value(in_msg->channel_id);

    json_send["content"] = json_content;
    std::string send = json_send.toStyledString();

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BGUI);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##send_bgui_in 发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 向bgui发送消息失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI,  __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##send_bgui_in 发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bgui发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }
    close(sock);

    /*
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
        addr.sin_port = htons(5094);
        addr.sin_addr.s_addr = inet_addr(host_server_ip);
    n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##send_client_in 发送失败\n",time_now);
    }
    else
    {
        fprintf(fp_log,"%s##send_client_in 发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());
    }
    close(sock);
    */
    return 0;

}
/**************************************************end******************************************/
/*********************************************入口处理线程******************************/
void* in_thread(void *)
{
    car_msg *in_msg;
    char time_now[64];
    time_t tm;
    flagOpenDoor =false;

    time_printf(time(&tm),time_now);   //获取当前时间
    printf("in_thread 启动成功 \n");
    fprintf(fp_log,"%s##bin_thread 启动成功\n",time_now);
    fflush(fp_log);
    sleep(1);
    memset(in_plate,0,PLATE_LENGTH);
    memset(in_channel,0,256);
    memset(in_time,0,24);
    memset(in_plate_last,0,PLATE_LENGTH);

    in_plate_time=get_tick(time_now);
    while(1)
    {
        usleep(200);

        in_msg = BoonMsgQueue::Instance()->get_in(); //取入口队列里的数据
        if(in_msg == NULL)
        {
            continue;
        }
        pthread_mutex_lock(&mongo_mutex_car);
        if(mongodb_flag)
            mongodb_connect();
        memset(speak_send,0,256);
        time_printf(time(&tm),time_now);   //获取当前时间
        printf("================ 1 ====================================\n");

        writelog("\n**************** 入口 *********************");

        in_led_guoqi_flag = false;
        in_led_noauth_flag = false;
        if(mongodb_flag) //查询mongodb
        {
            if(mongodb_query_channel(in_msg->channel_id,auth_type,open_door,one_way,mohu_match,park_id) < 0) //根据channel_id查询通道属性
            {
		        mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                continue;
            }
        }
        else //查询mysql
        {
            mysql_query_channel((char*)"入口",in_msg->channel_id,auth_type,open_door,one_way,mohu_match,park_id);//根据channel_id查询通道属性
        }
        printf("================ 2 ====================================\n");

        fprintf(fp_log,"%s##入口得到通道属性  通道id:%s 授权类型:%s 开闸类型:%s 同口进出:%s 模糊匹配:%s\n",time_now,in_msg->channel_id,auth_type,open_door,one_way,mohu_match);

        if(process_fuji(in_msg) < 0 ) //处理辅机
        {
            fflush(fp_log);
	        mongodb_exit();
            pthread_mutex_unlock(&mongo_mutex_car);
            continue;
        }
        fprintf(fp_log,"%s##入口辅机处理 %d个相机 收到的车牌号为 1:%s 2:%s 处理后车牌号为%s \n",time_now,in_msg->num,in_msg->plate,in_msg->plate1,plate_ori);
        printf("3================ ====================================\n");
        if(strlen(plate_ori) < 4)
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 入口车牌号为空, 不处理, 继续等待下一次消息, continue. line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);

            fprintf(fp_log,"%s##车牌号为空\n",time_now);
	        mongodb_exit();
            pthread_mutex_unlock(&mongo_mutex_car);
            continue;
        }

        memset(in_out_flag, 0, sizeof(in_out_flag));
        memcpy(in_out_flag, in_msg->in_out, strlen(in_msg->in_out));

        printf("4================ ====================================\n");
        if(strcmp(plate_last,plate_ori) == 0 && (get_tick(time_now)	-  plate_last_time < g_car_in_last_time_) )
        {
            fprintf(fp_log,"%s## 连续多辆车出现\n",time_now);
            fflush(fp_log);
	        mongodb_exit();
            pthread_mutex_unlock(&mongo_mutex_car);
            continue;
        }
        else
        {
            memcpy(plate_last,plate_ori,strlen(plate_ori));
            plate_last_time = get_tick(time_now);
        }
        printf("5================ ====================================\n");

        if(strcmp(one_way,"是") == 0 && strcmp(out_plate_last,plate_ori) == 0)
        {
            if(get_tick(time_now)	-  out_plate_time < 60)
            {
                fprintf(fp_log,"%s##同口进出 连续拍照\n",time_now);
                fflush(fp_log);
	            mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                continue;
            }
        }
        printf("6================ ====================================\n");
        memset(in_plate_last,0,PLATE_LENGTH);
        memcpy(in_plate_last,plate_ori,PLATE_LENGTH);
        in_plate_time = get_tick(time_now);

        int ret_black = mongodb_validate_blacklist(plate_ori,in_msg->blacklistreason);
        switch (ret_black)
        {
            case 0:
                printf("blacklist================ 0 \n");
                strcpy(in_msg->blacklist,"");
                break;
            case 1:
                printf("blacklist================ 1 \n");
                strcpy(in_msg->blacklist,"提醒");
                send_bled(in_msg);
                break;
            case 2:
                printf("blacklist================ 2 \n");
                strcpy(in_msg->blacklist,"警告");
                send_bled(in_msg);
                break;
            case 3:
                printf("blacklist================ 3 \n");
                strcpy(in_msg->blacklist,"禁入");
                strcpy(in_msg->plate,in_plate_last);
                strcpy(plate_final,in_plate_last);
                send_bled(in_msg);
                BoonMsgQueue::Instance()->release_in(in_msg); //释放队列

                mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                continue;

            default:
                {
                    snprintf(log_buf_, sizeof(log_buf_), "[%s] 没有找到 plate_ori[%s] 的黑名单记录 line[%d]", __FUNCTION__, plate_ori, __LINE__);
                    writelog(log_buf_);
                }
                break;
        }

        if(strcmp(plate_ori,"无车牌") != 0) //有车牌
        {
            if(process_mohu_match(in_msg) < 0) //进行模糊匹配
            {
                fflush(fp_log);
		        mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                continue;
            }
            printf("7================ ====================================\n");
            if(strcmp(plate_ori,plate_final) == 0) //原始车牌号与最终车牌号一致
            {
                fprintf(fp_log,"%s##入口车牌号为%s\n",time_now,plate_final);
            }
            else //原始车牌号与最终车牌号不一致
            {
                fprintf(fp_log,"%s##经过模糊匹配,入口车牌号由%s变为%s\n",time_now,plate_ori,plate_final);
            }

            delete_car_inpark(plate_final,park_id); //删除该车辆的在场记录

            if(strcmp(open_door,"有车放行") == 0 || strcmp(open_door,"有车牌号放行") == 0)
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] 放行方式 open_door[%s] line[%d]", __FUNCTION__, open_door, __LINE__);
                writelog(log_buf_);

                send_bipc_open_door(in_msg); //发送开杆信息

                auth = true;

                fprintf(fp_log,"%s##开闸类型为%s,发送入口开闸信息\n",time_now,open_door);

                memset(open_door,0,24);
                strcpy(open_door,"自动抬杆");
            }

            if(process_auth(in_msg) < 0) //处理授权方式
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! process_auth失败 continue line[%d]", __FUNCTION__, __LINE__);
                writelog(log_buf_);

                fflush(fp_log);
	            mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                continue;
            }
            printf("8================ ====================================\n");

            int tmp = validate_auth(in_msg);
            if(tmp  < 0) //验证授权方式
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 验证授权失败, 不再处理 ret[%d] line[%d]", __FUNCTION__, tmp, __LINE__);
                writelog(log_buf_);

                fflush(fp_log);
                mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                printf("9================ ====================================\n");
                continue;
            }
            else if(tmp == 2) //未找到收费规则
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到收费规则 ret[%d] goto wujifeiguize! line[%d]", __FUNCTION__, tmp, __LINE__);
                writelog(log_buf_);

                goto wujifeiguize;
            }

            fprintf(fp_log,"%s##删除车牌号为%s车场号为%s的车辆在场记录\n",time_now,plate_final,park_id);

            if(strcmp(open_door,"授权车放行") == 0)
            {
                if(auth == true)
                {
                    snprintf(log_buf_, sizeof(log_buf_), "[%s] 抬杆类型 open_door[%s] line[%d]", __FUNCTION__, open_door, __LINE__);
                    writelog(log_buf_);

                    send_bipc_open_door(in_msg); //发送开杆信息
                    fprintf(fp_log,"%s##开闸类型为%s,%s已授权,发送入口开闸信息\n",time_now,open_door,plate_final);
                    memset(open_door,0,24);
                    strcpy(open_door,"自动抬杆");
                }
                else
                {
                    snprintf(log_buf_, sizeof(log_buf_), "[%s] 未授权 不抬杆 open_door[%s] line[%d]", __FUNCTION__, open_door, __LINE__);
                    writelog(log_buf_);

                    fprintf(fp_log,"%s##开闸类型为%s,%s未授权,不发送入口开闸信息\n",time_now,open_door,plate_final);
                    memset(open_door,0,24);
                    strcpy(open_door,"人工处理");
                }
            }

            if(auth == false)
            {
                // sprintf(speak_send,"%s未授权",speak_send);
                in_led_noauth_flag = true;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 未授权, 可能是临时车. auth[%d] line[%d]", __FUNCTION__, auth, __LINE__);
                writelog(log_buf_);
            }

            if(process_top_park(in_msg) < 0) //处理上级停车场
            {
                fprintf(fp_log,"%s##处理上级停车场失败\n",time_now);
                fflush(fp_log);
		        mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                printf("10================ ====================================\n");
                continue;
            }

wujifeiguize:
            memset(in_plate,0,PLATE_LENGTH);
            memset(in_channel,0,256);
            memset(in_time,0,24);
            memcpy(in_plate,plate_final,strlen(plate_final));
            memcpy(in_channel,in_msg->channel_id,strlen(in_msg->channel_id));
            memcpy(in_time,in_msg->time,strlen(in_msg->time));
            if(write_in_park(in_msg) < 0) //写入口数据和在场数据到数据库
            {
                fflush(fp_log);
		        mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                printf("11================ ====================================\n");
                continue;
            }

            if(send_bled(in_msg) < 0) //发送led显示信息和语音信息
            {
                fflush(fp_log);
		        mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                printf("12================ ====================================\n");
                continue;
            }

            if(send_bgui(in_msg) < 0) //发送信息给bgui
            {
                fflush(fp_log);
		        mongodb_exit();
                printf("13================ ====================================\n");
                pthread_mutex_unlock(&mongo_mutex_car);
                continue;
            }
            fflush(fp_log);

        }
        else //无车牌
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] 无牌车 plate_ori[%s] line[%d]", __FUNCTION__, plate_ori, __LINE__);
            writelog(log_buf_);

            memset(plate_final,0,PLATE_LENGTH);
            strcpy(plate_final,"无车牌");
            memset(global_car_type,0,24);
            strcpy(global_car_type,"临时车");
            memset(speak_send,0,256);
            strcpy(speak_send,"临时车");

            if(strcmp(open_door,"有车放行") == 0)
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] send_bipc_open_door() 无牌车 plate_ori[%s] line[%d]", __FUNCTION__, plate_ori, __LINE__);
                writelog(log_buf_);

                memset(open_door,0,24);
                strcpy(open_door,"自动抬杆");
                send_bipc_open_door(in_msg); //发送开杆信息
            }
            else
            {
                memset(open_door,0,24);
                strcpy(open_door,"人工处理");
            }

            memset(in_plate,0,PLATE_LENGTH);
            memset(in_channel,0,256);
            memset(in_time,0,24);
            memcpy(in_plate,plate_final,strlen(plate_final));
            memcpy(in_channel,in_msg->channel_id,strlen(in_msg->channel_id));
            memcpy(in_time,in_msg->time,strlen(in_msg->time));

            char type[24] = {0};
            mongodb_query_cartype(in_plate_last,type);
            int retfullfangxing = mongodb_validate_full_fangxing(park_id);
            if(retfullfangxing == 2 && !flagOpenDoor)
            {
                fprintf(fp_log,"检测到车场设置为车满时所有车放行\n");
                send_bipc_open_door(in_msg); //发送开杆信息
            }
            /*if(write_in_park(in_msg) < 0) //写入口数据和在场数据到数据库
            {
                fflush(fp_log);
                pthread_mutex_unlock(&mongo_mutex_car);
                continue;
            }*/

            printf("14================ ====================================\n");
            if(send_bled(in_msg) < 0) //发送led显示信息和语音信息
            {
                fflush(fp_log);
		mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                continue;
            }

            if(send_bgui(in_msg) < 0) //发送信息给bgui
            {
                fflush(fp_log);
		mongodb_exit();
                pthread_mutex_unlock(&mongo_mutex_car);
                continue;
            }
        }
        printf("15================ ====================================\n");

        BoonMsgQueue::Instance()->release_in(in_msg); //释放队列
        if(mongodb_flag)
            mongodb_exit();

        pthread_mutex_unlock(&mongo_mutex_car);
    }
}
/**************************************************end******************************************/

