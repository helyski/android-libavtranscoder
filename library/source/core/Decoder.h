//
// Created by Tank on 2022/10/9.
//

#ifndef ANDROID_LIBTRANSCODE_DECODER_H
#define ANDROID_LIBTRANSCODE_DECODER_H


#include <SingleThread.h>
#include "DataStruct.h"
#include "RingQueue.h"
#include "timetool.h"
#include "RawVideoDataBuffer.h"

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

    int SetDecodeBuffer(RawVideoDataBuffer *decodeOutputBuffer);

private:
    bool process(int thread_id, void *env);
    static int GetFrameMaxHandleTimeMS(int fps);

private:
    SingleThread mThread;
    bool mIsThreadStart;
    bool mExit;
    const char *mThreadName = "AvDecoder";

    char mSrcVideoPath[256];
    float mDecodeSeekSeconds;

    RawVideoDataBuffer *mDecodeOutputBuffer;

    Lock mDecoderLock;
};


};

#endif //ANDROID_LIBTRANSCODE_DECODER_H
