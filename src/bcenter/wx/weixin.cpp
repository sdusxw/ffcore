//
// Created by boon on 17-11-24.
//

#include "rapidjson_lib.h"
#include "wx_def.h"
#include "boon_log.h"
#include "weixin.h"

//#include <unistd.h>

using namespace ONCALL;
using namespace BASE;

CWeiXin::CWeiXin()
    : udp_local_port_(PORT_UDP_BCENTER_SERVER)
{
    memset(udp_local_ip_, 0, sizeof(udp_local_ip_));

    memset(log_buf_, 0, sizeof(log_buf_));

    memset(recv_buf_, 0, sizeof(recv_buf_));

    // 初始化
    init();
}

CWeiXin::~CWeiXin()
{

}

void CWeiXin::init()
{
    if(!initUdpServer())
        throw runtime_error("error, 创建socket失败! initUdpServer failed!");
}

/**
 * @brief: 初始化udp服务
 */
bool CWeiXin::initUdpServer()
{
    if(!udp_local_.create(true, false)) {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, create套节字失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
        return false;
    }

    if(!udp_local_.bind(udp_local_port_, NULL)){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, bind套节字失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
        return false;
    }

    return true;
}

/**
 * @brief: 启动工作线程，用于接收oncallclient进程的消息
 */
void CWeiXin::start()
{
    twork_id_ = thread(&CWeiXin::workThread, this, this);
    twork_id_.detach();
}

/**
 * @brief: 工作线程
 */
void* CWeiXin::workThread(void* para)
{
    snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 启动支付线程! line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

    CWeiXin* p = (CWeiXin*)para;

    while(1){
        p->work();

        usleep(500);
    }
}

/**
 * @brief: 主工作模块
 */
void CWeiXin::work()
{
    memset(recv_buf_, 0, sizeof(recv_buf_));
    int len = udp_local_.recv(recv_buf_, sizeof(recv_buf_), udp_local_ip_, sizeof(udp_local_ip_), udp_local_port_);

    if(len == 0)
        return;

    if(len < 0){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 收到数据包长度异常 len[%d] line[%d]", __FUNCTION__, len, __LINE__);
        writelog(log_buf_);

        return;
    }

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 收到一个数据包[%s] len[%d] line[%d]", __FUNCTION__, recv_buf_, len, __LINE__);
    writelog(log_buf_);

    if(!checkJsonFormat(recv_buf_))
        return;

    unPackMessage(recv_buf_);
}

/**
 * @brief: 解析对端发过来的消息
 * @param _data： 消息包
 */
void CWeiXin::unPackMessage(const char* _data)
{
    if(!_data || _data == nullptr)
        return;

    Document doc;
    doc.Parse(_data);

    if(!doc.HasMember("cmd") || doc["cmd"] == "" || doc["cmd"].GetStringLength() <= 1 ){
        snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] error, 收到的消息异常, 没有包含cmd字段、或字段cmd为空、或者不是字符串类型. return false! line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return ;
    }

    // 出场消息
    if(doc["cmd"] == C_CMD_OUT) {
        processCarout(_data);
    }else if(doc["cmd"] == C_CMD_PAY) { // 场内支付消息
        processPay(_data);
    }else{ // 其他消息

    }
}

/**
 * @brief: 出场消息
 * @return
 */
bool CWeiXin::processCarout(const char* _data)
{
    Document doc;
    doc.Parse(_data);

    if(!doc.HasMember("plate") || !doc.HasMember("openid") || !doc.HasMember("userid") ){
        snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] error, 收到的消息异常, 没有包含cmd字段、或openid字段、或userid字段. return false! line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return false;
    }

    bool ret = false;

    // 车牌
    std::string plate_str = doc["plate"].GetString();
    std::string openid_str = doc["openid"].GetString();
    std::string userid_str = doc["userid"].GetString();

    std::string car_type_str = "";
    // 查询车辆类型
    mongodb_.queryCarType(plate_str, car_type_str);

    // 获取停车时间

    // 计费处理

}

/**
 * @brief: 场内支付消息
 * @return
 */
bool CWeiXin::processPay(const char* _data)
{

}

/**
 * @brief: 校验json包
 * @param _data
 * @return
 */
bool CWeiXin::checkJsonFormat(const char* _data)
{
    if(!_data || _data == nullptr)
        return false;

    Document doc;

    if (doc.Parse((char*)_data).HasParseError()) {
        snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] error, 收到的json消息格式异常[%s] line[%d]", __FUNCTION__, _data, __LINE__);
        writelog(log_buf_);
        return false;
    }
    else
        return true;
}

/**
 * @brief: 写日志，注意日志的文件目录
 * @param _buf： 待写入的数据
 */
void CWeiXin::writelog(const char* _buf)
{
    BLog::writelog(_buf, PKGDATA, LOG_DIR_NAME, LOG_FILE_NAME);
}