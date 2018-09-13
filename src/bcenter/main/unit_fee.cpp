//
// Created by boon on 18-4-9.
//

//#include "boon_global.h"
//#include "../global/boon_global.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <math.h>

using namespace std;
 FILE* fp_log; //日志句柄
int fun();

long get_tick(char *str_time);

int main(int argc, char** argv)
{
    fun();

    return 0;
}

int fun()
{
    char uninttime_night[24] = {"60"};
    char rate_night[24] = {"2"};
    char day_start_time[24];
    char night_start_time[24] = {"480"};
    char day_free_time[24];
    char night_free_time[24];
    char night_max_cost[24] = {"5"};

    char tmp_in[24];
    char tmp_out[24];

    char* out_time = "2018-08-03 08:24:34";
    char* in_time = "2018-08-03 07:56:34";

    int mil_now = get_tick(out_time); //得到当前时间的秒数

    int mil_in = get_tick(in_time);	 //得到进入时间的秒数
    mil_now = mil_now - 60;
    if(mil_in >= mil_now) mil_in = mil_now - 10;

    printf("mil_in: %d, mil_now: %d\n", mil_in, mil_now );

    std::string in(in_time);
    std::string out(out_time);

    memset(tmp_in,0,24);
    sprintf(tmp_in,"%s 00:00:00",in.substr(0,10).c_str()); //获取进入时间的凌晨时间
    memset(tmp_out,0,24);
    sprintf(tmp_out,"%s 00:00:00",out.substr(0,10).c_str()); //获取离开时间的凌晨时间

    printf("tmp_in: %s, tmp_out: %s\n", tmp_in, tmp_out );

    int mil_tmp_in = mil_in -  get_tick(tmp_in); //化为24小时以内 得到进入时间
    int mil_tmp_out =  mil_now - get_tick(tmp_out); //化为24小时以内 得到离开时间

    printf("mil_now: %d, get_tick(tmp_out): %d, mil_tmp_out: %d\n", mil_now, get_tick(tmp_out), mil_tmp_out);

    //printf("night_max_cost: %s\n", night_max_cost);

    printf("(mil_tmp_out/60- atoi(night_start_time)： %d\n", (mil_tmp_out/60- atoi(night_start_time)));

    printf("((int)floor((mil_tmp_out/60- atoi(night_start_time))/atoi(uninttime_night)) + 1)： %d\n",
           ((int)floor((mil_tmp_out/60- atoi(night_start_time))/atoi(uninttime_night)) + 1));

    printf("出场分钟 mil_tmp_out/60 : %d\n", mil_tmp_out/60);
    printf("出场分钟 atoi(night_start_time): %d\n", atoi(night_start_time));
    printf("出场分钟 (mil_tmp_out/60- atoi(night_start_time)): %d\n", (mil_tmp_out/60- atoi(night_start_time)));
    printf("出场分钟 (mil_tmp_out/60- atoi(night_start_time))/atoi(uninttime_night): %d\n",
           (mil_tmp_out/60- atoi(night_start_time))/atoi(uninttime_night));



int  fee1 =0;
    int fee2 =4;
    //fee2 = ((int)floor((atoi(day_start_time) - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) ;
    int mils_dur;
    mils_dur = get_tick(out_time) - get_tick(in_time);
    fee1 = (((int)floor(mils_dur/60.0/60.0)+1)*2);

    if(fee1<fee2 && fee1>0)
    {

        printf("[mongodb_process_fee]矫正前计费%d,矫正后交费%d\n",fee2,fee1);
        //fprintf(fp_log,"[mongodb_process_fee]进入计费矫正");
        //fprintf(fp_log,"[mongodb_process_fee]矫正前计费%d,矫正后交费%d\n",fee2,fee1);
        fee2 = fee1;
    }


    //                                                480                       60
    //int fee = ((int)floor((mil_tmp_out/60- atoi(night_start_time))/atoi(uninttime_night)) + 1) * atoi(rate_night);
    int fee = ((int)floor((mil_tmp_out/60- mil_tmp_in/60)/atoi(uninttime_night)) + 1) * atoi(rate_night);

    printf("fee1: %d\n", fee);

    if(fee > atoi(night_max_cost))   fee = atoi(night_max_cost);

    printf("fee2: %d\n", fee);
    printf("test  fee1: %d\n",fee1);
    printf("test  fee2: %d\n",fee2);
    return fee;
}

long get_tick(char *str_time)
{
    struct tm stm;
    int iY, iM, iD, iH, iMin, iS;

    memset(&stm,0,sizeof(stm));

    iY = atoi(str_time);
    iM = atoi(str_time+5);
    iD = atoi(str_time+8);
    iH = atoi(str_time+11);
    iMin = atoi(str_time+14);
    iS = atoi(str_time+17);

    stm.tm_year=iY-1900;
    stm.tm_mon=iM-1;
    stm.tm_mday=iD;
    stm.tm_hour=iH;
    stm.tm_min=iMin;
    stm.tm_sec=iS;

    return mktime(&stm);
}