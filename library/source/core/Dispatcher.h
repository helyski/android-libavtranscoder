//
// Created by Tank on 2022/11/4.
//

#ifndef ANDROID_LIBTRANSCODE_DISPATCHER_H
#define ANDROID_LIBTRANSCODE_DISPATCHER_H

#include <SingleThread.h>
#include "DataStruct.h"
#include "EncVideoDataBuffer.h"
#include "tool.h"
#include "common.h"
#include "timetool.h"



class Dispatcher:public SingleThread::ThreadProc {
public:
    Dispatcher();
    ~Dispatcher();


    bool StartThread();

    void StopThread();

    void SetDateBuffer(EncVideoDataBuffer *dataBuffer);

    void SetVideoFrameCallBack(video_frame_call_back callBack);


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


    video_frame_call_back mVideoCallBack;
};


#endif //ANDROID_LIBTRANSCODE_DISPATCHER_H
