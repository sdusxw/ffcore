/***************************boon_bop.cpp***************************************
			  功能：与bop通信
			  创建时间：2017-02-16
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
***************************************************************************/
#include "boon_bop.h"
extern pthread_mutex_t mongo_mutex_car;
/**************************************************bipc监听线程******************************/
void* bop_thread(void *)
{
	char time_now[64];
	char recv_buffer[4096];
	Json::Reader reader;
     	Json::Value value;
	time_t tm;
	int recv_len = 0;
	time_printf(time(&tm),time_now);   //获取当前时间

	int server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(5011);

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	int client_addr_len = sizeof(client_addr);

	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) //绑定地址
	{
		fprintf(fp_log,"%s##bop_thread bind 失败\n",time_now);
		printf("bop_thread bind error\n");
		return 0;
	}

	printf("bop_thread 启动成功 \n");
	fprintf(fp_log,"%s##bop_thread 启动成功\n",time_now);
	fflush(fp_log);

	while(1)
	{
		memset(recv_buffer, 0, sizeof(recv_buffer));
		recv_len = recvfrom(server_sock, recv_buffer, sizeof(recv_buffer), 0,(struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len); //接收数据
		time_printf(time(&tm),time_now);   //获取当前时间

		std::string cmd;
		if(!reader.parse(recv_buffer, value)) //解析json数据
		{
			fprintf(fp_log,"%s##bop_thread 解析json数据失败\n",time_now);
			continue;
		}
		fprintf(fp_log,"%s##bop_thread 收到json数据:\n",time_now);
		fprintf(fp_log,"%s\n",recv_buffer);//把收到的数据写到日志里
		cmd = value["cmd"].asString(); //解析cmd命令
	
	
		pthread_mutex_lock(&mongo_mutex_car);
		if(mongodb_flag)
		mongodb_connect();
		if(strcmp(cmd.c_str(),"get_ipc_config") == 0) //返回硬件配置信息
		{
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_get_ipc_config_bop() < 0)
				{
					fprintf(fp_log,"%s##bop_thread mongodb ipc_config失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				mysql_get_ipc_config_bop();
			}	
		}
		if(mongodb_flag)
		mongodb_exit();
		pthread_mutex_unlock(&mongo_mutex_car);
	                            
		fflush(fp_log);
	}
}
/**************************************************end******************************************/

