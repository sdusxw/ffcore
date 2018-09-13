//
// Created by boon on 17-9-22.
//

#include "base_socketapi.h"

base_tcpclient::base_tcpclient(): _connected(false), _socket(-1), _blocked(true)
{
    BASE_ZERO_VALUE(_peer_address);
    BASE_ZERO_VALUE(_local_address);
}

base_tcpclient::~base_tcpclient()
{

}

bool base_tcpclient::connect_block(const char* ip, unsigned short port)
{
    if (_connected)
    {
        return true;
    }
    _socket = base_tcp_create();
    if ( !base_valid_socket(_socket) )
    {
        return false;
    }

    base_make_address(&_peer_address, port, ip);


    _connected = BASE_CHECK_SOCK_RESULT(::connect(_socket, (struct sockaddr*)&_peer_address, (_socklen_t)sizeof(struct sockaddr_in)));
    if( _connected )
    {

        base_set_tcpnodelay(_socket);
        base_get_address(&_local_address, _socket);
    }
    else
    {
        close();
    }

    return _connected;
}

bool base_tcpclient::connect(const char* ip, unsigned short port, int ms)
{
    int sret = 0;
    int ret = 0;
    int retmask = 0;

    int opt = 0;
    _socklen_t len = (_socklen_t)sizeof(opt);
    int optret = 0;

    if (_connected)
    {
        //printf("[%s] .... line[%d]\n", __FUNCTION__, __LINE__);
        return true;
    }

    _blocked = false;

    _socket = base_tcp_create();
    if ( !base_valid_socket(_socket) )
    {
        //printf("[%s] .... line[%d]\n", __FUNCTION__, __LINE__);
        return false;
    }

    base_make_address(&_peer_address, port, ip);

    if (! base_set_block(_socket,false))
    {
        close();
        return false;
    }


    errno = 0;
    ret = ::connect(_socket, (struct sockaddr*)&_peer_address, (_socklen_t)sizeof(struct sockaddr_in));
    if ( ret == 0 )
    {
        _connected = true;
        //printf("[%s] ....con[%d] line[%d]\n", __FUNCTION__, _connected, __LINE__);
        goto __end;
    }
#if defined(WIN32) || defined(WIN64)
    if ( ret  == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK )
	{
		_connected = false;
		goto __end;
	}
#else
    if ( ret  < 0 && errno != EINPROGRESS )
    {
        _connected = false;
        goto __end;
    }
#endif
    sret = poll(BASEPOLL_READABLE|BASEPOLL_WRITABLE ,ms ,retmask);
    if ( sret  <= 0 )
    {
        _connected = false;
        goto __end;
    }


    optret = getsockopt( _socket, SOL_SOCKET, SO_ERROR, (char*)&opt, &len );
    if (optret == 0 && opt == 0)
    {
        _connected = true;
    }
    else
    {
        _connected = false;
    }


    __end:
    if ( _connected )
    {
        base_get_address(&_local_address, _socket);

        base_set_tcpnodelay(_socket);

    }
    else
    {
        close();
    }

    //printf("[%s] ....con[%d] line[%d]\n", __FUNCTION__, _connected, __LINE__);
    return _connected;
}

bool base_tcpclient::close(bool rst)
{
    bool ret = false;
    if ( base_valid_socket(_socket) )
    {
        if(rst)
        {
            struct linger tmplinger;
            tmplinger.l_onoff = 1;
            tmplinger.l_linger = 0;
            base_set_linger(_socket,&tmplinger);
        }
        ret =  base_close(_socket);
    }
    if(ret)
    {
        _connected = false;
#if defined(WIN32) || defined(WIN64)
        _socket = INVALID_SOCKET;
#else
        _socket = -1;
#endif
    }
    return ret;
}


int base_tcpclient::poll(int mask, const long ms, int &retmask)
{
    return BaseWait(_socket,mask,ms,&retmask);
}

int base_tcpclient::recv(void *buf,unsigned int len)
{
    errno = 0;
    int rlen = ::recv(_socket,(char *)buf,len,0);
    if(rlen <= 0)
    {
        if(rlen == 0)
        {
            return BASE_ERR_SOCKET;
        }

#if defined(WIN32) || defined(WIN64)
        int err = WSAGetLastError();
		if( (err==WSAEWOULDBLOCK && !_blocked) || err==WSAEINTR )
#else
        int err = errno;
        if( ( (err==EWOULDBLOCK || err == EAGAIN) && !_blocked ) || err==EINTR )
#endif
        {
            return 0;
        }
        else
        {
            return BASE_ERR_SOCKET;
        }
    }

    return rlen;
}

