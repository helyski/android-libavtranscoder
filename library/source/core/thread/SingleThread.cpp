#ifdef __COMMON_ANDROID__
#include <jni.h>
#include <hvjava.h>
#endif
#include <debug.h>
#include <pthread.h>
#include<sys/resource.h>
#include "SingleThread.h"


namespace LibTranscode {

SingleThread::SingleThread(int id): running_(false), proc_(NULL), id_(id) {
}

SingleThread::~SingleThread() {

}

void SingleThread::start(const char *name) {
    {
        AutoLock l(l_);
        if (running_)
            return;
    }

    running_ = true;
    pthread_create(&thread_, NULL, thread_func, this);

    char thread_name[16] = {0};
    int len = strlen(name);
    if(len > 15)
        len = 15;
    strncpy(thread_name,name,16);

#ifndef __LITEOS_HIMIX__
    pthread_setname_np(thread_,thread_name);
#endif
}

void SingleThread::stop() {
    {
        AutoLock l(l_);
        if (!running_)
            return;
        running_ = false;
    }
    void* dummy;
    pthread_join(thread_, &dummy);
}

void SingleThread::process() {
    int nead_detach = 0;

    void *env = 0;
#ifdef __COMMON_ANDROID__
    env  = hv_get_jni_env(&nead_detach);
    if (!env){
        return;
    }
#endif

#ifdef __LITEOS_HIMIX__
    id_ = 0;
#else
    id_ = gettid();
#endif
    while (1) {

        {
            AutoLock l(l_);
            if (!running_) {
                break;
            }
        }

        if (proc_ != NULL) {
            bool bret = proc_->process(id_,env);
            if (!bret) {
                AutoLock l(l_);
                running_ = false;
                break;
            }
        } else {
            //assert(!"there is no thread proc!!");
            break;
        }
    }

#ifdef __COMMON_ANDROID__
    if(nead_detach > 0)
        hv_detach_jni_env((JNIEnv*)env);
#endif
}

void SingleThread::registerThreadProc(SingleThread::ThreadProc& proc) {
    AutoLock l(l_);
    if (!running_) {
        proc_ = &proc;
    }
}

int SingleThread::getId() {
    return id_;
}

void* SingleThread::thread_func(void* param) {
    SingleThread* obj = (SingleThread*)param;
    obj->process();

    return NULL;
}





}
