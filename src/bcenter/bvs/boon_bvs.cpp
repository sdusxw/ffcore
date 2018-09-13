/***************************boon_bvs.cpp***************************************
			  功能：与bvs通信
			  创建时间：2017-02-16
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
***************************************************************************/
#include "../bcenter_def.h"
#include "common_def.h"
#include "boon_bvs.h"
extern pthread_mutex_t mongo_mutex_car;

// 日志
static char log_buf_[LOG_BUF_SIZE] = {0};
// 记录日志
extern void writelog(const char* _buf);

/**************************************************bvs监听线程******************************/
void* bvs_thread(void *)
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
	server_addr.sin_port = htons(PORT_UDP_BVS_TO_BCENTER);

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	int client_addr_len = sizeof(client_addr);

	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) //绑定地址
	{
		fprintf(fp_log,"%s##bvs_thread bind 失败\n",time_now);
		printf("bvs_thread bind error\n");
		return 0;
	}

	printf("bvs_thread 启动成功 \n");
	fprintf(fp_log,"%s##bvs_thread 启动成功\n",time_now);
	fflush(fp_log);

	while(1)
	{
		memset(recv_buffer, 0, sizeof(recv_buffer));
		recv_len = recvfrom(server_sock, recv_buffer, sizeof(recv_buffer), 0,(struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len); //接收数据
		time_printf(time(&tm),time_now);   //获取当前时间

		writelog("\n**************** bvs线程 *********************");

		std::string cmd;
		if(!reader.parse(recv_buffer, value)) //解析json数据
		{
			fprintf(fp_log,"%s##bvs_thread 解析json数据失败\n",time_now);
			continue;
		}
		fprintf(fp_log,"%s##bvs_thread 收到json数据:\n",time_now);
		fprintf(fp_log,"%s\n",recv_buffer);//把收到的数据写到日志里
		cmd = value["cmd"].asString(); //解析cmd命令

		snprintf(log_buf_, sizeof(log_buf_), "[%s] 收到一个数据包[%s] port[%d] line[%d]", __FUNCTION__, recv_buffer, PORT_UDP_BVS_TO_BCENTER, __LINE__);
		writelog(log_buf_);
	
		pthread_mutex_lock(&mongo_mutex_car);
		if(mongodb_flag)
		mongodb_connect();
		if(strcmp(cmd.c_str(),"get_ipc_config") == 0) //返回硬件配置信息
		{
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_get_ipc_config_bvs() < 0)
				{
					fprintf(fp_log,"%s##bvs_thread mongodb ipc_config失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				mysql_get_ipc_config_bvs();
			}	
		}
		if(mongodb_flag)
		mongodb_exit();
		pthread_mutex_unlock(&mongo_mutex_car);
	                            
		fflush(fp_log);
	}
}
/**************************************************end******************************************/

