/***************************main.cpp***************************************
              功能：主函数
              创建时间：2017-02-04
              创建人：孙振行
              单位：山东博昂信息科技有限公司
              修改时间：
***************************************************************************/
#include "common_def.h"
#include "../global/boon_global.h"   
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../bipc/boon_bipc.h"
#include "../bop/boon_bop.h"
#include "../bvs/boon_bvs.h"
#include "../in/boon_in.h"
#include "../wx/boon_wx.h"
#include "../wx_tcp/wx_tcp.h"
#include "../out/boon_out.h"
#include "../bgui/boon_bgui.h"
#include "../chewei/boon_chewei.h"
#include "../test_mongo/boon_test_mongo.h"
#include "../bled/boon_bled.h"
#include "booncrypto.h"


FILE* fp_log;
char  host_ip[64];//主机ip
char  host_server_ip[64];//服务器ip
bool  mongodb_flag;//为1，使用mongodb，为0，使用mysql

pthread_mutex_t mongo_mutex_channel;
pthread_mutex_t mongo_mutex_carinpark;
pthread_mutex_t mongo_mutex_car;
pthread_mutex_t mongo_mutex_device;
pthread_t pid_in;
pthread_t pid_out;
pthread_t pid_bipc;
pthread_t pid_bop;
pthread_t pid_bvs;

pthread_t pid_bgui;
pthread_t pid_db;
pthread_t pid_chewei;
pthread_t pid_bled;
pthread_t pid_wx;
pthread_t pid_wx_tcp;

// 是否加入万能语音功能， 不在此赋值
bool g_has_wanwangyuyin_;
// 是否加入车队模式，不在此赋值
bool g_has_cheduimoshi_;

// 车出场时候持续时间，单位秒， 默认15
int g_car_out_last_time_;

// 车入场时候持续时间，单位秒, 默认15
int g_car_in_last_time_;

// 日志
static char log_buf_[LOG_BUF_SIZE] = {0};
// 记录日志
extern void writelog(const char* _buf);

// 初始化参数
void initParm();

