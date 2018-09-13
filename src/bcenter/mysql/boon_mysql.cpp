/***************************mysql.cpp***************************************
			  功能：mysql数据库增删改查
			  创建时间：2017-02-04
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
***************************************************************************/
#include "../bcenter_def.h"
#include "boon_mysql.h"

MYSQL mysql_con;   //mysql连接变量
MYSQL mysql_con_bipc;   //mysql连接变量 bipc线程用
MYSQL mysql_con_in;   //mysql连接变量 入口线程用
MYSQL mysql_con_out;   //mysql连接变量 出口线程用
MYSQL mysql_con_bgui;   //mysql连接变量 bguis线程用
MYSQL mysql_con_chewei;   //mysql连接变量 车位线程用
MYSQL mysql_con_bled;   //mysql连接变量 车位线程用

extern bool in_guoqi_flag; //入口车过期标志
extern bool out_guoqi_flag; //出口车过期标志
extern char in_plate[PLATE_LENGTH]; //最近的车牌号
extern char in_channel[256];//最近的通道id号
extern char in_time[24];//最近的进入时间
extern char out_plate[PLATE_LENGTH]; //最近的车牌号
extern char out_channel[256];//最近的通道id号
extern char out_time[24];//最近的进入时间
extern bool in_fleet; //入口车队模式标志
 extern bool out_fleet; //出口车队模式标志
extern int fmoney; //收费金额
extern char  in_led_ip[24];
extern  char out_led_ip[24];
/**************************************mysql连接**************************/
int mysql_connect()
{
	mysql_init(&mysql_con);
	mysql_init(&mysql_con_bipc);
	mysql_init(&mysql_con_in);
	if(!mysql_real_connect(&mysql_con,"localhost","root","a789654","boonpark",0,NULL,0))
	{
		return -1;
	}
	
	if(!mysql_real_connect(&mysql_con_in,"localhost","root","a789654","boonpark",0,NULL,0))
	{
		return -1;
	}
	
	if(!mysql_real_connect(&mysql_con_out,"localhost","root","a789654","boonpark",0,NULL,0))
	{
		return -1;
	}

	if(!mysql_real_connect(&mysql_con_bgui,"localhost","root","a789654","boonpark",0,NULL,0))
	{
		return -1;
	}

	if(!mysql_real_connect(&mysql_con_chewei,"localhost","root","a789654","boonpark",0,NULL,0))
	{
		return -1;
	}

	if(!mysql_real_connect(&mysql_con_bled,"localhost","root","a789654","boonpark",0,NULL,0))
	{
		return -1;
	}

	if(!mysql_real_connect(&mysql_con_bipc,"localhost","root","a789654","boonpark",0,NULL,0))
	{
		return -1;
	}
	else
	{
		mysql_query(&mysql_con, "SET NAMES UTF8");
		mysql_query(&mysql_con_in, "SET NAMES UTF8");
		mysql_query(&mysql_con_bipc, "SET NAMES UTF8");
		mysql_query(&mysql_con_out, "SET NAMES UTF8");
		mysql_query(&mysql_con_bgui, "SET NAMES UTF8");
		mysql_query(&mysql_con_bled, "SET NAMES UTF8");
		return 0;
	}
}
/**************************************end*******************************/


