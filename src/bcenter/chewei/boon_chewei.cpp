/***************************boon_chewei.cpp***************************************
			  功能：处理车位
			  创建时间：2017-02-05
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
***************************************************************************/
#include "boon_chewei.h"
extern pthread_mutex_t mongo_mutex_car;
/*********************************************车位处理线程******************************/
void* chewei_thread(void *)
{
	car_pass *in_msg;
	char time_now[64];
	time_t tm;
	
	time_printf(time(&tm),time_now);   //获取当前时间
	printf("chewei_thread 启动成功 \n");
	fprintf(fp_log,"%s##chewei_thread 启动成功\n",time_now);
	fflush(fp_log);

	
	while(1)
	{
		usleep(200);
		in_msg = BoonMsgQueue::Instance()->get_carpass(); //取车位队列里的数据
		if(in_msg == NULL)
		{
			continue;
		}
		char time_now[64];
		time_t tm;
		time_printf(time(&tm),time_now);   //获取当前时间
		
		
		pthread_mutex_lock(&mongo_mutex_car);
		if(mongodb_flag)
		mongodb_connect();
		if(mongodb_flag) //查询mongodb
		{
			if(mongodb_process_chewei(in_msg->channel_id,in_msg->in_out,in_msg->flag) < 0)
			continue;
			
		}
		else //查询mysql
		{
			mysql_process_chewei(in_msg->channel_id,in_msg->in_out,in_msg->flag);
		}
		
		BoonMsgQueue::Instance()->release_carpass(in_msg); //释放队列
		fflush(fp_log);
		if(mongodb_flag)
		mongodb_exit();
		pthread_mutex_unlock(&mongo_mutex_car);
	}
}
/**************************************************end******************************************/

