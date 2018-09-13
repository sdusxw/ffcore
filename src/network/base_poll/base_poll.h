//
// Created by boon on 17-9-22.
//

#ifndef TLRS_BASE_POLL_H
#define TLRS_BASE_POLL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "base_headers.h"

#ifndef BASE_POLL_USE_SELECT
#define BASE_POLL_USE_SELECT
#endif

#if defined(WIN32) || defined(WIN64)
#if defined(BASE_POLL_LIB)
#define	BASEPOLL_IMEXPORT __declspec(dllexport)
#else
#define	BASEPOLL_IMEXPORT __declspec(dllimport)
#endif
#else
#define	BASEPOLL_IMEXPORT
#endif

#ifndef BASEPOLL_OK
#define BASEPOLL_OK 0
#endif

#ifndef BASEPOLL_ERR
#define BASEPOLL_ERR -1
#endif

#ifndef BASEPOLL_NONE
#define BASEPOLL_NONE 0
#endif
#ifndef BASEPOLL_READABLE
#define BASEPOLL_READABLE 1
#endif
#ifndef BASEPOLL_WRITABLE
#define BASEPOLL_WRITABLE 2
#endif

#if defined(WIN32) || defined(WIN64)
#ifndef base_fd
#	define base_fd		SOCKET
#endif
#else
#ifndef base_fd
#	define base_fd		int
#endif
#endif

#ifdef __cplusplus
extern "C"{
#endif

typedef struct BasePollFd
{
    base_fd  fd;
    int mask;
    void *data;
} BasePollFd;

typedef struct BasePollMachine
{
    int setsize;
    int size;
    void *state;
    BasePollFd *fired;
} BasePollMachine;

BASEPOLL_IMEXPORT BasePollMachine *BaseCreate(int setsize);
BASEPOLL_IMEXPORT void BaseFree(BasePollMachine *machine);
BASEPOLL_IMEXPORT int BaseAddFd(BasePollMachine *machine, base_fd fd, int mask,void *data);
BASEPOLL_IMEXPORT void BaseDelFd(BasePollMachine *machine, base_fd fd, int mask);

BASEPOLL_IMEXPORT int BasePoll(BasePollMachine *machine,const long ms);
BASEPOLL_IMEXPORT char *BaseName(void);

BASEPOLL_IMEXPORT int BaseWait(base_fd socket_fd, int mask, const long ms, int *retmask);

#ifdef __cplusplus
};
#endif

#endif //TLRS_BASE_POLL_H
