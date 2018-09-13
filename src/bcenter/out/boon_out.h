#ifndef BOON_OUT_H
#define BOON_OUT_H

#include "../global/boon_global.h"
#include "../mysql/boon_mysql.h"
#include "../mongodb/boon_mongodb.h"
#include "../msgqueue/boon_msgqueue.h"

#include "../global/clock.h"

void* out_thread(void *); //出口处理线程

// 车离开时间矫正
std::string carOutTime();

#endif
