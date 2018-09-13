#ifndef BOON_TEST_MONGO_H
#define BOON_TEST_MONGO_H

#include "../global/boon_global.h"
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../msgqueue/boon_msgqueue.h"


void* test_mongo_thread(void *); //数据库侦测处理线程


#endif
