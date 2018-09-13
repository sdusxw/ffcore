/***************************boon_bipc.cpp***************************************
			  功能：与bipc通信
			  创建时间：2017-02-05
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
***************************************************************************/
#include "../bcenter_def.h"
#include "common_def.h"
#include "boon_bipc.h"
extern pthread_mutex_t mongo_mutex_car;

// 日志
static char log_buf_[LOG_BUF_SIZE] = {0};
// 记录日志
extern void writelog(const char* _buf);

/**************************************************bipc监听线程******************************/
void* bipc_thread(void *)
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
	server_addr.sin_port = htons(PORT_UDP_BIPC_TO_BCENTER); // 由5001改为5022

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	int client_addr_len = sizeof(client_addr);

	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) //绑定地址
	{
		fprintf(fp_log,"%s##bipc_thread bind 失败\n",time_now);
		printf("bipc_thread bind error\n");
		return 0;
	}

	printf("bipc_thread 启动成功 \n");
	fprintf(fp_log,"%s##bipc_thread 启动成功\n",time_now);
	fflush(fp_log);

	while(1)
	{
		usleep(200);
		memset(recv_buffer, 0, sizeof(recv_buffer));
		recv_len = recvfrom(server_sock, recv_buffer, sizeof(recv_buffer), 0,(struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len); //接收数据
		time_printf(time(&tm),time_now);   //获取当前时间

		writelog("\n**************** bipc线程 *********************");

		std::string cmd;
		if(!reader.parse(recv_buffer, value)) //解析json数据
		{
			snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 解析json包失败 [%s] line[%d]", __FUNCTION__, recv_buffer, __LINE__);
			writelog(log_buf_);

			fprintf(fp_log,"%s##bipc_thread 解析json数据失败\n",time_now);
			continue;
		}

		snprintf(log_buf_, sizeof(log_buf_), "[%s] 收到一个数据包[%s] port[%d] line[%d]", __FUNCTION__, recv_buffer, PORT_UDP_BIPC_TO_BCENTER, __LINE__);
		writelog(log_buf_);

		fprintf(fp_log,"%s##bipc_thread 收到json数据:\n",time_now);
		fprintf(fp_log,"%s\n",recv_buffer);//把收到的数据写到日志里
		cmd = value["cmd"].asString(); //解析cmd命令

		pthread_mutex_lock(&mongo_mutex_car);
		if(mongodb_flag)
		mongodb_connect();
		if(strcmp(cmd.c_str(),"get_ipc_config") == 0) //返回硬件配置信息
		{
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_get_ipc_config(PORT_UDP_BCENTER_TO_BIPC) < 0)
				{
					fprintf(fp_log,"%s##bipc_thread mongodb ipc_config失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				mysql_get_ipc_config();
			}	
		}
		else if(strcmp(cmd.c_str(),"car_pass") == 0) //收到过车信息
		{
			car_pass *c_pass;
			c_pass = new car_pass();

			std::string channel_id;
			std::string in_out;
			std::string flag;
			
			channel_id = value["channel_id"].asString();
			in_out = value["in_out"].asString();
			flag = value["flag"].asString();

			memcpy(c_pass->channel_id,channel_id.c_str(),channel_id.length()); //通道号
			memcpy(c_pass->in_out,in_out.c_str(),in_out.length()); //进出口类型
			memcpy(c_pass->flag,flag.c_str(),flag.length());
	
			BoonMsgQueue::Instance()->put_carpass(c_pass); //写到队列里
		}
		else if(strcmp(cmd.c_str(),"car_come") == 0)
		{
			std::string channel_inout;
			std::string c_time;
			std::string c_channel_id;

			car_msg *c_msg;
			c_msg = new car_msg();
			channel_inout = value["in_out"].asString();
			c_time = value["time"].asString();
			c_channel_id = value["channel_id"].asString();

			memcpy(c_msg->channel_id,c_channel_id.c_str(),c_channel_id.length());
			memcpy(c_msg->in_out,channel_inout.c_str(),channel_inout.length());
			memcpy(c_msg->time,c_time.c_str(),c_time.length());
			Json::Value ipc;
			ipc = value["car_info"];
			int num = ipc.size();

			for( unsigned int j = 0;j < num;j++)
			{
				Json::Value tmp = ipc[j];

				std::string c_ipc_ip;
				std::string c_plate;
				std::string c_pcolor;
				std::string c_brand;
				std::string c_type;
				std::string c_color;
				std::string c_path;

				c_ipc_ip = tmp["ipc_ip"].asString();
				Json::Value vehicle = tmp["vehicle"];
				c_plate = vehicle["plate"].asString();
				c_pcolor = vehicle["pcolor"].asString();
				c_brand = vehicle["brand"].asString();
				c_type = vehicle["type"].asString();
				c_color = vehicle["color"].asString();
				c_path = vehicle["path"].asString();

				if(j == 0)
				{
					memcpy(c_msg->ipc_ip,c_ipc_ip.c_str(),c_ipc_ip.length());
					memcpy(c_msg->plate,c_plate.c_str(),c_plate.length());
					memcpy(c_msg->brand,c_brand.c_str(),c_brand.length());
					memcpy(c_msg->pcolor,c_pcolor.c_str(),c_pcolor.length());
					memcpy(c_msg->type,c_type.c_str(),c_type.length());
					memcpy(c_msg->color,c_color.c_str(),c_color.length());	
					memcpy(c_msg->path,c_path.c_str(),c_path.length());		
				}
				else
				{
					memcpy(c_msg->ipc_ip1,c_ipc_ip.c_str(),c_ipc_ip.length());
					memcpy(c_msg->plate1,c_plate.c_str(),c_plate.length());
					memcpy(c_msg->brand1,c_brand.c_str(),c_brand.length());
					memcpy(c_msg->pcolor1,c_pcolor.c_str(),c_pcolor.length());
					memcpy(c_msg->type1,c_type.c_str(),c_type.length());
					memcpy(c_msg->color1,c_color.c_str(),c_color.length());	
					memcpy(c_msg->path1,c_path.c_str(),c_path.length());
				}
			}

			c_msg->num = num;
			if(strcmp(channel_inout.c_str(),"入口") == 0)
			{
				fprintf(fp_log,"%s##写信息到入口队列\n",time_now);	
				BoonMsgQueue::Instance()->put_in(c_msg); //写到入口队列里	
			}
			else
			{
				fprintf(fp_log,"%s##写信息到出口队列\n",time_now);
				BoonMsgQueue::Instance()->put_out(c_msg); //写到出口队列里
			}
			
		}
		else if( 0 == cmd.compare("get_white_list"))
		{
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_get_white_list(PORT_UDP_BCENTER_TO_BIPC) < 0)
				{
					fprintf(fp_log,"%s##bipc_thread mongodb get_white_list失败\n",time_now);
				}
			}
			else ////mongo不正常
			{
				;
			}
		}else{
			snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 无法识别的消息类型 cmd[%s] line[%d]", __FUNCTION__, cmd.c_str(), __LINE__);
			writelog(log_buf_);
		}
		if(mongodb_flag)
		mongodb_exit();
	        pthread_mutex_unlock(&mongo_mutex_car);                   
		fflush(fp_log);
	}
}
/**************************************************end******************************************/

