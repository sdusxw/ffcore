//
// Created by boon on 17-9-22.
//

#include "base_socketapi.h"

#if defined(WIN32) || defined(WIN64) //

typedef u_long	ioctl_arg_t;
#ifndef SHUT_WR
#define SHUT_WR	SD_SEND
#endif
#ifndef SHUT_RD
#define SHUT_RD	SD_RECEIVE
#endif
#ifndef SHUT_RDWR
#define SHUT_RDWR	SD_BOTH
#endif

#else //

typedef int	ioctl_arg_t;

#endif

#ifndef IPV4_MAX_LEN
#	define IPV4_MAX_LEN	16
#endif

#ifdef WIN_VERSION_LOWER_VISTER
INT base_inet_pton(
	__in                                INT             Family,
	__in                                PCSTR           pszAddrString,
	__out_bcount(sizeof(IN6_ADDR))      PVOID           pAddrBuf
	)
{
	in_addr *p = (in_addr*)pAddrBuf;
	p->S_un.S_addr = ::inet_addr(pszAddrString);
	return 0;
}

PCSTR base_inet_ntop(
	__in                                INT             Family,
	__in                                PVOID           pAddr,
	__out_ecount(StringBufSize)         PSTR            pStringBuf,
	__in                                size_t          StringBufSize
	)
{
	in_addr *p = (in_addr*)pAddr;
	char* pstr = ::inet_ntoa(*p);
	if (NULL == pstr)
	{
		return NULL;
	}
	strcpy_s(pStringBuf, StringBufSize, pstr);
	return pStringBuf;
}
#endif

int base_startup()
{
#if defined(WIN32) || defined(WIN64)
    WSADATA wsa = {0};
	return ::WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    sigset(SIGPIPE,SIG_IGN);
    return 0;
#endif
}

int base_cleanup()
{
#if defined(WIN32) || defined(WIN64)
    return ::WSACleanup();
#else
    return 0;
#endif
}

bool base_valid_socket(base_fd socket_fd)
{
#if defined(WIN32) || defined(WIN64)
    return socket_fd !=	INVALID_SOCKET ;
#else
    return socket_fd >= 0;
#endif

}

int base_get_last_error(base_fd socket_fd)
{
#if defined(WIN32) || defined(WIN64)
    return WSAGetLastError();
#else
    if ( !base_valid_socket(socket_fd) )
    {
        return errno;
    }
    int opt = 0;
    _socklen_t len = (_socklen_t)sizeof(opt);
    getsockopt( socket_fd, SOL_SOCKET, SO_ERROR, (char*)&opt, &len );
    return opt;
#endif
}

bool base_is_error(base_fd socket_fd)
{
    if (! base_valid_socket(socket_fd)) return true;
    int err = base_get_last_error(socket_fd);
#if defined(WIN32) || defined(WIN64)
    return err != 0 && err != WSAEINPROGRESS && err != WSAEALREADY && err != WSAEWOULDBLOCK  && err != WSAEINTR;
#else
    return err != 0 && err != EINPROGRESS && err != EALREADY && err != EWOULDBLOCK && err != EAGAIN && err != EINTR;
#endif
}


bool base_set_block(base_fd socket_fd,bool block)
{

#ifndef FIONBIO
    int opt = fcntl(socket_fd, F_GETFL, 0);
	if (opt < 0) return false;

	if (block)
	{
		opt &= ~O_NONBLOCK;
	}
	else
	{
		opt |= O_NONBLOCK ;
	}
	if ( BASE_CHECK_SOCK_RESULT(fcntl(socket_fd, F_SETFL, opt)) )
	{
		return true;
	}
#else
    ioctl_arg_t arg = (block ? 0 : 1);
#if defined(WIN32) || defined(WIN64)
    if( BASE_CHECK_SOCK_RESULT(ioctlsocket(socket_fd, FIONBIO, &arg)) )
#else
    if( BASE_CHECK_SOCK_RESULT(ioctl(socket_fd, FIONBIO, &arg)) )
#endif
    {
        return true;
    }
#endif
    return false;
}

