//
// Created by boon on 17-12-12.
//

#include "utils.h"
#include <string.h>
#define OUTLEN 255

using namespace std;

void fun1();

void fun2();

void fun3();

void fun4();

void fun5();

void writeDataToLog( const unsigned char *_pBuf, const int& _len, int _type = 0 );

int main()
{
    fun5();

    return 0;
}

void fun5()
{
    char in_gb2312[OUTLEN] = "http://ipark.sdboon.com/ipark/in.php?park_id=&box_ip=192.168.88.20";
    char out[OUTLEN] = {0};

    int rc;

    //gb2312码转为unicode码
    rc = gb2312ToUnicode(in_gb2312,strlen(in_gb2312),out,OUTLEN);
    printf("ret:%d, gb2312-->unicode out=%s\n", rc, out);

    writeDataToLog((unsigned char*)out, strlen(out));
}

void fun4()
{
    std::string content = "临时车一路顺风收费4323元";

    std::string::size_type pos1 = content.find("收费");
//    content.find(费);

    printf("len: %ld\n", content.length());

    if(pos1 != std::string::npos){
        printf("找到收费　:%ld\n", pos1);
    }else{
        printf("no 收费\n");
    }

    std::string str = content.substr(0, pos1);
    printf("str: %s\n", str.c_str());

    std::string::size_type pos2 = content.find("元");
    if(pos2 != std::string::npos){
        printf("找到元　:%ld\n", pos2);
    }else{
        printf("no 元\n");
    }

    int i = pos2-pos1-strlen("元");
    printf("pos2: %ld, pos1: %ld, strlen(元): %ld, i: %d\n", pos2, pos1, strlen("元"), i);

    std::string str2 = content.substr(pos1+strlen("收费"), pos2-(pos1+strlen("收费")));
    printf("str2:%s\n", str2.c_str());

}

void fun3()
{
    //char *in_utf8 = "临时车";
    char in_utf8[] = {"临时车"};
    char in_gb2312[OUTLEN] = "车牌识别";
    char out[OUTLEN] = {0};

    printf("%s, len[%ld]\n", in_gb2312, strlen(in_gb2312));
    int rc;
    //unicode码转为gb2312码
    rc = unicodeToGb2312(in_utf8, strlen(in_utf8), out, sizeof(out));
    printf("ret:%d, unicode-->gb2312 out=%s\n", rc, out);

    writeDataToLog((unsigned char*)out, strlen(out));

    //gb2312码转为unicode码
    //rc = g2u(in_gb2312,strlen(in_gb2312),out,OUTLEN);
    //printf("ret:%d, gb2312-->unicode out=%s\n", rc, out);

    //writeDataToLog((unsigned char*)out, strlen(out));
}

void fun1()
{
    int sec = 1513064088; // 2017/12/12 15:34:48

    printf("sec: %d, time: 2017/12/12 15:34:48\n", sec);

    char time[24] = {0};

    //printf("sec: %d, sec - 180: %d, 2017/12/12 15:31:48\n", sec, sec - 60*3);

    unixTime2Str(sec - 60*3, time, sizeof(time));

    printf("time: %s\n", time);
}

void writeDataToLog( const unsigned char *_pBuf, const int& _len, int _type/* = 0*/ )
{
    if ( !_pBuf || _len <= 0 )	return ;

    char buf[10*1024] = {0};

    for ( int i = 0; i < _len; i++ )
    {
        sprintf( ( buf + ( i * 5 ) ), " [%02X]", *( _pBuf + i ) );			// 5 * 200 > 1024
    }

    if ( 0 == _type )
    {
        printf("[%s] RECV <<<<%s len[%d] line[%d]\n", __FUNCTION__, buf, _len, __LINE__ );
    }
    else
    {
        printf("[%s] SEND >>>>%s len[%d] line[%d]\n", __FUNCTION__, buf, _len, __LINE__ );
    }

    return;
}