int base_tcpclient:: send(void *buf,unsigned int len)
{
    errno = 0;
    int slen=::send(_socket,(char *)buf,len,0);
    if(slen<0)
    {

#if defined(WIN32) || defined(WIN64)
        int err = WSAGetLastError();
		if(  (err==WSAEWOULDBLOCK && !_blocked) || err==WSAEINTR  )
#else
        int err = errno;
        if( ( (err==EWOULDBLOCK || err == EAGAIN) && !_blocked ) || err==EINTR )
#endif
        {
            return 0;
        }
        else
        {
            return BASE_ERR_SOCKET;
        }
    }
    return slen;
}


void base_tcpclient::attach(base_fd socket_fd,struct sockaddr_in &peer_addr,bool block)
{
    this->_socket = socket_fd;
    BASE_ZERO_VALUE(_peer_address);
    BASE_ZERO_VALUE(_local_address);
    base_get_address(&_local_address, socket_fd);
    _peer_address = peer_addr;
    if(base_set_block(_socket,block))
    {
        _blocked = block;
    };

    base_set_tcpnodelay(_socket);
    _connected = true;
}

bool base_tcpclient::isconnected()
{
    if( _connected && base_valid_socket(_socket)  )
    {
        return true;
    }
    return false;
}

int base_tcpclient::recvn(void *buf,unsigned int len,int retry)
{
    if ( !isconnected() )
    {
        return BASE_ERR_SOCKET;
    }
    int  rxn,rxntime=0;
    char *ptr=(char *)buf;
    int  rxnum=0;

    if( len <= 0 )
    {
        return 0;
    }

    while(true)
    {
        if( rxntime > retry )
        {
            return rxnum;
        }
        int readable = BASEPOLL_NONE;
        int sret = poll(BASEPOLL_READABLE,BASE_WAIT_TIME_MS, readable);
        if ( sret ==  BASEPOLL_ERR)
        {
            close();
            return BASE_ERR_SOCKET;
        }
        if (sret == BASEPOLL_OK)
        {
            ++rxntime;
            continue;
        }

        if ( readable & BASEPOLL_READABLE )
        {
            rxn = recv(ptr,len);
            if(rxn<=0)
            {
                if( rxn == BASE_ERR_SOCKET )
                {
                    close();
                    return BASE_ERR_SOCKET;
                }
                else
                {
                    ++rxntime;
                    continue;
                }
            }
            rxnum += rxn;
            ptr += rxn;
            len -= rxn;
            if(len <= 0) break;
        }
        else
        {

            close();
            return BASE_ERR_SOCKET;
        }
    }
    return rxnum;
}

int base_tcpclient:: sendn(void *buf,unsigned int len,int retry)
{
    if ( !isconnected() )
    {
        return BASE_ERR_SOCKET;
    }
    int txn,txntime=0;
    char *ptr=(char *)buf;
    int txnum=0;

    if( len <= 0 )
    {
        return 0;
    }

    while(true)
    {
        if( txntime > retry )
        {
            return txnum;
        }

        int writeable = BASEPOLL_NONE;
        int sret = poll(BASEPOLL_WRITABLE, BASE_WAIT_TIME_MS, writeable);
        if (sret == BASEPOLL_ERR)
        {
            close();
            return BASE_ERR_SOCKET;
        }
        if (sret == BASEPOLL_OK )
        {
            ++txntime;
            continue;
        }

        txn=send(ptr,len);
        if(txn<=0)
        {
            if( txn == BASE_ERR_SOCKET )
            {
                close();
                return BASE_ERR_SOCKET;
            }
            else
            {
                ++txntime;
                continue;
            }
        }
        txnum += txn;
        ptr += txn;
        len -= txn;
        if(len <= 0 ) break;
    }
    return txnum;
}

base_tcpserver::base_tcpserver(bool block_newcli):_listened(false), _socket(-1), _blocked_listen(true),_block_newcli(block_newcli)
{
    BASE_ZERO_VALUE(_local_address);
}

base_tcpserver::~base_tcpserver()
{

}

