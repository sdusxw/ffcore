//
// Created by boon on 17-10-15.
//

#include "rapidjson_lib.h"

#include "common_def.h"
#include "utils.h"
#include "my_client.h"

my_client::my_client(base_cb_cli_attr* argv)
        :base_cb_cli(argv->send_buf_len, argv->recv_buf_len)
{
    memset(server_ip_, 0, sizeof(server_ip_));
}

int my_client::ondata(const void* data, unsigned int len)
{
    if (!data || len == 0 )
        return len;

    if(len >= MSG_BUFF_LEN)
        return len;

    //char buf[MSG_BUFF_LEN] = {0};
    //memcpy(buf, (char*)data, len);

    printf("[%s] recv[%s] len[%d] line[%d]\n", printTime().c_str(), (char*)data, len, __LINE__);

    parseWebMessage((char*)data, len);

    return len;
}

/**
 * @brief: 解析web的消息
 * @param _data
 * @param _len
 */
void my_client::parseWebMessage(const char* _data, const int& _len)
{
    if(!_data || _len <= 0)
        return;

    std::string recv_data = _data;

}

/**
 * @brief: 获取json数据
 * @param _cmd: 频道名
 * @return： “”， 失败；
 *          json数据，成功
 */
const bool my_client::getJsonData(const std::string& _cmd, std::string& _out_json)
{
    if(_cmd.empty())
        return false;

    bool ret = true;

    return false;
}

/**
 * @brief: 支持铁路，启动服务
 */
void my_client::sendStart(){
    std::string str = "StartWork|BOON";
    senddata(str.c_str(), str.length());

    printf("[%s] send: %s\n", printTime().c_str(), str.c_str());
};

/**
 * @brief: 支持铁路，停止服务
 */
void my_client::sendStop(){
    std::string str = "StopWork|X";
    senddata(str.c_str(), str.length());

    printf("[%s] send: %s\n", printTime().c_str(), str.c_str());
}

/**
 * @brief: 支持民用，入场
 */
void my_client::sendIn()
{
    if(!isconnected())
        return;

    std::string str;
    if(!getJsonData("in", str))
        return;

    sendMessage(str, str.length());
}

/**
 * @brief: 支持民用，场内支付
 */
void my_client::sendPay()
{
    if(!isconnected())
        return;

    std::string str;
    if(!getJsonData("pay", str))
        return;

    sendMessage(str, str.length());
}

/**
 * @brief: 支持民用，出场
 */
void my_client::sendOut()
{
    if(!isconnected())
        return;

    std::string str;
    if(!getJsonData("out", str))
        return;

    sendMessage(str, str.length());
}

void my_client::sendMessage(const std::string& str, const int& len)
{
    if(!isconnected()){
        return;
    }

    int iret = senddata(str.c_str(), str.length());

    printf("[%s] send[%s] len[%ld] ret[%d] line[%d]\n", printTime().c_str(), str.c_str(), str.length(), iret, __LINE__);
}

bool my_client::connect()
{
    return base_cb_cli::connect( server_ip_, server_port_ );
}

bool my_client::connect(char* _ip, short _port)
{
    memcpy(server_ip_, _ip, sizeof(server_ip_));
    server_port_ = _port;

    return base_cb_cli::connect( _ip, _port );
}

bool my_client::close()
{
    return base_cb_cli::close();
}

void my_client::start()
{
    thread id = thread(&my_client::clientThread, this, this);
    id.detach();
}

void my_client::clientThread(void* para)
{
    my_client* p = (my_client*)para;

    while(1){

        if(BASE_SUCCESS != p->checkConnect()){
            if(!p->connect()){
                printf("[%s] error, 连接服务端失败 ip[%s] port[%d]\n", printTime().c_str(), p->getServerIp(), p->getServerPort());
                sleep(5);
                continue;
            }else{
                printf("[%s] 连接服务端成功 ip[%s] port[%d]\n", printTime().c_str(), p->getServerIp(), p->getServerPort());
            }
        }

        p->workio();

        usleep(1);
    }

    return;
}


void my_client::work()
{
    if(BASE_SUCCESS != checkConnect())
    {
        bool ret = connect(server_ip_, server_port_);
        if(!ret)
        {
            sleep(5);
            printf("[%s] [%s] error, 连接失败! ip[%s] port[%d] line[%d]\n", printTime().c_str(), __FUNCTION__, server_ip_, server_port_, __LINE__);

            return;
        }else{
            printf("[%s] [%s] 连接成功! ip[%s] port[%d] line[%d]\n", printTime().c_str(), __FUNCTION__, server_ip_, server_port_, __LINE__);
        }
    }
}

int	my_client::checkConnect()
{
    int mask = BASEPOLL_READABLE, retmask = BASEPOLL_NONE;

    if ( get_send_buf_currentlen() )
    {
        mask |= BASEPOLL_WRITABLE;
    }

    int ret = -1;
    ret = poll(mask,1,retmask);//1ms
    if (ret == BASEPOLL_ERR)
    {
        return BASE_ERR_SOCKET;
    }

    ret = work_io(retmask);
    if(BASE_SUCCESS != ret)
        close();

    return ret;
}

int my_client::workio()
{
    int mask = BASEPOLL_READABLE, retmask = BASEPOLL_NONE;

    if ( get_send_buf_currentlen() )
    {
        mask |= BASEPOLL_WRITABLE;
    }

    int ret = -1;
    ret = poll(mask,1,retmask);//1ms
    if (ret == BASEPOLL_ERR)
    {
        return BASE_ERR_SOCKET;
    }

    ret = work_io(retmask);
    if(BASE_SUCCESS != ret)
        close();
}

int my_client::work_io(int _mask)
{
    unsigned int recv_len = 0;
    unsigned int send_len = 0;

    int ret = base_cb_cli::io(_mask, &recv_len, &send_len);
    if(ret != BASE_SUCCESS)
    {
        printf("[%s] [%s] error, io失败 line[%d]\n", printTime().c_str(), __FUNCTION__, __LINE__);
    }

    return ret;
}
