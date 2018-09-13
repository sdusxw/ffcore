//
// Created by boon on 17-9-22.
//

#ifndef TLRS_BASE_SOCKETAPI_H
#define TLRS_BASE_SOCKETAPI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "base_headers.h"
#include "base_poll.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define BASE_SUCCESS 0

#define BASE_ERR_SOCKET -1
#define BASE_ERR_DATA -2
#define BASE_ERR_BUFLEN -3
#define BASE_ERR_BUFFULL -4

#define BASE_WAIT_TIME_MS 10

#ifndef _socklen_t
#	if defined(__hpux) || defined(HPUX)
#		if defined(_XOPEN_SOURCE_EXTENDED)
#			define _socklen_t	socklen_t
#		else
#			define _socklen_t	int
#		endif
#	elif  defined(WIN32) || defined(WIN64)
#		define _socklen_t	int
#	else
#		define _socklen_t	socklen_t
#	endif
#endif

#if defined(WIN32) || defined(WIN64)
#if defined(_BASE_SOCKET_)
#define	BASESOCKET_IMEXPORT __declspec(dllexport)
#else
#define	BASESOCKET_IMEXPORT __declspec(dllimport)
#endif
#else
#define	BASESOCKET_IMEXPORT
#endif

#define BASE_CHECK_SOCK_RESULT(n)	(0 == (n))
#define BASE_ZERO_VALUE(v)		memset(&v, 0, sizeof(v))

#ifdef __cplusplus
extern "C"{
#endif

BASESOCKET_IMEXPORT int base_startup();
BASESOCKET_IMEXPORT int base_cleanup();
BASESOCKET_IMEXPORT bool base_valid_socket(base_fd socket_fd);
BASESOCKET_IMEXPORT int base_get_last_error(base_fd socket_fd);
BASESOCKET_IMEXPORT bool base_is_error(base_fd socket_fd);
BASESOCKET_IMEXPORT bool base_set_block(base_fd socket_fd,bool block);
BASESOCKET_IMEXPORT bool base_set_tcpnodelay(base_fd socket_fd);
BASESOCKET_IMEXPORT bool base_set_reuseaddress(base_fd socket_fd,bool reuse);
BASESOCKET_IMEXPORT bool base_get_reuseaddress(base_fd socket_fd, bool *_reuse);
BASESOCKET_IMEXPORT bool base_set_recvbuf(base_fd socket_fd,int _size);
BASESOCKET_IMEXPORT bool base_get_recvbuf(base_fd socket_fd,int *_size);
BASESOCKET_IMEXPORT bool base_set_sendbuf(base_fd socket_fd,int _size);
BASESOCKET_IMEXPORT bool base_get_sendbuf(base_fd socket_fd,int *_size);
BASESOCKET_IMEXPORT bool base_set_linger(base_fd socket_fd,const struct linger  *_linger);
BASESOCKET_IMEXPORT bool base_get_linger(base_fd socket_fd,struct linger *_linger);
BASESOCKET_IMEXPORT bool base_set_opt(base_fd socket_fd,int _level, int _optname, const void* _optval, _socklen_t _size);
BASESOCKET_IMEXPORT bool base_get_opt(base_fd socket_fd,int _level, int _optname, void* _optval, _socklen_t* _size);
BASESOCKET_IMEXPORT void base_get_address(struct sockaddr_in *addr, base_fd sock_fd);
BASESOCKET_IMEXPORT void base_parse_address(const sockaddr_in *addr, char *ip,int iplen,int *port);
BASESOCKET_IMEXPORT void base_make_address(sockaddr_in *_addr, unsigned short _port, const char *_ip);
BASESOCKET_IMEXPORT bool base_set_keepalive(base_fd socket_fd,bool keepalive);
BASESOCKET_IMEXPORT bool base_set_fast_keepalive(base_fd socket_fd,int _idle, int _interval, int _retry);
BASESOCKET_IMEXPORT bool base_get_keepalive(base_fd socket_fd,bool *_keepalive);
BASESOCKET_IMEXPORT bool base_set_broadcast(base_fd socket_fd,bool _broadcast);
BASESOCKET_IMEXPORT bool base_get_broadcast(base_fd socket_fd,bool *_broadcast);
BASESOCKET_IMEXPORT bool base_is_port_used(unsigned short port);
BASESOCKET_IMEXPORT base_fd base_create(int af, int type, int protocol);
BASESOCKET_IMEXPORT base_fd base_tcp_create();
BASESOCKET_IMEXPORT base_fd base_udp_create();
BASESOCKET_IMEXPORT bool base_close(base_fd socket_fd);
BASESOCKET_IMEXPORT bool base_bind(base_fd socket_fd,unsigned short _port,  const char* _ip);
BASESOCKET_IMEXPORT bool base_listen(base_fd socket_fd, int backlog);

#ifdef __cplusplus
};
#endif


