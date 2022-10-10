//
// Created by Tank on 2022/10/9.
//

#ifndef ANDROID_LIBTRANSCODE_ENCODER_H
#define ANDROID_LIBTRANSCODE_ENCODER_H


#include <SingleThread.h>

extern "C"
{
#include "hw_decode.h"

namespace LibTranscode {

    class Encoder : public SingleThread::ThreadProc{
    public:
        Encoder();

        ~Encoder();

        bool StartThread();
        void StopThread();

        int StartEncode();



    private:
        bool process(int thread_id, void *env);

    private:
        SingleThread mThread;
        bool mIsThreadStart;
        bool mExit;
        const char * mThreadName = "AvTranscoder_encoder";
    };

}

};


#endif //ANDROID_LIBTRANSCODE_ENCODER_H