/******************************查询host表，得到主机IP和服务器ip**************/
int mysql_query_host(char * host_ip,char * host_server_ip)
{
	int rtn;
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	sprintf(sqlcmd,"select host_ip,host_server_ip from host"); //sql语句

	rtn = mysql_real_query(&mysql_con, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		mysql_close(&mysql_con);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con);
	if(!mysql_res)
	{
		mysql_close(&mysql_con);
		return -1;
	}
	while((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 	{
		for (int t = 0; t < mysql_num_fields(mysql_res); t++) 
		{
			if(t == 0)
			{
				memcpy(host_ip,mysql_row[t],strlen(mysql_row[t])); //每条记录里的第一个字段
			}
			else
			{
				memcpy(host_server_ip,mysql_row[t],strlen(mysql_row[t])); //每条记录里的第二个字段
			}
		}
		
	}
	mysql_free_result(mysql_res);
	mysql_close(&mysql_con); 
	return 0;
}
/**************************************end*******************************/
/******************************给bipc发送ipc_config信息********************/
int mysql_get_ipc_config()
{
	int rtn;
	char sqlcmd[200];
	MYSQL_RES *mysql_res_channel;
	MYSQL_ROW mysql_row_channel;
	MYSQL_RES *mysql_res_device;
	MYSQL_ROW mysql_row_device;
	sprintf(sqlcmd, "select channel_id,channel_in_out from channel where channel_ip='%s'",host_ip); //sql语句

	char channel_id[256];
	char channel_in_out[16];
	char device_type[32];
	char device_ip_id[32];
	char device_username[32];
	char device_password[32];

	Json::Value json_ipc_config; 
	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);

	rtn = mysql_real_query(&mysql_con_bipc, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##bipc ipc_config mysql channel 查询失败\n",time_now);
		return -1;
	}
	
	
	mysql_res_channel = mysql_store_result(&mysql_con_bipc);
	if(!mysql_res_channel)
	{
		fprintf(fp_log,"%s##bipc ipc_config mysql channel 查询失败\n",time_now);
		return -1;
	}


	while(mysql_row_channel = mysql_fetch_row(mysql_res_channel)) //取每条记录
 	{
		Json::Value json_channel; 

		memset(channel_id,0,256);
		memset(channel_in_out,0,16);
		memcpy(channel_id,mysql_row_channel[0],strlen(mysql_row_channel[0]));
		memcpy(channel_in_out,mysql_row_channel[1],strlen(mysql_row_channel[1]));
		

		json_channel["channel_id"] = Json::Value(channel_id);  
		json_channel["in_out"] = Json::Value(channel_in_out);
		if(strcmp(channel_in_out,"入口") == 0)
		{
			memset(in_channel,0,256);
			memcpy(in_channel,channel_id,strlen(channel_id));
		}
		else
		{
			memset(out_channel,0,256);
			memcpy(out_channel,channel_id,strlen(channel_id));
		}
		
		sprintf(sqlcmd, "select * from device where device_channel_id='%s'",channel_id); //sql语句
		rtn = mysql_real_query(&mysql_con_bipc, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##bipc ipc_config mysql device 查询失败\n",time_now);
			return -1;
		}

		mysql_res_device = mysql_store_result(&mysql_con_bipc);
		if(!mysql_res_device)
		{
			fprintf(fp_log,"%s##bipc ipc_config mysql device 查询失败\n",time_now);
			return -1;
		}
		while((mysql_row_channel = mysql_fetch_row(mysql_res_device))) //取每条记录
 		{
			for (int t = 0; t < mysql_num_fields(mysql_res_device); t++)
			{
				if(t == 1)
				{
					memset(device_type,0,32);
					memcpy(device_type,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
				}
			} 
			if(strcmp(device_type,"中维抓拍相机") == 0  || strcmp(device_type,"中维智能相机") == 0)
			{
				for (int t = 0; t < mysql_num_fields(mysql_res_device); t++)
				{
					if(t == 4)
					{
						memset(device_ip_id,0,32);
						memcpy(device_ip_id,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}
					if(t == 5)
					{
						memset(device_username,0,32);
						memcpy(device_username,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}
					if(t == 6)
					{
						memset(device_password,0,32);
						memcpy(device_password,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}
				} 
				
				Json::Value json_tmp; 
				json_tmp["device_type"] = Json::Value(device_type);
				json_tmp["device_ip_id"] = Json::Value(device_ip_id);
				json_tmp["device_username"] = Json::Value(device_username);
				json_tmp["device_password"] = Json::Value(device_password);

				json_channel["ipc"].append(json_tmp);
							
			}
			else if(strcmp(device_type,"停车场控制器") == 0 || strcmp(device_type,"LED语音一体机") == 0)	
			{
				for (int t = 0; t < mysql_num_fields(mysql_res_device); t++)
				{
					if(t == 4)
					{
						memset(device_ip_id,0,32);
						memcpy(device_ip_id,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}	
				}
			
				Json::Value json_tmp; 
				json_tmp["device_type"] = Json::Value(device_type);
				json_tmp["device_ip_id"] = Json::Value(device_ip_id);
		
				json_channel["ipc"].append(json_tmp);
			}	
		}
		mysql_free_result(mysql_res_device);
		json_ipc_config["channel"].append(json_channel);
		
	}
	mysql_free_result(mysql_res_channel);
	
	json_ipc_config["cmd"] = Json::Value("ipc_config");
	std::string send = json_ipc_config.toStyledString();
	struct sockaddr_in addr;
    	int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    	addr.sin_port = htons(5002);
    	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr)); //给bipc发送
	if(n < 0)
	{
		fprintf(fp_log,"%s##bipc ipc_config发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##bipc ipc_config发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);
	return 0;

	
}
/**************************************end*******************************/
/******************************给bop发送ipc_config信息********************/
int mysql_get_ipc_config_bop()
{
	int rtn;
	char sqlcmd[200];
	MYSQL_RES *mysql_res_channel;
	MYSQL_ROW mysql_row_channel;
	MYSQL_RES *mysql_res_device;
	MYSQL_ROW mysql_row_device;
	sprintf(sqlcmd, "select channel_id,channel_in_out from channel where channel_ip='%s'",host_ip); //sql语句

	char channel_id[256];
	char channel_in_out[16];
	char device_type[32];
	char device_ip_id[32];
	char device_username[32];
	char device_password[32];

	Json::Value json_ipc_config; 
	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);

	rtn = mysql_real_query(&mysql_con_bipc, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##bipc ipc_config mysql channel 查询失败\n",time_now);
		return -1;
	}
	
	
	mysql_res_channel = mysql_store_result(&mysql_con_bipc);
	if(!mysql_res_channel)
	{
		fprintf(fp_log,"%s##bipc ipc_config mysql channel 查询失败\n",time_now);
		return -1;
	}


	while(mysql_row_channel = mysql_fetch_row(mysql_res_channel)) //取每条记录
 	{
		Json::Value json_channel; 

		memset(channel_id,0,256);
		memset(channel_in_out,0,16);
		memcpy(channel_id,mysql_row_channel[0],strlen(mysql_row_channel[0]));
		memcpy(channel_in_out,mysql_row_channel[1],strlen(mysql_row_channel[1]));
		

		json_channel["channel_id"] = Json::Value(channel_id);  
		json_channel["in_out"] = Json::Value(channel_in_out);

		
		sprintf(sqlcmd, "select * from device where device_channel_id='%s'",channel_id); //sql语句
		rtn = mysql_real_query(&mysql_con_bipc, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##bipc ipc_config mysql device 查询失败\n",time_now);
			return -1;
		}

		mysql_res_device = mysql_store_result(&mysql_con_bipc);
		if(!mysql_res_device)
		{
			fprintf(fp_log,"%s##bipc ipc_config mysql device 查询失败\n",time_now);
			return -1;
		}
		while((mysql_row_channel = mysql_fetch_row(mysql_res_device))) //取每条记录
 		{
			for (int t = 0; t < mysql_num_fields(mysql_res_device); t++)
			{
				if(t == 1)
				{
					memset(device_type,0,32);
					memcpy(device_type,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
				}
			} 
			if(strcmp(device_type,"中维抓拍相机") == 0  || strcmp(device_type,"中维智能相机") == 0)
			{
				for (int t = 0; t < mysql_num_fields(mysql_res_device); t++)
				{
					if(t == 4)
					{
						memset(device_ip_id,0,32);
						memcpy(device_ip_id,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}
					if(t == 5)
					{
						memset(device_username,0,32);
						memcpy(device_username,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}
					if(t == 6)
					{
						memset(device_password,0,32);
						memcpy(device_password,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}
				} 
				
				Json::Value json_tmp; 
				json_tmp["device_type"] = Json::Value(device_type);
				json_tmp["device_ip_id"] = Json::Value(device_ip_id);
				json_tmp["device_username"] = Json::Value(device_username);
				json_tmp["device_password"] = Json::Value(device_password);

				json_channel["ipc"].append(json_tmp);
							
			}
			else if(strcmp(device_type,"停车场控制器") == 0 || strcmp(device_type,"LED语音一体机") == 0)	
			{
				for (int t = 0; t < mysql_num_fields(mysql_res_device); t++)
				{
					if(t == 4)
					{
						memset(device_ip_id,0,32);
						memcpy(device_ip_id,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}	
				}
			
				Json::Value json_tmp; 
				json_tmp["device_type"] = Json::Value(device_type);
				json_tmp["device_ip_id"] = Json::Value(device_ip_id);
		
				json_channel["ipc"].append(json_tmp);
			}	
		}
		mysql_free_result(mysql_res_device);
		json_ipc_config["channel"].append(json_channel);
		
	}
	mysql_free_result(mysql_res_channel);
	
	json_ipc_config["cmd"] = Json::Value("ipc_config");
	json_ipc_config["server_ip"] = Json::Value(host_server_ip);
	json_ipc_config["local_ip"] = Json::Value(host_ip);
	std::string send = json_ipc_config.toStyledString();
	struct sockaddr_in addr;
    int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BOP);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr)); //给bipc发送
	if(n < 0)
	{
		fprintf(fp_log,"%s##bop ipc_config发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##bop ipc_config发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);
	return 0;

	
}
/**************************************end*******************************/
/******************************给bvs发送ipc_config信息********************/
int mysql_get_ipc_config_bvs()
{
	int rtn;
	char sqlcmd[200];
	MYSQL_RES *mysql_res_channel;
	MYSQL_ROW mysql_row_channel;
	MYSQL_RES *mysql_res_device;
	MYSQL_ROW mysql_row_device;
	sprintf(sqlcmd, "select channel_id,channel_in_out from channel where channel_ip='%s'",host_ip); //sql语句

	char channel_id[256];
	char channel_in_out[16];
	char device_type[32];
	char device_ip_id[32];
	char device_username[32];
	char device_password[32];

	Json::Value json_ipc_config; 
	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);

	rtn = mysql_real_query(&mysql_con_bipc, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##bipc ipc_config mysql channel 查询失败\n",time_now);
		return -1;
	}
	
	
	mysql_res_channel = mysql_store_result(&mysql_con_bipc);
	if(!mysql_res_channel)
	{
		fprintf(fp_log,"%s##bipc ipc_config mysql channel 查询失败\n",time_now);
		return -1;
	}


	while(mysql_row_channel = mysql_fetch_row(mysql_res_channel)) //取每条记录
 	{
		Json::Value json_channel; 

		memset(channel_id,0,256);
		memset(channel_in_out,0,16);
		memcpy(channel_id,mysql_row_channel[0],strlen(mysql_row_channel[0]));
		memcpy(channel_in_out,mysql_row_channel[1],strlen(mysql_row_channel[1]));
		

		json_channel["channel_id"] = Json::Value(channel_id);  
		json_channel["in_out"] = Json::Value(channel_in_out);

		
		sprintf(sqlcmd, "select * from device where device_channel_id='%s'",channel_id); //sql语句
		rtn = mysql_real_query(&mysql_con_bipc, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##bipc ipc_config mysql device 查询失败\n",time_now);
			return -1;
		}

		mysql_res_device = mysql_store_result(&mysql_con_bipc);
		if(!mysql_res_device)
		{
			fprintf(fp_log,"%s##bipc ipc_config mysql device 查询失败\n",time_now);
			return -1;
		}
		while((mysql_row_channel = mysql_fetch_row(mysql_res_device))) //取每条记录
 		{
			for (int t = 0; t < mysql_num_fields(mysql_res_device); t++)
			{
				if(t == 1)
				{
					memset(device_type,0,32);
					memcpy(device_type,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
				}
			} 
			if(strcmp(device_type,"中维抓拍相机") == 0  || strcmp(device_type,"中维智能相机") == 0)
			{
				for (int t = 0; t < mysql_num_fields(mysql_res_device); t++)
				{
					if(t == 4)
					{
						memset(device_ip_id,0,32);
						memcpy(device_ip_id,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}
					if(t == 5)
					{
						memset(device_username,0,32);
						memcpy(device_username,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}
					if(t == 6)
					{
						memset(device_password,0,32);
						memcpy(device_password,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}
				} 
				
				Json::Value json_tmp; 
				json_tmp["device_type"] = Json::Value(device_type);
				json_tmp["device_ip_id"] = Json::Value(device_ip_id);
				json_tmp["device_username"] = Json::Value(device_username);
				json_tmp["device_password"] = Json::Value(device_password);

				json_channel["ipc"].append(json_tmp);
							
			}
			else if(strcmp(device_type,"停车场控制器") == 0 || strcmp(device_type,"LED语音一体机") == 0)	
			{
				for (int t = 0; t < mysql_num_fields(mysql_res_device); t++)
				{
					if(t == 4)
					{
						memset(device_ip_id,0,32);
						memcpy(device_ip_id,mysql_row_channel[t],strlen(mysql_row_channel[t])); //每条记录里的第一个字段	
					}	
				}
			
				Json::Value json_tmp; 
				json_tmp["device_type"] = Json::Value(device_type);
				json_tmp["device_ip_id"] = Json::Value(device_ip_id);
		
				json_channel["ipc"].append(json_tmp);
			}	
		}
		mysql_free_result(mysql_res_device);
		json_ipc_config["channel"].append(json_channel);
		
	}
	mysql_free_result(mysql_res_channel);
	
	json_ipc_config["cmd"] = Json::Value("ipc_config");
	std::string send = json_ipc_config.toStyledString();
	struct sockaddr_in addr;
    int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BVS);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr)); //给bipc发送
	if(n < 0)
	{
		fprintf(fp_log,"%s##bvs ipc_config发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##bvs ipc_config发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);
	return 0;

	
}
/**************************************end*******************************/
/******************************入口线程查询这辆车在car表里的个数*******************************************/
int mysql_query_car(char *flag,char * plate)
{
	int count = 0;
	int rtn;
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);
	sprintf(sqlcmd, "select count(*) from car where car_plate_id='%s'",plate); //sql语句
	
	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_query_car %s 失败\n",time_now,flag);
		return -1;
	}

	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_query_car %s 失败\n",time_now,flag);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 	{
		count = atoi(mysql_row[0]);
	}
	mysql_free_result(mysql_res);
	return count;
}
/**************************************************end******************************************/
/******************************入口线程查询通道属性*******************************************/
int mysql_query_channel(char *flag,char * id,char *auth_type,char * open_door,char * one_way,char * mohu_match,char *park_id)
{
	char res[16];
	int rtn;
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);
	sprintf(sqlcmd, "select channel_auth_type,channel_open_door,channel_one_way,channel_fuzzy_match,channel_park_id from channel where channel_id='%s'",id); //sql语句
	
	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_query_channel %s 失败\n",time_now,flag);
		return -1;
	}

	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_query_channel %s 失败\n",time_now,flag);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 	{
		memset(auth_type,0,24);
		memcpy(auth_type,mysql_row[0],strlen(mysql_row[0])); //得到授权类型
		memset(open_door,0,24);
		memcpy(open_door,mysql_row[1],strlen(mysql_row[1])); //得到开闸类型
		memset(one_way,0,24);
		memcpy(one_way,mysql_row[2],strlen(mysql_row[2])); //得到同口进出属性
		memset(mohu_match,0,24);
		memcpy(mohu_match,mysql_row[3],strlen(mysql_row[3])); //得到模糊匹配属性
		memset(park_id,0,256);
		memcpy(park_id,mysql_row[4],strlen(mysql_row[4])); //得到车场id号
	}
	mysql_free_result(mysql_res);
	
	return 0;
	
	
	
}
/**************************************************end******************************************/
int mysql_query_carinpark(char *flag,char *channel_id,char * plate,char *in_time,char *in_pic_path,char *in_channel_id,char *people_name,char *in_channel_name)
{
	char park_id[256];
	int rtn;
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	char time_now[64];
	time_t tm;
	int count = 0;
	time_printf(time(&tm),time_now);
	sprintf(sqlcmd, "select channel_park_id from channel where channel_id='%s'",channel_id); //sql语句
	
	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_count_carinpark %s 失败\n",time_now,flag);
		return -1;
	}

	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_count_carinpark %s 失败\n",time_now,flag);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 	{
		memset(park_id,0,256);
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0])); //得到授权类型
		
	}
	mysql_free_result(mysql_res);


	sprintf(sqlcmd, "select carinpark_in_time,carinpark_pic_path,carinpark_channel_id,carinpark_people_name from carinpark where carinpark_park_id='%s' and carinpark_plate_id='%s'",park_id,plate); //sql语句
	
	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_count_carinpark %s 失败\n",time_now,flag);
		return -1;
	}

	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_count_carinpark %s 失败\n",time_now,flag);
		return -1;
	}
	memset(in_time,0,24);
	memset(in_pic_path,0,128);
	memset(in_channel_id,0,256);
	memset(people_name,0,24);
	while((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 	{
		
		memcpy(in_time,mysql_row[0],strlen(mysql_row[0])); //得到授权类型
		
		memcpy(in_pic_path,mysql_row[1],strlen(mysql_row[1])); //得到授权类型
		
		memcpy(in_channel_id,mysql_row[2],strlen(mysql_row[2])); //得到授权类型
		
		memcpy(people_name,mysql_row[3],strlen(mysql_row[3])); //得到授权类型
		
	}
	mysql_free_result(mysql_res);


	sprintf(sqlcmd, "select channel_name from channel where channel_id='%s'",in_channel_id); //sql语句
	
	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_count_carinpark %s 失败\n",time_now,flag);
		return -1;
	}

	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_count_carinpark %s 失败\n",time_now,flag);
		return -1;
	}
	memset(in_channel_name,0,256);
	while((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 	{
		
		memcpy(in_channel_name,mysql_row[0],strlen(mysql_row[0])); 
	}
	mysql_free_result(mysql_res);

	sprintf(sqlcmd, "select count(*) from carinpark where carinpark_plate_id='%s' and carinpark_park_id='%s' ",plate,park_id); 
	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}

	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_query_car %s 失败\n",time_now,flag);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		count = atoi(mysql_row[0]);
	}
	mysql_free_result(mysql_res);

	
	return count;

	
}
/******************************入口线程mysql的模糊匹配*******************************************/
int mysql_mohu_match(char *flag,char *pcolor,char *inplate,char *outplate)
{
	char res[16];
	int rtn;
	int count = 0;
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;

	char compare[24];
	std::string plate(inplate);

	sprintf(sqlcmd, "select car_plate_id from car"); //sql语句

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);
 
	fprintf(fp_log,"%s##模糊匹配收到的pcolor为%s,车牌号为%s\n",time_now,pcolor,inplate);
	if(strcmp("blue",pcolor) == 0 || strcmp("yellow",pcolor) == 0) //车牌为蓝牌或黄牌
	{

		memset(compare,0,24);
		sprintf(compare,"*%s",plate.substr(2,6).c_str()); //匹配条件为*AB925E
		fprintf(fp_log,"%s##模糊匹配 汉字匹配为%s\n",time_now,compare);
		if(strcmp("入口",flag) == 0)
		{
			rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd)); //查询car表
		}
		else
		{
			rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd)); //查询car表
		}
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_mohu_match %s 失败\n",time_now,flag);
			return -1;
		}
		if(strcmp("入口",flag) == 0)
		{
			mysql_res = mysql_store_result(&mysql_con_in);
		}
		else
		{
			mysql_res = mysql_store_result(&mysql_con_out);
		}
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_mohu_match %s 失败\n",time_now,flag);
			return -1;
		}
		while((mysql_row = mysql_fetch_row(mysql_res))) //得到car表里的每一个车牌号
 		{
			memset(res,0,16);
			memcpy(res,mysql_row[0],strlen(mysql_row[0]));
			if(!fnmatch(compare,res,FNM_PATHNAME|FNM_PERIOD)) //匹配到则返回
			{
				mysql_free_result(mysql_res);
                memcpy(outplate,res,PLATE_LENGTH);
				return 0;
			}
		}
		mysql_free_result(mysql_res);


		for(int i = 0;i<6;i++)
		{
			int seg = i + 3;
			std::string tmp1=plate.substr(seg,1);
			int val_asc = (int) tmp1.c_str()[0];
			fprintf(fp_log,"%s##模糊匹配 匹配的字符为%s\n",time_now,tmp1.c_str());
			if(strlen(tmp1.c_str()) < 1)
			break;
			if(val_asc < 60)  //数字
			{
				val_asc = val_asc - 48;
				for(int j = 0;j++;j<10)
				{
					if(number[val_asc][j] == '@') //遇到数组里的'@'，则跳出循环
					break;
					
					
                    char plate_query[PLATE_LENGTH];
                    memset(plate_query,0,PLATE_LENGTH);
					sprintf(plate_query,"%s%c%s",plate.substr(0,seg).c_str(),number[val_asc][j],plate.substr(seg + 1,plate.length() - seg - 1).c_str());  //组成查询的车牌号
					fprintf(fp_log,"%s##模糊匹配 把%s变为%s到数据库查询\n",time_now,inplate,plate_query);
					sprintf(sqlcmd, "select count(*) from car where car_plate_id='%s'",plate_query); //sql语句
					if(strcmp("入口",flag) == 0)
					{
						rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
					}
					else
					{
						rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
					}
					if(rtn)
					{
						fprintf(fp_log,"%s##mysql_mohu_match %s 失败\n",time_now,flag);
						return -1;
					}
					if(strcmp("入口",flag) == 0)
					{
						mysql_res = mysql_store_result(&mysql_con_in);
					}
					else
					{
						mysql_res = mysql_store_result(&mysql_con_out);
					}
					if(!mysql_res)
					{
						fprintf(fp_log,"%s##mysql_mohu_match %s 失败\n",time_now,flag);
						return -1;
					}
					count = 0;
					while((mysql_row = mysql_fetch_row(mysql_res))) //获取个数
 					{
						count = atoi(mysql_row[0]);
		
					}
					mysql_free_result(mysql_res);
					if(count > 0)  //匹配到则返回
					{
						fprintf(fp_log,"%s##模糊匹配成功 车牌号为%s\n",time_now,plate_query);
                        memcpy(outplate,plate_query,PLATE_LENGTH);
						return 0;	
					}
				}
			}
			else //字母
			{
				val_asc = val_asc - 65;
				for(int j = 0;j++;j<10)
				{
					if(alpha[val_asc][j] == '@') //遇到数组里的'@'，则跳出循环
					break;
					
					
                    char plate_query[PLATE_LENGTH];
                    memset(plate_query,0,PLATE_LENGTH);
					sprintf(plate_query,"%s%c%s",plate.substr(0,seg).c_str(),alpha[val_asc][j],plate.substr(seg + 1,plate.length() - seg - 1).c_str()); //组成查询的车牌号
					fprintf(fp_log,"%s##模糊匹配 把%s变为%s到数据库查询\n",time_now,inplate,plate_query);
					sprintf(sqlcmd, "select count(*) from car where car_plate_id='%s'",plate_query); //sql语句
					if(strcmp("入口",flag) == 0)
					{
						rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
					}
					else
					{
						rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
					}
					if(rtn)
					{
						fprintf(fp_log,"%s##mysql_mohu_match %s 失败\n",time_now,flag);
						return -1;
					}
					if(strcmp("入口",flag) == 0)
					{
						mysql_res = mysql_store_result(&mysql_con_in);
					}
					else
					{
						mysql_res = mysql_store_result(&mysql_con_out);
					}
					if(!mysql_res)
					{
						fprintf(fp_log,"%s##mysql_mohu_match %s 失败\n",time_now,flag);
						return -1;
					}
					count = 0;
					while((mysql_row = mysql_fetch_row(mysql_res))) //获取个数
 					{
						count = atoi(mysql_row[0]);
		
					}
					mysql_free_result(mysql_res);
					if(count > 0) //匹配到则返回
					{
						fprintf(fp_log,"%s##模糊匹配成功 车牌号为%s\n",time_now,plate_query);
                        memcpy(outplate,plate_query,PLATE_LENGTH);
						return 0;	
					}
				}
			}

			
		}

		

	}

    memcpy(outplate,inplate,PLATE_LENGTH); // 匹配失败，则两个车牌号一致
	return 0;
}
/**************************************************end******************************************/
/******************************删除该车在在场表里的记录*******************************************/
int mysql_delete_car_inpark(char *flag,char * plate,char *id)
{
	char res[16];
	int rtn;
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;

	sprintf(sqlcmd, "delete from carinpark where carinpark_plate_id='%s' and carinpark_park_id='%s'",plate,id); //sql语句
	if(strcmp("入口",flag) == 0)
	{	
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else if(strcmp("出口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
		
	}
	if(rtn)
	{
		return -1;
	}
	return 0;
}
/**************************************************end******************************************/
/******************************按车牌号授权*******************************************************/
int mysql_auth_plate(char *flag,char *id,char *plate,bool *auth)
{
	int count = 0;
	int rtn;
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	*auth = false;

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	//根据通道id号和车牌号查询plateauth表，如果能查到，则该车已授权，查不到，则该车未授权
	sprintf(sqlcmd, "select count(*) from plateauth where plateauth_plate_id='%s' and plateauth_channel_id='%s'",plate,id); //sql语句
	
	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_auth_plate 失败\n",time_now);
		return -1;
	}
	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_auth_plate 失败\n",time_now);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		count = atoi(mysql_row[0]);
		
	}

	if(count > 0) 
	{
		*auth = true;
	}
	
	mysql_free_result(mysql_res);
	return 0;
	
}
/**************************************************end******************************************/
/******************************按车辆类型授权*******************************************************/
int mysql_auth_type(char *flag,char *id,char *plate,bool *auth)
{
	int count = 0;
	int rtn;
	char car_type[24];
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	*auth = false;

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	
	sprintf(sqlcmd, "select car_type from car where car_plate_id='%s'",plate); //sql语句
	
	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_auth_type 失败\n",time_now);
		return -1;
	}
	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_auth_type 失败\n",time_now);
		return -1;
	}
	
	memset(car_type,0,24);
	if((mysql_row = mysql_fetch_row(mysql_res))) //得到车辆类型
 	{
		memcpy(car_type,mysql_row[0],strlen(mysql_row[0]));
	}
	else
	{
		memcpy(car_type,"临时车",strlen("临时车"));	
	}

	fprintf(fp_log,"%s##车辆类型为%s\n",time_now,car_type);
	mysql_free_result(mysql_res);

	if(strcmp("临时车",car_type) == 0)
		return 0;
	//根据通道id号和车辆类型查询typeauth表，如果能查到，则该车已授权，查不到，则该车未授权
	sprintf(sqlcmd, "select count(*) from typeauth where typeauth_channel_id='%s' and typeauth_car_type='%s'",id,car_type); //sql语句

	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_auth_type_in 失败\n",time_now);
		return -1;
	}
	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_auth_type 失败\n",time_now);
		return -1;
	}

	while((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 	{
		count = atoi(mysql_row[0]);
	}

	if(count > 0)  
	{
		*auth = true;
	}
	
	mysql_free_result(mysql_res);
	
	return 0;
	
}
/**************************************************end******************************************/
/******************************车位数验证*******************************************************/
int mysql_validate_chewei_num(char *plate,bool *auth)
{
	int count = 0;
	int rtn;
	char car_num[24];
	char name[24];
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int chewei_count = 0;
	int carinpark_count = 0;

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	char park_id[256];

	memset(park_id,0,256);
	/*************************根据车牌号查询车位数和车主姓名*******************/	
	sprintf(sqlcmd, "select car_place_num,car_name from car where car_plate_id='%s'",plate); //sql语句
	
	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_validate_chewei_num 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_in);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_validate_chewei_num 失败\n",time_now);
		return -1;
	}
	
	memset(car_num,0,24);
	memset(name,0,24);
	if((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 	{
		memcpy(car_num,mysql_row[0],strlen(mysql_row[0]));
		memcpy(name,mysql_row[1],strlen(mysql_row[1]));
	}
	
	chewei_count = atoi(car_num);

	mysql_free_result(mysql_res);

	sprintf(sqlcmd, "select channel_park_id from channel where channel_ip='%s'",host_ip); //sql语句
	
	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_validate_chewei_num 失败\n",time_now);
		return -1;
	}
	
	mysql_res = mysql_store_result(&mysql_con_in);
	
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_validate_chewei_num 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);


	/******************************end*****************************/
	//根据车主姓名到carinpark里查询，该车主有几辆在场车
	sprintf(sqlcmd, "select count(*) from carinpark where carinpark_people_name='%s' and carinpark_park_id = '%s'",name,park_id); //sql语句

	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_validate_chewei_num 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_in);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_validate_chewei_num 失败\n",time_now);
		return -1;
	}

	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		carinpark_count = atoi(mysql_row[0]);
	}

	
	
	mysql_free_result(mysql_res);

	if(chewei_count <= carinpark_count) //车位数量小于等于在场车数量
	{
		*auth = false; //把已授权改为未授权
		fprintf(fp_log,"%s##车位数验证 车位数为%d 在场车数量为%d,车位数超限\n",time_now,chewei_count,carinpark_count);
	}
	else
	{
		fprintf(fp_log,"%s##车位数验证 车位数为%d 在场车数量为%d,车位数正常\n",time_now,chewei_count,carinpark_count);
	}
	
	return 0;
	
}
/**************************************************end******************************************/
/******************************月租车，储值车，储时车过期验证******************************************************/
int mysql_validate_guoqi(char *flag,char *id,char *plate,char *type,bool *auth,int *remain_day,int *remain_hour,int * remain_money,char * charge_rule)
{
	int count = 0;
	int rtn;
	char car_num[24];
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int chewei_count = 0;
	int carinpark_count = 0;
	long mil_now = 0;
	long mil_stop = 0;

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	//根据车牌号得到车辆类型
	sprintf(sqlcmd, "select car_type from car where car_plate_id='%s'",plate); //sql语句
	
	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_validate_guoqi   失败\n",time_now);
		return -1;
	}
	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_validate_guoqi 失败\n",time_now);
		return -1;
	}
	
	memset(type,0,24);
	if((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 	{
		memcpy(type,mysql_row[0],strlen(mysql_row[0]));
	}
	else
	{
		memcpy(type,"临时车",strlen("临时车"));
	}
	
	
	mysql_free_result(mysql_res);

	
	/*********************根据通道id号和车辆类型到chargerule表查询收费方案***************/
	sprintf(sqlcmd, "select chargerule_name_id from chargerule where chargerule_park_id='%s' and chargerule_car_type='%s'",id,type); //sql语句
	if(strcmp("入口",flag) == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_validate_guoqi 失败\n",time_now);
		return -1;
	}
	if(strcmp("入口",flag) == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_validate_guoqi 失败\n",time_now);
		return -1;
	}

	if((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 	{
		
		memset(charge_rule,0,256);
       		memcpy(charge_rule,mysql_row[0],strlen(mysql_row[0]));
	}
	else
	{
		fprintf(fp_log,"%s##通道%s 车辆类型%s 未找到对应的收费规则\n",time_now,id,type);
		return -1;
	}
	mysql_free_result(mysql_res);

	if(strcmp(type,"临时车") == 0) //临时车不验证
	return 0;
	/******************************end******************************************/
	fprintf(fp_log,"%s##开始验证授权，车辆类型为%s,收费规则为:%s\n",time_now,type,charge_rule);

	if(strcmp(charge_rule,"指定时间免费，过期按小时收费") == 0) //租期车
	{
		char stop_time[24];
		memset(stop_time,0,24);
		/**********根据车牌号到car表查询租期结束时间***********/
		sprintf(sqlcmd, "select car_stop_time from car where car_plate_id='%s'",plate); //sql语句
	
		if(strcmp("入口",flag) == 0)
		{
			rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		}
		else
		{
			rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
		}
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_validate_guoqi 失败\n",time_now);
			return -1;
		}
		if(strcmp("入口",flag) == 0)
		{
			mysql_res = mysql_store_result(&mysql_con_in);
		}
		else
		{
			mysql_res = mysql_store_result(&mysql_con_out);
		}
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_validate_guoqi 失败\n",time_now);
			return -1;
		}
	
		
		if((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 		{
			memcpy(stop_time,mysql_row[0],strlen(mysql_row[0]));
		}
		if(strlen(stop_time) < 6)
		{
			fprintf(fp_log,"%s##验证租期，未找到租期结束时间\n",time_now);
			return -1;	
		}
		
	
		mysql_free_result(mysql_res);	

		/***********************end**********************/
		mil_now = get_tick(time_now); //得到当前时间的秒数
		mil_stop = get_tick(stop_time);	//得到结束时间的秒数
		if(mil_now < mil_stop) //秒数比较
		{
			*remain_day = ((mil_stop - mil_now)/3600)/24;
			*remain_hour = ((mil_stop - mil_now)/3600)%24;
			fprintf(fp_log,"%s##验证租期，结束时间为%s 当前时间为%s,未过期。还有%d天%d小时过期\n",time_now,stop_time,time_now,*remain_day,*remain_hour);
		}
		else
		{
			fprintf(fp_log,"%s##验证租期，结束时间为%s 当前时间为%s,已过期。\n",time_now,stop_time,time_now);
			*auth = false;
		}
	}
	else if(strcmp(charge_rule,"储时，剩余时间不足，按小时收费") == 0) //储时车
	{
		/**********根据车牌号到car表查询剩余时间***********/
		sprintf(sqlcmd, "select car_remain_time from car where car_plate_id='%s'",plate); //sql语句
	
		if(strcmp("入口",flag) == 0)
		{
			rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		}
		else
		{
			rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
		}
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_validate_guoqi 失败\n",time_now);
			return -1;
		}
		if(strcmp("入口",flag) == 0)
		{
			mysql_res = mysql_store_result(&mysql_con_in);
		}
		else
		{
			mysql_res = mysql_store_result(&mysql_con_out);
		}
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_validate_guoqi 失败\n",time_now);
			return -1;
		}
	
		
		if((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 		{
			*remain_hour = atoi(mysql_row[0]);
		}
		
		mysql_free_result(mysql_res);

		/***********************end**********************/
		if(*remain_hour <= 0) //剩余时间为0
		{
			fprintf(fp_log,"%s##验证储时，剩余时间为0小时,时间不足。\n",time_now);
			*auth = false;	
		}
		else
		{
			fprintf(fp_log,"%s##验证储时，剩余时间为%d小时。\n",time_now,remain_hour);	
		}		
	
	}
	else if(strcmp(charge_rule,"储值，剩余金额不足，按小时收费") == 0) //储值车
	{
		/**********根据车牌号到car表查询剩余金额***********/
		sprintf(sqlcmd, "select car_remain_money from car where car_plate_id='%s'",plate); //sql语句
	
		if(strcmp("入口",flag) == 0)
		{
			rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		}
		else
		{
			rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
		}
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_validate_guoqi 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_in);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_validate_guoqi 失败\n",time_now);
			return -1;
		}
	
		
		if((mysql_row = mysql_fetch_row(mysql_res))) //取每条记录
 		{
			*remain_money = atoi(mysql_row[0]);
		}
		
		mysql_free_result(mysql_res);
		/***********************end**********************/
	
		if(*remain_money <= 0) //剩余金额不足
		{
			fprintf(fp_log,"%s##验证储值，剩余金额为0元,余额不足。\n",time_now);
			*auth = false;	
		}
		else
		{
			fprintf(fp_log,"%s##验证储值，剩余金额为%d元。\n",time_now,remain_money);	
		}		
	
	}

	
	
	return 0;
}
/******************************白天按小时收费，晚上按次收费***********************************************/
int mysql_day_hour_night_ci(char *id,char *car_type,char *in_time,char *out_time)
{
	char rate[24];
	char uninttime[24];
	char ci[24];
	char day_start_time[24];
	char night_start_time[24];
	char day_free_time[24];
	char night_free_time[24];
	int rtn;
	char sqlcmd[500];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	char tmp_in[24];
	char tmp_out[24];
	
	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	/******************************查询白天开始时间，夜间开始时间，白天收费费率和单位时间，白天免费时间，夜间免费时间***********************************************/
	sprintf(sqlcmd, "select chargerule_day_start_time,chargerule_night_start_time,chargerule_day_first_period_rate,chargerule_day_first_period_uninttime,chargerule_day_free_period_duration,chargerule_night_free_period_duration,chargerule_night_first_period_rate from chargerule where chargerule_park_id='%s' and chargerule_car_type='%s'",id,car_type); //sql语句
	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_in);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(day_start_time,0,24);
		memcpy(day_start_time,mysql_row[0],strlen(mysql_row[0]));
		memset(night_start_time,0,24);
		memcpy(night_start_time,mysql_row[1],strlen(mysql_row[1]));
		memset(rate,0,24);
		memcpy(rate,mysql_row[2],strlen(mysql_row[2]));
		memset(uninttime,0,24);
		memcpy(uninttime,mysql_row[3],strlen(mysql_row[3]));
		memset(day_free_time,0,24);
		memcpy(day_free_time,mysql_row[4],strlen(mysql_row[4]));
		memset(night_free_time,0,24);
		memcpy(night_free_time,mysql_row[5],strlen(mysql_row[5]));
		memset(ci,0,24);
		memcpy(ci,mysql_row[6],strlen(mysql_row[6]));
	}
	mysql_free_result(mysql_res);
	/**************************************************end******************************************/
	

	fprintf(fp_log,"%s##白天开始时间为%s,夜间开始时间为%s,白天费率为%s元/%s分钟,夜晚费率为%s元/次，白天免费时间为%s,夜间免费时间为%s\n",time_now,day_start_time,night_start_time,rate,uninttime,ci,day_free_time,night_free_time);
	
	/**************************************************end******************************************/
	int mil_now = get_tick(time_now); //得到当前时间的秒数
		
	int mil_in = get_tick(in_time);	 //得到进入时间的秒数

	std::string in(in_time);
	std::string out(time_now);
	
	memset(tmp_in,0,24);
	sprintf(tmp_in,"%s 00:00:00",in.substr(0,10).c_str()); //获取进入时间的凌晨时间
	memset(tmp_out,0,24);
	sprintf(tmp_out,"%s 00:00:00",out.substr(0,10).c_str()); //获取离开时间的凌晨时间

	int mil_tmp_in = mil_in -  get_tick(tmp_in); //化为24小时以内 得到进入时间
	int mil_tmp_out =  mil_now - get_tick(tmp_out); //化为24小时以内 得到离开时间

	

	if((mil_in - mil_tmp_in)/60 >= atoi(day_start_time) && (mil_in - mil_tmp_in)/60 <= atoi(night_start_time)) //白天
	{
		if(mil_now/60 - mil_in/60 < (atoi(day_free_time) +1 )) //停留时长小于免费时间
		{
			return 0;
		}
	}
	else //夜间
	{
		if(mil_now/60 - mil_in/60 < (atoi(night_free_time) + 1)) //停留时长小于免费时间
		{
			return 0;
		}	
	}
		
	if(mil_now - mil_in >= 86400) //停留时长大于24小时
	{
		int days = floor((mil_now - mil_in)/86400); //停留整天数
		
		int days_fee = days*((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)*atoi(rate) + atoi(ci)); //停留整天数的费用
		
		if(mil_tmp_in < mil_tmp_out) //化为24小时，进入时间在离开时间之前
		{
			if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  <= atoi(day_start_time)) //0点到8点进  0点到8点出
				return atoi(ci)+days_fee;
			else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  >atoi(day_start_time)&& mil_tmp_out/60  <= atoi(night_start_time)) //0点到8点进  8点到20点出
				return atoi(ci) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
			else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  > atoi(night_start_time))  //0点到8点进  20点到24点出
				return atoi(ci)*2 +  ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
			else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  <= atoi(night_start_time)) //8点到20点进  8点到20点出
				return ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
			else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  > atoi(night_start_time)) //8点到20点进  20点到24点出
				return  atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
			else  //20点到24点进  20点到24点出
				return  atoi(ci) + days_fee;
		}
		else //化为24小时，进入时间在离开时间之后
		{
			if(mil_tmp_in/60 <= atoi(day_start_time)) //0点到8点进  0点到8点出
				return atoi(ci)*2 + ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1 )*atoi(rate) + days_fee;
			else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //8点到20点进  第二天的0点到8点出
				return atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime) ) +1)*atoi(rate)+ days_fee;
			else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 < atoi(night_start_time)) //8点到20点进  第二天的8点到20点出
				return atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
			else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //20点到24点进  第二天的0点到8点出
				return atoi(ci)  + days_fee;
			else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 > atoi(day_start_time) && mil_tmp_out/60 <= atoi(night_start_time)) //20点到24点进  第二天的8点到20点出
				return atoi(ci)  + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee; 
			else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 >  atoi(night_start_time)) //20点到24点进  第二天的20点到24点出
				return atoi(ci)*2  + ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
		}
			
	}
	else ////停留时长小于24小时
	{
		int days_fee = 0;
		if(mil_tmp_in < mil_tmp_out) //化为24小时，进入时间在离开时间之前
		{
			if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  <= atoi(day_start_time)) //0点到8点进  0点到8点出
				return atoi(ci)+days_fee;
			else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  >atoi(day_start_time)&& mil_tmp_out/60  <= atoi(night_start_time)) //0点到8点进  8点到20点出
				return atoi(ci) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
			else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  > atoi(night_start_time))  //0点到8点进  20点到24点出
				return atoi(ci)*2 +  ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
			else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  <= atoi(night_start_time)) //8点到20点进  8点到20点出
				return ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
			else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  > atoi(night_start_time)) //8点到20点进  20点到24点出
				return  atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
			else  //20点到24点进  20点到24点出
				return  atoi(ci) + days_fee;
		}
		else //化为24小时，进入时间在离开时间之后
		{
			if(mil_tmp_in/60 <= atoi(day_start_time)) //0点到8点进  0点到8点出
				return atoi(ci)*2 + ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1 )*atoi(rate) + days_fee;
			else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //8点到20点进  第二天的0点到8点出
				return atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime) ) +1)*atoi(rate)+ days_fee;
			else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 < atoi(night_start_time)) //8点到20点进  第二天的8点到20点出
				return atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
			else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //20点到24点进  第二天的0点到8点出
				return atoi(ci)  + days_fee;
			else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 > atoi(day_start_time) && mil_tmp_out/60 <= atoi(night_start_time)) //20点到24点进  第二天的8点到20点出
				return atoi(ci)  + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee; 
			else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 >  atoi(night_start_time)) //20点到24点进  第二天的20点到24点出
				return atoi(ci)*2  + ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
		}
	}		
	
}
/******************************全天按小时收费***********************************************/
int mysql_calfee_all_day_by_hour(char *id,char *car_type,char *in_time,char *out_time)
{
	char rate[24];
	char uninttime[24];
	char free_time[24];
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;

	int rtn;
	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	/******************************查询费率和单位时间***********************************************/
	sprintf(sqlcmd, "select chargerule_day_first_period_rate,chargerule_day_first_period_uninttime,chargerule_day_free_period_duration from chargerule where chargerule_park_id='%s' and chargerule_car_type='%s'",id,car_type); //sql语句
	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_in);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(rate,0,24);
		memcpy(rate,mysql_row[0],strlen(mysql_row[0]));
		memset(uninttime,0,24);
		memcpy(uninttime,mysql_row[1],strlen(mysql_row[1]));
		memset(free_time,0,24);
		memcpy(free_time,mysql_row[2],strlen(mysql_row[2]));
	}
	mysql_free_result(mysql_res);
	/**************************************************end******************************************/
	
	
	int mil_out = get_tick(out_time); //得到离开时间的秒数
	int mil_in = get_tick(in_time);	 //得到进入时间的秒数	

	
	if((mil_out - mil_in)/60 < (atoi(free_time)+ 1))
	{
		return 0;
	}

	return ((int)floor((mil_out - mil_in)/60/atoi(uninttime)) + 1)*atoi(rate);	
}


