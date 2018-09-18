#ifndef BOON_WX_TCP_H
#define BOON_WX_TCP_H

#include "../global/boon_global.h"
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../msgqueue/boon_msgqueue.h"


void* wx_tcp_thread(void *); //wx_tcp监听线程

// 写入日志
void writelog(const char* _buf);

#endif
