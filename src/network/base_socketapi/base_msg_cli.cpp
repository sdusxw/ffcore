//
// Created by boon on 17-11-6.
//

#include <assert.h>
#include "base_socketapi.h"
#include "base_msg.h"
#include "base_oscore.h"

base_msg_cli::base_msg_cli(base_msg_cli_attr *attr): base_cb_cli(attr->send_buf_len,attr->recv_buf_len)
{
    //
    _use_heartbeat = attr->use_heartbeat;
    _heartbeat_idle = attr->heartbeat_idle;
    _heartbeat_interval = attr->heartbeat_interval;
    _heartbeat_retry = attr->heartbeat_retry;
    _tv_last.tv_sec = 0;
    _tv_last.tv_usec = 0;
    _start_count = 0;
    _heartbeat_count = 0;
}

base_msg_cli::~base_msg_cli()
{
}


int base_msg_cli::ondata(const void* data,unsigned int len)
{
    return getmsgfrombuf((char *)data,len);
};

int  base_msg_cli::getmsgfrombuf(char *buf,unsigned int len)
{
    char *p = buf;
    int tmplen = len;
    for(;;)
    {
        struct msg_hdr *hdr = base_unpack_msg(p,tmplen);
        if( NULL != hdr)
        {
            if((unsigned int )tmplen >= hdr->len )
            {
                if(  hdr->len > MSG_HDR_LEN  && hdr->cmd ==  BASE_MSG_DATA_NOTIFY )
                {
                    this->onmsg( p+MSG_HDR_LEN ,hdr->len-MSG_HDR_LEN);
                }
                else if(  hdr->len == MSG_HDR_LEN  &&  hdr->cmd ==  BASE_HEARTBEAT_REQ )
                {
                    send_heartbeat_rsp_msg();
                }
                else if(  hdr->len == MSG_HDR_LEN  &&  hdr->cmd ==  BASE_HEARTBEAT_RSP )
                {
                }
                else
                {
                    return BASE_ERR_DATA;
                }
                tmplen -= hdr->len;
                p = p + hdr->len;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    return len-tmplen;
}

int  base_msg_cli::sendmsg(const void *msg,unsigned int len)
{
    if ( !isconnected() )
    {
        return BASE_ERR_SOCKET;
    }
    if( len + MSG_HDR_LEN > get_send_buf_totallen() )
    {
        return BASE_ERR_BUFLEN;
    }

    unsigned int freelen = get_send_buf_totallen() - get_send_buf_currentlen();
    if( len + MSG_HDR_LEN  <= freelen )
    {
        //
        struct msg_hdr hdr;
        hdr.cmd = BASE_MSG_DATA_NOTIFY;
        hdr.endin = BASE_MSG_ENDIN;
        hdr.ver = BASE_MSG_VER;
        hdr.len = len + MSG_HDR_LEN;
        senddata(&hdr,MSG_HDR_LEN);
        return senddata(msg,len);
    }
    else
    {
        return BASE_ERR_BUFFULL;
    }
}

int base_msg_cli::io(int mask,unsigned int *recv_len,unsigned int *send_len)
{
    int ret = base_cb_cli::io(mask,recv_len,send_len);
    if (ret == BASE_ERR_BUFFULL)
    {
        //this->ondisconnect();
        return BASE_ERR_DATA;
    }
    return ret;
}

int base_msg_cli::work(int mask,timeval &tv_now)
{
    unsigned int recv_len = 0;
    unsigned int send_len = 0;
    int io_ret =  io(mask,&recv_len,&send_len);
    if(io_ret != BASE_SUCCESS)
    {
        return io_ret;
    }
    int hret = heartbeat_count(recv_len,send_len,tv_now);//
    return hret;
}

int base_msg_cli::autowork()
{
    int mask = BASEPOLL_READABLE,retmask = BASEPOLL_NONE;
    if ( get_send_buf_currentlen() )//
    {
        mask |= BASEPOLL_WRITABLE;
    }
    int ret = poll(mask,1,retmask);
    if (ret == BASEPOLL_ERR)
    {
        //this->ondisconnect();//
        return BASE_ERR_SOCKET;
    }
    timeval tv_now;
    base_gettimeofday(&tv_now);
    return work(retmask, tv_now);
}

//
int base_msg_cli::heartbeat_count(unsigned int recv_len,unsigned int send_len,timeval &tv_now)
{

    if ( !_use_heartbeat )//
    {
        return BASE_SUCCESS;
    }
    if ( recv_len > 0 && send_len > 0 )//
    {
        _start_count = 0;
        _heartbeat_count = 0;
        return BASE_SUCCESS;
    }

    if ( recv_len > 0 )//
    {
        _start_count = 0;
        _heartbeat_count = 0;
    }
    if(send_len == 0 || recv_len == 0 )
    {
        if( _start_count == 0 )
        {
            _tv_last  = tv_now;
        }
        _start_count++;

        time_t t = tv_now.tv_sec - _tv_last.tv_sec+ (tv_now.tv_usec - _tv_last.tv_usec)/1000000;
        time_t idle = 0;
        if(_start_count == 2)
        {
            idle = _heartbeat_idle;
        }
        else
        {
            idle = _heartbeat_interval;
        }
        if ( t > idle )
        {

            if( send_heartbeat_req_msg() != BASE_SUCCESS )
            {
                printf("heartbeat req failed\n");
            }
            _heartbeat_count++;
            _tv_last  = tv_now;
            printf("heartbeat req  _heartbeat_count=%d \n",_heartbeat_count);
        }

    }

    if ( _heartbeat_count > _heartbeat_retry )
    {
        printf("heartbeat time out \n");
        _start_count = 0;
        _heartbeat_count = 0;

        return BASE_ERR_SOCKET;
    }
    return BASE_SUCCESS;
}

int base_msg_cli::send_heartbeat_msg(int cmd)
{
    struct msg_hdr hdr;
    hdr.cmd = cmd;
    hdr.endin = BASE_MSG_ENDIN;
    hdr.ver = BASE_MSG_VER;
    hdr.len = MSG_HDR_LEN;
    return senddata(&hdr,MSG_HDR_LEN);
}

int base_msg_cli::send_heartbeat_rsp_msg()
{
    return send_heartbeat_msg(BASE_HEARTBEAT_RSP);
}

int base_msg_cli::send_heartbeat_req_msg()
{
    return send_heartbeat_msg(BASE_HEARTBEAT_REQ);
}

bool  base_msg_cli::connect(const char* ip, unsigned short port, int ms)
{
    bool ret =  base_cb_cli::connect(ip, port,ms);
    if (ret)
    {
        base_set_fast_keepalive(getsocketfd(),_heartbeat_idle,_heartbeat_interval,_heartbeat_retry);
    }
    return ret;
};

void base_msg_cli::attach(base_fd socket_fd,struct sockaddr_in &peer_addr,bool block)
{
    base_cb_cli::attach(socket_fd,peer_addr,block);
    base_set_fast_keepalive(socket_fd,_heartbeat_idle,_heartbeat_interval,_heartbeat_retry);
};