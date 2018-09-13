/****************************************************************************************************************************************
 文件名:boon_condition.h
 公司名：山东博昂
 创建人:孙振行
 创建日期：20151111
 简要描述：linux条件变量操作
 修改记录：
 ****************************************************************************************************************************************/
#ifndef	__BOONCONDITION_H__
#define __BOONCONDITION_H__

#include <pthread.h>
#include "boon_mutex.h"

namespace common
{
class BoonCondition
{
public:
	BoonCondition();
	virtual ~BoonCondition();
	void Signal();
	void Wait(BoonMutex* pMutex,unsigned int milSecond = 0);
	
private:

   pthread_cond_t		m_condition;

};

}

#endif

