//
// Created by boon on 17-9-22.
//

#include "base_socketapi.h"

#ifndef BASE_SELECT_WAIT_TIME
#define BASE_SELECT_WAIT_TIME (100)
#endif

base_udp ::base_udp():_socket(-1),_blocked(false)
{
    BASE_ZERO_VALUE(_peer_address);
    BASE_ZERO_VALUE(_local_address);
};

base_udp ::~base_udp()
{
    close();
};

bool base_udp :: create( bool broadcast,bool block)
{
    _socket = base_udp_create();
    if (  !base_valid_socket(_socket) )
    {
        printf("%s error, %d\n", __FUNCTION__, __LINE__);
        return false;
    }

    if ( broadcast)
    {
        base_set_broadcast(_socket,broadcast);
    }

    _blocked = block;

    if (!_blocked)
    {
        //printf("[%s] error, %d\n", __FUNCTION__, __LINE__);
        base_set_block(_socket,_blocked);
    }
    return	true;
}

bool base_udp :: close()
{
    if (  base_valid_socket(_socket) )
    {
        base_close(_socket);
    }
#if defined(WIN32) || defined(WIN64)
    _socket = INVALID_SOCKET;
#else
    _socket = -1;
#endif
    _blocked = false;
    return true;
}

bool base_udp :: bind(unsigned short port,char *ip)
{
    base_make_address(&_local_address, port,ip);
    //
    int ret = ::bind( _socket, (struct sockaddr*) &_local_address, (_socklen_t) sizeof(struct sockaddr_in) );
    if ( ret < 0 )
    {
        base_close(_socket);
        return false;
    }
    return true;
}

bool base_udp :: connect(unsigned short port,char *ip)
{
    base_make_address(&_peer_address, port,ip);

    int connected = ::connect( _socket, (struct sockaddr*) &_peer_address, (_socklen_t) sizeof(struct sockaddr_in) );
    if ( connected < 0 )
    {
        base_close(_socket);
        return false;
    }

    errno = 0;
    int ret = ::send(_socket, (const char*)NULL, 0, 0);
    if ( ret < 0 )
    {
#if defined(WIN32) || defined(WIN64)
        int err = WSAGetLastError();
		if( err==0 || err==WSAEWOULDBLOCK || err==WSAEINTR )
#else
        int err = errno;
        if( err== 0 ||err==EWOULDBLOCK || err==EINTR )
#endif
        {
            return true;
        }
        else
        {
            base_close(_socket);
            return false;
        }

    }
    return true;
}

int base_udp::recvfrom(void *buf, size_t len, int flags, struct sockaddr_in *addr)
{
    if (NULL != addr)
    {
        _socklen_t  addr_len = (_socklen_t)sizeof(struct sockaddr_in);
        return  ::recvfrom(_socket, (char*)buf, len, flags, (struct sockaddr*)addr, &addr_len);
    }
    else
    {
        return  ::recv(_socket, (char*)buf, len, flags);
    }
}

int base_udp::sendto(const void *buf, size_t len, int flags, const struct sockaddr_in *addr)
{
    if (NULL != addr)
    {
        //printf("%s %d\n", __FUNCTION__, __LINE__);
        _socklen_t  addr_len = 	addr_len = (_socklen_t)sizeof(struct sockaddr_in);
        return  ::sendto(_socket, (const char*)buf, len, flags, (struct sockaddr*)addr, addr_len);
    }
    else
    {
        //printf("%s %d\n", __FUNCTION__, __LINE__);

        return  ::send(_socket, (char*)buf, len, flags);
    }
}

int base_udp::broadcast(void *buf, size_t len, unsigned short port)
{

    if (port != 0 && port != ntohs(this->_peer_address.sin_port))
    {
        base_make_address(&(this->_peer_address), port, "255.255.255.255");
    }
    return this->send(buf,len, &this->_peer_address);
}

