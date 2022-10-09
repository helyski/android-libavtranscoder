#include "Lock.h"
#include <stdio.h>




Lock::Lock() {
    Holder = 0;
    HolderCount = 0;
    pthread_mutex_init(&mutex_, NULL);
}

Lock::~Lock() {
    pthread_mutex_destroy(&mutex_);
}

void Lock::lock() {
    unsigned int caller = ((unsigned int) pthread_self());
    if (caller == Holder)
    {
        HolderCount += 1;
        return;
    }

    if( 0 == pthread_mutex_lock(&mutex_)){
        Holder = caller;
        HolderCount = 1;
    }
}

void Lock::unlock() {
    unsigned int caller = ((unsigned int) pthread_self());
    if (caller != Holder)
        return;
    HolderCount -= 1;

    if (HolderCount == 0) {
        Holder = 0;
        pthread_mutex_unlock(&mutex_);
    }
}

AutoLock::AutoLock(Lock& l): l_(l) {
    l_.lock();
}

AutoLock::~AutoLock() {
    l_.unlock();
}