#ifdef __cplusplus

class base_tcpclient_object
{
public:
    virtual void attach(base_fd socket_fd,struct sockaddr_in &peer_addr,bool block) = 0;
};

class base_tcpclient: public base_tcpclient_object
{
public:
    base_tcpclient();
    virtual ~base_tcpclient();
    bool connect_block(const char* ip, unsigned short port);
    bool connect(const char* ip, unsigned short port, int ms = 100);

    bool close(bool rst = true);

    int recv(void *buf,unsigned int len);

    int send(void *buf,unsigned int len);

    int poll(int mask, const long ms, int &retmask);

    int recvn(void *buf,unsigned int len,int retry);

    int sendn(void *buf,unsigned int len,int retry);

public:
    virtual void attach(base_fd socket_fd,struct sockaddr_in &peer_addr,bool block);

public:
    bool isconnected();
    bool isblocked(){	return _blocked;}
    base_fd getsocketfd(){	return _socket;}
    void setsocketfd(base_fd fd){	_socket = fd;}
    const struct sockaddr_in& get_local() const {return _local_address;}
    const struct sockaddr_in& get_peer() const {return _peer_address;}
private:
    base_fd			_socket;
    struct sockaddr_in	_peer_address;
    struct sockaddr_in	_local_address;
    bool				_connected;
    bool				_blocked;
};

class base_tcpserver
{
public:
    base_tcpserver(bool block_newcli = false);
    ~base_tcpserver();

    bool listen(bool block_listen,unsigned short port,char *ip = NULL, int backlog = SOMAXCONN);

    int accept(base_tcpclient_object *client,int ms);

    int accept(base_tcpclient_object *client);
    bool	close();

    void attach(base_fd socket_fd,bool blocked_listen,bool block_newcli);

public:
    bool islistened(){	return _listened;}
    bool isblocked(){	return _blocked_listen;}
    base_fd getsocketfd(){	return _socket;}
    void setsocketfd(base_fd fd){	_socket = fd;}
    const struct sockaddr_in& get_local() const {return _local_address;}

private:
    int accept(	base_fd &socket_fd,struct sockaddr_in &peer_address, int ms);
    int accept(	base_fd &socket_fd,struct sockaddr_in &peer_address);

private:
    base_fd			_socket;
    struct sockaddr_in	_local_address;
    bool				_listened;
    bool				_blocked_listen;
    bool                _block_newcli;
};


/**
 * @brief: udp服务
 */
class base_udp{
public:
    base_udp();
    ~base_udp();

    bool  create(bool broadcast, bool block);
    bool  close();
    bool  bind(unsigned short port, char *ip);
    bool  connect(unsigned short port, char *ip);

    int recv(void *buf,unsigned int len, struct sockaddr_in *addr = NULL);
    int send(void *buf,unsigned int len, const struct sockaddr_in *addr = NULL);
    int recv(void *buf,unsigned int len, char *ip,int iplen,int &port);
    int send(void *buf,unsigned int len,  char *ip,int port);
    int broadcast(void *buf, size_t len, unsigned short port);
    int poll(int mask, const long ms, int &retmask);

    int read(void *buf,unsigned int len, struct sockaddr_in *addr = NULL);
    int read(void *buf,unsigned int len, char *ip,int iplen,int &port);

