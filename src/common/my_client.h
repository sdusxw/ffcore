/*********************************************************************************
  *Copyright(C):        sdboon.com
  *FileName:            my_client.h
  *Author:              diaoguangqiang
  *Version:             2.0
  *Date:                2017.10.15
  *Description:         测试类：模拟厂家发包
  *                         1. 发送startwork消息
  *                         2. 发送stopwork消息
  *History:             (修改历史记录列表，每条修改记录应包含修改日期、修改者及修改内容简介)
     1. Date:           2017.10.15
        Author:         diaoguagnqiang
        Modification:   首次生成文件
     2. Data:
        Author:
        Modification:
**********************************************************************************/
#ifndef TLRS_CMYCLIENT_H
#define TLRS_CMYCLIENT_H

#include "utils.h"
#include "base_socketapi.h"

#include <thread>
using namespace std;

class my_client : public base_cb_cli{
public:
    my_client(base_cb_cli_attr* argv);


    bool connect(char* _ip, short _port);
    bool close();

    void start();

    int	checkConnect();

    // 启动服务，支持铁路
    void sendStart();
    // 停止服务，支持铁路
    void sendStop();

    // 入场，支持民用
    void sendIn();
    // 场内支付，支持民用
    void sendPay();
    // 出场，支持民用
    void sendOut();

    int workio();

    // 获取服务端ip
    char* getServerIp(){return this->server_ip_;};
    // 获取服务端port
    int getServerPort(){return this->server_port_;};

private:
    bool connect();

    int ondata(const void* data, unsigned int len);

    void clientThread(void* para);

    void work();
    int work_io(int _mask);

    const bool getJsonData(const std::string& _cmd, std::string& _out_json);

    void sendMessage(const std::string& str, const int& len);

    // 解析web的消息
    void parseWebMessage(const char* _data, const int& _len);

private:
    //
    char server_ip_[32];
    //
    short server_port_;

};


#endif //TLRS_CMYCLIENT_H