int main(void)
{
    std::string version = "程序版本号: 3.14_20190913 ffcore过渡版!";

    writelog(version.c_str());
    std::cout<< version <<std::endl;

    // 初始化参数
    initParm();

    mongoc_init();

    Booncrypto cpy;
    if(!cpy.verifyfile())
    {
        std::cout<<"本设备未经授权，程序退出"<<std::endl;
        //return -1;
    }

    int rtn = -1;
    /**************************************进程互斥***************************************/
    rtn = is_have_instance();
    if(rtn == -1)
    {
        printf("请先杀死上一个bcenter进程,bcenter退出\n");
        return 0;
    }
    /*****************************************end***************************************/

    /**************************************创建日志文件***********************************/
    rtn = create_log_file();
    if(rtn == -1)
    {
        printf("打开日志文件失败,bcenter退出\n");
        return 0;
    }
    fprintf(fp_log,"版本号:3.1415_20171031 标准版\n");
    /*****************************************end***************************************/

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);   //获取当前时间

    /**************************************连接mysql数据库*******************************/
    rtn = mysql_connect();
    if(rtn == -1)
    {
        fprintf(fp_log,"%s##mysql连接失败,bcenter退出\n",time_now);
        printf("mysql连接失败,bcenter退出\n");
        fclose(fp_log);
        return 0;
    }
    else
    {
        fprintf(fp_log,"%s##mysql连接成功\n",time_now);
    }
    /*****************************************end***************************************/

    /*********************查询mysql数据库里的host表，得到本机ip和服务器ip********************/
    memset(host_ip,0,64);
    memset(host_server_ip,0,64);
    rtn = mysql_query_host(host_ip,host_server_ip);
    if(rtn == 0)
    {
        fprintf(fp_log,"%s##mysql host表查询成功,本机ip为%s,服务器ip为%s\n",time_now,host_ip,host_server_ip);
    }
    else
    {
        printf("mysql host表查询失败，bcenter退出\n");
        fprintf(fp_log,"%s##mysql host表查询失败,bcenter退出\n",time_now);
        fclose(fp_log);
        return 0;
    }
    /*****************************************end***************************************/

    /*************************************连接mongodb数据库******************************/

    int sockfd=socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in addr_ser;
    bzero(&addr_ser,sizeof(addr_ser));
    addr_ser.sin_family=AF_INET;
    addr_ser.sin_addr.s_addr=inet_addr(host_server_ip);
    addr_ser.sin_port=htons(27017);

    int err=connect(sockfd,(struct sockaddr *)&addr_ser,sizeof(addr_ser));
    shutdown(sockfd,2);
    close(sockfd);

    if(err >= 0)
    {
        mongodb_flag = 1;  //mongodb_flag为1，则使用mongodb数据库
        fprintf(fp_log,"%s##mongodb 连接成功\n",time_now);
    }
    else
    {
        mongodb_flag = 0; //mongodb_flag为0， 则使用mysql数据库
        fprintf(fp_log,"%s##mongodb 连接失败,使用mysql数据库\n",time_now);
    }

    /*****************************************end***************************************/

    /***********************************创建线程******************************************
                    1.bipc监听线程
                    2.入口处理线程
                    3.出口处理线程
                    4.bgui监听线程
                    5.服务器数据库侦测线程
                    6.车位计算线程
    ************************************************************************************/

    pthread_create(&pid_bipc, NULL, bipc_thread, NULL);
    pthread_detach(pid_bipc);
    sleep(1);
    pthread_create(&pid_bop, NULL, bop_thread, NULL);
    pthread_detach(pid_bop);

    pthread_create(&pid_bvs, NULL, bvs_thread, NULL);
    pthread_detach(pid_bvs);
    sleep(1);
    pthread_create(&pid_in, NULL, in_thread, NULL);
    pthread_detach(pid_in);

    pthread_create(&pid_out, NULL, out_thread, NULL);
    pthread_detach(pid_out);

    pthread_create(&pid_bgui, NULL, bgui_thread, NULL);
    pthread_detach(pid_bgui);

    pthread_create(&pid_db, NULL, test_mongo_thread, NULL);
    pthread_detach(pid_db);

    pthread_create(&pid_chewei, NULL, chewei_thread, NULL);
    pthread_detach(pid_chewei);

    pthread_create(&pid_bled, NULL, bled_thread, NULL);
    pthread_detach(pid_bled);

    pthread_create(&pid_wx, NULL, wx_thread, NULL);
    pthread_detach(pid_wx);
    //WX_TCP 2018-09-20 孙希伟、孙振行加入
    pthread_create(&pid_wx_tcp, NULL, wx_tcp_thread, NULL);
    pthread_detach(pid_wx_tcp);

    /*****************************************end***************************************/

    pthread_mutex_init(&mongo_mutex_channel,NULL);
    pthread_mutex_init(&mongo_mutex_carinpark,NULL);
    pthread_mutex_init(&mongo_mutex_car,NULL);
    pthread_mutex_init(&mongo_mutex_device,NULL);
    printf("程序启动成功!\n");
    fprintf(fp_log,"%s##bcenter启动成功!\n",time_now);
    fflush(fp_log);

    while(1)
    {
        usleep(10000000);
    }

    return 0;
}


/**
 * @brief: 初始化全局参数
 *          1. 是否采用车队模式
 *          2. 是否使用万能语音
 *          3. 设置出场持续间隔
 */
void initParm()
{
    // 车队模式
    g_has_cheduimoshi_ = false; // true: 使用车队模式； false：不使用车队模式

    // 万能语音设置
    /*  使用万能语音true： 建筑大学, 汇隆广场
     *  不使用万能语音false： 交运集团, 金色阳光, 吉化大厦
     * */
    g_has_wanwangyuyin_ = false;

    // 车出场时持续间隔 秒数
    /*  交运间隔设置： 600
     *  金色阳光设置： 180
     *  其他设置： 15
     * */
    g_car_out_last_time_ = 15; // 单位秒，默认15

    // 车入场时持续间隔 秒数
    /*  交运洗车间隔设置： 600
     *  金色阳光设置： 180
     *  其他设置： 15
     * */
    g_car_in_last_time_ = 15; // 单位秒，默认15

    snprintf(log_buf_, sizeof(log_buf_), "[%s] main 万能语音[%d] 车队模式[%d] 入场持续间隔[%d] 出场持续间隔[%d] line[%d]", __FUNCTION__,
             g_has_wanwangyuyin_, g_has_cheduimoshi_, g_car_in_last_time_, g_car_out_last_time_, __LINE__);
    writelog(log_buf_);
}
