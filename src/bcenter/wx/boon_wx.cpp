/***************************boon_wx.cpp***************************************
			  功能：与wx通信
			  创建时间：2017-02-05
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
			        1. 2017/11/21 14:21
			            继时旭之后，修改/添加微信支付宝双码支付功能
***************************************************************************/
#include "boon_log.h"
#include "common_def.h"
#include "../bcenter_def.h"
//#include "mongodb_model.h"

#include "boon_wx.h"

using namespace BASE;

static char log_buf_[LOG_BUF_SIZE] = {0};

extern pthread_mutex_t mongo_mutex_car;

/**************************************************wx监听线程******************************/
void* wx_thread(void *)
{
	char time_now[64];
	char recv_buffer[4096];
	Json::Reader reader;
    Json::Value value;
	time_t tm;
	int recv_len = 0;
	time_printf(time(&tm),time_now);   //获取当前时间

    int local_port = 6001;

	int server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(local_port);

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	int client_addr_len = sizeof(client_addr);

	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) //绑定地址
	{
		fprintf(fp_log,"%s##wx_thread bind 失败\n",time_now);
		printf("wx_thread bind error\n");

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 绑定端口失败[%d]. line[%d]", __FUNCTION__, local_port, __LINE__);
        writelog(log_buf_);

		return 0;
	}

	printf("wx_thread 启动成功 \n");
	fprintf(fp_log,"%s##wx_thread 启动成功\n",time_now);
	fflush(fp_log);

	int ret = -1;

	while(1)
	{
		usleep(200);
		memset(recv_buffer, 0, sizeof(recv_buffer));
		recv_len = recvfrom(server_sock, recv_buffer, sizeof(recv_buffer), 0,(struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len); //接收数据
		time_printf(time(&tm),time_now);   //获取当前时间

		std::string cmd;
		if(!reader.parse(recv_buffer, value)) //解析json数据
		{
			fprintf(fp_log,"%s##wx_thread 解析json数据失败\n",time_now);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 解析json数据格式失败[%s] return! line[%d]", __FUNCTION__, recv_buffer, __LINE__);
            writelog(log_buf_);

			continue;
		}

		writelog("\n**************** 移动支付 *********************");

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收到一个消息包[%s] return! line[%d]", __FUNCTION__, recv_buffer, __LINE__);
        writelog(log_buf_);

		//fprintf(fp_log,"%s##wx_thread 收到json数据:\n",time_now);
		//fprintf(fp_log,"%s\n",recv_buffer);//把收到的数据写到日志里
		cmd = value["cmd"].asString(); //解析cmd命令

		pthread_mutex_lock(&mongo_mutex_car);

		if(mongodb_flag)
			mongodb_connect();

		std::string name;
		std::string park_id;
		std::string box_ip;
		std::string plate;
		std::string openid;
		std::string userid;
		std::string money;
		std::string flag;
		std::string outime;

		if(strcmp(cmd.c_str(), C_CMD_OUT) == 0) //来车
		{
            // 车主姓名，停车场id，停车场ip地址，车牌号，出场时间
			name = value["name"].asString();
			park_id = value["park_id"].asString();
			box_ip = value["box_ip"].asString();
			plate = value["plate"].asString();
			openid = value["openid"].asString();
			userid = value["userid"].asString();
			outime = value["outime"].asString();

            snprintf(log_buf_, sizeof(log_buf_), "[%s] 出场 查询停车费用 cmd[%s] line[%d]", __FUNCTION__, cmd.c_str(), __LINE__);
            writelog(log_buf_);

			ret = mongodb_process_wx_carout((char *)name.c_str(),(char *)park_id.c_str(),(char *)box_ip.c_str(),(char *)plate.c_str(),(char *)openid.c_str(),time_now, (char*)userid.c_str());
			if( ret < 0 )
			{
				fprintf(fp_log,"%s##wx_thread mongodb carout失败\n",time_now);
			}
		}
		else if(strcmp(cmd.c_str(), C_CMD_PAY) == 0) //场内支付，或出场支付
		{
			park_id = value["park_id"].asString();

			plate = value["plate"].asString();
			openid = value["openid"].asString();
			userid = value["userid"].asString();
			money = value["money"].asString();
			flag = value["flag"].asString();

			if(0 == flag.compare("open")){ // 出场口支付
                snprintf(log_buf_, sizeof(log_buf_), "[%s] 出场口支付费用 cmd[%s] flag[%s] line[%d]", __FUNCTION__, cmd.c_str(), flag.c_str(), __LINE__);
                writelog(log_buf_);

                // 优惠额度
                std::string dis_money = value["dis_money"].asString();
                // 应收额度
                std::string fact_money = value["fact_money"].asString();

                box_ip = value["box_ip"].asString();

				ret = mongodb_process_wx_pay_open((char *)money.c_str(),(char *)park_id.c_str(),(char *)box_ip.c_str(),
                                                  (char *)plate.c_str(),(char *)openid.c_str(),(char *)flag.c_str(),
                                                  (char*)userid.c_str(), (char*)dis_money.c_str(), (char*)fact_money.c_str());

				if( ret < 0){
					fprintf(fp_log,"%s##wx_thread mongodb pay失败\n",time_now);
				}

			}else if(0 == flag.compare("pay")){ // 场内支付

				snprintf(log_buf_, sizeof(log_buf_), "[%s] 场内支付费用 cmd[%s] flag[%s] line[%d]", __FUNCTION__, cmd.c_str(), flag.c_str(), __LINE__);
				writelog(log_buf_);

                // 优惠额度
                std::string dis_money = value["dis_money"].asString();
                // 应收额度
                std::string fact_money = value["fact_money"].asString();

                mongodb_process_wx_pay_in(money.c_str(), park_id.c_str(), plate.c_str(),
                                          openid.c_str(), userid.c_str(), flag.c_str(),
                                          dis_money.c_str(), fact_money.c_str());

			}else{
				snprintf(log_buf_, sizeof(log_buf_), "[%s] error 无法解析flag[%s] cmd[%s] line[%d]", __FUNCTION__, flag.c_str(), cmd.c_str(), __LINE__);
				writelog(log_buf_);
			}
		}else if(strcmp(cmd.c_str(), C_CMD_QUERY_PAY_IN) == 0){  // 场内查询停车费用

            park_id = value["park_id"].asString();
            box_ip = value["box_ip"].asString();
            plate = value["plate"].asString();
            openid = value["openid"].asString();
            userid = value["userid"].asString();
            outime = value["outime"].asString();

            // 场内查询支付费用 char *park_id, char *box_ip, char *plate, char *openid, char* userid, char *outime
            mongodb_process_wx_query_fee_in(cmd.c_str(), park_id.c_str(), plate.c_str(), openid.c_str(), userid.c_str(), outime.c_str());

        }else{
            snprintf(log_buf_, sizeof(log_buf_), "[%s] error 无法解析 cmd[%s] line[%d]", __FUNCTION__, cmd.c_str(), __LINE__);
            writelog(log_buf_);
        };
		
		if(mongodb_flag)
			mongodb_exit();

	    pthread_mutex_unlock(&mongo_mutex_car);

		fflush(fp_log);
	}// end while
}

// 写入日志
void writelog(const char* _buf)
{
    BLog::writelog(_buf, PKGDATA, LOG_DIR_NAME, LOG_FILE_NAME);
}

/**************************************************end******************************************/