    int write(void *buf,unsigned int len, const struct sockaddr_in *addr = NULL);
    int write(void *buf,unsigned int len, char *ip,int port);

public:
    bool isblocked(){	return _blocked;}
    base_fd getsocketfd(){	return _socket;}
    void setsocketfd(base_fd fd){	_socket = fd;}
    const struct sockaddr_in& get_local() const {return _local_address;}
    const struct sockaddr_in& get_peer() const {return _peer_address;}

private:
    int recvfrom(void *buf, size_t len, int flags = 0, struct sockaddr_in *addr = NULL);
    int sendto(const void *buf, size_t len, int flags = 0, const struct sockaddr_in *addr = NULL);
private:
    base_fd			_socket;
    struct sockaddr_in	_peer_address;
    struct sockaddr_in	_local_address;
    bool				_blocked;
};

struct base_cb_cli_attr
{
    unsigned int send_buf_len;
    unsigned int recv_buf_len;
};

class base_cb_cli:public base_tcpclient_object
{
public:
    virtual int ondata(const void* data, unsigned int len)=0;
public:
    base_cb_cli(unsigned int send_buf_len,unsigned int recv_buf_len);
    virtual ~base_cb_cli();

    int senddata(const void *data, unsigned int len);

    int io(int mask,unsigned int *recv_len,unsigned int *send_len);

    int poll(int mask, const long ms, int &retmask);
public:
    inline const unsigned int get_recv_buf_totallen();
    inline const unsigned int get_recv_buf_currentlen();
    inline const unsigned int get_send_buf_totallen(){return _send_buf_len;};
    inline const unsigned int get_send_buf_currentlen(){return _p_write;}
public:
    bool connect(const char* ip, unsigned short port, int ms = 100);
    bool close();

    virtual void attach(base_fd socket_fd,struct sockaddr_in &peer_addr,bool block);
    bool isconnected();
    base_fd getsocketfd();
    const struct sockaddr_in& get_local() const ;
    const struct sockaddr_in& get_peer() const ;
private:
    inline int recv(void *buf,unsigned int len);
    inline int send(void *buf,unsigned int len);
    int on_recvdata();
private:
    base_tcpclient _tcp_cli;
    char  *_send_buf;
    const unsigned int   _send_buf_len;
    unsigned int   _p_write;
    char *_recv_buf;
    const unsigned int   _recv_buf_len;
    unsigned int   _p_read;
};

struct base_msg_cli_attr
{
    unsigned int send_buf_len;
    unsigned int recv_buf_len;
    bool use_heartbeat;
    int heartbeat_idle;
    int heartbeat_interval;
    int heartbeat_retry;

    base_msg_cli_attr(){
        send_buf_len = 2048;
        recv_buf_len = 2048;
        use_heartbeat = true;
        heartbeat_idle = 60;
        heartbeat_interval = 5;
        heartbeat_retry = 5;
    }
};

class BASESOCKET_IMEXPORT base_msg_cli:public base_cb_cli
{
public:
    base_msg_cli(base_msg_cli_attr *attr);
    virtual ~base_msg_cli();
    virtual void onmsg(const void* msg, unsigned int len) = 0;
    int sendmsg(const void *msg,unsigned int len);

    int work(int mask,timeval &tv_now);
    bool connect(const char* ip, unsigned short port, int ms = 100);
    void attach(base_fd socket_fd,struct sockaddr_in &peer_addr,bool block);

    int autowork();

    int io(int mask,unsigned int *recv_len,unsigned int *send_len);
private:
    int ondata(const void* data, unsigned int len);
    int  getmsgfrombuf(char *buf,unsigned int len);

    int heartbeat_count(unsigned int recv_len,unsigned int send_len,timeval &now);
    int send_heartbeat_req_msg();
    int send_heartbeat_rsp_msg();
    int send_heartbeat_msg(int cmd);

private:
    bool _use_heartbeat;
    int _heartbeat_idle;
    int _heartbeat_interval;
    int _heartbeat_retry;
    timeval _tv_last;
    int  _start_count;
    int  _heartbeat_count;
};

#endif // __cplusplus

#endif //TLRS_BASE_SOCKETAPI_H