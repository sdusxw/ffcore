//利用Socket,网络中进程之间进行个通信
//http://blog.csdn.net/nana_93/article/details/8743525
#include "net_tcp.h"

NetTcpServer::NetTcpServer()
{
	cur_sock = -1;
}

NetTcpServer::~NetTcpServer()
{

}

//socket是“open—write/read—close”模式的一种实现
bool NetTcpServer::open_bind_listen()
{
	/*参数1:AF_INET决定了要用ipv4地址（32位的）与端口号（16位的）的组合
			参数2:SOCK_STREAM：socket类型
			参数3:指定协议。
			并不是上面的type和protocol可以随意组合的，如SOCK_STREAM不可以跟IPPROTO_UDP组合。
			当protocol为0时，会自动选择type类型对应的默认协议。
			*/
	if ((listenfd = socket( AF_INET, SOCK_STREAM, 0)) == -1)
	{
		     //调用socket创建用于监听客户端的socket
		printf("Create socket Error : %d\n", errno);
		exit( EXIT_FAILURE);
	}

	//!>
	//!> 下面是接口信息
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;/* address family: AF_INET */
	servaddr.sin_addr.s_addr = htonl( INADDR_ANY); /* sin_addr：internet address */   /*s_addr： address in network byte order */
	servaddr.sin_port = htons( SERV_PORT); /* port in network byte order */
	//!>
	//!> 绑定
	//在将一个地址绑定到socket的时候，请先将主机字节序转换成为网络字节序，而不要假定主机字节序跟网络字节序一样使用的是Big-Endian。
	int n = 1;
	/*设置socket属性 s：标识一个套接口的描述字。
	level：选项定义的层次；目前仅支持SOL_SOCKET和IPPROTO_TCP层次。
	optname：需设置的选项。
	optval：指针，指向存放选项值的缓冲区。
	optlen：optval缓冲区的长度。
	*/
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));

	//bind()函数把一个地址族中的特定地址赋给socket。
	//例如对应AF_INET、AF_INET6就是把一个ipv4或ipv6地址和端口号组合赋给socket。
	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1)
	{      //调用bind绑定地址
		printf("Bind Error : %d\n", errno);
		exit(EXIT_FAILURE);
	}

	//!>
	//!> 监听   //第二个参数为相应socket可以排队的最大连接个数
	if (listen(listenfd, MAX_BACK) == -1)
	{
		printf("Listen Error : %d\n", errno);
		exit( EXIT_FAILURE);
	}

	//!> 当前最大的感兴趣的套接字fd
	//初始化select
	maxfd = listenfd;    //!> 当前可通知的最大的fd
	maxi = -1;            //!> 仅仅是为了client数组的好处理

	for (i = 0; i < FD_SIZE; i++)    //!> 首先置为全-1
	{
		client[i] = -1;        //!> 首先client的等待队列中是没有的，所以全部置为-1
	}

	FD_ZERO(&allset);        //!> 清空，先将其置为0
	FD_SET(listenfd, &allset);//将监听socket加入select检测的描述符集合
	//!> 说明当前我对此套接字有兴趣，下次select的时候通知我！
	return true;
}

