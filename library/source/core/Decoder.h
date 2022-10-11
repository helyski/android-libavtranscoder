//
// Created by Tank on 2022/10/9.
//

#ifndef ANDROID_LIBTRANSCODE_DECODER_H
#define ANDROID_LIBTRANSCODE_DECODER_H


#include <SingleThread.h>
#include "DataStruct.h"
#include "RingQueue.h"

extern "C"
{
// C file must be contained in extern C statement
#include "hw_decode.h"

class Decoder : public SingleThread::ThreadProc {
public:
    Decoder();

    ~Decoder();

    bool StartThread();

    void StopThread();

    int SetDecodeFileInfo(const char *srcVideoPath, float seek_seconds);

    int SetDecodeBuffer(RingQueue<YUVFrame> *decodeOutputBuffer);

private:
    bool process(int thread_id, void *env);

private:
    SingleThread mThread;
    bool mIsThreadStart;
    bool mExit;
    const char *mThreadName = "AvDecoder";

    char mSrcVideoPath[256];
    float mDecodeSeekSeconds;

    RingQueue<YUVFrame> *mDecodeOutputBuffer;
};


};

#endif //ANDROID_LIBTRANSCODE_DECODER_H
