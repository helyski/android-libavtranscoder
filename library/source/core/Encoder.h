//
// Created by Tank on 2022/10/9.
//

#ifndef ANDROID_LIBTRANSCODE_ENCODER_H
#define ANDROID_LIBTRANSCODE_ENCODER_H


#include <SingleThread.h>
#include "debug.h"
#include "RingQueue.h"
#include "DataStruct.h"
#include "timetool.h"
#include "AVCCoder.h"
#include "hvjava.h"
#include "YUVWrapper.h"
#include "RawVideoDataBuffer.h"

extern "C"
{


#include "filetool.h"


    class Encoder : public SingleThread::ThreadProc{
    public:
        Encoder(CAVCCoder coder);

        ~Encoder();

        bool StartThread();
        void StopThread();

        int SetInputBuffer(RawVideoDataBuffer *yuvBuffer);

//        int SetOutputBuffer(RingQueue<H264Frame> *h264Buffer);

        int StartEncode();



    private:
        bool process(int thread_id, void *env);

    private:
        SingleThread mThread;
        bool mIsThreadStart;
        bool mExit;
        const char * mThreadName = "Xencoder";

        RawVideoDataBuffer  *mYuvBuffer;
        RingQueue<H264Frame> *mH264Buffer;

        CAVCCoder avcCoder;

        int mSrcWidth;
        int mSrcHeight;

        int mDestWidth;
        int mDestHeight;

        int mDestBitrate;
        int mFPS;

        Lock mEncoderLock;

    };



};


#endif //ANDROID_LIBTRANSCODE_ENCODER_H
