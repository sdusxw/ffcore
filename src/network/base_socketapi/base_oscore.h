//
// Created by boon on 17-9-22.
//

#ifndef TLRS_BASE_OSCORE_H
#define TLRS_BASE_OSCORE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(WIN32) || defined(WIN64)
#if defined(_BASE_SOCKET_)
#define	BASESOCKET_IMEXPORT __declspec(dllexport)
#else
#define	BASESOCKET_IMEXPORT __declspec(dllimport)
#endif
#else
#define	BASESOCKET_IMEXPORT
#endif

#if defined(WIN32) || defined(WIN64)
#  include <windows.h>

typedef HANDLE          pthread_t;
typedef DWORD           pthread_id_t;
typedef pthread_id_t     base_pthread_id;
typedef unsigned(__stdcall * pthread_fun_addr) (void *);

#else //linux
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>
typedef pthread_t base_pthread_id;
typedef void *(*pthread_fun_addr)(void*);
#endif

#ifdef __cplusplus
extern "C" {
#endif

BASESOCKET_IMEXPORT int base_pthread_create_detach(pthread_fun_addr fun, void* arg);

BASESOCKET_IMEXPORT int base_pthread_create(pthread_t* thread, pthread_fun_addr fun, void* arg);

BASESOCKET_IMEXPORT int base_pthread_join(pthread_t thread);

BASESOCKET_IMEXPORT void base_pthread_exit();

BASESOCKET_IMEXPORT base_pthread_id  base_pthread_getid(void);//

BASESOCKET_IMEXPORT int base_sleep(int  sec, int  msec);

BASESOCKET_IMEXPORT int base_gettimeofday(timeval *tv);

#ifdef __cplusplus
};
#endif

class BASESOCKET_IMEXPORT base_mutex
{
public:
    base_mutex();
    ~base_mutex();
    bool lock();
    bool unlock();
    bool trylock();

private:
#ifdef WIN32
    HANDLE _hmutex;
#else
    pthread_mutex_t _mutex;
    pthread_mutexattr_t _attr;
#endif
};

class BASESOCKET_IMEXPORT base_auto_lock
{
public:
    base_auto_lock(base_mutex *mux) :_mux(mux)
    {
        _mux->lock();
    };
    ~base_auto_lock()
    {
        _mux->unlock();
    };
private:
    base_mutex *_mux;
};

class BASESOCKET_IMEXPORT base_semaphore
{
public:
    base_semaphore(int initvalue);
    ~base_semaphore();
    bool wait();
    bool post();
    bool trywait();
private:
#ifdef WIN32
    HANDLE _sema;
#else
    sem_t _sem;
#endif

};

#endif //TLRS_BASE_OSCORE_H
