/***************************boon_in.cpp***************************************
			  功能：处理出口
			  创建时间：2017-02-13
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
			  	1. 时间：20171201 17:41 , add by diaoguangqiang
                   修改： 针对交运停车场洗车功能，出口直接抬杆处理, 相应判断vip车等其他车，直接放行

***************************************************************************/
#include "boon_log.h"
#include "common_def.h"
#include "../bcenter_def.h"

#include "boon_out.h"

static char plate_ori[PLATE_LENGTH]; // 原始车牌
static char plate_last[PLATE_LENGTH]; // 上一次车牌
static char plate_final[PLATE_LENGTH]; //最终车牌
static char auth_type[24]; //授权类型
static char open_door[24]; //开闸类型
static char one_way[24]; //同口进出
static char mohu_match[24]; //模糊匹配
static char park_id[256]; //车场id号
static char park_parent_id[256]; //车场id号
static bool auth; //是否授权
static char global_car_type[24]; //车辆类型
static int global_remain_day; //剩余天数
static int global_remain_hour; //剩余小时
static int global_remain_money; //剩余钱数 /元
static char global_charge_rule[256]; //收费方案
static char speak_send[256]; //语音信息
static bool zhuji; //使用主机数据标识
int fmoney = 0; //收费金额
static char in_time[24]; //入场时间
static char in_pic_path[128]; //入场图片
static char in_channel_id[256];//入场通道
static char in_channel_name[256];//入场名称
static char charge_type[24]; //收费类型
static char stay_time[24]; //收费类型
static char people_name[24];
extern bool out_fleet;
char out_plate_last[PLATE_LENGTH];
extern char in_plate_last[PLATE_LENGTH];
long   out_plate_time;
static long plate_last_time = 0;
extern long in_plate_time;
bool out_guoqi_flag;//出口车辆过期标志
bool out_led_guoqi_flag = false; 
//bool out_led_noauth_flag = false;
char out_plate[PLATE_LENGTH];
char out_channel[256];
char out_time[24];
char out_led_ip[24];
char out_pic_path[256];
char out_pcolor[24];
char in_out_flag[24] = {0}; // 入口或出口标记
extern pthread_mutex_t mongo_mutex_channel;
extern pthread_mutex_t mongo_mutex_carinpark;
extern pthread_mutex_t mongo_mutex_car;
extern pthread_mutex_t mongo_mutex_device;
extern char remain_car_time[128];

// 日志
static char log_buf_[LOG_BUF_SIZE] = {0};
// 记录日志
extern void writelog(const char* _buf);
// 是否加入车队模式，不在此赋值， 在main.cpp中定义
extern bool g_has_cheduimoshi_;
// 车出场时候持续时间，单位秒, 不在此赋值， 在main.cpp中定义
extern int g_car_out_last_time_;

