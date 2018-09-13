//
// Created by boon on 17-11-10.
//

#ifndef BPARK_MY_SERVER_H
#define BPARK_MY_SERVER_H

#include "utils.h"
#include "base_socketapi.h"
#include "my_client.h"

#include <thread>
using namespace std;

class my_server {
public:
    my_server();

    void start();

private:

    void* listenThread(void* para);

    void* sendThread(void* para);

    std::string getJsonData(const std::string& _cmd,
                            const int imageSn, const string videoSn,
                            const string label, const string port, const string path);

    int acceptWork();

    void sendRecognition();

    void sendOver();

private:
    // 服务端
    base_tcpserver tcp_server_;

    my_client *tc;

    bool has_recv_;
};


#endif //BPARK_MY_SERVER_H
