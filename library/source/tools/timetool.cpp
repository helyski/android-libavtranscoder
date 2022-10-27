//
// Created by tank on 2022/10/25.
//


#include "timetool.h"

unsigned long long get_system_current_time_millis()
{
    struct timeval tv;
    if(gettimeofday(&tv,0) < 0)
    {
        return -1;
    }

    return (((unsigned long long)tv.tv_sec) * 1000 + ((unsigned long long)tv.tv_usec) / 1000);
}