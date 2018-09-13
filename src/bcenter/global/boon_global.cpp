/***************************boon_global.cpp***************************************
              功能：公共函数
              创建时间：2017-02-04
              创建人：孙振行
              单位：山东博昂信息科技有限公司
              修改时间：
***************************************************************************/
#include "boon_global.h"

char operator_name[256];

// 日志
static char log_buf_[1024*8] = {0};

// 记录日志
extern void writelog(const char* _buf);

/*********************进程互斥（用文件）***********************************/
int is_have_instance()
{
    int file_id = open("./bcenter.tmp", O_RDWR | O_CREAT, 0640);
    if(file_id < 0)
    {
        return -1;
    }

    if(flock(file_id, LOCK_EX | LOCK_NB) < 0)
    {
        return -1;
    }

    return 0;
}
/*********************end**********************************************/


/*********************获取当前时间***************************************/
void time_printf(time_t   t,char *pctime)
{
    struct   tm   tmWk;
    memset(&tmWk,0,sizeof( tm));
    localtime_r(&t, &tmWk);

    sprintf(pctime, "%04d-%02d-%02d %02d:%02d:%02d", tmWk.tm_year   +   1900, tmWk.tm_mon   +   1,  tmWk.tm_mday , tmWk.tm_hour, tmWk.tm_min,tmWk.tm_sec);
}
/*********************end**********************************************/
/*********************创建日志文件***************************************/
int create_log_file()
{
	char bcenter_log_name[256];
	time_t tm1;
	time_t tm2;
	tm2 = time(&tm1);
	struct   tm   tm3;
	memset(&tm3,0,sizeof(tm));
	localtime_r(&tm2, &tm3);
	sprintf(bcenter_log_name,"../log/bcenter-%04d%02d%02d%02d%02d%02d.log",tm3.tm_year + 1900,tm3.tm_mon + 1,tm3.tm_mday,tm3.tm_hour,tm3.tm_min,tm3.tm_sec);//日志格式bcenter-20170204173333.log
	fp_log = fopen(bcenter_log_name,"a");
    if(NULL == fp_log)
    {
	  	return -1;
    }

	return 0;
}
/*********************end**********************************************/
/*********************根据标准时间字符串返回秒数***************************************/
// 2017-08-21 19:47:48
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
/*********************end**********************************************/  
void unixTime2Str(int n, char strTime[], int bufLen)  
{
    snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 时间转换 秒数n[%d] bufLen[%d] line[%d]", __FUNCTION__, n, bufLen, __LINE__);
    writelog(log_buf_);

    time_t time_t_tmp;
    time_t_tmp = n;

    struct tm *tm_tmp;
    tm_tmp = localtime(&time_t_tmp);
    snprintf(strTime, bufLen -1, "%04d-%02d-%02d %02d:%02d:%02d", tm_tmp->tm_year+1900, tm_tmp->tm_mon+1, tm_tmp->tm_mday,
                                                                  tm_tmp->tm_hour, tm_tmp->tm_min, tm_tmp->tm_sec);

/*
    struct tm tm = *localtime((time_t *)&n);
    strftime(strTime, bufLen - 1, "%Y-%m-%d %H:%M:%S", &tm);
    strTime[bufLen - 1] = '\0';
*/
    snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 时间转换 转换后的出场时间(已减去免费时间) strTime[%s] line[%d]", __FUNCTION__, strTime, __LINE__);
    writelog(log_buf_);
}  


bool getLocalIp(std::string dev, std::string & str_ip)
{
    bool result = false;
    struct ifaddrs * ifAddrStruct = NULL;
    void * tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct != NULL)
    {
        if (ifAddrStruct->ifa_addr->sa_family == AF_INET) // check it is IP4
        {
            tmpAddrPtr =
                    &((struct sockaddr_in *) ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer,
            INET_ADDRSTRLEN);
            if (dev == ifAddrStruct->ifa_name)
            {
                str_ip = addressBuffer;
                result = true;
                break;
            }
        }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }
    return result;
}
std::string getLocalIp()
{
    std::string ip_addr="127.0.0.1";
    for(int i = 0;i<10;i++)
    {
        char dev[10]="";
        sprintf(dev, "eth%d", i);
        if(getLocalIp(dev, ip_addr))
        {
            return ip_addr;
        }
    }
    return ip_addr;
}