int base_udp::recv(void *buf,unsigned int len, struct sockaddr_in *addr )
{
    int  rxn=0;
    char *ptr=(char *)buf;

    if( len <= 0 )
    {
        return 0;
    }
    if( !base_valid_socket(_socket) )
    {
        return BASE_ERR_SOCKET;
    }
#if defined(WIN32) || defined(WIN64)
    int flag = 0;
#else
    int flag = MSG_TRUNC;
    errno = 0;
#endif
    rxn = this->recvfrom(ptr,len,flag,addr);
    if(rxn < 0)
    {

#if defined(WIN32) || defined(WIN64)
        int err = WSAGetLastError();
		if(err==0 || err==WSAEWOULDBLOCK || err==WSAEINTR || err==WSAECONNRESET )
#else
        int err = errno;
        if( err==0 || err==EWOULDBLOCK || err==EINTR ||err == EAGAIN || err==ECONNRESET)
#endif
        {
            return 0;
        }
#if defined(WIN32) || defined(WIN64)
            else if( err == WSAEMSGSIZE )
#else
        else if( err == EMSGSIZE )
#endif
        {
            return BASE_ERR_BUFLEN;
        }
        else
        {
            return BASE_ERR_SOCKET;
        }
    }
    else
    {
        if ( (unsigned int)rxn > len )
        {
            return BASE_ERR_BUFLEN;
        }
    }
    return rxn;
}

int base_udp:: send(void *buf,unsigned int len,const struct sockaddr_in *addr)
{
    char *ptr=(char *)buf;

    if( len <= 0 )
    {
        return 0;
    }
    if( !base_valid_socket(_socket) )
    {
        printf("%s error, %d\n", __FUNCTION__, __LINE__);
        return BASE_ERR_SOCKET;
    }
    errno = 0;
    int txn=this->sendto(ptr,len,0,addr);
    if(txn<0)
    {

#if defined(WIN32) || defined(WIN64)
        int err = WSAGetLastError();
		if (err == 0 || err == WSAEWOULDBLOCK  || err == WSAEINTR || err==WSAECONNRESET )
#else
        int err = errno;
        if (err == 0 || err == EWOULDBLOCK || err == EAGAIN || err == EINTR || err==ECONNRESET)
#endif
        {
            return 0;
        }
#if defined(WIN32) || defined(WIN64)
            else if( err == WSAEMSGSIZE )
#else
        else if( err == EMSGSIZE )
#endif
        {
            return BASE_ERR_BUFLEN;
        }
        else
        {
            printf("%s error, %d\n", __FUNCTION__, __LINE__);
            return BASE_ERR_SOCKET;
        }
    }
    return txn;
}

int base_udp::recv(void *buf,unsigned int len,  char *ip,int iplen,int &port)
{
    struct sockaddr_in  remote_addr;
    BASE_ZERO_VALUE(remote_addr);
    int ret =  recv(buf,len,&remote_addr);
    base_parse_address(&remote_addr,ip,iplen,&port);
    return ret;
};

int base_udp::send(void *buf,unsigned int len, char *ip,int port)
{
    struct sockaddr_in  remote_addr;
    BASE_ZERO_VALUE(remote_addr);
    base_make_address(&remote_addr,port,ip);
    return send(buf,len,&remote_addr);
};

int base_udp::poll(int mask, const long ms, int &retmask)
{
    return BaseWait(_socket,mask,ms,&retmask);
}

