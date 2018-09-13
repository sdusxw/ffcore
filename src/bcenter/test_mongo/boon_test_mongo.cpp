/***************************boon_test_mongo.cpp***************************************
			  功能：数据库侦测
			  创建时间：2017-02-19
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
***************************************************************************/
#include "boon_test_mongo.h"
#include "../in/boon_in.h"
#include "../out/boon_out.h"
#include "../mongodb/boon_mongodb.h"
#include "../bgui/boon_bgui.h"
#include "../chewei/boon_chewei.h"
#include "../test_mongo/boon_test_mongo.h"
#include "../bled/boon_bled.h"
#include "../bipc/boon_bipc.h"
#include "../bop/boon_bop.h"
#include "../bvs/boon_bvs.h"

extern bool  mongodb_flag;
extern pthread_t pid_in;
extern pthread_t pid_out;
extern pthread_t pid_bipc;
extern pthread_t pid_bop;
extern pthread_t pid_bvs;
	
extern pthread_t pid_bgui;
extern pthread_t pid_db;
extern pthread_t pid_chewei;
extern pthread_t pid_bled;
extern pthread_mutex_t mongo_mutex_channel;
extern pthread_mutex_t mongo_mutex_carinpark;
extern pthread_mutex_t mongo_mutex_car;
extern pthread_mutex_t mongo_mutex_device;
/*********************************************数据库侦测线程******************************/
void* test_mongo_thread(void *)
{
	
	char time_now[64];
	time_t tm;
	time_printf(time(&tm),time_now);   //获取当前时间


	

	printf("test_mongo_thread 启动成功 \n");
	fprintf(fp_log,"%s##test_mongo_thread 启动成功\n",time_now);
	fflush(fp_log);

	int flag = 0;
	while(1)
	{
        sleep(60);
		
		int sockfd=socket(AF_INET,SOCK_STREAM,0); 
	
		struct sockaddr_in addr_ser;  
		bzero(&addr_ser,sizeof(addr_ser));  
    		addr_ser.sin_family=AF_INET;  
    		addr_ser.sin_addr.s_addr=inet_addr(host_server_ip);  
    		addr_ser.sin_port=htons(27017);  

		
    		int err=connect(sockfd,(struct sockaddr *)&addr_ser,sizeof(addr_ser));
		shutdown(sockfd,2);
		close(sockfd);
		if(err < 0)
		{
			flag = 10;
		}
		else
		{
			
			flag = 0;
			
		}
		
		if(flag == 10 && mongodb_flag == true)
		{
			
			pthread_cancel(pid_in);
			pthread_cancel(pid_out);
			
			pthread_mutex_unlock(&mongo_mutex_car);
			sleep(1);
		
	
			pthread_create(&pid_in, NULL, in_thread, NULL);
			pthread_detach(pid_in);

			pthread_create(&pid_out, NULL, out_thread, NULL);
			pthread_detach(pid_out);

			time_printf(time(&tm),time_now);   //获取当前时间
			fprintf(fp_log,"%s##切换到mysql\n",time_now);
			fflush(fp_log);	
		}
		if(flag == 0 && mongodb_flag == false)
		{
			
			pthread_cancel(pid_in);
			pthread_cancel(pid_out);
		
			pthread_mutex_unlock(&mongo_mutex_car);
			sleep(1);
			
		
	
			pthread_create(&pid_in, NULL, in_thread, NULL);
			pthread_detach(pid_in);

			pthread_create(&pid_out, NULL, out_thread, NULL);
			pthread_detach(pid_out);

			time_printf(time(&tm),time_now);   //获取当前时间
			fprintf(fp_log,"%s##切换到mongodb\n",time_now);	
			fflush(fp_log);
		}
		if(flag == 10)
		{
			mongodb_flag = false;	
		}
		if(flag == 0)
		{
			mongodb_flag = true;
		}
		fflush(fp_log);
	}
}
/**************************************************end******************************************/

