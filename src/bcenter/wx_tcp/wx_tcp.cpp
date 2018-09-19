/***************************boon_wx.cpp***************************************
			  功能：与wx通信
			  创建时间：2017-02-05
              修改时间：2018-09-18
			  创建人：孙振行
			  单位：山东博昂信息科技有限公司
			  修改时间：
			        1. 2017/11/21 14:21
			            继时旭之后，修改/添加微信支付宝双码支付功能
                    2. 2018/09/18 09:50
                        改成tcp通讯接收aipayclient的消息 孙希伟
***************************************************************************/
#include "boon_log.h"
#include "common_def.h"
#include "../bcenter_def.h"

#include "net_tcp.h"
#include "wx_tcp.h"

using namespace BASE;

static char log_buf_[LOG_BUF_SIZE] = {0};

extern pthread_mutex_t mongo_mutex_car;

bool wx_tcp_init() //初始化微信tcp支付消息
{
    return true;
}

/**************************************************wx监听线程******************************/
void* wx_tcp_thread(void *)
{
    return NULL;
}

// 写入日志
void writelog(const char* _buf)
{
    BLog::writelog(_buf, PKGDATA, LOG_DIR_NAME, LOG_FILE_NAME);
}

/**************************************************end******************************************/

