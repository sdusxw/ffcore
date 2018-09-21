/***************************boon_wx.cpp***************************************
			  功能：与wx通信
			  创建时间：2017-02-05
              修改时间：2018-09-18
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
			        1. 2017/11/21 14:21
			            继时旭之后，修改/添加微信支付宝双码支付功能
                    2. 2018/09/18 09:50
                        改成tcp通讯接收aipayclient的消息 孙希伟
***************************************************************************/
#include "boon_log.h"
#include "common_def.h"
#include "../bcenter_def.h"

#include "net_tcp.h"
#include "wx_tcp.h"

#include <pthread.h>
#include <jsoncpp/json/json.h>

using namespace BASE;

static char log_buf_[LOG_BUF_SIZE] = {0};

extern pthread_mutex_t mongo_mutex_car;

static NetTcpServer tcp_server;     //wx_tcp

static concurrent_queue<std::pair<int, string> > g_msg_buff;        //消息队列

bool wx_tcp_init() //初始化微信tcp支付消息
{
    //初始化网络设置 tcp_server网络服务器
    write_log("Network Initializing...");
    if (tcp_server.open_bind_listen()) {
        tcp_server.msg_buff = &g_msg_buff;
    } else {
        write_log("TCP initialization failed, Please check port 6000.");
        exit(EXIT_FAILURE);
    }
    write_log("Network Initialization OK");
    return true;
}

/**************************************************wx监听线程******************************/
void* wx_tcp_thread(void *)
{
    //初始化tcp_server
    wx_tcp_init();
    //开启消息处理线程
    pthread_t pid_wx_msg;
    pthread_create(&pid_wx_msg, NULL, wx_tcp_msg, NULL);
    pthread_detach(pid_wx_msg);
    printf("wx_tcp线程启动成功\n");
    //开始侦听消息
    while (true) {
        tcp_server.get_message();
    }
    return NULL;
}

void* wx_tcp_msg(void *) //wx_tcp消息处理线程
{
    std::string log_str="";
    char time_now[64];
    time_t tm;
    while (true) {
        std::pair<int, string> msg_info;
        g_msg_buff.wait_and_pop(msg_info);
        int sockfd = msg_info.first;
        string msg = msg_info.second;
        // 将收到的消息内容写入日志
        cout << " recv_msg " << msg << endl;
        write_log(msg.c_str());
        // Analyze the message
        Json::Reader reader;
        Json::Value json_object;
        
        if (!reader.parse(msg, json_object))
        {
            //JSON格式错误导致解析失败
            log_str = "[json]解析失败";
            write_log(log_str.c_str());
        }
        else
        {
            time_printf(time(&tm),time_now);   //获取当前时间
            pthread_mutex_lock(&mongo_mutex_car);
            
            if(mongodb_flag)
                mongodb_connect();
            
            std::string str_cmd = json_object["cmd"].asString();
            std::string name;
            std::string park_id;
            std::string box_ip;
            std::string plate;
            std::string openid;
            std::string userid;
            std::string money;
            std::string flag;
            std::string outime;
            if (str_cmd == "query_pay") {
                park_id = json_object["park_id"].asString();
                box_ip = json_object["box_ip"].asString();
                plate = json_object["plate"].asString();
                openid = json_object["openid"].asString();
                userid = json_object["userid"].asString();
                outime = json_object["outime"].asString();
                
                // 场内查询支付费用 char *park_id, char *box_ip, char *plate, char *openid, char* userid, char *outime
                mongodb_process_wx_tcp_query_fee_in(str_cmd.c_str(), park_id.c_str(), plate.c_str(), openid.c_str(), userid.c_str(), outime.c_str(), sockfd);
            }
            else if (str_cmd == "pay") {
                park_id = json_object["park_id"].asString();
                
                plate = json_object["plate"].asString();
                openid = json_object["openid"].asString();
                userid = json_object["userid"].asString();
                money = json_object["money"].asString();
                flag = json_object["flag"].asString();
                
                if(0 == flag.compare("open")){ // 出场口支付
                    snprintf(log_buf_, sizeof(log_buf_), "[%s] 出场口支付费用 cmd[%s] flag[%s] line[%d]", __FUNCTION__, str_cmd.c_str(), flag.c_str(), __LINE__);
                    write_log(log_buf_);
                    
                    // 优惠额度
                    std::string dis_money = json_object["dis_money"].asString();
                    // 应收额度
                    std::string fact_money = json_object["fact_money"].asString();
                    
                    box_ip = json_object["box_ip"].asString();
                    
                    mongodb_process_wx_tcp_pay_open((char *)money.c_str(),(char *)park_id.c_str(),(char *)box_ip.c_str(),
                                                      (char *)plate.c_str(),(char *)openid.c_str(),(char *)flag.c_str(),
                                                      (char*)userid.c_str(), (char*)dis_money.c_str(), (char*)fact_money.c_str(), sockfd);
                    
                }else if(0 == flag.compare("pay")){ // 场内支付
                    
                    snprintf(log_buf_, sizeof(log_buf_), "[%s] 场内支付费用 cmd[%s] flag[%s] line[%d]", __FUNCTION__, str_cmd.c_str(), flag.c_str(), __LINE__);
                    write_log(log_buf_);
                    
                    // 优惠额度
                    std::string dis_money = json_object["dis_money"].asString();
                    // 应收额度
                    std::string fact_money = json_object["fact_money"].asString();
                    
                    mongodb_process_wx_tcp_pay_in(money.c_str(), park_id.c_str(), plate.c_str(),
                                              openid.c_str(), userid.c_str(), flag.c_str(),
                                              dis_money.c_str(), fact_money.c_str(), sockfd);
                }
                
            }
            else if(str_cmd == "out")
            {
                // 车主姓名，停车场id，停车场ip地址，车牌号，出场时间
                name = json_object["name"].asString();
                park_id = json_object["park_id"].asString();
                box_ip = json_object["box_ip"].asString();
                plate = json_object["plate"].asString();
                openid = json_object["openid"].asString();
                userid = json_object["userid"].asString();
                outime = json_object["outime"].asString();
                
                snprintf(log_buf_, sizeof(log_buf_), "[%s] 出场 查询停车费用 cmd[%s] line[%d]", __FUNCTION__, str_cmd.c_str(), __LINE__);
                write_log(log_buf_);
                
                mongodb_process_wx_tcp_carout((char *)name.c_str(),(char *)park_id.c_str(),(char *)box_ip.c_str(),(char *)plate.c_str(),(char *)openid.c_str(),time_now, (char*)userid.c_str(), sockfd);
            }
            if(mongodb_flag)
                mongodb_exit();
            
            pthread_mutex_unlock(&mongo_mutex_car);
        }
    }
}

// 写入日志
void write_log(const char* _buf)
{
    BLog::writelog(_buf, PKGDATA, LOG_DIR_NAME, LOG_FILE_NAME);
}

/**************************************************end******************************************/

