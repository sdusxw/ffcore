/****************************************************************************************************************************************
 文件名:boon_condition.cpp
 公司名：山东博昂
 创建人:孙振行
 创建日期：20151111
 简要描述：linux条件变量操作
 修改记录：
 ****************************************************************************************************************************************/
#include "boon_condition.h"
#include <sys/time.h>


namespace common {

BoonCondition::BoonCondition()
{

   pthread_cond_init(&m_condition, NULL);

}

BoonCondition::~BoonCondition()
{

    pthread_cond_destroy(&m_condition);

}

void BoonCondition::Signal()
{

    pthread_cond_signal(&m_condition);

}

void BoonCondition::Wait(BoonMutex* pMutex, unsigned int  milSecs)
{

    struct timespec ts;
    struct timeval tv;
    struct timezone tz;
    int sec, usec;

    if(milSecs == 0)
    {
        pthread_cond_wait(&m_condition, &pMutex->m_mutex);
    }
    else
    {
        gettimeofday(&tv, &tz);
        sec = milSecs / 1000;
        milSecs = milSecs - (sec * 1000);
        usec = milSecs * 1000;
        ts.tv_sec = tv.tv_sec + sec;
        ts.tv_nsec = (tv.tv_usec + usec) * 1000;
        if(ts.tv_nsec > 999999999)
        {
             ts.tv_sec++;
             ts.tv_nsec -= 1000000000;
        }
        pthread_cond_timedwait(&m_condition, &pMutex->m_mutex, &ts);
    }
 
}

}