bool base_tcpserver::listen(bool block_listen,unsigned short port,char *ip, int backlog)
{
    if (_listened)
    {
        return true;
    }

    _socket = base_tcp_create();
    if ( !base_valid_socket(_socket) )
    {
        return false;
    }

    base_set_reuseaddress(_socket,true);

    base_set_block(_socket,block_listen);

    base_make_address(&_local_address, port,ip);

    int ret = ::bind( _socket, (struct sockaddr*) &_local_address, (_socklen_t) sizeof(struct sockaddr_in) );
    if ( ret < 0 )
    {
        base_close(_socket);
        return false;
    }
    //printf("[%s] bind [%d]\n", __FUNCTION__, __LINE__);

    if ( !base_listen(_socket, backlog) )
    {
        base_close(_socket);
        return false;
    }
    printf("listen %d\n", port);

    _blocked_listen = block_listen;
    _listened = true;
    return true;
}

int base_tcpserver::accept(base_tcpclient_object *client,int ms)
{
    if ( !_listened )
    {
        return BASE_ERR_SOCKET;
    }
    struct sockaddr_in	peer_address;
    BASE_ZERO_VALUE(peer_address);
    base_fd socket_fd = -1;
    int ret = accept(socket_fd, peer_address,ms);

    if(  ret > 0 )
    {
        //printf("[%s] fd[%d] ip[%s] port[%d] ret[%d] [%d]\n", __FUNCTION__, socket_fd, inet_ntoa(peer_address.sin_addr), peer_address.sin_port, ret, __LINE__);
        client->attach(socket_fd,peer_address,this->_block_newcli);
    }
    return ret;
}

int base_tcpserver::accept(base_tcpclient_object *client)
{
    if ( !_listened )
    {
        return BASE_ERR_SOCKET;
    }
    struct sockaddr_in	peer_address;
    BASE_ZERO_VALUE(peer_address);
    base_fd socket_fd = -1;
    int ret = accept(socket_fd, peer_address);
    if(  ret > 0 )
    {
        client->attach(socket_fd,peer_address,this->_block_newcli);
    }
    return ret;
}

int base_tcpserver::accept(	base_fd &socket_fd,struct sockaddr_in &peer_address, int ms)
{

    int retmask = BASEPOLL_NONE;
    int ret =  BaseWait(_socket,BASEPOLL_READABLE,ms,&retmask);
    if ( ret > 0  )
    {
        return accept(socket_fd,peer_address);
    }

    return 0;
}

int base_tcpserver::accept(	base_fd &socket_fd,struct sockaddr_in &peer_address)
{
    _socklen_t len 	= (_socklen_t) sizeof(struct sockaddr_in);
    socket_fd 	= ::accept(_socket, (struct sockaddr*) &peer_address, &len);
    if( base_valid_socket(socket_fd) )
    {
        if(!base_set_block(socket_fd,false))
        {
            return 0;
        }

        char buf[2];
        errno = 0;
#ifdef MSG_PEEK
        if(::recv(socket_fd,buf,2,MSG_PEEK)<=0)
#else
        if(::recv(socket_fd,buf,0,0)<=0)
#endif
        {
#if defined(WIN32) || defined(WIN64)
            int err = WSAGetLastError();
			if (err!= WSAEWOULDBLOCK && err!=WSAEINTR && err!=WSAEINPROGRESS)
#else
            int err = errno;
            if (err!=EWOULDBLOCK && err!=EINTR && err != EAGAIN)
#endif
            {
                base_close(socket_fd);
                return 0;
            }
        }
        return 1;
    }
    else
    {

#if defined(WIN32) || defined(WIN64)
        int err = WSAGetLastError();
		if ( err==WSAEINTR || (err== WSAEWOULDBLOCK&&!_blocked_listen) || err==WSAECONNABORTED || err==WSAEINPROGRESS )
#else
        int err = errno;
        if (err==EINTR || (( err== EWOULDBLOCK || err== EAGAIN )&&!_blocked_listen) || err==ECONNABORTED )
#endif
        {
            return 0;
        }
        return BASE_ERR_SOCKET;
    }
}

bool  base_tcpserver::close()
{
    if ( base_valid_socket(_socket) )
    {
        base_close(_socket);
    }
    _listened = false;
#if defined(WIN32) || defined(WIN64)
    _socket = INVALID_SOCKET;
#else
    _socket = -1;
#endif
    return true;
}

void base_tcpserver::attach(base_fd socket_fd,bool blocked_listen,bool block_newcli)
{
    _socket = socket_fd;
    _block_newcli = block_newcli;
    _blocked_listen = blocked_listen;
    _listened = true;
    base_get_address(&_local_address, _socket);
}