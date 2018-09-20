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
    writelog("Network Initializing...");
    if (tcp_server.open_bind_listen()) {
        tcp_server.msg_buff = &g_msg_buff;
    } else {
        writelog("TCP initialization failed, Please check port 6000.");
        exit(EXIT_FAILURE);
    }
    writelog("Network Initialization OK");
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
    //开始侦听消息
    while (true) {
        tcp_server.get_message();
    }
    return NULL;
}

void* wx_tcp_msg(void *) //wx_tcp消息处理线程
{
    while (true) {
        std::pair<int, string> msg_info;
        g_msg_buff.wait_and_pop(msg_info);
        int sockfd = msg_info.first;
        string msg = msg_info.second;
        // 将收到的消息内容写入日志
        cout << " recv_msg " << msg << endl;
        writelog(msg.c_str());
        char response_buf[1024] = "response";
        // Analyze the message
        
        memcpy(response_buf, msg.c_str(), msg.length());//test
        
        int n = write(sockfd, response_buf, strlen(response_buf));
        string str_response = response_buf;
        cout << " str_response " << str_response << endl;
        // 将发送的消息内容写入日志
        writelog(str_response.c_str());
    }
}

// 写入日志
void writelog(const char* _buf)
{
    BLog::writelog(_buf, PKGDATA, LOG_DIR_NAME, LOG_FILE_NAME);
}

/**************************************************end******************************************/

