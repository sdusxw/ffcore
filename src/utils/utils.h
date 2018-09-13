/*********************************************************************************
  *Copyright(C):        sdboon.com
  *FileName:            utils.h
  *Author:              diaoguangqiang
  *Version:             2.0
  *Date:                2017.09.20
  *Description:         工具类
  *                         1. 文件操作
  *                         2. 时间操作
  *History:             (修改历史记录列表，每条修改记录应包含修改日期、修改者及修改内容简介)
     1. Date:           2017.09.20
        Author:         diaoguagnqiang
        Modification:   首次生成文件
     2. Data:
        Author:
        Modification:
**********************************************************************************/
#ifndef TLRS_UTILS_H
#define TLRS_UTILS_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <string>
//using namespace std;
#include <stdio.h>
#include <string>
#include <unistd.h>

#if defined(WIN32)
#ifdef _BASE_SOCKET_
#define	EXPORT_API __declspec(dllimport)
#else
#define	EXPORT_API __declspec(dllexport)
#endif
#else // linux
#define EXPORT_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 获取时间
EXPORT_API std::string getTimeUs();

// 获取时间，以文件名的方式
EXPORT_API std::string getTimeFileName();

// 获取时间，以文件名的方式 20171226-002123.527
EXPORT_API std::string getTimeFileName2();

// 获取时间
EXPORT_API std::string getTimex();

// 打印日志时间 %04d%02d%02d %02d:%02d:%02d
EXPORT_API std::string printTime();

// 获取时间 %04d-%02d-%02d %02d:%02d:%02d
EXPORT_API std::string getCurTime();

// 获取时间 %04d%02d%02d%02d%02d%02d
EXPORT_API std::string getYmdHms();

// 获取年月日 %04d%02d%02d
EXPORT_API std::string getYmd();

// 获取时间 xx月xx日
EXPORT_API std::string getmd();

// 获取时间 xx时xx分
EXPORT_API std::string getHm();

// 判断文件是否存在
EXPORT_API int isFileExist(const char* _file_path, const int& _mode = F_OK);

// 校验文件扩展名
EXPORT_API bool checkFileExt(const std::string& _in_file, const std::string& _in_ext, const int& _in_ext_len = 3);

// 获取时间格式 10:00:00
EXPORT_API std::string getHourMinSec();

EXPORT_API void unixTime2Str(int _in_sec, char _out_time[], int _in_size);

// 转换编码
EXPORT_API int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen);

//UNICODE码转为GB2312码
EXPORT_API int unicodeToGb2312(char *inbuf,int inlen,char *outbuf,int outlen);

//GB2312码转为UNICODE码
EXPORT_API int gb2312ToUnicode(char *inbuf,size_t inlen,char *outbuf,size_t outlen);

// 获取小时，分钟，秒数
EXPORT_API void getHms(int& _out_hour, int& _out_min, int& _out_sec);

#ifdef __cplusplus
    };
#endif


#endif //TLRS_UTILS_H
