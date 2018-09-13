/***************************boon_bgui.cpp***************************************
			  功能：与bgui通信
			  创建时间：2017-02-17
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
***************************************************************************/
#include "bcenter_def.h"
#include "common_def.h"
#include "boon_bgui.h"

extern bool in_guoqi_flag; //入口车过期标志
extern bool out_guoqi_flag; //出口车过期标志
extern char in_plate[PLATE_LENGTH]; //最近的车牌号
extern char in_channel[256];//最近的通道id号
extern char in_time[24];//最近的进入时间
extern char out_plate[PLATE_LENGTH]; //最近的车牌号
extern char out_channel[256];//最近的通道id号
extern char out_time[24];//最近的进入时间
bool in_fleet = false; //入口车队模式标志
bool out_fleet = false; //出口车队模式标志
extern char  out_pic_path[256];
extern char out_pcolor[24];

extern pthread_mutex_t mongo_mutex_car;

// 是否加入车队模式，不在此赋值, 在main.cpp定义
extern bool g_has_cheduimoshi_;

// 日志
static char log_buf_[LOG_BUF_SIZE] = {0};
// 写日志
extern void writelog(const char* _buf);

/**************************车队模式****************************************/
int bgui_process_fleet(char *type,char *op)
{
	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	Json::Value json_send; 

	if(strcmp(type,"入口") == 0)
	{
		if(strcmp(op,"开") == 0)
		{
			json_send["cmd"] = Json::Value("open_door");
			json_send["channel_id"] = Json::Value(in_channel);
			json_send["in_out"] = Json::Value("入口");
			json_send["flag"] = Json::Value("keep");
			in_fleet = true;
		}
		else
		{
			json_send["cmd"] = Json::Value("open_door");
			json_send["channel_id"] = Json::Value(in_channel);
			json_send["in_out"] = Json::Value("入口");
			json_send["flag"] = Json::Value("once");
			in_fleet = false;
		}
	}
	else
	{
		if(strcmp(op,"开") == 0)
		{
			json_send["cmd"] = Json::Value("open_door");
			json_send["channel_id"] = Json::Value(out_channel);
			json_send["in_out"] = Json::Value("出口");
			json_send["flag"] = Json::Value("keep");
			out_fleet = true;
		}
		else
		{
			json_send["cmd"] = Json::Value("open_door");
			json_send["channel_id"] = Json::Value(out_channel);
			json_send["in_out"] = Json::Value("出口");
			json_send["flag"] = Json::Value("once");
			out_fleet = false;
		}	
	}
    
	// 如果没有车队模式
	if(false == g_has_cheduimoshi_){
		json_send["flag"] = Json::Value("once");
	}

	std::string send = json_send.toStyledString();

    char ip[] = {"127.0.0.1"};

	struct sockaddr_in addr;
	int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	
	addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BLED);
    addr.sin_addr.s_addr = inet_addr(ip);
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##bgui_process_fleet 发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 向bled发送消息失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
	}
	else
	{
		fprintf(fp_log,"%s##bgui_process_fleet 发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
	}
	close(sock);
	return 0;
}
/**************************抓拍****************************************/
int bgui_process_shot(char *type)
{

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	Json::Value json_send; 

	if(strcmp(type,"入口") == 0)
	{
		
		json_send["cmd"] = Json::Value("snap_shot");
		json_send["channel_id"] = Json::Value(in_channel);
		json_send["in_out"] = Json::Value("入口");
		json_send["flag"] = Json::Value("once");
		
	}
	else
	{
		json_send["cmd"] = Json::Value("snap_shot");
		json_send["channel_id"] = Json::Value(out_channel);
		json_send["in_out"] = Json::Value("出口");
		json_send["flag"] = Json::Value("once");
			
	}

	std::string send = json_send.toStyledString();

    char ip[] = {"127.0.0.1"};

	struct sockaddr_in addr;
    int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BLED);
    addr.sin_addr.s_addr = inet_addr(ip);
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##bgui_process_shot 发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 向bled发送消息失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
	}
	else
	{
		fprintf(fp_log,"%s##bgui_process_shot 发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
	}
	close(sock);
	return 0;
}