/**************************************************end******************************************/
/******************************计费规则***********************************************/
int mysql_cal_fee(char *rule,char *in_time,char *out_time,char *car_type,char *id,char *plate)
{
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	if(strcmp(rule,"免费") == 0)//收费规则为免费
	{
		fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
		return 0;
	}
	else if(strcmp(rule,"指定时间免费") == 0)//收费规则为指定时间免费
	{
		char stop_time[24];
		
		char guanlian_rule[256];
		fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
		/******************************查询车辆到期时间***********************************************/
		sprintf(sqlcmd, "select car_stop_time from car where car_plate_id='%s'",plate); //sql语句
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_in);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			memset(stop_time,0,24);
			memcpy(stop_time,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);
		/**************************************************end******************************************/
		/******************************查询过期关联收费规则***********************************************/
		sprintf(sqlcmd, "select chargerule_guanlian_id from chargerule where chargerule_park_id='%s' and chargerule_car_type='%s'",id,car_type); //sql语句
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_in);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			memset(guanlian_rule,0,256);
			memcpy(guanlian_rule,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);
		/**************************************************end******************************************/

		int mil_now = get_tick(time_now); //得到当前时间的秒数
		int mil_stop = get_tick(stop_time);	 //得到结束时间的秒数
		int mil_in = get_tick(in_time);	 //得到进入时间的秒数
		
		if(mil_now <= mil_stop) //未过期
		{
			return 0;
		}
		else if(mil_in > mil_stop) //已过期
		{
			if(strcmp("免费",guanlian_rule) == 0)  //关联免费
			{
				return 0;
			}
			else if(strcmp("白天按小时收费，晚上按次收费",guanlian_rule) == 0) //关联 白天按小时收费，晚上按次收费
				return 	mysql_day_hour_night_ci(id,car_type,in_time,out_time);
			else if(strcmp("全天按小时收费",guanlian_rule) == 0)
				return mysql_calfee_all_day_by_hour(id,car_type,in_time,out_time); //关联 全天按小时收费
			else
				return 0;
		}
		else
		{
			return 0;	
		}
	}
	else if(strcmp(rule,"白天按小时收费，晚上按次收费") == 0) //收费规则为白天按小时收费，晚上按次收费
	{
		fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
		return mysql_day_hour_night_ci(id,car_type,in_time,out_time);
	}
	else if(strcmp(rule,"储时") == 0) //收费规则为储时
	{
		char remain_time[24];
		int mil_now = get_tick(time_now); //得到当前时间的秒数
		int mil_in = get_tick(in_time);	 //得到进入时间的秒数
		char guanlian_rule[256];

		fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
		/******************************查询剩余时间***********************************************/
		sprintf(sqlcmd, "select car_remain_time from car where car_plate_id='%s'",plate); //sql语句
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_in);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			memset(remain_time,0,24);
			memcpy(remain_time,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);
		/**************************************************end******************************************/

		/******************************查询关联收费规则***********************************************/
		sprintf(sqlcmd, "select chargerule_guanlian_id from chargerule where chargerule_park_id='%s' and chargerule_car_type='%s'",id,car_type); //sql语句
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_in);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			memset(guanlian_rule,0,256);
			memcpy(guanlian_rule,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);
		/**************************************************end******************************************/
		
		int time_final = 0;
		int money_final = 0;

		if((mil_now - mil_in)/60 <= atoi(remain_time)) //剩余时间充足
		{
			time_final = atoi(remain_time) - (mil_now - mil_in)/60;
			money_final = 0;
		}
		else if(atoi(remain_time) < (mil_now - mil_in)/60 && atoi(remain_time) != 0) //剩余时间小于该次停留时长
		{
			time_final = 0;
			money_final = 0;
		}
		else if(atoi(remain_time) == 0) //剩余时间为0 
		{
			time_final = 0;
			if(strcmp("免费",guanlian_rule) == 0) //关联 免费
			{
				money_final = 0;
			}
			else if(strcmp("白天按小时收费，晚上按次收费",guanlian_rule) == 0) //关联 白天按小时收费，晚上按次收费
				money_final = mysql_day_hour_night_ci(id,car_type,in_time,out_time);
			else if(strcmp("全天按小时收费",guanlian_rule) == 0)
				money_final = mysql_calfee_all_day_by_hour(id,car_type,in_time,out_time);  //关联 全天按小时收费
			else
				money_final = 0;	
		}

		memset(remain_time,0,24);
		sprintf(remain_time,"%d",time_final);
		sprintf(sqlcmd, "update car set car_remain_time='%s' where car_plate_id='%s'",remain_time,plate); //更新car表里的车辆剩余时间
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}

		return money_final;
	}
	else if(strcmp(rule,"储值") == 0) //收费规则为储值
	{
		char remain_money[24];
		char guanlian_rule[256];

		fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
		/******************************查询剩余金额***********************************************/
		sprintf(sqlcmd, "select car_remain_money from car where car_plate_id='%s'",plate); //sql语句
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_in);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			memset(remain_money,0,24);
			memcpy(remain_money,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);

		/**************************************************end******************************************/

		/******************************查询关联收费规则***********************************************/
		sprintf(sqlcmd, "select chargerule_guanlian_id from chargerule where chargerule_park_id='%s' and chargerule_car_type='%s'",id,car_type); //sql语句
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_in);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			memset(guanlian_rule,0,256);
			memcpy(guanlian_rule,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);
		/**************************************************end******************************************/

	
		int charge = 0;
		int money_final  = 0;

		if(strcmp("免费",guanlian_rule) == 0) //关联 免费
		{
			money_final = atoi(remain_money);
			charge = 0;
		}
		else if(strcmp("白天按小时收费，晚上按次收费",guanlian_rule) == 0) //关联 白天按小时收费，晚上按次收费
		{
			charge = mysql_day_hour_night_ci(id,car_type,in_time,out_time);
			if(charge <= atoi(remain_money))
			{
				money_final = atoi(remain_money) - charge;
				charge = 0;
			}
			else if(atoi(remain_money) < charge && atoi(remain_money) != 0)
			{
				charge = 0;
				money_final = 0;
			}
			else
			{
				money_final = 0;	
			}

			
		}
		else if(strcmp("全天按小时收费",guanlian_rule) == 0)  //关联 全天按小时收费
		{
			charge = mysql_calfee_all_day_by_hour(id,car_type,in_time,out_time);
			if(charge <= atoi(remain_money))
			{
				money_final = atoi(remain_money) - charge;
				charge = 0;
			}
			else if(atoi(remain_money) < charge && atoi(remain_money) != 0)
			{
				charge = 0;
				money_final = 0;
			}
			else
			{
				money_final = 0;	
			}

			
		}

		memset(remain_money,0,24);
		sprintf(remain_money,"%d",money_final);
		sprintf(sqlcmd, "update car set car_remain_money='%s' where car_plate_id='%s'",remain_money,plate); //更新car表里的车辆剩余金额
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_cal_fee 失败\n",time_now);
			return -1;
		}
		
			
		return charge;
		
	}	
	else if(strcmp(rule,"全天按小时收费") == 0)
	{
		fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
		return mysql_calfee_all_day_by_hour(id,car_type,in_time,out_time); //收费规则为全天按小时收费
	}
	else
	{
		return 0;
	}

}


