//
// Created by Tank on 2022/10/9.
//

#ifndef ANDROID_LIBTRANSCODE_DECODER_H
#define ANDROID_LIBTRANSCODE_DECODER_H

#include <SingleThread.h>

extern "C"
{
#include "hw_decode.h"

namespace LibTranscode {
    class Decoder : public SingleThread::ThreadProc{
    public:
        Decoder();

        ~Decoder();

        bool StartThread();
        void StopThread();

        int StartDecode(const char* srcVideoPath,float seek_seconds);



    private:
        bool process(int thread_id, void *env);

    private:
        SingleThread mThread;
        bool mIsThreadStart;
        bool mExit;
        const char * mThreadName = "AvTranscoder_decoder";
    };

}

};

#endif //ANDROID_LIBTRANSCODE_DECODER_H
