#ifndef BOON_GLOBAL_H
#define BOON_GLOBAL_H

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <map>
#include <vector>
#include <iostream>
#include <math.h>
#include <mongoc.h>
#include <fnmatch.h>
#include <sys/file.h>
#include "json/json.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/file.h>

#define PLATE_LENGTH 1024


extern char operator_name[256];

extern FILE* fp_log; //日志句柄
extern char  host_ip[64];//主机ip
extern char  host_server_ip[64];//服务器ip
extern bool  mongodb_flag;//为1，使用mongodb，为0，使用mysql
extern char number[10][10]; //模糊匹配 易混淆数字
extern char alpha[26][10]; //模糊匹配 易混淆字母


class  car_pass
{
public:
	car_pass(){
		memset(channel_id,0,256);
		memset(in_out,0,16);
		memset(flag,0,16);
	}
	char channel_id[256];
	char in_out[16];
	char flag[16];
};


class  car_msg
{
public:
	car_msg(){
		num = 0;
		memset(channel_id,0,256);
		memset(in_out,0,24);
		memset(time,0,24);

        memset(plate,0,PLATE_LENGTH);
		memset(pcolor,0,24);
		memset(brand,0,24);
		memset(type,0,24);
		memset(color,0,24);
		memset(path,0,256);
		memset(ipc_ip,0,24);

        memset(plate1,0,PLATE_LENGTH);
		memset(pcolor1,0,24);
		memset(brand1,0,24);
		memset(type1,0,24);
		memset(color1,0,24);
		memset(path1,0,256);
		memset(ipc_ip1,0,24);
        memset(in_time,0,24);

        memset(blacklist,0,256);
        memset(blacklistreason,0,1024);
        memset(in_time,0,24);
    }
	int num;
	char time[24];
	char channel_id[256];
	char in_out[24];

    char plate[PLATE_LENGTH];
	char pcolor[24];
	char brand[24];
	char type[24];
	char color[24];
	char path[256];
	char ipc_ip[24];

    char plate1[PLATE_LENGTH];
	char pcolor1[24];
	char brand1[24];
	char type1[24];
	char color1[24];
	char path1[256];
	char ipc_ip1[24];
    char in_time[24];

    char blacklist[256];
    char blacklistreason[1024];
	
};


int is_have_instance();//进程互斥
void time_printf(time_t   t,char *pctime);//获取当前时间
int create_log_file(); //创建日志文件
long get_tick(char *str_time) ;//根据标准时间字符串返回秒数
void unixTime2Str(int n, char strTime[], int bufLen) ;
bool getLocalIp(std::string dev, std::string & str_ip);
std::string getLocalIp();
#endif