/******************************处理上级停车场***********************************************/
int mysql_process_top_park(char * id,char *plate,char * charge_rule,char * car_type,car_msg *msg,char *open_door)
{
	int count = 0;
	int rtn;
	char park_id[256];
	char in_time[24];
	char park_parent_id[256];
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int chewei_count = 0;
	int carinpark_count = 0;
	long mil_now = 0;
	long mil_stop = 0;
	char parent_charge_rule[256];

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	/******************************根据通道id查找停车场id***********************************************/
	sprintf(sqlcmd, "select channel_park_id from channel where channel_id='%s'",id); //sql语句

	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_in);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(park_id,0,256);
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);
	/******************************end***********************************************************/	
	
	/******************************查询本停车场是否有上级停车场***********************************************/
	sprintf(sqlcmd, "select park_parent_id from park where park_id='%s'",park_id); //sql语句

	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_in);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(park_parent_id,0,256);
		memcpy(park_parent_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);
	/******************************end***********************************************************/	

	if(atoi(park_parent_id) == -1) //无上级停车场，则返回
	{
		fprintf(fp_log,"%s##无上级停车场\n",time_now);
		return 0;
	}
	else
	{
		fprintf(fp_log,"%s##有上级停车场\n",time_now);	
	}
	/******************************根据车场id和车辆类型查询收费规则***********************************************/
	sprintf(sqlcmd, "select chargerule_name_id from chargerule where chargerule_park_id='%s' and chargerule_car_type='%s'",park_parent_id,car_type); //sql语句
	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_in);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(parent_charge_rule,0,256);
		memcpy(parent_charge_rule,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);

	char in_pic_path[256];
	char in_channel_id[256];
	memset(in_pic_path,0,256);
	memset(in_channel_id,0,256);
	/******************************end***********************************************************/	
	/******************************根据车场id和车牌号查询进入时间***********************************************/
	sprintf(sqlcmd, "select carinpark_in_time,carinpark_pic_path,carinpark_channel_id from carinpark where carinpark_park_id='%s' and carinpark_plate_id='%s'",park_parent_id,plate); //sql语句
	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_in);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(in_time,0,24);
		memcpy(in_time,mysql_row[0],strlen(mysql_row[0]));
		memcpy( in_pic_path,mysql_row[1],strlen(mysql_row[1]));
		memcpy(in_channel_id,mysql_row[2],strlen(mysql_row[2]));
	}
	mysql_free_result(mysql_res);
	/******************************end***********************************************************/	

	int fee = mysql_cal_fee(parent_charge_rule,in_time,time_now,car_type,park_parent_id,plate); //计费
	char fee_tmp[24];
	memset(fee_tmp,0,24);
	sprintf(fee_tmp,"%d",fee);
	char stay_time[24];
	memset(stay_time,0,24);
	sprintf(stay_time,"%d",(get_tick(time_now) - get_tick(in_time))/60);
	/******************************把计费结果写到caroutrec表***********************************************/
	sprintf(sqlcmd, "insert into caroutrec  caroutrec_pay_charge,caroutrec_real_charge,caroutrec_in_time,caroutrec_out_time,caroutrec_in_channel_id,caroutrec_out_channel_id,caroutrec_park_id,caroutrec_in_pic_path,caroutrec_out_pic_path,caroutrec_out_car_type,caroutrec_open_door_type,caroutrec_stay_time values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",fee_tmp,"0",in_time,time_now,in_channel_id,msg->channel_id,park_id,in_pic_path,msg->path,car_type,open_door,stay_time); //sql语句
	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	/******************************end***********************************************************/		
	sprintf(sqlcmd, "delete from  carinpark where carinpark_park_id='%s' and carinpark_plate_id='%s'",park_parent_id,plate); //删除上级停车场该车辆的在场记录
	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}	
	return 0;
}

