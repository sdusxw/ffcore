/*********************************************************************************
  *Copyright(C):        sdboon.com
  *FileName:            unit_client.cpp
  *Author:              diaoguangqiang
  *Version:             2.0
  *Date:                2017.09.15
  *Description:         单元测试，模拟发包
  *History:             (修改历史记录列表，每条修改记录应包含修改日期、修改者及修改内容简介)
     1. Date:           2017.09.15
        Author:         diaoguagnqiang
        Modification:   首次生成文件
     2. Data:
        Author:
        Modification:
**********************************************************************************/

#include "my_server.h"

#include <stdio.h>

#define IP "127.0.0.1"
#define PORT (6000)

#define BACKLOG 1
#define MAXRECVLEN 1024

int test_start(int argc, char** argv);

int test_server();

int sendmsg(int);

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv)
{
    test_start(argc, argv);
    //test_server();
}

int test_server()
{
    char buf[MAXRECVLEN];
    int listenfd, connectfd;  /* socket descriptors */
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    socklen_t addrlen;
    /* Create TCP socket */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* handle exception */
        perror("socket() error. Failed to initiate a socket");
        exit(1);
    }

    /* set socket option */
    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        /* handle exception */
        perror("Bind() error.");
        exit(1);
    }

    if(listen(listenfd, BACKLOG) == -1)
    {
        perror("listen() error. \n");
        exit(1);
    }

    addrlen = sizeof(client);
    while(1){
        if((connectfd=accept(listenfd,(struct sockaddr *)&client, &addrlen))==-1)
        {
            perror("accept() error. \n");
            exit(1);
        }

        struct timeval tv;
        gettimeofday(&tv, NULL);
        printf("接收到连接 client's ip %s, port %d at time %ld.%ld\n",inet_ntoa(client.sin_addr),htons(client.sin_port), tv.tv_sec,tv.tv_usec);

        int iret=-1;
        while(1)
        {
            /*
            iret = recv(connectfd, buf, MAXRECVLEN, 0);
            if(iret>0)
            {
                printf("%s\n", buf);
            }else
            {
                close(connectfd);
                break;
            }*/

            /* print client's ip and port */
            //send(connectfd, buf, iret, 0); /* send to the client welcome message */
            sendmsg(connectfd);

            sleep(5);
        }
    }

    close(listenfd); /* close listenfd */
    return 0;
}

int sendmsg(int id){

    char data[1024] = {0};
    std::string str = "PushLabel|1#DF4D-4008#201711011425/1_DF4D-4008_2.jpg#2";

    snprintf(data, sizeof(data), "%s\r\n", str.c_str());

    int len = strlen(data);

    int ret = send(id, data, len, 0);

    printf("send: len[%d][%ld], msg[%s]\n", ret, str.length(), data);
}

int test_start(int argc, char** argv)
{
    my_server server;

    server.start();

    while(1){
        sleep(1);
    }

    printf("end\n");
}

