#include "clock.h"

clock_b::clock_b(char *name, FILE *fp)
{
    this->fp = fp;
    this->name = (char *)malloc(256);
    memset(this->name,0,256);
    memcpy(this->name,name,strlen(name));
    gettimeofday(&startTime,NULL);
}

void clock_b::inter(char *name)
{
    gettimeofday(&endTime,NULL);
    Timeuse = 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec);
    Timeuse /= 1000;
    printf("%s TimeCost = %fms\n",name,Timeuse);
    if(fp != NULL)
        fprintf(fp,"%s TimeCost = %fms\n",name,Timeuse);
}

clock_b::~clock_b()
{
    gettimeofday(&endTime,NULL);
    Timeuse = 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec);
    Timeuse /= 1000;
    printf("%s TimeCost = %fms\n",name,Timeuse);
    if(fp != NULL)
        fprintf(fp,"%s TimeCost = %fms\n",name,Timeuse);
}
