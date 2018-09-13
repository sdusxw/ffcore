#ifndef BOON_BVS_H
#define BOON_BVS_H

#include "../global/boon_global.h"
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../msgqueue/boon_msgqueue.h"


void* bvs_thread(void *); //bvs监听线程


#endif
