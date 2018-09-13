/****************************************************************************************************************************************
 文件名:boon_mutex.h
 公司名：山东博昂
 创建人:孙振行
 创建日期：20151111
 简要描述：互斥锁
 修改记录：
 ****************************************************************************************************************************************/
#include "boon_mutex.h"

namespace common {


BoonMutex::BoonMutex()
{
    pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&m_mutex, &attr);
    pthread_mutexattr_destroy(&attr);

}

BoonMutex::~BoonMutex()
{
    pthread_mutex_destroy(&m_mutex);
}

void BoonMutex::Lock()
{
    pthread_mutex_lock(&m_mutex);
}

void BoonMutex::Unlock()
{
    pthread_mutex_unlock(&m_mutex);
}




BoonMutexLocker::BoonMutexLocker(BoonMutex* pMutex) : m_pMutex(pMutex)
{
    if(m_pMutex != NULL)
        m_pMutex->Lock();
}

BoonMutexLocker::~BoonMutexLocker()
{
    if(m_pMutex != NULL)
        m_pMutex->Unlock();
}

void BoonMutexLocker::Lock()
{
    if(m_pMutex != NULL)
        m_pMutex->Lock();
}

void BoonMutexLocker::Unlock()
{
    if(m_pMutex != NULL)
       m_pMutex->Unlock();
}

} 