/******************************end***********************************************************/
int mysql_process_top_park_out(char * id,char *plate,char * charge_rule,char * car_type,char *park_parent_id)
{
	int count = 0;
	int rtn;
	
	char in_time[24];
	char park_id[256];
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int chewei_count = 0;
	int carinpark_count = 0;
	long mil_now = 0;
	long mil_stop = 0;
	char parent_charge_rule[256];

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	

	/******************************根据通道id查找停车场id***********************************************/
	sprintf(sqlcmd, "select channel_park_id from channel where channel_id='%s'",id); //sql语句

	rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_out);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(park_id,0,256);
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);
	/******************************end***********************************************************/
	
	/******************************查询本停车场是否有上级停车场***********************************************/
	sprintf(sqlcmd, "select park_parent_id from park where park_id='%s'",park_id); //sql语句

	rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_out);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_top_park 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(park_parent_id,0,256);
		memcpy(park_parent_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);
	/******************************end***********************************************************/	

	if(atoi(park_parent_id) == -1 || strlen(park_parent_id) < 10) //无上级停车场，则返回
	{
		fprintf(fp_log,"%s##无上级停车场\n",time_now);
		return 0;
	}
	else
	{
		fprintf(fp_log,"%s##有上级停车场\n",time_now);	
	}

	return 0;
}
/******************************写数据到入口表和在场表***********************************************/
int mysql_write_in_park(car_msg *in_msg,char *ori_plate,char *final_plate,char *park_id,char *global_car_type,bool *auth,char *open_door)
{
	int count = 0;
	int rtn;
	char people_name[24];
	char park_parent_id[24];
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int chewei_count = 0;
	int carinpark_count = 0;
	long mil_now = 0;
	long mil_stop = 0;
	char time_stamp[24];

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	
	/******************************写数据到入口表***********************************************/
	memset(time_stamp,0,24);
	sprintf(time_stamp,"%d",get_tick(in_msg->time));
	sprintf(sqlcmd, "insert into  carinrec values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",in_msg->time,time_stamp,ori_plate,final_plate,in_msg->path,in_msg->channel_id,park_id,global_car_type,open_door,in_msg->brand,in_msg->color,in_msg->plate1,in_msg->plate1,in_msg->brand1,in_msg->color1); //sql语句
	fprintf(fp_log,"%s##写数据到入口表\n",time_now);

	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_write_in_park 失败\n",time_now);
		return -1;
	}

	/******************************end************************************/
	if(strcmp(final_plate,"无车牌") == 0 || *auth == false) //无车牌不往在场表写
		return 0;

	/******************************查询车主姓名***********************************************/
	sprintf(sqlcmd, "select car_name from car where car_plate_id='%s'",final_plate); //sql语句
	memset(people_name,0,24);
	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_write_in_park 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_in);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_write_in_park 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		
		memcpy(people_name,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);
	/******************************end************************************/
	
	/******************************写数据到在场表***********************************************/
	fprintf(fp_log,"%s##写数据到在场表\n",time_now);
	sprintf(sqlcmd, "insert into  carinpark values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",in_msg->time,time_stamp,ori_plate,final_plate,in_msg->path,in_msg->channel_id,park_id,global_car_type,in_msg->brand,in_msg->color,in_msg->plate1,in_msg->plate1,in_msg->brand1,in_msg->color1,people_name); //sql语句
	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_write_in_park 失败\n",time_now);
		return -1;
	}
	/******************************end************************************/
	
	return 0;
}
/******************************end***********************************************************/
/******************************根据数据库配置，拼接LED显示信息***********************************************/

int mysql_process_led(char *flag,char *plate,char* show_type,char * show_text,char *msg)
{
	int count = 0;
	int rtn;
	
	char park_id[256];
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	memset(park_id,0,256);

	sprintf(sqlcmd, "select channel_park_id from channel where channel_ip='%s'",host_ip); //sql语句
	if(strcmp(flag,"入口") == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_led 失败\n",time_now);
		return -1;
	}
	if(strcmp(flag,"入口") == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_led 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);


	if(strcmp(show_type,"固定内容") == 0)
	{
		sprintf(msg,"%s",show_text);	
	}
	else if(strcmp(show_type,"剩余车位") == 0)
	{
		char remain[24];
		memset(remain,0,24);
		sprintf(sqlcmd, "select park_space_count_remain from park where park_id='%s'",park_id); //sql语句
		if(strcmp(flag,"入口") == 0)
		{
			rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		}
		else
		{
			rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
		}
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_process_led 失败\n",time_now);
			return -1;
		}
		if(strcmp(flag,"入口") == 0)
		{
			mysql_res = mysql_store_result(&mysql_con_in);
		}
		else
		{
			mysql_res = mysql_store_result(&mysql_con_out);
		}
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_process_led 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
		
			memcpy(remain,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);

		sprintf(msg,"剩余车位%s",remain);	
	}
	else if(strcmp(show_type,"日期") == 0)
	{
		char tmp[24];
		memset(tmp,0,24);
		memcpy(tmp,time_now,10);
		sprintf(msg,"%s",tmp);	
	}
	else if(strcmp(show_type,"时间") == 0)
	{
		char tmp[24];
		memset(tmp,0,24);
		memcpy(tmp,time_now + 10 ,9);
		sprintf(msg,"%s",tmp);	
	}
	else if(strcmp(show_type,"入场时间") == 0)
	{
		char time_ru[24];
		memset(time_ru,0,24);
		
		sprintf(sqlcmd, "select carinpark_in_time from carinpark where carinpark_park_id='%s' and carinpark_plate_id='%s' ",park_id,plate); //sql语句
		if(strcmp(flag,"入口") == 0)
		{
			rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		}
		else
		{
			rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
		}
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_process_led 失败\n",time_now);
			return -1;
		}
		if(strcmp(flag,"入口") == 0)
		{
			mysql_res = mysql_store_result(&mysql_con_in);
		}
		else
		{
			mysql_res = mysql_store_result(&mysql_con_out);
		}
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_process_led 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
		
			memcpy(time_ru,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);
		
		sprintf(msg,"%s",time_ru);
		if(strlen(time_ru) < 4 )
		sprintf(msg,"%s","未找到入场时间");	
	}
	else if(strcmp(show_type,"车类型") == 0)
	{
		char car_type[24];
		memset(car_type,0,24);
		
		sprintf(sqlcmd, "select car_type from car where car_plate_id='%s' ",plate); //sql语句
		if(strcmp(flag,"入口") == 0)
		{
			rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		}
		else
		{
			rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
		}
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_process_led 失败\n",time_now);
			return -1;
		}
		if(strcmp(flag,"入口") == 0)
		{
			mysql_res = mysql_store_result(&mysql_con_in);
		}
		else
		{
			mysql_res = mysql_store_result(&mysql_con_out);
		}
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_process_led 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
		
			memcpy(car_type,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);
		sprintf(msg,"%s",car_type);	
	}
	else if(strcmp(show_type,"车牌") == 0)
	{
		sprintf(msg,"%s",plate);	
	}
	else if(strcmp(show_type,"收费") == 0)
	{
		sprintf(msg,"收费%d元",fmoney);	
	}
	else if(strcmp(show_type,"停车时长") == 0)
	{
		char in_time[24];
		memset(in_time,0,24);
		
		sprintf(sqlcmd, "select carinpark_in_time from carinpark where carinpark_park_id='%s' and carinpark_plate_id='%s' ",park_id,plate); //sql语句
		if(strcmp(flag,"入口") == 0)
		{
			rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
		}
		else
		{
			rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
		}
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_process_led 失败\n",time_now);
			return -1;
		}
		if(strcmp(flag,"入口") == 0)
		{
			mysql_res = mysql_store_result(&mysql_con_in);
		}
		else
		{
			mysql_res = mysql_store_result(&mysql_con_out);
		}
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_process_led 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
		
			memcpy(in_time,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);
		int hour = 0;
		if(strlen(in_time) > 6)
		{			
			int mil = (get_tick(time_now) - get_tick(in_time))/60;
			if(mil > 60)
			{
				hour = floor(mil /60) + 1;
				sprintf(msg,"停车%d小时",hour);	
			}
			else
			{
				sprintf(msg,"停车%d分钟",mil);	
			}
		}
		else
		{
			sprintf(msg,"未找到入场时间");		
		}
	}

	return 0;
}
/******************************end***********************************************************/

