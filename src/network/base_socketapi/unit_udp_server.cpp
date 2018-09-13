//
// Created by boon on 17-10-23.
//

#include "rapidjson_lib.h"
#include "utils.h"
#include "base_socketapi.h"

#include <iostream>
#include <stdio.h>
using namespace std;

int port = 5022;

int test_server();

int test_brocast();

bool parse_json(const std::string& _json, std::string& _out);

int main(int argc, char** argv)
{
    int ret = test_server();
    //int ret = test_brocast();

    if(ret <= 0)
        printf("%s send error, ret:%d line[%d]\n", __FUNCTION__, ret, __LINE__);

    //getchar();

    printf("end\n");

    return 0;
}

int test_brocast()
{
    printf("%s line[%d]\n", __FUNCTION__, __LINE__);
    int sockfd;
    struct sockaddr_in saddr;
    int r;
    char recvline[1025];
    struct sockaddr_in presaddr;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(5022);
    bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
    while (1)
    {
        r = recvfrom(sockfd, recvline, sizeof(recvline), 0 , (struct sockaddr*)&presaddr, &len);
        if (r <= 0)
        {
            perror("");
            exit(-1);
        }
        recvline[r] = 0;
        cout <<"recvfrom "<< inet_ntoa(presaddr.sin_addr) <<" " << recvline << endl;
    }

    return 0;
}


int test_server()
{
    int ret = 0;
    char buf[1024] = {0};
    strcpy(buf,"qoijwjeajoiwfeioefwijoewfioewfa");

    bool bret = false;
    base_udp udp;
    bret = udp.create(true, false);
    if(!bret) {
        printf("error line[%d]\n", __LINE__);
        return -1;
    }

    char ip[] = "127.0.0.1";
    bret = udp.bind(5022, NULL);
    if(!bret) {
        printf("error line[%d]\n", __LINE__);
        return -2;
    }

    printf("socket id: %d\n", udp.getsocketfd());

    char recv_buf[1024] = {0};

    while(1){
        sleep(1);

        ret = udp.recv(recv_buf, 1024, ip, sizeof(ip), port);
        if(ret <= 0){

            continue;
        }

        std::string recv = recv_buf;

        if(ret > 0 ){
            printf("[%s] [%s] recv[%s] ret[%d] line[%d]\n", printTime().c_str(),
                __FUNCTION__, recv.c_str(), ret, __LINE__);
        }

        std::string send = "";
        if(!parse_json(recv, send)){
            printf("[%s] [%s] error! line[%d]\n", printTime().c_str(), __FUNCTION__, __LINE__);
            continue;
        }

        ret = -1;
        ret = udp.write(const_cast<char*>(send.c_str()), send.length(), ip, 5021);
        if(ret <= 0){
            printf("[%s] [%s] error! send[%s] ret[%d] line[%d]\n", printTime().c_str(),
                   __FUNCTION__, send.c_str(), ret, __LINE__);
            continue;
        }else{
            printf("[%s] [%s] send[%s] ret[%d] line[%d]\n", printTime().c_str(),
                   __FUNCTION__, send.c_str(), ret, __LINE__);
            break;
        }

        usleep(500);

        printf("\n");
    }

    return ret;
}

// 解析json包
bool parse_json(const std::string& _str, std::string& _out)
{
    Document in_doc;
    in_doc.Parse(_str.c_str());

    std::string cmd = in_doc["cmd"].GetString();

    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    Value value(rapidjson::kObjectType);

    if(0 == cmd.compare("get_ipc_config")){

        doc.AddMember("cmd", "ipc_config", doc.GetAllocator());

        //rapidjson::Document body;
        //body.SetObject();

        rapidjson::Value info_array(rapidjson::kArrayType);
        for (int i = 0; i < 2; i++) {
            rapidjson::Value info_object(rapidjson::kObjectType);
            info_object.SetObject();

            // 成员
            if(i == 0){
                info_object.AddMember("channel_id", "551ce9c9fe6c3f94883f73b3dca9d33e", allocator);
                info_object.AddMember("in_out", "出口", allocator);
            }else{
                info_object.AddMember("channel_id", "5639f70ca4ae933159c7c46aaa1a250c", allocator);
                info_object.AddMember("in_out", "入口", allocator);
            }
            info_object.AddMember("one_way", "否", allocator);

            //******** 成员数组 **********
            rapidjson::Value ipc_array(rapidjson::kArrayType);
            for (int j = 0; j < 2; j++) {
                rapidjson::Value ipc_object(rapidjson::kObjectType);
                ipc_object.SetObject();

                if(i == 0){
                    if(j == 0){
                        ipc_object.AddMember("device_ip_id", "192.168.88.12", allocator);
                        ipc_object.AddMember("device_password", "12345", allocator);
                        ipc_object.AddMember("device_type", "中维智能相机", allocator);
                        ipc_object.AddMember("device_username", "admin", allocator);
                    }else{
                        ipc_object.AddMember("device_ip_id", "192.168.88.14", allocator);
                        ipc_object.AddMember("device_type", "LED语音一体机", allocator);
                    }
                }else{
                    if(j == 0){
                        ipc_object.AddMember("device_ip_id", "192.168.88.11", allocator);
                        ipc_object.AddMember("device_password", "12345", allocator);
                        ipc_object.AddMember("device_type", "中维智能相机", allocator);
                        ipc_object.AddMember("device_username", "admin", allocator);
                    }else{
                        ipc_object.AddMember("device_ip_id", "192.168.88.13", allocator);
                        ipc_object.AddMember("device_type", "LED语音一体机", allocator);
                    }
                }

                ipc_array.PushBack(ipc_object, allocator);

            }
            info_object.AddMember("ipc", ipc_array, allocator);
            //******** 成员数组 **********

            info_array.PushBack(info_object, allocator);
        }

        doc.AddMember("channel", info_array, allocator);

        rapidjson::StringBuffer buffer;//in rapidjson/stringbuffer.h
        rapidjson::Writer<StringBuffer> writer(buffer); //in rapidjson/writer.h
        doc.Accept(writer);

        _out = buffer.GetString();

        //printf("[%s] [%s] out[%s] line[%d]\n", printTime().c_str(), __FUNCTION__, _out.c_str(), __LINE__);

        return true;
    }

    return false;
}
