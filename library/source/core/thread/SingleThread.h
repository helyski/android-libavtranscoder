#ifndef _SINGLE_THREAD_H_
#define _SINGLE_THREAD_H_

#include "Lock.h"

namespace LibTranscode {


class SingleThread {
public:

class ThreadProc {
public:

    ThreadProc(){};
    virtual ~ThreadProc(){}
    virtual bool process(int thread_id,void *env) = 0;
};

    SingleThread(int id = 0);
    ~SingleThread();

    void start(const char *name = "");
    void stop();

    void registerThreadProc(ThreadProc& proc);
    int getId();

protected:
    static void* thread_func(void* param);
    void process();

private:
    pthread_t thread_;
    bool running_;
    Lock l_;
    ThreadProc* proc_;
    int id_;
};


}


#endif
