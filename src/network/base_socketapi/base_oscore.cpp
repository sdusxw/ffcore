//
// Created by boon on 17-9-22.
//

#include "base_oscore.h"
#include <time.h>

#ifdef WIN32
#include <process.h>
struct pthread_attr_t
{
	LPSECURITY_ATTRIBUTES   security;
	unsigned int            stack;
	unsigned int            initflag;
	unsigned int*           address;
	int                     priority;
};
int pthread_create(pthread_t* thread, pthread_attr_t* attr, pthread_fun_addr fun, void* arg)
{
	uintptr_t p = NULL;
	if (NULL == attr)
	{
		p = ::_beginthreadex(NULL, 0, fun, arg, 0, NULL);
	}
	else
	{
		p = ::_beginthreadex(attr->security, attr->stack, fun, arg, attr->initflag, attr->address);
		if (0 != THREAD_PRIORITY_NORMAL)
		{
			::SetThreadPriority(thread, attr->priority);
		}
	}
	*thread = (pthread_t)p;
	return NULL == p ? -1 : 0;
}

int pthread_join(pthread_t thread, void**ret)
{
	if (WAIT_OBJECT_0 == ::WaitForSingleObject(thread, INFINITE))
	{
		if (NULL != ret)
		{
			DWORD dw;
			if (FALSE != ::GetExitCodeThread(thread, &dw))
			{
				*ret = (void**)dw;
				return 0;
			}
			return -1;
		}
		return 0;
	}
	return -1;
}

#endif

int base_pthread_create_detach(pthread_fun_addr fun, void* arg)
{
    pthread_t pth_id;
#ifdef WIN32
    int ret = pthread_create(&pth_id, NULL, fun, arg);
	if (ret == 0)
	{
		::CloseHandle(pth_id);//
	}
	return ret;
#else

    pthread_attr_t   tmpattr;
    ::pthread_attr_init(&tmpattr);//
    ::pthread_attr_setdetachstate(&tmpattr, PTHREAD_CREATE_DETACHED);//
    int ret = ::pthread_create(&pth_id, &tmpattr, fun, arg);
    ::pthread_attr_destroy(&tmpattr);//
    return ret;
#endif

}

int base_pthread_create(pthread_t* thread, pthread_fun_addr fun, void* arg)
{
    return pthread_create(thread, NULL, fun, arg);
}

extern int base_pthread_join(pthread_t thread)
{
    int ret = pthread_join(thread, NULL);
#ifdef WIN32
    if (ret == 0)
	{
		::CloseHandle(thread);//
	}
#endif
    return ret;
}

void base_pthread_exit()
{
#ifdef WIN32
    ::_endthreadex(0);
#else
    ::pthread_exit(NULL);
#endif
}


base_pthread_id  base_pthread_getid(void)
{
#ifdef WIN32
    return (unsigned long long)::GetCurrentThreadId();
#else
    return (unsigned long long)::pthread_self();
#endif

}


base_mutex::base_mutex()
{
#ifdef WIN32
    _hmutex = ::CreateMutexA(NULL, FALSE, NULL);
#else
    ::pthread_mutexattr_init(&_attr);
    ::pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE); /* make the mutex recursive */
    ::pthread_mutex_init(&_mutex, &_attr);
#endif
}

base_mutex::~base_mutex()
{
#ifdef WIN32
    ::CloseHandle(_hmutex);
#else
    ::pthread_mutex_destroy(&_mutex);
    ::pthread_mutexattr_destroy(&_attr);
#endif
}



bool base_mutex::lock()
{
#ifdef WIN32
    return WAIT_OBJECT_0 == ::WaitForSingleObject(_hmutex, INFINITE);
#else
    return 0 == ::pthread_mutex_lock(&_mutex);
#endif
}


bool base_mutex::unlock()
{
#ifdef WIN32
    return TRUE == ::ReleaseMutex(_hmutex);
#else
    return 0 == ::pthread_mutex_unlock(&_mutex);
#endif
}



bool base_mutex::trylock()
{
#ifdef WIN32
    return WAIT_OBJECT_0 == ::WaitForSingleObject(_hmutex, 0);
#else
    /** returned values :
    *  0, lock ok
    *  EBUSY, The mutex is already locked.
    *  EINVAL, Mutex is not an initialized mutex.
    *  EFAULT, Mutex is an invalid pointer.
    */
    return 0 == ::pthread_mutex_trylock(&_mutex);
#endif
}



base_semaphore::base_semaphore(int initvalue)
{
#ifdef WIN32
    _sema = ::CreateSemaphoreA(
		NULL, // security attributes
		initvalue, // initial count
		0x7fffffff, // maximum value
		NULL); // name
#else
    ::sem_init(&_sem, 0, initvalue);
#endif
}

base_semaphore::~base_semaphore()
{
#ifdef WIN32
    ::CloseHandle(_sema);
#else
    ::sem_destroy(&_sem);
#endif
}

bool base_semaphore::wait()
{
#ifdef WIN32
    return WAIT_OBJECT_0 == ::WaitForSingleObject(_sema, INFINITE);
#else
    return 0 == ::sem_wait(&_sem);
#endif
}

bool base_semaphore::post()
{
#ifdef WIN32
    return  TRUE == ::ReleaseSemaphore(_sema, 1, NULL);
#else
    return 0 == ::sem_post(&_sem);
#endif
}

bool base_semaphore::trywait()
{
#ifdef WIN32
    return WAIT_OBJECT_0 == ::WaitForSingleObject(_sema, 0);
#else
    return 0 == ::sem_trywait(&_sem);
#endif
}

int base_sleep(int  sec, int  msec)
{
#ifdef WIN32
    Sleep(msec + sec * 1000);
	return(0);
#else
    struct timeval  stoptime;
    stoptime.tv_sec = sec + (int)(msec / 1000);
    stoptime.tv_usec = (int)(msec % 1000) * 1000;
    return(select(0, 0, 0, 0, &stoptime));
#endif
}

int base_gettimeofday(timeval *tv)
{
#if defined(WIN32) || defined(WIN64)
    SYSTEMTIME st;
	::GetLocalTime(&st);

	struct tm tm;
	tm.tm_year	= st.wYear - 1900;
	tm.tm_mon	= st.wMonth - 1;
	tm.tm_mday	= st.wDay;
	tm.tm_hour	= st.wHour;
	tm.tm_min	= st.wMinute;
	tm.tm_sec	= st.wSecond;
	tm.tm_isdst	= -1;

	time_t sec_tm = ::mktime(&tm);
	tv->tv_sec	= (long)sec_tm;
	tv->tv_usec	= st.wMilliseconds*1000;
	return 0;
#else
    return gettimeofday(tv,NULL);
#endif
}
