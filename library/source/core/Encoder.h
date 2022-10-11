//
// Created by Tank on 2022/10/9.
//

#ifndef ANDROID_LIBTRANSCODE_ENCODER_H
#define ANDROID_LIBTRANSCODE_ENCODER_H


#include <SingleThread.h>
#include "debug.h"
#include "RingQueue.h"
#include "DataStruct.h"

extern "C"
{




    class Encoder : public SingleThread::ThreadProc{
    public:
        Encoder();

        ~Encoder();

        bool StartThread();
        void StopThread();

        int SetInputBuffer(RingQueue<YUVFrame> *yuvBuffer);

        int SetOutputBuffer(RingQueue<H264Frame> *h264Buffer);

        int StartEncode();



    private:
        bool process(int thread_id, void *env);

    private:
        SingleThread mThread;
        bool mIsThreadStart;
        bool mExit;
        const char * mThreadName = "AvEncoder";

        RingQueue<YUVFrame>  *mYuvBuffer;
        RingQueue<H264Frame> *mH264Buffer;

    };



};


#endif //ANDROID_LIBTRANSCODE_ENCODER_H
