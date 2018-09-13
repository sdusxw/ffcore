//
// Created by boon on 17-9-22.
//

#ifdef BASE_POLL_USE_SELECT

#ifndef __B_SELECT_CPP__
#define __B_SELECT_CPP__

#include <string.h>
#include <map>

#include "base_poll.h"
#include "base_headers.h"


typedef struct BaseState {
    fd_set rfds, wfds;
    fd_set _rfds, _wfds;
	base_fd maxfd;
	std::map<base_fd,BasePollFd*> eventmap;
} BaseState;


int BaseStateCreate(BasePollMachine *machine)
{
    BaseState *state = new BaseState;
    if (NULL == state)
	{
		return BASEPOLL_ERR;
	}
    FD_ZERO(&state->rfds);
    FD_ZERO(&state->wfds);
	state->maxfd = 0;
    machine->state = state;
    return BASEPOLL_OK;
}

void BaseStateFree(BasePollMachine *machine)
{
	BaseState *state = (BaseState *)machine->state;
	std::map<base_fd,BasePollFd *>::iterator it = state->eventmap.begin();
	for ( ; it != state->eventmap.end(); it++)
	{
		delete (BasePollFd *)it->second;
	}
	state->eventmap.clear();
    delete state;
}

int BaseAddFd(BasePollMachine *machine, base_fd fd, int mask, void *data)
{
	if (mask == BASEPOLL_NONE)
	{
		return BASEPOLL_ERR;
	}
	if (machine->size == machine->setsize)
    {
		 return BASEPOLL_ERR;
	}
    BaseState *state = (BaseState *)machine->state;

	std::map<base_fd,BasePollFd *>::iterator it = state->eventmap.find(fd);
	if( it == state->eventmap.end() )
	{
		BasePollFd *pfd = new BasePollFd;
		pfd->data = data;
		pfd->fd = fd;
		pfd->mask = mask;
		state->eventmap[fd] = pfd;
		machine->size++;
		if (mask & BASEPOLL_READABLE)
		{
			FD_SET(fd,&state->rfds);
		}
		if (mask & BASEPOLL_WRITABLE)
		{
			FD_SET(fd,&state->wfds);
		}
	}
	else
	{
		if (it->second->mask != mask)
		{
			if (!(it->second->mask & BASEPOLL_READABLE) && mask & BASEPOLL_READABLE)
			{
				FD_SET(fd,&state->rfds);
			}
			if ( !(it->second->mask & BASEPOLL_WRITABLE) && mask & BASEPOLL_WRITABLE)
			{
				FD_SET(fd,&state->wfds);
			}
			it->second->mask |= mask;
		}
	}
	if (fd > state->maxfd)
	{
		state->maxfd = fd;
	}

    return BASEPOLL_OK;
}

 void BaseDelFd(BasePollMachine *machine, base_fd fd, int mask)
{
	if (mask == BASEPOLL_NONE) return;

	BaseState *state = (BaseState *)machine->state;

	std::map<base_fd,BasePollFd *>::iterator it = state->eventmap.find(fd);
	if( it == state->eventmap.end() )
	{
		return;
	}
    if ( it->second->mask & BASEPOLL_READABLE && mask & BASEPOLL_READABLE )
	{
		FD_CLR(fd,&state->rfds);
	}
    if ( it->second->mask & BASEPOLL_WRITABLE && mask & BASEPOLL_WRITABLE )
	{
		FD_CLR(fd,&state->wfds);
	}

	it->second->mask = it->second->mask & (~mask);

	if (it->second->mask == BASEPOLL_NONE )
	{
		delete (BasePollFd *)it->second;
		state->eventmap.erase(it);
		machine->size--;
		if ( machine->size == 0 )
		{
			state->maxfd = 0;
		}
		else
		{
			if (fd == state->maxfd )
			{
				it = state->eventmap.begin();
				for ( ; it != state->eventmap.end(); it++)
				{
					if (it->first > state->maxfd)
					{
						state->maxfd = it->first;
					}
				}
			}
		}

	}
}

 int BasePoll(BasePollMachine *machine, const long ms)
{
	BaseState *state = (BaseState *)machine->state;
	int retval = 0,ret = 0;

	memcpy(&state->_rfds,&state->rfds,sizeof(fd_set));
	memcpy(&state->_wfds,&state->wfds,sizeof(fd_set));

	struct timeval tv = {0},*ptv = NULL;
	if ( ms != -1)
	{
		tv.tv_sec	= ms/1000;
		tv.tv_usec	= (ms%1000)*1000;
		ptv = &tv;
	}

	retval = select(state->maxfd+1,&state->_rfds,&state->_wfds,NULL,ptv);
	if (retval > 0)
	{
		std::map<base_fd,BasePollFd *>::iterator it = state->eventmap.begin();
		int i = 0;
		for ( ; it != state->eventmap.end(); it++)
		{

			if (it->second->mask == BASEPOLL_NONE)
			{
				continue;
			}
			int mask = 0;
			if (it->second->mask & BASEPOLL_READABLE && FD_ISSET(it->first,&state->_rfds))
			{
				mask |= BASEPOLL_READABLE;
			}
			if (it->second->mask & BASEPOLL_WRITABLE && FD_ISSET(it->first,&state->_wfds))
			{
				mask |= BASEPOLL_WRITABLE;
			}

			machine->fired[i].fd = it->first;
			machine->fired[i].mask = mask;
			machine->fired[i].data = it->second->data;
			i++;
			ret++;
		}
	}
	else if ( retval == 0 )
	{
		ret = BASEPOLL_OK;
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
			ret = BASEPOLL_ERR;
			return BASEPOLL_ERR;
		}
		ret = BASEPOLL_OK;
		return BASEPOLL_OK;
	}

    return ret;
}

char *BaseName(void)
{
    return ((char *)"select");
}

#endif  //__B_SELECT_CPP__

#endif //BASE_POLL_USE_SELECT