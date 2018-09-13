#ifndef BOON_BLED_H
#define BOON_BLED_H

#include "../global/boon_global.h"
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../msgqueue/boon_msgqueue.h"


void* bled_thread(void *); //bled监听线程


#endif
