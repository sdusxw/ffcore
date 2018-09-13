#ifndef CLOCK_H
#define CLOCK_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>

class clock_b
{
public:
    clock_b(char *name,FILE *fp = NULL);
    void inter(char *name);
    ~clock_b();
private:
    struct timeval startTime;
    struct timeval endTime;
    float Timeuse;

    char *name;

    FILE *fp;
};

#endif // CLOCK_H
