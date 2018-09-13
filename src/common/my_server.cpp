//
// Created by boon on 17-11-10.
//

#include "common_def.h"
#include "my_server.h"

my_server::my_server()
        : tc(nullptr)
        , has_recv_(false)
{

}

void my_server::start()
{
    thread id = thread(&my_server::listenThread, this, this);
    id.detach();

    sleep(1);

    thread id2 = thread(&my_server::sendThread, this, this);
    id2.detach();
}

void* my_server::listenThread(void* para)
{
    my_server *p = (my_server*)para;

    if(!p->tcp_server_.listen(true, PORT_SERVER_BCENTER)){

        p->tcp_server_.close();

        return (void*)-1;
    }else{
        printf("listen success!\n");
    }

    while(1){

        if(0 == p->acceptWork()){
            //break;
            ;
        }

        usleep(5);
    }

    printf("收到一个客户端连接，退出监听{该server只接收一个连接!}\n");

    return (void*)0;
}

void* my_server::sendThread(void* para)
{
    my_server *p = (my_server*)para;

    while(1){

        usleep(1);

        if(!p->has_recv_){
            printf("等待客户端连接\n");
            sleep(5);
            continue;
        }

        if(!p->tc || p->tc == nullptr)
            continue;

        p->tc->workio();

        // 铁路测试
        //sendRecognition();
        //sendOver();
    }

}

void my_server::sendRecognition()
{
    char data[1024] = {0};
    std::string str = "PushLabel|1#DF4D-4008#201711011425/1_DF4D-4008_2.jpg#2";

    snprintf(data, sizeof(data), "%s\r\n", str.c_str());

    int len = strlen(data);

    int ret = tc->senddata(data, len);

    printf("send: len[%d][%ld], msg[%s]\n", ret, str.length(), data);

    sleep(5);
}

void my_server::sendOver()
{
    char data[1024] = {0};
    std::string str = "PushLabel|0#OVER#BOON Intelligent Visual Train Identification System#0";

    snprintf(data, sizeof(data), "%s\r\n", str.c_str());

    int len = strlen(data);

    int ret = tc->senddata(data, len);

    printf("send: len[%d][%ld], msg[%s]\n", ret, str.length(), data);

    sleep(5);
}

int my_server::acceptWork()
{
    base_cb_cli_attr g_cli_cfg;
    g_cli_cfg.recv_buf_len = MSG_BUFF_LEN;
    g_cli_cfg.send_buf_len = MSG_BUFF_LEN;

    tc = new my_client(&g_cli_cfg);

    int ret = tcp_server_.accept(tc, 3);
    if (BASE_ERR_SOCKET == ret)
    {
        delete tc;
        tc = nullptr;

        printf("error, accept error!\n");

        return -1;
    }

    if (ret > 0)
    {
        has_recv_ = true;
        printf("收到一个客户端连接\n");

        tc->workio();

        // 开始收发数据线程
        //tc->start();

        return 0;
    }
    else
    {
        //printf("没有连接\n");

        delete tc;
        tc = nullptr;

        return -1;
    }
}


std::string my_server::getJsonData(const std::string& _cmd,
                                   const int imageSn, const string videoSn,
                                   const string label, const string port, const string path)
{
    if(_cmd.empty())
        return "";

    Json::Value json_data;

    if( 0 == _cmd.compare("recognization_result")){

        json_data["cmd"] = "recognization_result";
        char sn[32] = {0};
        snprintf(sn, sizeof(sn), "%d", imageSn);
        json_data["image_sn"] = sn;
        json_data["video_sn"] = videoSn;
        json_data["image_timestamp"] = getYmdHms();
        json_data["image_label"] = label;
        json_data["image_port"] = port;
        json_data["image_path"] = path;
    }else{
        return "";
    }

    json_data["mod"] = "tbrain";

    return json_data.toStyledString();
}