/**************************************************处理辅机******************************/
static int process_fuji(car_msg *out_msg)  
{
    memset(plate_ori,0,PLATE_LENGTH);
	if(out_msg->num == 1)  //只有一个相机，则返回
	{
		zhuji = true;
		memcpy(plate_ori,out_msg->plate,strlen(out_msg->plate));
		return 0;
	}

	if(strcmp(out_msg->plate,out_msg->plate1) == 0) //两者车牌一致，取主机车牌号
	{
		zhuji = true;
		memcpy(plate_ori,out_msg->plate,strlen(out_msg->plate));
		return 0;	
	}

	if(strcmp("无车牌",out_msg->plate) == 0) //主机车牌号为无车牌，则取辅机车牌号
	{
		zhuji = false;
		memcpy(plate_ori,out_msg->plate1,strlen(out_msg->plate1));
		return 0;
	}

	if(strcmp("无车牌",out_msg->plate1) == 0)//辅机车牌号为无车牌，则取主机车牌号
	{
		zhuji = true;
		memcpy(plate_ori,out_msg->plate,strlen(out_msg->plate));
		return 0;
	}

	if(mongodb_flag) //查询mongodb
	{
		if(mongodb_query_car(out_msg->plate) > 0) // 主机车牌号在car表查到，则取主机车牌号
		{
			zhuji = true;
			memcpy(plate_ori,out_msg->plate,strlen(out_msg->plate));
			return 0;
		}
		if(mongodb_query_car(out_msg->plate) < 0)
		{
			return -1;
		}
		if(mongodb_query_car(out_msg->plate1) > 0) //辅机车牌号在car表查到，则取辅机车牌号
		{
			zhuji = false;
			memcpy(plate_ori,out_msg->plate1,strlen(out_msg->plate1));
			return 0;
		}
		if(mongodb_query_car(out_msg->plate1) < 0)
		{
			return -1;
		}
		memcpy(plate_ori,out_msg->plate,strlen(out_msg->plate)); //两个车牌在car表都未查到，则取主机车牌号
	}
	else //查询mysql
	{
		if(mysql_query_car((char*)"出口",out_msg->plate) > 0) // 主机车牌号在car表查到，则取主机车牌号
		{
			zhuji = true;
			memcpy(plate_ori,out_msg->plate,strlen(out_msg->plate));
			return 0;
		}
		if(mysql_query_car((char*)"出口",out_msg->plate1) > 0) //辅机车牌号在car表查到，则取辅机车牌号
		{
			zhuji = false;
			memcpy(plate_ori,out_msg->plate1,strlen(out_msg->plate1));
			return 0;
		}
		zhuji = true;
		memcpy(plate_ori,out_msg->plate,strlen(out_msg->plate)); //两个车牌在car表都未查到，则取主机车牌号
	}
	
		
}
/**************************************************end******************************************/
/**************************************************模糊匹配******************************/
static int process_mohu_match(car_msg *out_msg)  
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 出口 开始模糊匹配 line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);

	if(zhuji != true)
	{
		memset(out_msg->pcolor,0,strlen(out_msg->pcolor));
		memcpy(out_msg->pcolor,out_msg->pcolor1,strlen(out_msg->pcolor1));
	}

    memset(plate_final,0,PLATE_LENGTH);
	if(mongodb_flag)  //查询mongodb
	{
		if(strcmp("是",mohu_match) == 0) //进行模糊匹配
		{
			if(mongodb_query_car(plate_ori)) //在car表查到该车牌号，就不进行模糊匹配
			{
				fprintf(fp_log,"%s##出口模糊匹配，在car表查到该车辆，模糊匹配退出\n",time_now);	
                memcpy(plate_final,plate_ori,PLATE_LENGTH);
				return 0;	
			}
			if(mongodb_mohu_match_out(out_msg->pcolor,plate_ori,plate_final) < 0) //mongodb的模糊匹配	
			return -1;		
		}	
		else //不进行模糊匹配
		{
			memcpy(plate_final,plate_ori,strlen(plate_ori));
			fprintf(fp_log,"%s##该通道不进行模糊匹配\n",time_now);	
			return 0;
		}	
	}
	else  //查询mysql
	{
		if(strcmp("是",mohu_match) == 0)  //进行模糊匹配
		{
			if(mysql_query_car((char*)"出口",plate_ori)) //在car表查到该车牌号，就不进行模糊匹配
			{
				fprintf(fp_log,"%s##模糊匹配，在car表查到该车辆，模糊匹配退出\n",time_now);	
                memcpy(plate_final,plate_ori,PLATE_LENGTH);
				return 0;	
			}
			mysql_mohu_match((char*)"出口",out_msg->pcolor,plate_ori,plate_final); //mysql的模糊匹配
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
static int delete_car_inpark(char * plate,char *id)
{
	if(mongodb_flag)
	{
		mongodb_delete_car_inpark(plate,id);	
	}
	else
	{
		mysql_delete_car_inpark((char*)"出口",plate,id);
	}	
}
/**************************************************end******************************************/
/*********************************************处理通道授权****************************************/
static int process_auth(car_msg *out_msg)
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
			if(mongodb_auth_plate(out_msg->channel_id,plate_final,&auth) < 0)
				return -1;		
		}
		else
		{
			if(mysql_auth_plate((char*)"出口",out_msg->channel_id,plate_final,&auth) < 0)
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
	}
	else if(strcmp("按车辆类型授权",auth_type) == 0) //按车辆类型授权
	{
		if(mongodb_flag) //查询mongodb
		{
			if(mongodb_auth_type(out_msg->channel_id,plate_final,&auth) < 0)
				return -1;		
		}
		else
		{
			if(mysql_auth_type((char*)"出口",out_msg->channel_id,plate_final,&auth) < 0)
				return -1;
		}

		if(auth == true)
		{
			fprintf(fp_log,"%s##按车辆类型授权，该车辆已授权\n",time_now);	
		}
		else
		{
			fprintf(fp_log,"%s##按车辆类型授权，该车辆未授权\n",time_now);
		}

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 授权方式[%s] 该车auth[%d] line[%d]", __FUNCTION__, auth_type, auth, __LINE__);
        writelog(log_buf_);
	}
	else if(strcmp("混合授权",auth_type) == 0) //混合授权
	{
		if(mongodb_flag) //查询mongodb
		{
			if(mongodb_auth_type(out_msg->channel_id,plate_final,&auth) < 0)
				return -1;	
			if(mongodb_auth_plate(out_msg->channel_id,plate_final,&auth) < -1)
				return -1;		
		}
		else
		{
			if(mysql_auth_type((char*)"出口",out_msg->channel_id,plate_final,&auth) < 0)
				return -1;
			if(mysql_auth_plate((char*)"出口",out_msg->channel_id,plate_final,&auth) < 0)
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
	}
	else
	{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 未找到通道授权方式 auth_type[%s] line[%d]", __FUNCTION__, auth_type, __LINE__);
        writelog(log_buf_);

		fprintf(fp_log,"%s##未找到通道授权方式\n",time_now);
		return -1;
	}
	return 0;
	
}
/**************************************************end******************************************/
/*********************************************验证通道授权****************************************/
static int validate_auth(car_msg *in_msg)
{
	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);
	bool tmp;

	if(mongodb_flag) //查询mongodb
	{
		if(mongodb_validate_guoqi((char*)"出口",park_id,plate_final,global_car_type,&auth,&global_remain_day,&global_remain_hour,&global_remain_money,global_charge_rule) < 0) //验证月租车,储时车，储值车
			return -1;
	}
	else
	{
		if(mysql_validate_guoqi((char*)"出口",park_id,plate_final,global_car_type,&auth,
								&global_remain_day,&global_remain_hour,&global_remain_money,
								global_charge_rule) < 0) //验证月租车,储时车，储值车
			return -1;
	}

	snprintf(log_buf_, sizeof(log_buf_), "[%s] 出口 验证通道权限 收费规则 global_charge_rule[%s] line[%d]", __FUNCTION__, global_charge_rule, __LINE__);
    writelog(log_buf_);

	/*********************************************拼语音信息****************************************/
       if(strcmp(global_charge_rule,"免费") == 0 ||
            strcmp(global_charge_rule,"白天按小时收费，晚上按次收费") == 0||
            strcmp(global_charge_rule,"全天按小时收费") == 0 ||
            strcmp(global_charge_rule,"济南2017") == 0 ) //免费，白天按小时晚上按次，全天按小时
	{
		sprintf(speak_send,"%s",global_car_type); //车牌号 车辆类型
	}
	else if(strcmp(global_charge_rule,"指定时间免费") == 0) //租期车
	{
        sprintf(speak_send,"%s%s",global_car_type,remain_car_time); //车牌号 车辆类型  剩余时间

        /*
        if(out_guoqi_flag == true) //未过期
        {
            sprintf(speak_send,"%s%s",global_car_type,remain_car_time); //车牌号 车辆类型  剩余时间
        }
        else //过期
        {
            sprintf(speak_send,"%s已过期",global_car_type); //车牌号 车辆类型  已过期
            out_led_guoqi_flag = true;
        }*/

	}
	else if(strcmp(global_charge_rule,"储时，剩余时间不足，按小时收费") == 0) //储时车
	{
		if(out_guoqi_flag == true) //有剩余时间
		{
            if(global_remain_day <=10)
			    sprintf(speak_send,"%s剩余时间%d天%d小时",global_car_type,global_remain_day,global_remain_hour); //车牌号 车辆类型  剩余时间
			else
			    sprintf(speak_send,"%s",global_car_type); //车牌号 车辆类型  剩余时间
		}
		else //无剩余时间
		{
			sprintf(speak_send,"%s剩余时间0小时",global_car_type); //车牌号 车辆类型  剩余时间
		}
	}
	else if(strcmp(global_charge_rule,"储值，剩余金额不足，按小时收费")  == 0)//储值车
	{
		if(out_guoqi_flag == true) //有余额
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
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 车辆类型global_car_type[%s] 未找到对应的收费规则 line[%d]", __FUNCTION__, global_car_type, __LINE__);
        writelog(log_buf_);

		fprintf(fp_log,"%s##车辆类型为%s,未找到对应的收费规则\n",time_now, global_car_type);
		return -1;
	}

	/*********************************************end****************************************/
	sprintf(speak_send,"%s一路顺风",speak_send);
	fprintf(fp_log,"%s##语音为%s\n",time_now,speak_send);
	return 0;
	
}
/**************************************************end******************************************/
/******************************给bipc发送抬杆信息***********************************************/
static int send_bipc_open_door(car_msg *in_msg)
{
	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);   //获取当前时间

	Json::Value json_send; 
	json_send["cmd"] = Json::Value("open_door");
	json_send["channel_id"] = Json::Value(in_msg->channel_id);
	json_send["in_out"] = Json::Value(in_msg->in_out); //"入口"　“出口”
	if(out_fleet == true)
	{
		json_send["flag"] = Json::Value("keep");
	}
	else
	{
		json_send["flag"] = Json::Value("once");
	}

	// 是否有车队模式
	if(false == g_has_cheduimoshi_){
		json_send["flag"] = Json::Value("once");
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
		fprintf(fp_log,"%s##send_bipc_open_door 发送失败\n",time_now);

		snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 向bled发送消息失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BIPC, __LINE__);
		writelog(log_buf_);
	}
	else
	{
		fprintf(fp_log,"%s##send_bipc_open_door 发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());

		snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BIPC, __LINE__);
		writelog(log_buf_);
	}
	close(sock);

	return 0;
}
/**************************************************end******************************************/
/******************************处理上级停车场***********************************************/
static int process_top_park(car_msg *out_msg)
{
	if(mongodb_flag) //查询mongodb
	{
		return mongodb_process_top_park_out(out_msg->channel_id,plate_final,global_charge_rule,global_car_type,park_parent_id);
				
	}
	else
	{
        return 	mysql_process_top_park_out(out_msg->channel_id,plate_final,global_charge_rule,global_car_type,park_parent_id);
	}
}
/**************************************************end******************************************/

