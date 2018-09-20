#ifndef BOON_WX_TCP_H
#define BOON_WX_TCP_H

#include "../global/boon_global.h"
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../msgqueue/boon_msgqueue.h"

bool wx_tcp_init(); //初始化微信tcp支付消息

void* wx_tcp_thread(void *); //wx_tcp监听线程

void* wx_tcp_msg(void *); //wx_tcp消息处理线程

// 写入日志
static void writelog(const char* _buf);

#endif
