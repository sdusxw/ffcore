/****************************************************************************************************************************************
 文件名:boonmutex.h
 公司名：山东博昂
 创建人:孙振行
 创建日期：20151111
 简要描述：互斥锁
 修改记录：
 ****************************************************************************************************************************************/
#ifndef __BOONMUTEX_H__
#define __BOONMUTEX_H__

#include <pthread.h>
#include <stdio.h>

namespace common
{

class BoonMutex
{
public:
	BoonMutex();
	virtual ~BoonMutex();
	void Lock();
	void Unlock();
	
private:
	pthread_mutex_t 	m_mutex;
	friend class BoonCondition;
};

class BoonMutexLocker
{
public:
	BoonMutexLocker(BoonMutex* pMutex);
	~BoonMutexLocker();
	void Lock();
	void Unlock();

private:
	BoonMutex*			m_pMutex;
};
}

#endif