bool NetTcpServer::get_message()
{
	rset = allset;        //!> 由于allset可能每次一个循环之后都有变化，所以每次都赋值一次


	/* select (int __nfds, fd_set *__restrict __readfds,
		   fd_set *__restrict __writefds,
		   fd_set *__restrict __exceptfds,
		   struct timeval *__restrict __timeout);
	 *
	 * select:Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.*/
	if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) == -1)
	{                    //!> if 存在关注，调用select
		printf("Select Error : %d\n", errno);
		exit( EXIT_FAILURE);
	}

	if (nready <= 0)            //!> if 所有的感兴趣的没有就接着回去select
	{
		return false;
	}

	if (FD_ISSET(listenfd, &rset))            //!> if 是监听接口上的“来电”
	{            //是否有新客户端请求             //!>
		//!> printf("server listen ...\n");
		clilen = sizeof(chiaddr);

		printf("Start doing... \n");
//第二个参数为指向struct sockaddr *的指针，用于返回客户端的协议地址，第三个参数为协议地址的长度。
		if ((connfd = accept(listenfd, (struct sockaddr *) &chiaddr, &clilen))
				== -1)
		{                                        //!> accept 返回的还是套接字
			printf("Accept Error : %d\n", errno);
			return false;
		}
		 //将新客户端的加入数组
		for (i = 0; i < FD_SIZE; i++)    //!> 注意此处必须是循环，刚开始我认
										 //!> 为可以直接设置一个end_i来直接处
										 //!> 理,实质是不可以的！因为每个套接
		{                                    //!> 字的退出时间是不一样的，后面的
			if (client[i] < 0)                //!> 可能先退出，那么就乱了，所以只
			{                                //!> 有这样了！
				client[i] = connfd;            //!> 将client的请求连接保存
				cur_sock = connfd;//保存客户端描述符
				break;
			}
		}

		if (i == FD_SIZE)                //!> The last one
		{
			printf("To many ... ");
			close(connfd);            //!> if 满了那么就不连接你了，关闭吧
			return false;                    //!> 返回
		}
		//!> listen的作用就是向数组中加入套接字！
		FD_SET(connfd, &allset);    //!> 说明现在对于这个连接也是感兴趣的！ //!> 所以加入allset的阵容
			//将新socket连接放入select监听集合
		if (connfd > maxfd)            //!> 这个还是为了解决乱七八糟的数组模型的处理
		{                      //确认maxfd是最大描述符
			maxfd = connfd;
		}

		if (i > maxi)                    //!> 同上 //数组最大元素值
		{
			maxi = i;
		}
	}

	//!> 下面就是处理数据函数( 其实说本质的select还是串行 )
	for (i = 0; i <= maxi; i++)        //!> 对所有的连接请求的处理
	{
		if ((sockfd = client[i]) > 0)    //!> 还是为了不规整的数组
		{            //!> 也就说client数组不是连续的全正数或者-1，可能是锯齿状的
			//有客户连接，检测是否有数据
			if (FD_ISSET(sockfd, &rset))    //!> if 当前这个数据套接字有要读的
			{
				memset(buf, 0, sizeof(buf));    //!> 此步重要，不要有时候出错

				n = read(sockfd, buf, BUF_LEN);
				if (n < 0)
				{
					printf("Error!\n");
					close(sockfd);            //!> 说明在这个请求端口上出错了！
					FD_CLR(sockfd, &allset);
					client[i] = -1;
					cur_sock = -1;
					continue;
				}
				if (n == 0)
				{
					printf("Disconnect\n");
					cur_sock = -1;
					close(sockfd);            //!> 说明在这个请求端口上读完了！
					FD_CLR(sockfd, &allset);
					client[i] = -1;
					//exit(-1);
					continue;
				}

				string msg = buf;
				msg_buff->push(make_pair(sockfd, msg));

				if (strcmp(buf, "q") == 0)                //!> 客户端输入“q”退出标志
				{
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
					continue;
				}
			}
		}
	}
	return true;
}

NetTcpClient::NetTcpClient()
{
	connfd = -1;
}

NetTcpClient::~NetTcpClient()
{
	close( connfd );
}

bool NetTcpClient::Connect(string server_ip, int server_port)
{
	//!> 建立套接字
	if ((connfd = socket( AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Tcp Client Socket Error %d ...\n", errno);
		return false;
	}

	//!> 套接字信息
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(server_port);
	inet_pton(AF_INET, server_ip.c_str(), &servaddr.sin_addr);

	//!> 链接server
	/*
	 * connect函数的第一个参数即为客户端的socket描述字，第二参数为服务器的socket地址，第三个参数为socket地址的长度。
	 * 客户端通过调用connect函数来建立与TCP服务器的连接。
	 */
	if (connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	{
		printf("Tcp Client Connect error..\n");
		return false;
	}
	return true;
}

size_t NetTcpClient::SendData(char * pData, size_t len)
{
	return write( connfd, pData, len );
}
