#ifndef NET_TCP_H_
#define NET_TCP_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <queue>

#include "concurrent_queue.h"

using namespace std;

#define BUF_LEN 1024   //缓冲区大小
#define SERV_PORT 6000 //通讯端口
#define FD_SIZE 100    //FD_SIZE为select函数支持的最大描述符个数
#define MAX_BACK 100   //listen队列中等待的连接数

class NetTcpServer
{
public:
	int listenfd, connfd, sockfd, maxfd, maxi, i;  //socket文件描述符
	int cur_sock;
	int nready, client[FD_SIZE];        //!> 接收select返回值、保存客户端套接字。客户端私有数据指针
	int lens;
	ssize_t n;                //!> read字节数
	fd_set rset, allset;    //!> 不要理解成就只能保存一个，其实fd_set有点像封装的数组；//select所需的文件描述符集合
	char buf[BUF_LEN];
	socklen_t clilen;
	struct sockaddr_in servaddr, chiaddr;//服务器地址信息结构体

	//queue<std::pair<int, string> > *msg_buff;
	concurrent_queue<std::pair<int, string> > *msg_buff;

public:
	NetTcpServer();
	~NetTcpServer();
	bool open_bind_listen();

	bool get_message();

	bool send_message(string msg);
};

class NetTcpClient
{
public:
	NetTcpClient();
	~NetTcpClient();

	bool Connect(string server_ip, int server_port);
	size_t SendData(char * pData, size_t len);

public:
	int connfd;
	struct sockaddr_in servaddr;//服务器地址信息结构体
};

#endif
