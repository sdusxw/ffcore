//
// Created by boon on 17-9-22.
//



#include "base_poll.h"
#include "bselect.cpp"
//#include "bkqueue.cpp"
//#include "bpoll.cpp"
//#include "bepoll.cpp"
//#include "bevport.cpp"

BasePollMachine *BaseCreate(int setsize)
{
    BasePollMachine *machine = new BasePollMachine;
    machine->fired = new BasePollFd[setsize];
    machine->state = NULL;
    machine->setsize = setsize;
    machine->size = 0;
    if ( BaseStateCreate(machine) == BASEPOLL_ERR)
    {
        delete machine->fired;
        delete machine;
        return NULL;
    }
    return machine;
};

void BaseFree(BasePollMachine *machine)
{
    BaseStateFree(machine);
    delete machine->fired;
    delete machine;
};

#ifdef BASE_POLL_USE_SELECT
static int Base_Select(base_fd socket_fd,int mask, const long ms, int *retmask)
{
	struct timeval tv = {0},*ptv = NULL;
	if ( ms != -1)
	{
		tv.tv_sec	= ms/1000;
		tv.tv_usec	= (ms%1000)*1000;
		ptv = &tv;
	}
	fd_set fr,fs;
	FD_ZERO(&fr);
	FD_ZERO(&fs);
	fd_set*	fdr = NULL;
	fd_set*	fds = NULL;
	if (mask & BASEPOLL_READABLE)
	{
		FD_SET(socket_fd, &fr);
		fdr	= &fr;
	}
	if (mask & BASEPOLL_WRITABLE)
	{
		FD_SET(socket_fd, &fs);
		fds	= &fs;
	}
	int sret = ::select(socket_fd + 1, fdr, fds, NULL, ptv);
	if ( sret < 0)
	{
#if defined(WIN32) || defined(WIN64)
		int err = WSAGetLastError();
		if ( err != WSAEINTR )
#else
		int err = errno;
		if ( err != EINTR )
#endif
		{
			return BASEPOLL_ERR;
		}
		return BASEPOLL_OK;
	}
	if (sret == 0)
	{
		return BASEPOLL_OK;
	}

	if (mask & BASEPOLL_READABLE)
	{
		if ( FD_ISSET(socket_fd,fdr)  )
		{
			*retmask |= BASEPOLL_READABLE;
		}
	}
	if (mask & BASEPOLL_WRITABLE)
	{
		if( FD_ISSET(socket_fd, fds) )
		{
			*retmask |= BASEPOLL_WRITABLE;
		}
	}
	return sret;
}
#else
#include <string.h>
#if defined(WIN32) || defined(WIN64)
#if  _WIN32_WINNT < 0x0600
#error Windows version lower vista
#endif
#else
#include <poll.h>
#endif

static int Base_Poll(base_fd socket_fd,int mask, const long ms, int *retmask)
{
    struct pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = socket_fd;
    if (mask & BASEPOLL_READABLE)
    {
        pfd.events |= POLLIN;
    }

    if (mask & BASEPOLL_WRITABLE)
    {
        pfd.events |= POLLOUT;
    }
#if defined(WIN32) || defined(WIN64)
    int  retval = WSAPoll(&pfd, 1, ms);
#else
    int  retval = poll(&pfd, 1, ms);
#endif
    if ( retval > 0 )
    {
        if (pfd.revents & POLLIN)
        {
            *retmask |= BASEPOLL_READABLE;
        }
        if (pfd.revents & POLLOUT)
        {
            *retmask |= BASEPOLL_WRITABLE;
        }
        if (pfd.revents & POLLERR)
        {
            *retmask |= BASEPOLL_READABLE;
            *retmask |= BASEPOLL_WRITABLE;
        }
        if (pfd.revents & POLLHUP)
        {
            *retmask |= BASEPOLL_READABLE;
            *retmask |= BASEPOLL_WRITABLE;
        }
    }
    else if(retval == 0 )
    {
        return BASEPOLL_OK;
    }
    else
    {

#if defined(WIN32) || defined(WIN64)
        int err = WSAGetLastError();
		if ( err != WSAEINTR )
#else
        int err = errno;
        if ( err != EINTR )
#endif
        {
            return BASEPOLL_ERR;
        }
        return BASEPOLL_OK;
    }
    return retval;
}
#endif

int BaseWait(base_fd socket_fd,int mask, const long ms, int *retmask)
{
	*retmask = 0;
#ifdef BASE_POLL_USE_SELECT
	return Base_Select(socket_fd,mask, ms, retmask);
#else
	return Base_Poll(socket_fd,mask, ms, retmask);
#endif
}