/******************************给bled发送led显示信息和语音信息***********************************************/
int mysql_send_bled(char *flag,char *car_type,char *plate,char *id,char *speak_send)
{
	int count = 0;
	int rtn;
	char people_name[24];
	char park_parent_id[24];
	char sqlcmd[200];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;


	char ledset_type[24];
	char ledset_show_type[24];
	char ledset_show_text[256];
	char num[24];

	char tmp_type[24];
	char led_ip[24];


	char num1_text[1024];
	char num2_text[1024];
	char num3_text[1024];
	char num4_text[1024];
	
	
	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	
	

	memset(tmp_type,0,24);
	memset(led_ip,0,24);

	if(strcmp(flag,"入口") == 0)
	{
		strcpy(ledset_type,"入口有车");
	}
	else
	{
		strcpy(ledset_type,"出口有车");
	}


	memset(num1_text,0,1024);
	memset(num3_text,0,1024);
	memset(num2_text,0,1024);
	memset(num4_text,0,1024);
	/******************************查询数据库，每一行该显示什么内容***********************************************/
	sprintf(sqlcmd, "select * from ledset"); //sql语句
	if(strcmp(flag,"入口") == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_send_bled 失败\n",time_now);
		return -1;
	}
	if(strcmp(flag,"入口") == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_send_bled 失败\n",time_now);
		return -1;
	}
	
	
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		if(strcmp(ledset_type,mysql_row[0]) != 0)
				continue;
		if(atoi(mysql_row[1]) == 1)
		{
			mysql_process_led(flag,plate,mysql_row[2],mysql_row[3],num1_text); //拼接第一行
		}
		else if(atoi(mysql_row[1]) == 2)
		{
			mysql_process_led(flag,plate,mysql_row[2],mysql_row[3],num2_text); //拼接第二行
		}
		else if(atoi(mysql_row[1]) == 3)
		{
			mysql_process_led(flag,plate,mysql_row[2],mysql_row[3],num3_text); //拼接第三行
		}
		else if(atoi(mysql_row[1]) == 4)
		{
			mysql_process_led(flag,plate,mysql_row[2],mysql_row[3],num4_text); //拼接第四行
		}
	}
	mysql_free_result(mysql_res);

	/******************************end***********************************************************/	

	/******************************查询数据库，找LED 的 ip***********************************************/
	sprintf(sqlcmd, "select device_ip_id from device where device_channel_id='%s' and device_type='LED语音一体机'",id); //sql语句
	if(strcmp(flag,"入口") == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_send_bled 失败\n",time_now);
		return -1;
	}
	if(strcmp(flag,"入口") == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_send_bled 失败\n",time_now);
		return -1;
	}

	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		
		memcpy(led_ip,mysql_row[0],strlen(mysql_row[0]));
	}

	if(strlen(led_ip) < 0)
	{
		fprintf(fp_log,"%s##未找到ledip\n",time_now);
		return 0;	
	}		
	/******************************end***********************************************************/	

	/******************************按协议给bled发送LED显示信息和语音信息***********************************************/
	memset(in_led_ip,0,24);
	memset(out_led_ip,0,24);
	if(strcmp(flag,"入口") == 0)
	{
		memcpy(in_led_ip,led_ip,strlen(led_ip));
	}
	else
	{
		memcpy(out_led_ip,led_ip,strlen(led_ip));
	}
	Json::Value json_send;  //LED显示信息
	json_send["cmd"] = Json::Value("sendled");
	json_send["row_num"] = Json::Value("4");
	json_send["row1"] = Json::Value(num1_text);
	json_send["row2"] = Json::Value(num2_text);
	json_send["row3"] = Json::Value(num3_text);
	json_send["row4"] = Json::Value(num4_text);
	json_send["led_ip"] = Json::Value(led_ip);

	Json::Value json_send1;  //语音信息
	json_send1["cmd"] = Json::Value("sendvoice");
	json_send1["content"] = Json::Value(speak_send);
	json_send1["led_ip"] = Json::Value(led_ip);

	std::string send = json_send.toStyledString(); //转成字符串
	std::string send1 = json_send1.toStyledString();

	struct sockaddr_in addr;
    int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr)); //发送LED信息
	if(n < 0)
	{
		fprintf(fp_log,"%s##mysql_send_bled 发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##mysql_send_bled 发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}

	n = sendto(sock,send1.c_str(),send1.length(), 0, (struct sockaddr *)&addr, sizeof(addr)); //发送语音信息
	if(n < 0)
	{
		fprintf(fp_log,"%s##mysql_send_bled 语音信息发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##mysql_send_bled 语音信息发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send1.c_str());
	}
	close(sock);
	/******************************end***********************************************************/	
	return 0;

}
/******************************end***********************************************************/

int mysql_process_fee(char *channel_id,char *plate)
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;

	char tmp1[24];
	char park_id[256];
	char in_time[24];
	char car_type[24];
	char charge_rule[256];
	int fee = 0;
	int rtn;

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	sprintf(sqlcmd, "select channel_park_id from channel where channel_id='%s'",channel_id); //sql语句
	rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_fee1 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_out);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_fee1 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(park_id,0,256);
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);	


	sprintf(sqlcmd, "select carinpark_in_time from carinpark where carinpark_park_id='%s' and carinpark_plate_id='%s'",park_id,plate); //sql语句
	rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_fee2 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_out);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_fee2 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		
		memset(in_time,0,24);
		memcpy(in_time,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);	

	
	memset(car_type,0,24);
	sprintf(sqlcmd, "select car_type from car where car_plate_id='%s'",plate); //sql语句
	rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_fee3 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_out);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_fee3 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		
		memcpy(car_type,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);	

	

	if(strlen(car_type) < 3)
	{
		memset(car_type,0,24);
		strcpy(car_type,"临时车");
	}
	

	sprintf(sqlcmd, "select chargerule_name_id from chargerule where chargerule_park_id='%s' and chargerule_car_type='%s'",park_id,car_type); //sql语句
	rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_fee4 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_out);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_fee4 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(charge_rule,0,256);
		memcpy(charge_rule,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);	

	
	fee = mysql_cal_fee(charge_rule,in_time,time_now,car_type,park_id,plate);

	sprintf(sqlcmd, "select caroutrec_real_charge from caroutrec where caroutrec_plate_id='%s' and caroutrec_parent_park_id='%s'",plate,park_id); //sql语句
	
	rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_fee5 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_out);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_fee5 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(tmp1,0,24);
		memcpy(tmp1,mysql_row[0],strlen(mysql_row[0]));
	}
	
	fee = fee + atoi(tmp1);
	mysql_free_result(mysql_res);	
	

	sprintf(sqlcmd, "select caroutrec_real_charge from caroutrec where caroutrec_plate_id='%s' and caroutrec_park_id='%s'",plate,park_id); //sql语句
	rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_fee6 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_out);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_fee6 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(tmp1,0,24);
		memcpy(tmp1,mysql_row[0],strlen(mysql_row[0]));
	}
	fee = fee + atoi(tmp1);
	mysql_free_result(mysql_res);

	/*微信缴费和中心缴费*/

	
	fflush(fp_log);
	return fee;

}
int mysql_write_caroutrec(char *in_time,int pay_charge,int real_charge,char *plate_ori,char *plate_final,char *in_pic_path,char *in_channel_id,char *park_id,char *park_parent_id,char *car_type,char *operator_name,char *open_door,char *charge_type,car_msg *out)
{
	char sqlcmd[5000];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	char tmp_pay_charge[24];
	memset(tmp_pay_charge,0,24);
	sprintf(tmp_pay_charge,"%d",pay_charge);

	char tmp_real_charge[24];
	memset(tmp_real_charge,0,24);
	sprintf(tmp_real_charge,"%d",real_charge);

	char tmp_time_stamp[24];
	memset(tmp_time_stamp,0,24);
	sprintf(tmp_time_stamp,"%d",get_tick(time_now));

	char tmp_stay_time[24];
	memset(tmp_stay_time,0,24);
	if(strlen(in_time) < 3)
	{
		sprintf(tmp_stay_time,"%d",0);
	}
	else
	{
		sprintf(tmp_stay_time,"%d",(get_tick(out->time) -get_tick(in_time) )/60);
	}
	
	
	sprintf(sqlcmd, "insert into caroutrec values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",in_time,out->time,tmp_pay_charge,tmp_real_charge,tmp_time_stamp,plate_ori,plate_final,in_pic_path,out->path,in_channel_id,out->channel_id,park_id,park_parent_id,car_type,operator_name,open_door,charge_type,"",tmp_stay_time,out->brand,out->color,out->plate1,out->brand1,out->color1); //sql语句



	rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_write_caroutrec 失败\n",time_now);
		return -1;
	}
	
}


int mysql_write_carinpark(char *plate_ori,char *plate_final,char *park_id,char *car_type,char *people_name,car_msg *msg)
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	char tmp[24];
	memset(tmp,0,24);
	sprintf(tmp,"%d",get_tick(time_now));	

	sprintf(sqlcmd, "insert into  carinpark values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",time_now,tmp,plate_ori,plate_final,msg->path,msg->channel_id,park_id,car_type,msg->brand,msg->color,msg->plate1,msg->brand1,msg->color1,people_name); //sql语句

	rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_write_carinpark 失败\n",time_now);
		return -1;
	}

	return 0;
}
int mysql_send_bgui_start()
{
	Json::Value bgui_send;
	Json::Value content;

	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	char park_id[256];//车场id
	char park_name[24];//车位名称
	char space_count[24];//车位数
	char space_count_remain[24];//剩余车位数
	memset(sqlcmd,0,300);
	sprintf(sqlcmd, "select people_name from people"); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_send_bgui_start1 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_bgui);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_send_bgui_start 失败\n",time_now);
		return -1;
	}
	
	int nu = 0;
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		
		if(nu > 0)
		{
			content["user"].append(mysql_row[0]);
		}
		nu++;
	}
	mysql_free_result(mysql_res);

	memset(sqlcmd,0,300);
	sprintf(sqlcmd, "select channel_park_id from channel where channel_ip='%s'",host_ip); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_send_bgui_start2 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_bgui);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_send_bgui_start 失败\n",time_now);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(park_id,0,256);
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);
	memset(sqlcmd,0,300);
	sprintf(sqlcmd, "select park_name,park_space_count,park_space_count_remain from park where park_id='%s'",park_id); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_send_bgui_start3 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_bgui);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_send_bgui_start 失败\n",time_now);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		
		memset(park_name,0,24);
		memcpy(park_name,mysql_row[0],strlen(mysql_row[0]));
		memset(space_count,0,24);
		memcpy(space_count,mysql_row[1],strlen(mysql_row[1]));
		memset(space_count_remain,0,24);
		memcpy(space_count_remain,mysql_row[2],strlen(mysql_row[2]));
	}
	mysql_free_result(mysql_res);

	memset(sqlcmd,0,300);
	sprintf(sqlcmd, "select tmpcartype_cartype from tmpcartype"); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_send_bgui_start4 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_bgui);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_send_bgui_start 失败\n",time_now);
		return -1;
	}
	nu = 0;
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		if(nu > 0)
		{
			content["cartype"].append(mysql_row[0]);
			
		}
		nu++;
	}
	mysql_free_result(mysql_res);

	
	content["parkname"] = Json::Value(park_name);
	content["totalparkposition"] = Json::Value(space_count);
	char tmp3[24];
	int use = atoi(space_count) - atoi(space_count_remain);
	memset(tmp3,0,24);
	sprintf(tmp3,"%d",use);
	content["usedparkpostion"] = Json::Value(tmp3);
	content["remainingparkposition"] = Json::Value(space_count_remain);
	content["local_ip"] = Json::Value(host_ip);
	content["server_ip"] = Json::Value(host_server_ip);

	bgui_send["cmd"] = Json::Value("start");
	bgui_send["content"] = content;

	std::string send = bgui_send.toStyledString();
	
	
	struct sockaddr_in addr;
    	int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    	addr.sin_port = htons(5004);
    	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##bgui mysql_send_bgui_start发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##bgui mysql_send_bgui_start发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);
	return 0;

	
}

int mysql_bgui_login(char *name,char *password)
{
	Json::Value bgui_send;
	Json::Value content;

	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	int count = 0;

	sprintf(sqlcmd, "select count(*) from people where people_name='%s'and people_pass_word='%s'",name,password); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_bgui);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		count = atoi(mysql_row[0]);	
	}
	mysql_free_result(mysql_res);

	bgui_send["cmd"] = Json::Value("login");
	if(count > 0)
	{
		content["result"] = Json::Value("成功");
	}
	else
	{
		content["result"] = Json::Value("失败");
	}
	bgui_send["content"] = content;

	std::string send = bgui_send.toStyledString();
	struct sockaddr_in addr;
    	int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    	addr.sin_port = htons(5004);
    	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##bgui mysql_bgui_login发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##bgui mysql_bgui_login发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);
	return 0;	

	
}


