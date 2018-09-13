//
// Created by boon on 17-9-22.
//

#include "base_socketapi.h"


base_cb_cli::base_cb_cli(unsigned int send_buf_len,unsigned int recv_buf_len): _send_buf_len(send_buf_len),_recv_buf_len(recv_buf_len)
{
    _p_write = 0;
    _send_buf = new char[_send_buf_len];
    _p_read = 0;
    _recv_buf = new char[_recv_buf_len];
    memset(_send_buf,0,_send_buf_len);
    memset(_recv_buf,0,_recv_buf_len);

}

base_cb_cli::~base_cb_cli()
{
    delete _send_buf;
    delete _recv_buf;
}

int base_cb_cli::io(int mask,unsigned int *recv_len,unsigned int *send_len)
{
    if ( !isconnected() )
    {
        return BASE_ERR_SOCKET;
    }
    int ret = BASE_SUCCESS;
    int  rxn=0,txn=0;

    if ( mask & BASEPOLL_READABLE)
    {
        //printf("base_socketapi io _recv_buf_len:%d,_p_read:%d,free_len:%d\n",_recv_buf_len,_p_read,_recv_buf_len-_p_read);
        rxn = recv(_recv_buf+_p_read,_recv_buf_len-_p_read);
        //printf("base_socketapi recv len:%d\n",rxn);
        if(rxn<=0)
        {
            if( rxn == BASE_ERR_SOCKET )
            {
                //this->ondisconnect();
                return BASE_ERR_SOCKET;
            }
        }
        else
        {
            _p_read += rxn;

            if( ret = on_recvdata() == BASE_ERR_DATA )
            {
                //this->ondisconnect();
                return BASE_ERR_DATA;
            }
        }

    }
    if( mask & BASEPOLL_WRITABLE )
    {
        if (  _p_write != 0  )
        {
            txn=send(_send_buf,_p_write);
            //printf("base_socketapi*** send len:%d\n",txn);
            if(txn<=0)
            {
                if( txn == BASE_ERR_SOCKET )
                {
                    //this->ondisconnect();
                    return BASE_ERR_SOCKET;
                }
            }
            else
            {
                if ( txn < (int)_p_write )
                {
                    memmove(_send_buf,_send_buf + txn,_p_write - txn);
                }
                _p_write -= txn;

            }
        }
    }
    if ((NULL != recv_len) && (rxn >= 0))
    {
        *recv_len = (unsigned int)rxn;
        //printf("base_socketapi out recv_len:%d\n",recv_len);
    }
    if ((NULL != send_len) && (txn >= 0))
    {
        *send_len = (unsigned int)txn;
    }
    return ret;
}

int base_cb_cli::on_recvdata()
{
    if ( _p_read == 0 )
    {
        return BASE_SUCCESS;
    }
    int glen = this->ondata(_recv_buf,_p_read);
    if( BASE_ERR_DATA == glen )
    {
        printf("on_recvdata BASE_ERR_DATA\n");
        return BASE_ERR_DATA;
    }

    if( (glen != 0) && (glen < (int)_p_read) )
    {
        //printf("on_recvdata memmove glen:%d _p_read:%d\n",glen,_p_read);
        memmove(_recv_buf,_recv_buf+glen,_p_read - glen);
    }
    _p_read -= glen;
    if ( _p_read == _recv_buf_len )
    {
        return BASE_ERR_BUFFULL;
    }
    return BASE_SUCCESS;
}

int  base_cb_cli::senddata(const void *data, unsigned int len)
{
    if ( !isconnected() )
    {
        return BASE_ERR_SOCKET;
    }
    if (len == 0)
    {
        return BASE_SUCCESS;
    }
    if( len > _send_buf_len )
    {
        return BASE_ERR_BUFLEN;
    }
    if( len + _p_write <= _send_buf_len )
    {
        memcpy(_send_buf+_p_write,data,len);
        _p_write += len;

        this->io(BASEPOLL_WRITABLE,NULL,NULL);
        return BASE_SUCCESS;
    }
    else
    {
        return BASE_ERR_BUFFULL;
    }
}

bool base_cb_cli::close()
{
    _p_read = 0;
    _p_write = 0;
    return _tcp_cli.close(false);
};

void base_cb_cli::attach(base_fd socket_fd,struct sockaddr_in &peer_addr,bool block)
{
    if (block)
    {
        abort();
    }

    return _tcp_cli.attach(socket_fd,peer_addr,block);
};

bool base_cb_cli::isconnected()
{
    return _tcp_cli.isconnected();
};

bool base_cb_cli::connect(const char* ip, unsigned short port, int ms)
{
    return _tcp_cli.connect(ip,port,ms);
};

int base_cb_cli::poll(int mask, const long ms, int &retmask)
{
    return _tcp_cli.poll(mask,ms,retmask);
};

int base_cb_cli::recv(void *buf,unsigned int len)
{
    return _tcp_cli.recv(buf,len);
};

int base_cb_cli::send(void *buf,unsigned int len)
{
    return _tcp_cli.send(buf,len);
};

base_fd base_cb_cli::getsocketfd()
{
    return _tcp_cli.getsocketfd();
};

const struct sockaddr_in&base_cb_cli:: get_local() const
{
    return _tcp_cli.get_local();
};

const struct sockaddr_in& base_cb_cli::get_peer() const
{
    return _tcp_cli.get_peer();
}

const unsigned int base_cb_cli::get_recv_buf_totallen()
{
    return _recv_buf_len;
};

const unsigned int base_cb_cli::get_recv_buf_currentlen()
{
    return _p_read;
};
