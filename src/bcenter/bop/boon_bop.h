#ifndef BOON_BOP_H
#define BOON_BOP_H

#include "../global/boon_global.h"
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../msgqueue/boon_msgqueue.h"


void* bop_thread(void *); //bop监听线程


#endif