bool base_set_tcpnodelay(base_fd socket_fd)
{
    int opt = 1;
    _socklen_t size = (_socklen_t) sizeof(opt);
    return BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&opt,size ));
}

bool base_set_reuseaddress(base_fd socket_fd,bool reuse)
{
    int opt = (reuse ? 1 : 0);
#if defined(WIN32) || defined(WIN64)
    return BASE_CHECK_SOCK_RESULT(setsockopt (socket_fd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
		(const char*) &opt, sizeof (opt)));
#else
    return BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, (_socklen_t) sizeof(opt)));
#endif
}

bool base_get_reuseaddress(base_fd socket_fd, bool *_reuse)
{
    int opt = 0;
    _socklen_t size = (_socklen_t) sizeof(opt);
    bool ret = BASE_CHECK_SOCK_RESULT(getsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, &size));
    *_reuse = (opt != 0);
    return ret;
}

bool base_set_recvbuf(base_fd socket_fd,int _size)
{

    return BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, (char*)&_size, (_socklen_t) sizeof(int)));
}

bool base_get_recvbuf(base_fd socket_fd,int *_size)
{

    _socklen_t size = (_socklen_t) sizeof(int);
    return BASE_CHECK_SOCK_RESULT(getsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, (char*)_size, &size));
}

bool base_set_sendbuf(base_fd socket_fd,int _size)
{

    return BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (char*)&_size, (_socklen_t) sizeof(int)));
}

bool base_get_sendbuf(base_fd socket_fd,int *_size)
{

    _socklen_t size = (_socklen_t) sizeof(int);
    return BASE_CHECK_SOCK_RESULT(getsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (char*)_size, &size));
}

bool base_set_linger(base_fd socket_fd,const struct linger *_linger)
{

    return BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, (const char*)_linger, (_socklen_t) sizeof( struct linger)));
}

bool base_get_linger(base_fd socket_fd,struct linger *_linger)
{

    _socklen_t size = (_socklen_t) sizeof(struct linger);
    return BASE_CHECK_SOCK_RESULT(getsockopt(socket_fd, SOL_SOCKET, SO_LINGER, (char*)_linger, &size));
}

bool base_set_opt(base_fd socket_fd,int _level, int _optname, const void* _optval, _socklen_t _size)
{

    return BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, _level, _optname, (const char*)_optval, _size));
}

bool base_get_opt(base_fd socket_fd,int _level, int _optname, void* _optval, _socklen_t* _size)
{

    return BASE_CHECK_SOCK_RESULT(getsockopt(socket_fd, _level, _optname, (char*)_optval, _size));
}

void base_get_address(struct sockaddr_in *addr, base_fd sock_fd)
{
    _socklen_t len = (_socklen_t) sizeof(struct sockaddr_in);
    getsockname(sock_fd, (struct sockaddr *)addr, &len);
}

void  base_make_address(sockaddr_in *_addr, unsigned short _port, const char *_ip)
{
    memset(_addr, 0, sizeof(sockaddr_in));
    _addr->sin_family	= AF_INET;
    _addr->sin_port		= htons(_port);
    if (NULL != _ip)
    {
#ifdef WIN_VERSION_LOWER_VISTER
        base_inet_pton(AF_INET, _ip, &_addr->sin_addr);
#else
        inet_pton(AF_INET, _ip, &_addr->sin_addr);
#endif
    }
    else
    {
        _addr->sin_addr.s_addr	= INADDR_ANY;
    }
}

void base_parse_address(const sockaddr_in *addr, char *ip, int iplen, int *port)
{
#ifdef WIN_VERSION_LOWER_VISTER
    base_inet_ntop(AF_INET, (void*)&(addr->sin_addr), ip, iplen);
#else
    inet_ntop(AF_INET, (void*)&(addr->sin_addr), ip, iplen);
#endif
    *port = ntohs(addr->sin_port);
}

bool base_set_keepalive(base_fd socket_fd,bool keepalive)
{
    int opt = (keepalive ? 1 : 0);

    return BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&opt, (_socklen_t) sizeof(opt)));
}


