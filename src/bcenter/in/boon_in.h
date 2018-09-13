#ifndef BOON_IN_H
#define BOON_IN_H

#include "../global/boon_global.h"
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../msgqueue/boon_msgqueue.h"


void* in_thread(void *); //入口处理线程

#endif