int mysql_bgui_process_door(char *type,char *op)
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	int count = 0;
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
			if(in_fleet == true)
			{
				json_send["flag"] = Json::Value("keep");
			}
			else
			{
				json_send["flag"] = Json::Value("once");
			}
		}
		else
		{
			json_send["cmd"] = Json::Value("close_door");
			json_send["channel_id"] = Json::Value(in_channel);
			json_send["in_out"] = Json::Value("入口");
		}
	}
	else
	{
		if(strcmp(op,"开") == 0)
		{
			json_send["cmd"] = Json::Value("open_door");
			json_send["channel_id"] = Json::Value(out_channel);
			json_send["in_out"] = Json::Value("出口");
			if(out_fleet == true)
			{
				json_send["flag"] = Json::Value("keep");
			}
			else
			{
				json_send["flag"] = Json::Value("once");
			}
		}
		else
		{
			json_send["cmd"] = Json::Value("close_door");
			json_send["channel_id"] = Json::Value(out_channel);
			json_send["in_out"] = Json::Value("出口");
		}	
	}
	

	std::string send = json_send.toStyledString();

	struct sockaddr_in addr;
    	int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    	addr.sin_port = htons(5002);
    	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##mysql_bgui_process_door 发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##mysql_bgui_process_door 发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);
	
	if(strcmp(type,"入口") == 0)
	{
		sprintf(sqlcmd, "select count(*) from carinpark where carinpark_plate_id='%s'and carinpark_channel_id='%s'",in_plate,in_channel); //sql语句
		rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_bgui);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			count = atoi(mysql_row[0]);	
		}
		mysql_free_result(mysql_res);	

		if(count == 0)
		{
			char pic_path[256];
			char park_id[256];
			char car_type[24];
			char car_logo[24];
			char car_color[24];
			char people_name[24];
		

			memset(pic_path,0,256);
			memset(park_id,0,256);
			memset(car_logo,0,24);
			memset(car_type,0,24);
			memset(car_color,0,24);
			memset(people_name,0,24);

			sprintf(sqlcmd, "select carinrec_pic_path,carinrec_park_id,carinrec_car_type,carinrec_car_logo,carinrec_car_color from carinrec where carinrec_plate_id='%s'and carinrec_channel_id='%s' and carinrec_in_time='%s'",in_plate,in_channel,in_time); //sql语句
			rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
			if(rtn)
			{
				fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
				return -1;
			}
			mysql_res = mysql_store_result(&mysql_con_bgui);
			if(!mysql_res)
			{
				fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
				return -1;
			}
	
			if((mysql_row = mysql_fetch_row(mysql_res))) 
 			{
				memcpy(pic_path,mysql_row[0],strlen(mysql_row[0]));
				memcpy(park_id,mysql_row[1],strlen(mysql_row[1]));
				memcpy(car_type,mysql_row[2],strlen(mysql_row[2]));	
				memcpy(car_logo,mysql_row[3],strlen(mysql_row[3]));
				memcpy(car_color,mysql_row[4],strlen(mysql_row[4]));
			}
			mysql_free_result(mysql_res);	

			sprintf(sqlcmd, "select car_name from car where car_plate_id='%s'",in_plate); //sql语句
			rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
			if(rtn)
			{
				fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
				return -1;
			}
			mysql_res = mysql_store_result(&mysql_con_bgui);
			if(!mysql_res)
			{
				fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
				return -1;
			}
	
			if((mysql_row = mysql_fetch_row(mysql_res))) 
 			{
				memcpy(people_name,mysql_row[0],strlen(mysql_row[0]));
			}
			mysql_free_result(mysql_res);	

			char tmp[24];
			memset(tmp,0,24);
			sprintf(tmp,"%d",get_tick(in_time));	
			sprintf(sqlcmd, "insert into  carinpark values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",in_time,tmp,"",in_plate,pic_path,in_channel,park_id,car_type,car_logo,car_color,"","","",""); //sql语句

			rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
			if(rtn)
			{
				fprintf(fp_log,"%s##mysql_write_carinpark 失败\n",time_now);
				return -1;
			}
		}
			
	}
	else
	{
		char park_id[256];
		memset(park_id,0,256);
		sprintf(sqlcmd, "select channel_park_id from channel where channel_id='%s'",in_channel); //sql语句
		rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_bgui);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);

		sprintf(sqlcmd, "select count(*) from carinpark where carinpark_plate_id='%s'and carinpark_park_id='%s'",in_plate,park_id); //sql语句
		rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_bgui);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			count = atoi(mysql_row[0]);	
		}
		mysql_free_result(mysql_res);		
		
		if(count > 0)
		{
			mysql_delete_car_inpark("bgui",out_plate,park_id);	
		}
	
	}
	
	return 0;
	
}

int mysql_bgui_process_youhui(char *type,char *plate)
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	char park_id[256];
	char chargerule[256];
	char in_time[24];
	memset(park_id,0,256);
	memset(chargerule,0,256);
	memset(in_time,0,24);
	

	int fee = 0;

		
	sprintf(sqlcmd, "select carinpark_park_id,carinpark_in_time from carinpark where carinpark_plate_id='%s'",plate); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_bgui_process_youhui1 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_bgui);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_bgui_process_youhui 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
		memcpy(in_time,mysql_row[1],strlen(mysql_row[1]));
	}
	mysql_free_result(mysql_res);		

	
	if(strlen(park_id) < 10)
	{
		fee = 0;
	}
	else
	{
		sprintf(sqlcmd, "select chargerule_name_id from chargerule where chargerule_park_id='%s' and chargerule_car_type='%s'",park_id,type); //sql语句
		rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_bgui_process_youhui2 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_bgui);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_bgui_process_youhui 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			memcpy(chargerule,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);	

		fee = mysql_cal_fee(chargerule,in_time,time_now,type,park_id,plate);	
	}

	char paycharge[24];
	memset(paycharge,0,24);
	sprintf(paycharge,"%d",fee);
	sprintf(sqlcmd, "update  caroutrec  set  caroutrec_pay_charge='%s' where  caroutrec_out_time='%s' and caroutrec_plate_id='%s'",paycharge,out_time,plate); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_bgui_process_youhui3 失败\n",time_now);
		return -1;
	}

	sprintf(sqlcmd, "update  caroutrec  set  caroutrec_car_type='%s' where  caroutrec_out_time='%s' and caroutrec_plate_id='%s'",type,out_time,plate); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_bgui_process_youhui 失败\n",time_now);
		return -1;
	}

	Json::Value bgui_send;
	Json::Value content;

	bgui_send["cmd"] = Json::Value("changediscount");
	
	content["plate"] = Json::Value(plate);
	char fee_tmp[24];
	memset(fee_tmp,0,24);
	sprintf(fee_tmp,"%d",fee);
	content["charge"] = Json::Value(fee_tmp);
	
	bgui_send["content"] = content;

	std::string send = bgui_send.toStyledString();
	struct sockaddr_in addr;
    	int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    	addr.sin_port = htons(5004);
    	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##bgui mongodb_bgui_process_youhui发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##bgui mongodb_bgui_process_youhui发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);
	return 0;
}


int mysql_bgui_charge_pass(char *plate)
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	Json::Value json_send; 

	Json::Value json_send1;  //语音信息
	json_send1["cmd"] = Json::Value("sendvoice");
	json_send1["content"] = Json::Value("祝您一路顺风");
	json_send1["led_ip"] = Json::Value(out_led_ip);
	
	
	json_send["cmd"] = Json::Value("open_door");
	json_send["channel_id"] = Json::Value(out_channel);
	json_send["in_out"] = Json::Value("出口");
	json_send["flag"] = Json::Value("once");
	

	std::string send = json_send.toStyledString();
	std::string send1 = json_send1.toStyledString();
	struct sockaddr_in addr;
    	int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    	addr.sin_port = htons(5002);
    	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##mysql_bgui_charge_pass 发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##mysql_bgui_charge_pass 发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);


	struct sockaddr_in addr1;
    int sock1;
	sock1=socket(AF_INET, SOCK_DGRAM, 0);
	addr1.sin_family = AF_INET;
    addr1.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr1.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	n = sendto(sock1,send1.c_str(),send1.length(), 0, (struct sockaddr *)&addr1, sizeof(addr1)); //发送语音信息
	if(n < 0)
	{
		fprintf(fp_log,"%s##mysql_send_bled 语音信息发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##mysql_send_bled 语音信息发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send1.c_str());
	}
	close(sock1);

	sprintf(sqlcmd, "update  caroutrec  set  caroutrec_charge_type='现金' where  caroutrec_out_time='%s' and caroutrec_plate_id='%s'",out_time,plate); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_bgui_charge_pass 失败\n",time_now);
		return -1;
	}
	
	char park_id[256];
	memset(park_id,0,256);
	sprintf(sqlcmd, "select channel_park_id from channel where channel_id='%s'",out_channel); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_bgui_process_youhui 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_bgui);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_bgui_process_youhui 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);	

	mysql_delete_car_inpark("bgui",plate,park_id);	
	
	return 0;
}

int mysql_bgui_free_pass(char *plate)
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 

	Json::Value json_send; 

	Json::Value json_send1;  //语音信息
	json_send1["cmd"] = Json::Value("sendvoice");
	json_send1["content"] = Json::Value("祝您一路顺风");
	json_send1["led_ip"] = Json::Value(out_led_ip);
	
	
	json_send["cmd"] = Json::Value("open_door");
	json_send["channel_id"] = Json::Value(out_channel);
	json_send["in_out"] = Json::Value("出口");
	json_send["flag"] = Json::Value("once");
	

	std::string send = json_send.toStyledString();
	std::string send1 = json_send1.toStyledString();
	struct sockaddr_in addr;
    	int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    	addr.sin_port = htons(5002);
    	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##mysql_bgui_charge_pass 发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##mysql_bgui_charge_pass 发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);


	struct sockaddr_in addr1;
    int sock1;
	sock1=socket(AF_INET, SOCK_DGRAM, 0);
	addr1.sin_family = AF_INET;
    addr1.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr1.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	n = sendto(sock1,send1.c_str(),send1.length(), 0, (struct sockaddr *)&addr1, sizeof(addr1)); //发送语音信息
	if(n < 0)
	{
		fprintf(fp_log,"%s##mysql_send_bled 语音信息发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##mysql_send_bled 语音信息发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send1.c_str());
	}
	close(sock1);


	sprintf(sqlcmd, "update  caroutrec  set  caroutrec_charge_type='免费' where  caroutrec_out_time='%s' and caroutrec_plate_id='%s'",out_time,plate); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_bgui_charge_pass 失败\n",time_now);
		return -1;
	}
	
	char park_id[256];
	memset(park_id,0,256);
	sprintf(sqlcmd, "select channel_park_id from channel where channel_id='%s'",out_channel); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_bgui_process_youhui 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_bgui);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_bgui_process_youhui 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);	

	mysql_delete_car_inpark("bgui",plate,park_id);	
	
	return 0;
}

int mysql_bgui_modif_passwd(char *name,char *old_pass,char *new_pass)
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	int count = 0;
		
	sprintf(sqlcmd, "select count(*) from people where people_name='%s'and people_pass_word='%s'",name,old_pass); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_bgui);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		count = atoi(mysql_row[0]);	
	}
	mysql_free_result(mysql_res);

	if(count > 0)
	{
		sprintf(sqlcmd, "update table people set people_pass_word='%s' where people_name='%s'",new_pass,name); //sql语句
		rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_bgui_modif_passwd 失败\n",time_now);
			return -1;
		}	
	}

	Json::Value bgui_send;
	Json::Value content;

	bgui_send["cmd"] = Json::Value("modifypasswd");
	
	if(count > 0)
	{
		content["result"] = Json::Value("成功");
	}
	else
	{
		content["result"] = Json::Value("失败");
	}
	bgui_send["content"] = content;

	std::string send = bgui_send.toStyledString();
	struct sockaddr_in addr;
    	int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    	addr.sin_port = htons(5004);
    	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##bgui mysql_bgui_modif_passwd发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##bgui mysql_bgui_modif_passwd发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);
	return 0;
}

