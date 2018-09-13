#ifndef BOON_BIPC_H
#define BOON_BIPC_H

#include "../global/boon_global.h"
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../msgqueue/boon_msgqueue.h"


void* bipc_thread(void *); //bipc监听线程


#endif
