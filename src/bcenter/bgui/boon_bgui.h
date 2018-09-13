#ifndef BOON_BGUI_H
#define BOON_BGUI_H

#include "../global/boon_global.h"
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../msgqueue/boon_msgqueue.h"


void* bgui_thread(void *); //bgui监听线程


#endif
