//
// Created by boon on 17-11-24.
//

#ifndef BPARK_CWEIXIN_H
#define BPARK_CWEIXIN_H

//#include "base_socketapi.h"
#include "../../network/base_socketapi/base_socketapi.h"

#include "common_def.h"
#include "mongodb_model.h"

#include <thread>

using namespace std;

namespace ONCALL{
    class CWeiXin {
    public:
        CWeiXin();
        ~CWeiXin();

        // 启动服务
        void start();

    private:
        // 初始化
        void init();
        // 初始化udp服务
        bool initUdpServer();
        // 工作线程
        void* workThread(void* para);
        // 主工作
        void work();

        // 解析对端发过来的消息
        void unPackMessage(const char* _data);
        // 校验json包
        bool checkJsonFormat(const char* _data);

        // 写日志接口
        void writelog(const char* _buf);

        //***** begin: 业务消息包 ******
        // 出场消息
        bool processCarout(const char* _data);
        // 场内支付消息
        bool processPay(const char* _data);
        //***** end: 业务消息包 ******

    private:
        // 工作线程id
        thread twork_id_;

        // 操作mongodb数据库
        CMongoDbModel mongodb_;

        // 停车场ID， 来源于web端发送
        std::string park_ID_str_;

        // 本地udp服务
        base_udp udp_local_;

        // 本机ip
        char udp_local_ip_[32];
        // 绑定的udp服务端端口
        int udp_local_port_;

        // 接收数据缓冲区
        char recv_buf_[MSG_BUFF_LEN];

        // 日志缓冲区
        char log_buf_[LOG_BUF_SIZE];
    };
};

#endif //BPARK_CWEIXIN_H