/******************************给bled发送led显示信息和语音信息***********************************************/
static int send_bled(car_msg *msg)
{
	if(mongodb_flag) //查询mongodb
	{
        if(mongodb_send_bled(msg,(char*)"出口",global_car_type,plate_final,msg->channel_id,speak_send,fmoney,0) < 0)
			return -1;
	}
	else
	{
		if(mysql_send_bled((char*)"出口",global_car_type,plate_final,msg->channel_id,speak_send) < 0)
			return -1;
	}	
}
/**************************************************end******************************************/
/******************************给bgui发送显示信息***********************************************/
static int send_bgui(car_msg *msg)
{
	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);   //获取当前时间

	char space_count[24];
	char duration[24];
	char money[24];

	memset(money,0,24);
	memset(space_count,0,24);
	if(mongodb_flag) //查询mongodb
	{
		mongodb_query_space_count(space_count);
	}
	else
	{
		mysql_query_space_count((char*)"出口",space_count);
	}
	
	Json::Value json_send; 
	json_send["cmd"] = Json::Value("carout");

	Json::Value json_content;
	json_content["plate"] = Json::Value(plate_final);
	json_content["type"] = Json::Value(global_car_type);
	
	json_content["intime"] = Json::Value(in_time);
	json_content["outtime"] = Json::Value(msg->time);
	json_content["inpicpath"] = Json::Value(in_pic_path);
	sprintf(money,"%d",fmoney);
	json_content["charge"] = Json::Value(money);
	json_content["parkcount"] = Json::Value(space_count);
	json_content["inchannel"] = Json::Value(in_channel_name);
	
	int du = (get_tick(msg->time) - get_tick(in_time))/60;
	memset(duration,0,24);
	if(strlen(in_time) < 5)
	{
		sprintf(duration,"%d",0);
	}
	else
	{
		sprintf(duration,"%d",du);
	} 
	json_content["duration"] = Json::Value(duration);
	if(zhuji == true)
	{
		json_content["pcolor"] = Json::Value(msg->pcolor);
		json_content["outpicpath"] = Json::Value(msg->path);
	}
	else
	{
		json_content["pcolor"] = Json::Value(msg->pcolor1);
		json_content["outpicpath"] = Json::Value(msg->path1);
	}
    json_content["channel_id"] = Json::Value(msg->channel_id);
	json_send["content"] = json_content;
	std::string send = json_send.toStyledString();

	char ip[] = {"127.0.0.1"};

	struct sockaddr_in addr;
    int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
	{
		fprintf(fp_log,"%s##socket error\n",time_now);
	}
	addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BGUI);
    addr.sin_addr.s_addr = inet_addr(ip);
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
        fprintf(fp_log,"%s##send_bgui_out 发送失败\n",time_now);

		snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 向bgui发送消息失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
		writelog(log_buf_);
	}
	else
	{
        fprintf(fp_log,"%s##send_bgui_out 发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());

		snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bgui发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
		writelog(log_buf_);
	}
	close(sock);

	return 0;

}
/**************************************************end******************************************/
static int query_carinpark(char *channel_id,char *plate,char *in_time,char *in_pic_path,char *in_channel_id,char *people_name,char *in_channel_name)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

    int ret = 0;

	if(mongodb_flag) //查询mongodb
	{
		ret = mongodb_query_carinpark(channel_id,plate,in_time,in_pic_path,in_channel_id,people_name,in_channel_name);
	}
	else
	{
		ret = mysql_query_carinpark((char*)"出口",channel_id,plate,in_time,in_pic_path,in_channel_id,people_name,in_channel_name);
	}

    snprintf(log_buf_, sizeof(log_buf_), "[%s] ret[%d] line[%d]", __FUNCTION__, ret, __LINE__);
    writelog(log_buf_);

    return ret;
}

