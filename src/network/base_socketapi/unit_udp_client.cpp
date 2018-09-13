//
// Created by boon on 17-10-23.
//

#include "base_socketapi.h"
#include "utils.h"

#include <stdio.h>
#include <iostream>

using namespace std;

void test_client();

int test_brocast();

int test_udp();

int main(int argc, char** argv)
{
    // test_client();
    //test_brocast();
    test_udp();

    getchar();
}

int port = 6001;

int test_brocast()
{
    int sockfd;
    struct sockaddr_in des_addr;
    int r;
    char sendline[1024] = {"Hello"};
    const int on = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)); //设置套接字选项
    bzero(&des_addr, sizeof(des_addr));
    des_addr.sin_family = AF_INET;
    des_addr.sin_addr.s_addr = inet_addr("192.168.199.255"); //广播地址
    des_addr.sin_port = htons(port);
    r = sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr*)&des_addr, sizeof(des_addr));
    if (r <= 0)
    {
        perror("");
        exit(-1);
    }
    cout << "finish" << endl;
    return 0;
}

void test_client()
{
    int ret = 0;
    char buf[1024] = {"test helo"};

    bool bret = false;

    base_udp udp;
    bret = udp.create(true, false);
    if(!bret)
        printf("%s error line[%d]\n", __FUNCTION__,  __LINE__);

    char ip[] = "127.0.0.1";
//    bret = udp.connect(6000, ip);
//    if(!bret)
//        printf("error line[%d]\n", __LINE__);

    struct sockaddr_in des_addr;
    bzero(&des_addr, sizeof(des_addr));
    des_addr.sin_family = AF_INET;
    des_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //广播地址
    des_addr.sin_port = htons(port);

    while(1){
        int ret = udp.write(buf, strlen(buf), ip, port);
        //int ret = sendto(udp.getsocketfd(), buf, strlen(buf), 0, (struct sockaddr*)&des_addr, sizeof(des_addr));

        //int ret = udp.send(buf, 1024);
        printf("ret: %d line[%d]\n", ret, __LINE__);

        sleep(3);
    }
}

int test_udp()
{
    char buf[1024] = {"test helo world"};

    bool bret = false;

    base_udp udp;
    bret = udp.create(true, false);
    if(!bret)
        printf("%s error line[%d]\n", __FUNCTION__,  __LINE__);

    char ip[] = "127.0.0.1";

    while(1){
        int ret = udp.write(buf, strlen(buf), ip, port);
        printf("[%s] [%s] send[%s] len[%ld] ret[%d] ip[%s] port[%d] line[%d]\n",
               printTime().c_str(),
               __FUNCTION__, buf, strlen(buf), ret, ip, port, __LINE__);

        sleep(3);
    }
}