int mysql_process_chewei(char *channel_id,char *in_out,char *flag)
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 
	int count = 0;
	
	char park_id[256];
	char park_space_remain[24];
	char park_space[24];
	memset(park_id,0,256);
	memset(park_space_remain,0,24);
	memset(park_space,0,24);

	sprintf(sqlcmd, "select channel_park_id from channel where channel_id='%s'",channel_id); //sql语句
	rtn = mysql_real_query(&mysql_con_chewei, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_chewei 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_chewei);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_chewei 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);	

	sprintf(sqlcmd, "select park_space_count_remain,park_space_count from park where park_id='%s'",park_id); //sql语句
	rtn = mysql_real_query(&mysql_con_chewei, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_chewei 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_chewei);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_process_chewei 失败\n",time_now);
		return -1;
	}
	
	if((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memcpy(park_space_remain,mysql_row[0],strlen(mysql_row[0]));
		memcpy(park_space,mysql_row[1],strlen(mysql_row[1]));
	}
	mysql_free_result(mysql_res);		

	count = atoi(park_space_remain);
	if(strcmp(in_out,"入口") == 0)
	{
		count = count - 1;
	}
	else
	{
		count = count + 1;
	}

	if(count < 0)
	{
		count = 0;
	}
	if(count > atoi(park_space))
	{
		count = atoi(park_space);
	}
	memset(park_space_remain,0,24);
	sprintf(park_space_remain,"%d",count);

	sprintf(sqlcmd, "update park set park_space_count_remain='%s' where park_id='%s'",park_space_remain,park_id); //sql语句
	rtn = mysql_real_query(&mysql_con_chewei, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_chewei 失败\n",time_now);
		return -1;
	}

	fprintf(fp_log,"%s##总车位为%d,剩余车位为%d\n",time_now,atoi(park_space),count);	


	Json::Value json_msg; //发送给bled的json格式的信息
	json_msg["cmd"] = Json::Value("availablepark");
	
	json_msg["number"] = Json::Value(park_space_remain);
	std::string send = json_msg.toStyledString();

	
	struct sockaddr_in addr;
    int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##bled 车位发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##bled 车位发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);

	if(strcmp(in_out,"入口") == 0)
	{
		sprintf(sqlcmd, "select count(*) from carinpark where carinpark_plate_id='%s'and carinpark_channel_id='%s'",in_plate,in_channel); //sql语句
		rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_bgui);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			count = atoi(mysql_row[0]);	
		}
		mysql_free_result(mysql_res);	
		fprintf(fp_log,"%s##车牌号为%s,通道id为%s，进入时间%s,carinpark数量为%d\n",time_now,in_plate,in_channel,in_time,count);
		if(count == 0)
		{
			char pic_path[256];
			char park_id[256];
			char car_type[24];
			char car_logo[24];
			char car_color[24];
			char people_name[24];
		

			memset(pic_path,0,256);
			memset(park_id,0,256);
			memset(car_logo,0,24);
			memset(car_type,0,24);
			memset(car_color,0,24);
			memset(people_name,0,24);

			sprintf(sqlcmd, "select carinrec_pic_path,carinrec_park_id,carinrec_car_type,carinrec_car_logo,carinrec_car_color from carinrec where carinrec_plate_id='%s'and carinrec_channel_id='%s' and carinrec_in_time='%s'",in_plate,in_channel,in_time); //sql语句
			rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
			if(rtn)
			{
				fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
				return -1;
			}
			mysql_res = mysql_store_result(&mysql_con_bgui);
			if(!mysql_res)
			{
				fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
				return -1;
			}
	
			if((mysql_row = mysql_fetch_row(mysql_res))) 
 			{
				memcpy(pic_path,mysql_row[0],strlen(mysql_row[0]));
				memcpy(park_id,mysql_row[1],strlen(mysql_row[1]));
				memcpy(car_type,mysql_row[2],strlen(mysql_row[2]));	
				memcpy(car_logo,mysql_row[3],strlen(mysql_row[3]));
				memcpy(car_color,mysql_row[4],strlen(mysql_row[4]));
			}
			mysql_free_result(mysql_res);	

			sprintf(sqlcmd, "select car_name from car where car_plate_id='%s'",in_plate); //sql语句
			rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
			if(rtn)
			{
				fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
				return -1;
			}
			mysql_res = mysql_store_result(&mysql_con_bgui);
			if(!mysql_res)
			{
				fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
				return -1;
			}
	
			if((mysql_row = mysql_fetch_row(mysql_res))) 
 			{
				memcpy(people_name,mysql_row[0],strlen(mysql_row[0]));
			}
			mysql_free_result(mysql_res);	

			char tmp[24];
			memset(tmp,0,24);
			sprintf(tmp,"%d",get_tick(in_time));	
			sprintf(sqlcmd, "insert into  carinpark values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",in_time,tmp,"",in_plate,pic_path,in_channel,park_id,car_type,car_logo,car_color,"","","",""); //sql语句

			rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
			if(rtn)
			{
				fprintf(fp_log,"%s##mysql_write_carinpark 失败\n",time_now);
				return -1;
			}
		}
			
	}
	else
	{
		char park_id[256];
		memset(park_id,0,256);
		sprintf(sqlcmd, "select channel_park_id from channel where channel_id='%s'",in_channel); //sql语句
		rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_bgui);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
	
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
		}
		mysql_free_result(mysql_res);

		sprintf(sqlcmd, "select count(*) from carinpark where carinpark_plate_id='%s'and carinpark_park_id='%s'",in_plate,park_id); //sql语句
		rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
		mysql_res = mysql_store_result(&mysql_con_bgui);
		if(!mysql_res)
		{
			fprintf(fp_log,"%s##mysql_bgui_login 失败\n",time_now);
			return -1;
		}
		
		if((mysql_row = mysql_fetch_row(mysql_res))) 
 		{
			count = atoi(mysql_row[0]);	
		}
		mysql_free_result(mysql_res);		
		fprintf(fp_log,"%s##车牌号为%s,carinpark数量为%d\n",time_now,out_plate,count);
		if(count > 0)
		{
			mysql_delete_car_inpark("bgui",out_plate,park_id);	
		}
	
	}

	return 0;

}


int mysql_bgui_modif_parkcount( char *count)
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 	

	char park_id[256];
	memset(park_id,0,256);
	sprintf(sqlcmd, "select channel_park_id from channel where channel_ip='%s'",host_ip); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_send_bgui_start 失败\n",time_now);
		return -1;
	}
	mysql_res = mysql_store_result(&mysql_con_bgui);
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_send_bgui_start 失败\n",time_now);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(park_id,0,256);
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);

	sprintf(sqlcmd, "update park set park_space_count_remain='%s' where park_id='%s'",count,park_id); //sql语句
	rtn = mysql_real_query(&mysql_con_bgui, sqlcmd, strlen(sqlcmd));
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_process_chewei 失败\n",time_now);
		return -1;
	}

	return 0;

}

int mysql_query_space_count( char *flag,char *space_count)
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 	

	char park_id[256];
	memset(park_id,0,256);
	sprintf(sqlcmd, "select channel_park_id from channel where channel_ip='%s'",host_ip); //sql语句
	if(strcmp(flag,"入口") == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_query_space_count 失败\n",time_now);
		return -1;
	}
	if(strcmp(flag,"入口") == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_query_space_count 失败\n",time_now);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(park_id,0,256);
		memcpy(park_id,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);


	sprintf(sqlcmd, "select park_space_count_remain from park where park_id='%s'",park_id); //sql语句
	if(strcmp(flag,"入口") == 0)
	{
		rtn = mysql_real_query(&mysql_con_in, sqlcmd, strlen(sqlcmd));
	}
	else
	{
		rtn = mysql_real_query(&mysql_con_out, sqlcmd, strlen(sqlcmd));
	}
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_query_space_count 失败\n",time_now);
		return -1;
	}
	if(strcmp(flag,"入口") == 0)
	{
		mysql_res = mysql_store_result(&mysql_con_in);
	}
	else
	{
		mysql_res = mysql_store_result(&mysql_con_out);
	}
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_query_space_count 失败\n",time_now);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		
		memcpy(space_count,mysql_row[0],strlen(mysql_row[0]));
	}
	mysql_free_result(mysql_res);

	return 0;
	
}

int mysql_get_welcome_msg()
{
	char sqlcmd[300];
	MYSQL_RES *mysql_res;
	MYSQL_RES *mysql_res_device;
	MYSQL_ROW mysql_row;
	MYSQL_ROW mysql_row_device;
	int rtn;	

	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now); 	

	
	char led_row[24];
	char led_content[24];
	char led_type[24];
	char show_type[24];
	char show_text[256];	
	char channel_id[256];
	char channel_in_out[24];
	char led_ip[24];

	Json::Value json_msg; //发送给bled的json格式的信息
	Json::Value json_led_in; //发送给bled的json格式的信息
	Json::Value json_led_out; //发送给bled的json格式的信息


	
	sprintf(sqlcmd, "select ledset_row_id,ledset_type,ledset_show_type,ledset_show_text from ledset"); //sql语句
	
	rtn = mysql_real_query(&mysql_con_bled, sqlcmd, strlen(sqlcmd));
	
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_get_welcome_msg 失败\n",time_now);
		return -1;
	}
	
	mysql_res = mysql_store_result(&mysql_con_bled);
	
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_get_welcome_msg 失败\n",time_now);
		return -1;
	}
	
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(led_row,0,24);
		memset(led_type,0,24);
		memset(show_type,0,24);
		memset(show_text,0,256);

		memcpy(led_row,mysql_row[0],strlen(mysql_row[0]));
		memcpy(led_type,mysql_row[1],strlen(mysql_row[1]));
		memcpy(show_type,mysql_row[2],strlen(mysql_row[2]));
		memcpy(show_text,mysql_row[3],strlen(mysql_row[3]));


		if(strcmp(led_type,"入口无车") == 0)
			{
				if(atoi(led_row) == 1)
				{
					if(strcmp(show_type,"固定内容") == 0)
					{
						json_led_in["row1"] = Json::Value(show_text);					
					}
					else
					{
						json_led_in["row1"] = Json::Value(show_type);	
					}
				}
				else if(atoi(led_row) == 2)
				{
					if(strcmp(show_type,"固定内容") == 0)
					{
						json_led_in["row2"] = Json::Value(show_text);					
					}
					else
					{
						json_led_in["row2"] = Json::Value(show_type);	
					}
				}
				else if(atoi(led_row) == 3)
				{
					if(strcmp(show_type,"固定内容") == 0)
					{
						json_led_in["row3"] = Json::Value(show_text);					
					}
					else
					{
						json_led_in["row3"] = Json::Value(show_type);	
					}
				}
				else if(atoi(led_row) == 4)
				{
					if(strcmp(show_type,"固定内容") == 0)
					{
						json_led_in["row4"] = Json::Value(show_text);					
					}
					else
					{
						json_led_in["row4"] = Json::Value(show_type);	
					}
				}
			}
			else if(strcmp(led_type,"出口无车") == 0)
			{
				if(atoi(led_row) == 1)
				{
					if(strcmp(show_type,"固定内容") == 0)
					{
						json_led_out["row1"] = Json::Value(show_text);					
					}
					else
					{
						json_led_out["row1"] = Json::Value(show_type);	
					}
				}
				else if(atoi(led_row) == 2)
				{
					if(strcmp(show_type,"固定内容") == 0)
					{
						json_led_out["row2"] = Json::Value(show_text);					
					}
					else
					{
						json_led_out["row2"] = Json::Value(show_type);	
					}
				}
				else if(atoi(led_row) == 3)
				{
					if(strcmp(show_type,"固定内容") == 0)
					{
						json_led_out["row3"] = Json::Value(show_text);					
					}
					else
					{
						json_led_out["row3"] = Json::Value(show_type);	
					}
				}
				else if(atoi(led_row) == 4)
				{
					if(strcmp(show_type,"固定内容") == 0)
					{
						json_led_out["row4"] = Json::Value(show_text);					
					}
					else
					{
						json_led_out["row4"] = Json::Value(show_type);	
					}
				}
			}
	}
	mysql_free_result(mysql_res);


	sprintf(sqlcmd, "select channel_id,channel_in_out from channel where channel_ip='%s'",host_ip); //sql语句
	rtn = mysql_real_query(&mysql_con_bled, sqlcmd, strlen(sqlcmd));
	
	if(rtn)
	{
		fprintf(fp_log,"%s##mysql_get_welcome_msg 失败\n",time_now);
		return -1;
	}
	
	mysql_res = mysql_store_result(&mysql_con_bled);
	
	if(!mysql_res)
	{
		fprintf(fp_log,"%s##mysql_get_welcome_msg 失败\n",time_now);
		return -1;
	}
	while((mysql_row = mysql_fetch_row(mysql_res))) 
 	{
		memset(channel_id,0,256);
		memset(channel_in_out,0,24);
		memcpy(channel_id,mysql_row[0],strlen(mysql_row[0]));
		memcpy(channel_in_out,mysql_row[1],strlen(mysql_row[1]));
		
		memset(led_ip,0,24);
		sprintf(sqlcmd, "select device_ip_id from device where device_channel_id='%s' and device_type='LED语音一体机'",channel_id); //sql语句
		rtn = mysql_real_query(&mysql_con_bled, sqlcmd, strlen(sqlcmd));
	
		if(rtn)
		{
			fprintf(fp_log,"%s##mysql_get_welcome_msg 失败\n",time_now);
			return -1;
		}
	
		mysql_res_device = mysql_store_result(&mysql_con_bled);
	
		if(!mysql_res_device)
		{
			fprintf(fp_log,"%s##mysql_get_welcome_msg 失败\n",time_now);
			return -1;
		}
		while((mysql_row_device = mysql_fetch_row(mysql_res_device))) 
 		{
			memcpy(led_ip,mysql_row_device[0],strlen(mysql_row_device[0]));		
		}
		mysql_free_result(mysql_res_device);

		if(strcmp(channel_in_out,"入口") == 0)
		{
			json_led_in["led_ip"] = Json::Value(led_ip);				
		}
		else
		{
			json_led_out["led_ip"] = Json::Value(led_ip);	
		}

	}
	mysql_free_result(mysql_res);



	json_msg["cmd"] = Json::Value("welcome_msg");
	json_msg["content"].append(json_led_in);
	json_msg["content"].append(json_led_out);
	std::string send = json_msg.toStyledString();

	struct sockaddr_in addr;
    int sock;
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(n < 0)
	{
		fprintf(fp_log,"%s##bled welcome msg发送失败\n",time_now);
	}
	else
	{
		fprintf(fp_log,"%s##bled welcome msg发送成功:\n",time_now);
		fprintf(fp_log,"%s\n",send.c_str());
	}
	close(sock);

	return 0;
}