static int process_fee(char *channel_id,char *plate,char *in_time,char *out_time)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

    int ret = 0;

	if(mongodb_flag) //查询mongodb
	{
	    ret = mongodb_process_fee(channel_id,plate,in_time,out_time);
	}
	else
	{
	   ret = mysql_process_fee(channel_id,plate);
	}

    snprintf(log_buf_, sizeof(log_buf_), "[%s] ret[%d] line[%d]", __FUNCTION__, ret, __LINE__);
    writelog(log_buf_);

    return ret;
}

/*********************************************出口处理线程******************************/
void* out_thread(void *)
{
	car_msg *out_msg;
	char time_now[64];
	time_t tm;
	
	time_printf(time(&tm),time_now);   //获取当前时间
	printf("out_thread 启动成功 \n");
	fprintf(fp_log,"%s##bout_thread 启动成功\n",time_now);
	fflush(fp_log);
	sleep(1);
    memset(out_plate,0,PLATE_LENGTH);
	memset(out_channel,0,256);
	memset(out_time,0,24);
	memset(in_channel_name,0,256);
    memset(out_plate_last,0,PLATE_LENGTH);

	out_plate_time=get_tick(time_now);
	struct timeval DayTime;

	gettimeofday(&DayTime, NULL);
	int s = DayTime.tv_sec * 1000 + DayTime.tv_usec / 1000;
	int e = DayTime.tv_sec * 1000 + DayTime.tv_usec / 1000;
	while(1)
	{
		usleep(200);
		out_msg = BoonMsgQueue::Instance()->get_out(); //取出口队列里的数据
		
		if(out_msg == NULL)
		{
			continue;
		}

		pthread_mutex_lock(&mongo_mutex_car);
		gettimeofday(&DayTime, NULL);
		s = DayTime.tv_sec * 1000 + DayTime.tv_usec / 1000;
		if(mongodb_flag)
		mongodb_connect();
		memset(speak_send,0,256);
		time_printf(time(&tm),time_now);   //获取当前时间

		writelog("\n**************** 出口 *********************");

        //*****begin 时间戳格式： 2018-01-06 18:26:44, 针对bvs发包时发送空的时间戳，而修复的bug
        if(strlen(out_msg->time) <= 0){

            snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] warn! 离开时间戳为out_msg->time[%s] line[%d]", __FUNCTION__, out_msg->time, __LINE__);
            writelog(log_buf_);

            std::string out_time = carOutTime();
            memset(out_msg->time, 0, sizeof(out_msg->time));
            snprintf(out_msg->time, sizeof(out_msg->time), "%s", out_time.c_str());

            snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] warn! 离开时间戳为空, 修复为out_msg->time[%s] line[%d]", __FUNCTION__, out_msg->time, __LINE__);
            writelog(log_buf_);
        }
        //*****end 时间戳格式： 2018-01-06 18:26:44, 针对bvs发包时发送空的时间戳，而修复的bug

        out_led_guoqi_flag = false;
		//out_led_noauth_flag = false;
		if(mongodb_flag) //查询mongodb
		{
			if(mongodb_query_channel(out_msg->channel_id,auth_type,open_door,one_way,mohu_match,park_id) < 0) //根据channel_id查询通道属性
			{
			    mongodb_exit();
			    pthread_mutex_unlock(&mongo_mutex_car);
			    continue;
			}
		}
		else //查询mysql
		{
			mysql_query_channel((char*)"出口",out_msg->channel_id,auth_type,open_door,one_way,mohu_match,park_id);//根据channel_id查询通道属性
		}
		
		fprintf(fp_log,"%s##出口得到通道属性  通道id:%s 授权类型:%s 开闸类型:%s 同口进出:%s 模糊匹配:%s\n",time_now,out_msg->channel_id,auth_type,open_door,one_way,mohu_match);

		if(process_fuji(out_msg) < 0 ) //处理辅机
		{
			fflush(fp_log);
			mongodb_exit();
			pthread_mutex_unlock(&mongo_mutex_car);
			continue;
		}
		fprintf(fp_log,"%s##出口辅机处理 %d个相机 收到的车牌号为 1:%s 2:%s 处理后车牌号为%s \n",time_now,out_msg->num,out_msg->plate,out_msg->plate1,plate_ori);
		if(strlen(plate_ori) < 4)
		{
			fprintf(fp_log,"%s##车牌号为空\n",time_now);
			mongodb_exit();
			pthread_mutex_unlock(&mongo_mutex_car);
			continue;
		}
		
		memset(out_pic_path,0,256);
		memcpy(out_pic_path,out_msg->path,strlen(out_msg->path));
        memset(out_pcolor,0,24);
        memcpy(out_pcolor,out_msg->pcolor,strlen(out_msg->pcolor));
        memset(in_out_flag,0,sizeof(in_out_flag));
        memcpy(in_out_flag,out_msg->in_out,strlen(out_msg->in_out));

        fmoney = 0;

		if(strcmp(plate_last,plate_ori) == 0 && (get_tick(time_now)	-  plate_last_time < g_car_out_last_time_) )
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
		
        if(strcmp(one_way,"是") == 0 && strcmp(in_plate_last,plate_ori) == 0)
		{
            if(get_tick(time_now) - in_plate_time < 60)
			{
				fprintf(fp_log,"%s##同口进出 连续拍照\n",time_now);
				fflush(fp_log);
				mongodb_exit();
				pthread_mutex_unlock(&mongo_mutex_car);
				continue;	
			} 
		}
        memset(out_plate_last,0,PLATE_LENGTH);
        memcpy(out_plate_last,plate_ori,PLATE_LENGTH);
		out_plate_time = get_tick(time_now);

        switch (mongodb_validate_blacklist(out_msg->plate,out_msg->blacklistreason))
		{
			case 0:
				printf("blacklist================ 0 \n");
				strcpy(out_msg->blacklist,"");
				break;
			case 1:
				printf("blacklist================ 1 \n");
				strcpy(out_msg->blacklist,"提醒");
				send_bled(out_msg);
				break;
			case 2:
				printf("blacklist================ 2 \n");
				strcpy(out_msg->blacklist,"警告");
				send_bled(out_msg);
				break;
			case 3:
				printf("blacklist================ 3 \n");
				strcpy(out_msg->blacklist,"禁入");
				strcpy(out_msg->plate,out_plate_last);
				strcpy(plate_final,out_plate_last);
				send_bled(out_msg);
				BoonMsgQueue::Instance()->release_out(out_msg); //释放队列

				mongodb_exit();
				pthread_mutex_unlock(&mongo_mutex_car);
				continue;
        }

        if(strcmp(plate_ori,"无车牌") != 0) //有车牌
		{
			if(process_mohu_match(out_msg) < 0) //进行模糊匹配
			{
				fflush(fp_log);
				mongodb_exit();
				pthread_mutex_unlock(&mongo_mutex_car);
				continue;
			}				
			if(strcmp(plate_ori,plate_final) == 0) //原始车牌号与最终车牌号一致
			{
				fprintf(fp_log,"%s##出口车牌号为%s\n",time_now,plate_final);
			}
			else //原始车牌号与最终车牌号不一致
			{
				fprintf(fp_log,"%s##经过模糊匹配,出口车牌号由%s变为%s\n",time_now,plate_ori,plate_final);
			}

			if(strcmp(open_door,"有车放行") == 0 || strcmp(open_door,"有车牌号放行") == 0)
            {
				send_bipc_open_door(out_msg); //发送开杆信息
				fprintf(fp_log,"%s##开闸类型为%s,发送出口开闸信息\n",time_now,open_door);
				memset(open_door,0,24);
				strcpy(open_door,"自动抬杆");	
				memset(charge_type,0,24);
				strcpy(charge_type,"免费");
			}
			
			if(process_auth(out_msg) < 0) //处理授权方式
			{
				fflush(fp_log);
				mongodb_exit();
				pthread_mutex_unlock(&mongo_mutex_car);
				continue;
			}

			// 获取车辆类型 修改 global_car_type
			int tmp = validate_auth(out_msg);
			if(tmp < 0) //验证授权方式
			{
				fflush(fp_log);
				mongodb_exit();
				pthread_mutex_unlock(&mongo_mutex_car);
				continue;
			}
			
			bool have_in_park = false; //是否有在场记录
			int num = query_carinpark(out_msg->channel_id,plate_final,in_time,in_pic_path,in_channel_id,people_name,in_channel_name);
			if(num > 0) //查到在场记录
			{
				fprintf(fp_log,"%s##找到入场记录，进入时间为%s,离开时间为%s\n",time_now,in_time,time_now);
				have_in_park = true;
				fmoney = process_fee(out_msg->channel_id,plate_final,in_time,out_msg->time);
				if(fmoney < 0) 
				{
					snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] error! 收费为[%d]元 line[%d]", __FUNCTION__, fmoney, __LINE__);
					writelog(log_buf_);

					fprintf(fp_log,"%s##计费为负\n",time_now);
					mongodb_exit();
					pthread_mutex_unlock(&mongo_mutex_car);
					continue;
				}
			}
			else
			{
                snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] error! 没有找到 车牌plate_final[%s]的入场记录 line[%d]", __FUNCTION__, plate_final, __LINE__);
                writelog(log_buf_);

				memset(in_pic_path,0,128);
				fprintf(fp_log,"%s##未找到入场记录\n",time_now);

				memset(in_time,0,24);	
				memcpy(in_time,time_now,strlen(time_now));
				fmoney = 0;
				//sprintf(speak_send,"%s未找到入场记录",speak_send);		
			}

			if(fmoney >0 || (strcmp("临时车",global_car_type) == 0) || (strcmp("员工车辆",global_car_type) == 0))
			{
				if(fmoney == 0)
				{
					sprintf(speak_send,"%s收费零元",speak_send);
				}
				else
				{
					sprintf(speak_send,"%s收费%d元",speak_send,fmoney);
				}
			}

            memcpy(out_msg->in_time,in_time,24);
			if(send_bled(out_msg) < 0) //发送led显示信息和语音信息
			{
				fflush(fp_log);
				mongodb_exit();
				pthread_mutex_unlock(&mongo_mutex_car);
				continue;
			}

			if(strcmp(open_door,"有车放行") == 0 || strcmp(open_door,"有车牌号放行") == 0)
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] open_door[%s] line[%d]", __FUNCTION__, open_door, __LINE__);
                writelog(log_buf_);

				if(mongodb_flag) //查询mongodb
				{
					mongodb_delete_car_inpark(plate_final,park_id);	//删除在场记录
				}
				else
				{
					mysql_delete_car_inpark((char*)"出口",plate_final,park_id); //删除在场记录
				}
			}
			
			if(strcmp(open_door,"授权车放行") == 0)
			{
				if( (auth == true && fmoney ==0) ||(fmoney == 0 && have_in_park == true))
                // if( (auth == true && fmoney ==0) ||(fmoney == 0 ))
				{
                    snprintf(log_buf_, sizeof(log_buf_), "[%s] 发送出口开闸信息 auth[%d] have_in_park[%d] fmoney[%d] line[%d]", __FUNCTION__, auth, have_in_park, fmoney, __LINE__);
                    writelog(log_buf_);

					fprintf(fp_log,"%s##开闸类型为%s,授权信息为%d,收费为%d元,发送出口开闸信息\n",time_now,open_door,auth,fmoney);
					memset(open_door,0,24);
					strcpy(open_door,"自动抬杆");
					memset(charge_type,0,24);
                    send_bipc_open_door(out_msg); //发送开杆信息

					if(mongodb_flag) //查询mongodb
					{
						mongodb_delete_car_inpark(plate_final,park_id);	//删除在场记录
					}
					else
					{
						mysql_delete_car_inpark((char*)"出口",plate_final,park_id); //删除在场记录
					}	

					if(auth == true)
					{
						strcpy(charge_type,"免费");
					}
					else
					{
						strcpy(charge_type,"现金");
					}
				}
				else
				{
                    snprintf(log_buf_, sizeof(log_buf_), "[%s] 不发送出口开闸信息 权限auth[%d] 在场记录have_in_park[%d] 金额fmoney[%d] 需人工处理 line[%d]", __FUNCTION__, auth, have_in_park, fmoney, __LINE__);
                    writelog(log_buf_);

					fprintf(fp_log,"%s##开闸类型为%s,授权信息为%d,收费为%d元,不发送出口开闸信息\n",time_now,open_door,auth,fmoney);
					memset(open_door,0,24);
					strcpy(open_door,"人工处理");	
					memset(charge_type,0,24);
					strcpy(charge_type,"现金");
				}	
			}

			fflush(fp_log);
            memset(out_plate,0,PLATE_LENGTH);
			memset(out_channel,0,256);
			memset(out_time,0,24);
			memcpy(out_plate,plate_final,strlen(plate_final));
			memcpy(out_channel,out_msg->channel_id,strlen(out_msg->channel_id));
			memcpy(out_time,out_msg->time,strlen(out_msg->time));

			int result = process_top_park(out_msg) ;
			if(result< 0) 
			{
				fflush(fp_log);
				mongodb_exit();
				pthread_mutex_unlock(&mongo_mutex_car);
				continue;	
			}
			else 
			{
				if(mongodb_flag) //查询mongodb
				{
					mongodb_write_caroutrec(in_time,fmoney,0,plate_ori,plate_final,in_pic_path,in_channel_id,park_id,park_parent_id,global_car_type,operator_name,open_door,charge_type,out_msg);//写数据到出场表
				}
				else
				{
					mysql_write_caroutrec(in_time,fmoney,0,plate_ori,plate_final,in_pic_path,in_channel_id,park_id,park_parent_id,global_car_type,operator_name,open_door,charge_type,out_msg); //写数据到出场表
				}	
			}

			if(send_bgui(out_msg) < 0) //发送信息给bgui
			{
				fflush(fp_log);
				mongodb_exit();
				pthread_mutex_unlock(&mongo_mutex_car);
				continue;
			}
			gettimeofday(&DayTime, NULL);
			e = DayTime.tv_sec * 1000 + DayTime.tv_usec / 1000;
			fprintf(fp_log,"%s##出口线程耗时%d ms \n",time_now,e-s);
			fflush(fp_log);

		}
		else //无车牌
		{
            memset(plate_final,0,PLATE_LENGTH);
			strcpy(plate_final,"无车牌");
			memset(global_car_type,0,24);
			strcpy(global_car_type,"临时车");
			memset(speak_send,0,256);
			strcpy(speak_send,"临时车");

            memset(out_plate,0,PLATE_LENGTH);
			memset(out_channel,0,256);
			memset(out_time,0,24);
			memcpy(out_plate,plate_final,strlen(plate_final));
			memcpy(out_channel,out_msg->channel_id,strlen(out_msg->channel_id));
			memcpy(out_time,out_msg->time,strlen(out_msg->time));

			if(strcmp(open_door,"有车放行") == 0)
			{
                snprintf(log_buf_, sizeof(log_buf_), "[%s] send_bipc_open_door 无牌车 开闸类型open_door[%s] line[%d]", __FUNCTION__, open_door, __LINE__);
                writelog(log_buf_);

				send_bipc_open_door(out_msg); //发送开杆信息
				memset(open_door,0,24);
				strcpy(open_door,"自动抬杆");
				memset(charge_type,0,24);
				strcpy(charge_type,"免费");
			}
			else
			{
                snprintf(log_buf_, sizeof(log_buf_), "[%s] 无牌车 开闸类型open_door[%s] 需人工处理 line[%d]", __FUNCTION__, open_door, __LINE__);
                writelog(log_buf_);

				memset(open_door,0,24);
				strcpy(open_door,"人工处理");	
				memset(charge_type,0,24);
				strcpy(charge_type,"现金");	
			}

			if(mongodb_flag) //查询mongodb
			{
                // 写出场表
				mongodb_write_caroutrec(in_time,fmoney,0,plate_ori,plate_final,in_pic_path,in_channel_id,park_id,park_parent_id,global_car_type,operator_name,open_door,charge_type,out_msg);
			}
			else
			{
				mysql_write_caroutrec(in_time,fmoney,0,plate_ori,plate_final,in_pic_path,in_channel_id,park_id,park_parent_id,global_car_type,operator_name,open_door,charge_type,out_msg);
			}
			
			if(send_bled(out_msg) < 0) //发送led显示信息和语音信息
			{
				fflush(fp_log);
				mongodb_exit();
				pthread_mutex_unlock(&mongo_mutex_car);
				continue;
			}

			if(send_bgui(out_msg) < 0) //发送信息给bgui
			{
				fflush(fp_log);
				mongodb_exit();
				pthread_mutex_unlock(&mongo_mutex_car);
				continue;
			}
		}

		BoonMsgQueue::Instance()->release_out(out_msg); //释放队列
		if(mongodb_flag)
		mongodb_exit();
		pthread_mutex_unlock(&mongo_mutex_car);
		
	}
}

/**
 * @brief: 车离开时间矫正
 * @return
 */
std::string carOutTime()
{
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    snprintf(t, sizeof(t), "%04d-%02d-%02d %02d:%02d:%02d", 1900 + p->tm_year,
             1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    std::string str = t;

    return str;
}
/**************************************************end******************************************/

