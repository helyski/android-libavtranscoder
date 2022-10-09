#ifndef _LOCK_H_
#define _LOCK_H_

#include <pthread.h>

#ifndef PRINT
#include <stdio.h>
#define PRINT(format, args...)  {              \
                printf(format, ##args);        \
                printf("\n");                  \
        }
#endif




class Lock {
public:
    Lock();
    ~Lock();

    void lock();

    void unlock();
private:
    pthread_mutex_t mutex_;
    unsigned int Holder;
    unsigned int HolderCount;
};

class AutoLock {
public:
    AutoLock(Lock& l);
    ~AutoLock();
private:
    Lock& l_;
};




#endif
