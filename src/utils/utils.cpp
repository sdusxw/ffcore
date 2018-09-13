//
// Created by boon on 17-9-20.
//

#include "utils.h"

#include <sys/time.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <iconv.h>

using  namespace std;

/**
 * @brief: 获取时间
 * @return 时间格式
 */
std::string getTimeUs()
{
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    snprintf(t, sizeof(t), "%04d%02d%02d-%02d%02d%02d.%06ld", 1900 + p->tm_year,
            1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
            tv.tv_usec);
    std::string str = t;
    return str;
}

/**
 * @brief: 获取时间
 * @return 时间格式
 */
std::string getHourMinSec()
{
    char t[256] = {0};
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    snprintf(t, sizeof(t), "%02d:%02d:%02d", p->tm_hour, p->tm_min, p->tm_sec);
    std::string str = t;
    return str;
}

/**
 * @brief: 获取小时，分钟，秒数
 * @param _out_hour : 当前时
 * @param _out_min ：当前分
 * @param _out_sec ：当前秒
 */
void getHms(int& _out_hour, int& _out_min, int& _out_sec)
{
    char t[256] = {0};
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    _out_hour = p->tm_hour;
    _out_min = p->tm_min;
    _out_sec = p->tm_sec;
}

/**
 * @brief: 获取时间
 * @return 时间格式
 */
string getTimeFileName()
{
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    //todo
    snprintf(t, sizeof(t), "%04d%02d%02d_%02d%02d%02d_%06ld", 1900 + p->tm_year,
            1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
            tv.tv_usec);
    std::string str = t;
    return str;
}

/**
 * @brief: 获取时间
 * @return 时间格式 20171226-002123.527
 */
string getTimeFileName2()
{
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    //srand(unsigned(time(NULL)));
    //int a = rand()%(999-100)+100;

    int mod = tv.tv_usec%100;

    //todo
    snprintf(t, sizeof(t), "%04d%02d%02d-%02d%02d%02d.%02d", 1900 + p->tm_year,
             1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
             mod);
    std::string str = t;
    return str;
}

/**
 * @brief: 获取时间
 * @return 时间格式
 */
std::string getTimex(){
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    snprintf(t, sizeof(t), "%04d%02d%02d_%02d%02d", 1900 + p->tm_year,
            1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min);

    std::string str = t;

    return str;
}

/**
 * @brief: 获取时间
 * @return 时间格式
 */
std::string getCurTime()
{
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    snprintf(t, sizeof(t), "%04d-%02d-%02d %02d:%02d:%02d", 1900 + p->tm_year,
             1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    std::string str = t;

    return str;
}

/**
 * @brief: 获取时间
 * @return 时间格式
 */
std::string getYmdHms()
{
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    snprintf(t, sizeof(t), "%04d%02d%02d%02d%02d%02d", 1900 + p->tm_year,
            1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    std::string str = t;

    return str;
}

// 获取年月日 %04d%02d%02d
EXPORT_API std::string getYmd()
{
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    snprintf(t, sizeof(t), "%04d%02d%02d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday);

    std::string str = t;

    return str;
}

/**
 * @brief: 获取时间 xx月xx日
 * @return
 */
std::string getmd()
{
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    snprintf(t, sizeof(t), "%02d月%02d日", 1 + p->tm_mon, p->tm_mday);

    std::string str = t;

    return str;
}

/**
 * @brief: 获取时间 xx时xx分
 * @return
 */
std::string getHm()
{
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    snprintf(t, sizeof(t), "%02d时%02d分", p->tm_hour, p->tm_min);

    std::string str = t;

    return str;
}

/**
 * @brief: 获取时间
 * @return 时间格式
 */
std::string printTime()
{
    char t[256];
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    snprintf(t, sizeof(t), "%04d%02d%02d %02d:%02d:%02d", 1900 + p->tm_year,
            1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    std::string str = t;

    return str;
}

/**
 * @brief: 判断文件/文件夹是否存在
 * @param _file_path
 * @param _mode
 * @return
 */
int isFileExist(const char* _file_path, const int& _mode)
{
    if(_file_path == nullptr)
        return -1;

    if(0 == access(_file_path, F_OK | _mode)){
        //printf("[%s] is ok\n", _file_path);
        return 0;
    }

    return -1;
}

/**
 * @brief: utc时间转换
 * @param _in_sec
 * @param _out_time
 * @param _in_size
 */
void unixTime2Str(int _in_sec, char _out_time[], int _in_size)
{
    time_t time_t_tmp;
    time_t_tmp = _in_sec;
    //time(&time_t_tmp);

    //printf("sec: %ld\n",time_t_tmp);

    struct tm *tm_tmp;
    tm_tmp = localtime(&time_t_tmp);
    snprintf(_out_time, _in_size -1, "%04d-%02d-%02d %02d:%02d:%02d", tm_tmp->tm_year+1900, tm_tmp->tm_mon+1, tm_tmp->tm_mday,
             tm_tmp->tm_hour, tm_tmp->tm_min, tm_tmp->tm_sec);

    //printf("time: %s\n", _out_time);
}

/**
 * @brief: 校验文件扩展名
 * @param _in_file
 * @param _in_ext
 * @param _in_ext_len
 * @return
 */
bool checkFileExt(const std::string& _in_file, const std::string& _in_ext, const int& _in_ext_len/* = 3*/)
{
    if(!_in_file.empty() && !_in_ext.empty() && _in_ext_len > 0){

        int index = _in_file.find_last_of('.');

        if(index != -1){
            std::string ext = _in_file.substr( index+1, _in_ext_len);
            //printf("%s, %s\n", ext.c_str(), _in_ext.c_str());

            if(ext.compare(_in_ext) == 0){
                return true;
            }

            printf("[%s] [%s] error, 没有找到扩展名[%s] line[%d]\n", printTime().c_str(), __FUNCTION__, _in_ext.c_str(), __LINE__);
        }else{
            printf("[%s] [%s] error, 没有找到扩展名 line[%d]\n", printTime().c_str(), __FUNCTION__, __LINE__);
        }
    }

    return false;
}

/**
 * @brief: UNICODE码转为GB2312码
 * @param from_charset
 * @param to_charset
 * @param inbuf
 * @param inlen
 * @param outbuf
 * @param outlen
 * @return
 */
int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset,from_charset);
    if (cd==0) return -1;
    memset(outbuf,0,outlen);
    if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -2;
    iconv_close(cd);
    return 0;
}

/**
 * @brief: UNICODE码转为GB2312码
 * @param inbuf
 * @param inlen
 * @param outbuf
 * @param outlen
 * @return
 */
int unicodeToGb2312(char *inbuf,int inlen,char *outbuf,int outlen)
{
    char utf[] = {"utf-8"};
    char gb2312[] = {"gb2312"};
    return code_convert(utf, gb2312,inbuf,inlen,outbuf,outlen);
}

/**
 * @brief: GB2312码转为UNICODE码
 * @param inbuf
 * @param inlen
 * @param outbuf
 * @param outlen
 * @return
 */
int gb2312ToUnicode(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
    char utf[] = {"utf-8"};
    char gb2312[] = {"gb2312"};
    return code_convert(gb2312, utf, inbuf,inlen,outbuf,outlen);
}