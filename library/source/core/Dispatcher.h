//
// Created by Tank on 2022/11/4.
//

#ifndef ANDROID_LIBTRANSCODE_DISPATCHER_H
#define ANDROID_LIBTRANSCODE_DISPATCHER_H

#include <SingleThread.h>
#include "DataStruct.h"
#include "EncVideoDataBuffer.h"


class Dispatcher:public SingleThread::ThreadProc {
public:
    Dispatcher();
    ~Dispatcher();


    bool StartThread();

    void StopThread();

    void SetDateBuffer(EncVideoDataBuffer *dataBuffer);

    void SetJVMCallBack();


private:
    bool process(int thread_id, void *env);

private:
    SingleThread mThread;
    bool mIsThreadStart;
    bool mExit;
    const char *mThreadName = "Dispatcher";

    EncVideoDataBuffer *mVideoBuffer;

    int mLastIndex;

    Lock mThreadLock;

};


#endif //ANDROID_LIBTRANSCODE_DISPATCHER_H