bool base_set_fast_keepalive(base_fd socket_fd,int _idle, int _interval, int _retry)
{
#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif
    int opt = 1;
#if !(defined(WIN32) || defined(WIN64))
    bool ret = true;
#if defined(SO_KEEPALIVE)
    ret &= BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&opt, (_socklen_t) sizeof(opt)));
#endif
#if defined(TCP_KEEPIDLE)
    ret &= BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, SOL_TCP, TCP_KEEPIDLE, (char*)&_idle , (_socklen_t)sizeof(_idle)));
#endif
#if defined(TCP_KEEPINTVL)
    ret &= BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, SOL_TCP, TCP_KEEPINTVL, (char*)&_interval , (_socklen_t)sizeof(_interval)));
#endif
#if defined(TCP_KEEPCNT)
    ret &= BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, SOL_TCP, TCP_KEEPCNT, (char*)&_retry, (_socklen_t)sizeof(_retry)));
#endif
    return ret;
#else
    int ret = setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&opt, (_socklen_t) sizeof(opt));
	if (0 != ret)
	{
		return false;
	}
	else
	{

		struct tcp_keepalive {
			u_long onoff;
			u_long keepalivetime;
			u_long keepaliveinterval;
		} ;

#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)

		tcp_keepalive alive_in  = {0};
		tcp_keepalive alive_out  = {0};
		alive_in.keepalivetime  = _idle * 1000;
		alive_in.keepaliveinterval = _interval * 1000;
		alive_in.onoff    = 1;
		unsigned long ulBytesReturn = 0;
		ret = ::WSAIoctl(socket_fd, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in), &alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL);
		if (ret != 0)
		{
			return false;
		}
	}
	return true;
#endif
}


bool base_get_keepalive(base_fd socket_fd,bool *_keepalive)
{

    int opt = 0;
    _socklen_t size = (_socklen_t) sizeof(opt);
    bool ret = BASE_CHECK_SOCK_RESULT(getsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt, &size));
    *_keepalive = (opt != 0);
    return ret;
}


bool base_set_broadcast(base_fd socket_fd,bool _broadcast)
{

    int opt = (_broadcast ? 1 : 0);
    if ( BASE_CHECK_SOCK_RESULT(setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, (char*)&opt, (_socklen_t) sizeof(opt))))
    {
        return true;
    }

    printf("%s error, %d\n", __FUNCTION__, __LINE__);
    return false;
}


bool base_get_broadcast(base_fd socket_fd,bool *_broadcast)
{
    int opt = 0;
    _socklen_t size = (_socklen_t) sizeof(opt);
    bool ret = BASE_CHECK_SOCK_RESULT(getsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, (char*)&opt, &size));
    *_broadcast = (opt != 0);
    return ret;
}

bool base_is_port_used(unsigned short port)
{
    return NULL != getservbyport(htons(port), NULL);
}


base_fd base_create(int af, int type, int protocol)
{
    return ::socket(af, type, protocol);
}

base_fd base_tcp_create()
{
    return base_create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

base_fd base_udp_create()
{
    return base_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

bool base_close(base_fd socket_fd)
{
    if ( !base_valid_socket(socket_fd))
    {
        return true;
    }
    //shutdown(socket_fd, SHUT_WR);
#ifdef __INTERIX__
    base_set_block(socket_fd,false);
#endif
#if defined(WIN32) || defined(WIN64)
    return  BASE_CHECK_SOCK_RESULT(::closesocket(socket_fd));
#else
    return  BASE_CHECK_SOCK_RESULT(::close(socket_fd));
#endif
}


bool base_bind(base_fd socket_fd,unsigned short _port,  const char* _ip)
{

    struct sockaddr_in	local_address_;
    base_make_address(&local_address_, _port, _ip);
    return BASE_CHECK_SOCK_RESULT(::bind(socket_fd, (struct sockaddr*) &local_address_, (_socklen_t) sizeof(struct sockaddr_in)));
}

bool base_listen(base_fd socket_fd, int backlog)
{
    return  BASE_CHECK_SOCK_RESULT(::listen(socket_fd, backlog));
}