/**************************************************bgui监听线程******************************/
void* bgui_thread(void *)
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
	server_addr.sin_port = htons(PORT_UDP_BGUI_TO_BCENTER);

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	int client_addr_len = sizeof(client_addr);

	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) //绑定地址
	{
		fprintf(fp_log,"%s##bgui_thread bind 失败\n",time_now);
		printf("bgui_thread bind error\n");
		return 0;
	}

	printf("bgui_thread 启动成功 \n");
	fprintf(fp_log,"%s2222##bgui_thread 启动成功\n",time_now);
	fflush(fp_log);

	while(1)
	{
		memset(recv_buffer, 0, sizeof(recv_buffer));
		recv_len = recvfrom(server_sock, recv_buffer, sizeof(recv_buffer), 0,(struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len); //接收数据
		time_printf(time(&tm),time_now);   //获取当前时间

        writelog("\n**************** bgui消息 *********************");

		std::string cmd;
		if(!reader.parse(recv_buffer, value)) //解析json数据
		{
			fprintf(fp_log,"%s##bgui_thread 解析json数据失败 %s\n",time_now,recv_buffer);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 解析json包失败 [%s] line[%d]", __FUNCTION__, recv_buffer, __LINE__);
            writelog(log_buf_);
			
			continue;
		}
		fprintf(fp_log,"%s##bgui_thread 收到json数据:\n",time_now);
		fprintf(fp_log,"%s\n",recv_buffer);//把收到的数据写到日志里
		cmd = value["cmd"].asString(); //解析cmd命令

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收到一个数据包[%s] port[%d] line[%d]", __FUNCTION__, recv_buffer, PORT_UDP_BGUI_TO_BCENTER, __LINE__);
        writelog(log_buf_);
	
		pthread_mutex_lock(&mongo_mutex_car);
		if(mongodb_flag)
		mongodb_connect();
		if(strcmp(cmd.c_str(),"start") == 0) //开始
		{
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_send_bgui_start() < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mongodb mongodb_send_bgui_start失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				if(mysql_send_bgui_start() < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mysql mysql_send_bgui_start失败\n",time_now);		
				}
			}	
		}
		else if(strcmp(cmd.c_str(),"login") == 0) //登录
		{
			char name[48];
			char password[48];

			Json::Value content;
			content = value["content"];

			memset(name,0,48);
			memset(password,0,48);

			std::string c_name = content["user"].asString();
			std::string c_password = content["pwd"].asString();
			sprintf(name,"%s",c_name.c_str());
			sprintf(password,"%s",c_password.c_str());
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_bgui_login(name,password) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mongodb mongodb_bgui_login失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				if(mysql_bgui_login(name,password) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mysql mysql_bgui_login失败\n",time_now);		
				}
			}	
		}
		else if(strcmp(cmd.c_str(),"door") == 0) //闸机信号
		{
			char type[48];
			char op[48];

			Json::Value content;
			content = value["content"];

			memset(type,0,48);
			memset(op,0,48);

			std::string c_type = content["type"].asString();
			std::string c_op = content["op"].asString();
			sprintf(type,"%s",c_type.c_str());
			sprintf(op,"%s",c_op.c_str());
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_bgui_process_door(type,op) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mongodb mongodb_bgui_login失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				if(mysql_bgui_process_door(type,op) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mysql mysql_bgui_login失败\n",time_now);		
				}
			}	
		}
		else if(strcmp(cmd.c_str(),"fleet") == 0) //车队模式
		{
			char type[48];
			char op[48];

			Json::Value content;
			content = value["content"];

			memset(type,0,48);
			memset(op,0,48);

			std::string c_type = content["type"].asString();
			std::string c_op = content["op"].asString();
			sprintf(type,"%s",c_type.c_str());
			sprintf(op,"%s",c_op.c_str());
			
			if(bgui_process_fleet(type,op) < 0)
			{
				fprintf(fp_log,"%s##bgui_thread bgui_process_fleet失败\n",time_now);	
			}	
			
			
		}
		else if(strcmp(cmd.c_str(),"shot") == 0) //抓拍
		{
			char type[48];
			char op[48];

			Json::Value content;
			content = value["content"];

			memset(type,0,48);
			

			std::string c_type = content["type"].asString();
			
			sprintf(type,"%s",c_type.c_str());
			
			
			if(bgui_process_shot(type) < 0)
			{
				fprintf(fp_log,"%s##bgui_thread bgui_process_shot失败\n",time_now);	
			}	
			
			
		}
		else if(strcmp(cmd.c_str(),"changediscount") == 0) //临时车类型改变
		{
			char type[48];
            char plate[PLATE_LENGTH];

			Json::Value content;
			content = value["content"];

			memset(type,0,48);
            memset(plate,0,PLATE_LENGTH);

			std::string c_type = content["type"].asString();
			std::string c_plate = content["plate"].asString();
			sprintf(type,"%s",c_type.c_str());
			sprintf(plate,"%s",c_plate.c_str());
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_bgui_process_youhui(type,plate) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mongodb mongodb_bgui_process_youhui失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				if(mysql_bgui_process_youhui(type,plate) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mysql mysql_bgui_process_youhui失败\n",time_now);		
				}
			}		
			
			
		}
		else if(strcmp(cmd.c_str(),"chargepass") == 0) //收费放行
		{
            snprintf(log_buf_, sizeof(log_buf_), "[%s] 收到一个收费放行消息 msg[%s] line[%d]", __FUNCTION__, recv_buffer, __LINE__);
            writelog(log_buf_);

			char type[48];
            char plate[PLATE_LENGTH];

			Json::Value content;
			content = value["content"];
			
            memset(plate,0,PLATE_LENGTH);

			std::string c_plate = content["plate"].asString();
			
			sprintf(plate,"%s",c_plate.c_str());
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_bgui_charge_pass(plate) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mongodb mongodb_bgui_charge_pass失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				if(mysql_bgui_charge_pass(plate) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mysql mysql_bgui_charge_pass失败\n",time_now);		
				}
			}
			
		}
		else if(strcmp(cmd.c_str(),"freepass") == 0) //免费放行
		{
			char type[48];
            char plate[PLATE_LENGTH];

			Json::Value content;
			content = value["content"];
			
            memset(plate,0,PLATE_LENGTH);
			
			std::string c_plate = content["plate"].asString();
			
			sprintf(plate,"%s",c_plate.c_str());
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_bgui_free_pass(plate) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mongodb mongodb_bgui_free_pass失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				if(mysql_bgui_free_pass(plate) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mysql mysql_bgui_free_pass失败\n",time_now);		
				}
			}		
			
			
		}
		else if(strcmp(cmd.c_str(),"modifypasswd") == 0) //修改密码
		{
			char name[48];
			char old_pass[48];
			char new_pass[48];

			Json::Value content;
			content = value["content"];

			memset(name,0,48);
			memset(old_pass,0,48);
			memset(new_pass,0,48);

			std::string c_name = content["username"].asString();
			std::string c_old = content["oldpasswd"].asString();
			std::string c_new = content["newpasswd"].asString();
			sprintf(name,"%s",c_name.c_str());
			sprintf(old_pass,"%s",c_old.c_str());
			sprintf(new_pass,"%s",c_new.c_str());
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_bgui_modif_passwd(name,old_pass,new_pass) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mongodb mongodb_bgui_modif_passwd失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				if(mysql_bgui_modif_passwd(name,old_pass,new_pass)< 0)
				{
					fprintf(fp_log,"%s##bgui_thread mysql mysql_bgui_modif_passwd失败\n",time_now);		
				}
			}		
			
		}
		else if(strcmp(cmd.c_str(),"modifyparkcount") == 0) //修改密码
		{
			char count[48];
			

			Json::Value content;
			content = value["content"];

			memset(count,0,48);
		
			std::string c_count = content["count"].asString();
			
			sprintf(count,"%s",c_count.c_str());
		
			if(mongodb_flag) //mongo正常
			{
				if(mongodb_bgui_modif_parkcount(count) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mongodb_bgui_modif_parkcount失败\n",time_now);	
				}	
			}
			else ////mongo不正常
			{
				if(mysql_bgui_modif_parkcount(count) < 0)
				{
					fprintf(fp_log,"%s##bgui_thread mysql mysql_bgui_modif_parkcount(失败\n",time_now);		
				}
			}		
		}
		else if(strcmp(cmd.c_str(),"modifyplate") == 0) //修改出口车牌
		{
            char old_plate[PLATE_LENGTH];
            char new_plate[PLATE_LENGTH];

			Json::Value content;
			content = value["content"];

            memset(old_plate,0,PLATE_LENGTH);
            memset(new_plate,0,PLATE_LENGTH);
		
			std::string c_old = content["oldplate"].asString();
			std::string c_new = content["newplate"].asString();
			
			sprintf(old_plate,"%s",c_old.c_str());
			sprintf(new_plate,"%s",c_new.c_str());

			car_msg *c_msg;
			c_msg = new car_msg();
			memcpy(c_msg->channel_id,out_channel,strlen(out_channel));
			strcpy(c_msg->in_out,"出口");
			memcpy(c_msg->time,out_time,strlen(out_time));
			memcpy(c_msg->plate,new_plate,strlen(new_plate));
			memcpy(c_msg->path,out_pic_path,strlen(out_pic_path));
			memcpy(c_msg->pcolor,out_pcolor,strlen(out_pcolor));
			fprintf(fp_log,"%s##出口手动修改车牌号，由%s改为%s\n",time_now,old_plate,new_plate);
			BoonMsgQueue::Instance()->put_out(c_msg); //写到出口队列里

		}else if(strcmp(cmd.c_str(), C_CMD_SMQ_PAY) == 0){
			snprintf(log_buf_, sizeof(log_buf_), "[%s] 收到一个扫码枪支付消息 msg[%s] line[%d]", __FUNCTION__, recv_buffer, __LINE__);
			writelog(log_buf_);

            // 扫码枪开闸
            mongodb_bgui_smq_opendoor();

            // 扫码枪的数据入库
			bool ret = mongodb_bgui_smq_insert(value);

            std::string rsp = "";
            mongodb_get_jsondata(cmd, rsp, ret);

            char ip[] = {"127.0.0.1"};

            // 向bugi发送成功或失败消息
            mogodbb_process_udp_send(rsp, ip, PORT_UDP_BCENTER_TO_BGUI);

		}else{
			snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到合适的类型 cmd[%s] msg[%s] line[%d]", __FUNCTION__, cmd.c_str(), recv_buffer, __LINE__);
			writelog(log_buf_);
		}

		if(mongodb_flag)
		mongodb_exit();
	        pthread_mutex_unlock(&mongo_mutex_car);                    
		fflush(fp_log);
	}
}
/**************************************************end******************************************/