int base_udp::read(void *buf,unsigned int len, struct sockaddr_in *addr/* = NULL*/)
{
    int  rxn=0;
    char *ptr=(char *)buf;

    if( len <= 0 )
    {
        return 0;
    }
    if( !base_valid_socket(_socket) )
    {
        printf("%s error, %d\n", __FUNCTION__, __LINE__);
        return BASE_ERR_SOCKET;
    }

    //
    struct timeval tv;
    tv.tv_sec	= 0;
    tv.tv_usec	= BASE_SELECT_WAIT_TIME;
    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(_socket, &fs);
    fd_set*	fdr	= &fs;
    int sret = ::select(_socket + 1, fdr, NULL, NULL, &tv);
    if ( sret < 0)//
    {
        printf("%s error, %d\n", __FUNCTION__, __LINE__);
        return BASE_ERR_SOCKET;
    }
    if (sret == 0)//
    {
        return 0;
    }
    //
    if ( FD_ISSET(_socket,fdr)  )//
    {
#ifdef WIN32
        int flag = 0;
#else
        int flag = MSG_TRUNC;
#endif
        rxn = this->recvfrom(ptr,len,flag,addr);//
        if(rxn <= 0)
        {

#ifdef WIN32
            int err = WSAGetLastError();
			if( err==WSAEWOULDBLOCK || err==WSAEINTR )//
#else
            int err = errno;
            if( err==EWOULDBLOCK || err==EINTR ||err == EAGAIN)//
#endif
            {
                return 0;//
            }
#ifdef WIN32
                else if( err == WSAEMSGSIZE )
#else
            else if( err == EMSGSIZE )
#endif
            {
                return BASE_ERR_BUFLEN;

            }
            else //
            {
                printf("%s error, %d\n", __FUNCTION__, __LINE__);
                return BASE_ERR_SOCKET;
            }
        }
        else
        {
            if ( rxn > len )
            {
                return BASE_ERR_BUFLEN;
            }
        }

    }
    else
    {
        //
        return BASE_ERR_SOCKET;
    }

    return rxn;
}

int base_udp::read(void *buf,unsigned int len, char *ip,int iplen,int &port)
{
    struct sockaddr_in  remote_addr;
    BASE_ZERO_VALUE(remote_addr);
    int ret =  read(buf,len,&remote_addr);
    //base_parse_address((const struct sockaddr_in )remote_addr,ip,iplen, port);

    int p = port;
    base_parse_address(&remote_addr, ip, iplen, &p);
    return ret;
}

int base_udp::write(void *buf,unsigned int len, const struct sockaddr_in *addr/* = NULL*/)
{
    char *ptr=(char *)buf;

    if( len <= 0 )
    {
        return 0;
    }
    if( !base_valid_socket(_socket) )
    {
        printf("%s error, 无效socket %d\n", __FUNCTION__, __LINE__);

        return BASE_ERR_SOCKET;
    }

    //
    struct timeval tv;
    tv.tv_sec	= 0;
    tv.tv_usec	= BASE_SELECT_WAIT_TIME;//
    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(_socket, &fs);
    fd_set*	fds	= &fs;
    int sret = ::select(_socket + 1, NULL, fds, NULL, &tv);
    if ( sret <= 0)//
    {
        printf("%s error, %d\n", __FUNCTION__, __LINE__);
        return BASE_ERR_SOCKET;
    }
    if (sret == 0)//
    {
        return 0;
    }

    int txn=this->sendto(ptr,len,0,addr);
    if(txn<=0)
    {

#ifdef WIN32
        int err = WSAGetLastError();
		if (err == 0 || err == WSAEWOULDBLOCK  || err == WSAEINTR)
#else
        int err = errno;
        if (err == 0 || err == EWOULDBLOCK || err == EAGAIN || err == EINTR)
#endif
        {
            return 0;//
        }
#ifdef WIN32
            else if( err == WSAEMSGSIZE )
#else
        else if( err == EMSGSIZE )//
#endif
        {
            return BASE_ERR_BUFLEN;
        }
        else
        {
            printf("%s error, sendto failed! %d\n", __FUNCTION__, __LINE__);
            return BASE_ERR_SOCKET;
        }
    }

    //printf("[%s] sendto success len[%d] line[%d]\n", __FUNCTION__, txn, __LINE__);
    return txn;
}

int base_udp::write(void *buf,unsigned int len, char *ip,int port)
{
    struct sockaddr_in  remote_addr;
    BASE_ZERO_VALUE(remote_addr);
    base_make_address(&remote_addr,port,ip);
    return write(buf,len,&remote_addr);
}