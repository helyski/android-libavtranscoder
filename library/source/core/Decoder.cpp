//
// Created by Tank on 2022/10/9.
//

#include "Decoder.h"
#include "RingQueue.h"

extern "C"
{

    Decoder::Decoder(){
        mIsThreadStart = false;
        mExit = false;
        mDecodeOutputBuffer = 0;
    }

    Decoder::~Decoder() {
        LOGW("~Decoder");

//        if(mDecodeOutputBuffer){
//            LOGW("mDecodeOutputBuffer delete .......");
//            delete mDecodeOutputBuffer;
//            mDecodeOutputBuffer = NULL;
//        }
        LOGW("Decoder~");
    }

    bool Decoder::StartThread() {
        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
        }
        mThread.registerThreadProc(*this);
        mThread.start(mThreadName);
        mIsThreadStart = true;
        LOGW("Decoder::StartThread");
        return true;
    }

    void Decoder::StopThread() {
        stop_hw_decode();
        mExit = true;
        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
        }
        LOGW("Decoder::StopThread");
    }

    bool Decoder::process(int thread_id, void *env) {
        LOGW("Decoder_tid%d::start loop in process!",thread_id);
        int temp = 0;
        while(!mExit){
//            usleep(1000000);
//            LOGW("Decoder_tid%d::%d!",thread_id,temp++);

            start_hw_decode(mSrcVideoPath,mDecodeSeekSeconds);
            break;
        }
        LOGW("Decoder_tid%d::exit loop in process!",thread_id);
        return false;
    }

int Decoder::SetDecodeFileInfo(const char *srcVideoPath, float seek_seconds) {
    mDecodeSeekSeconds = seek_seconds;
//    if(!srcVideoPath){
//        return -1;
//    }
//    if(!mSrcVideoPath){
//        mSrcVideoPath = (unsigned char*)malloc(sizeof(srcVideoPath));
//    }
//    if(!mSrcVideoPath){
//        return -1;
//    }
//    memcpy(mSrcVideoPath,srcVideoPath,strlen(srcVideoPath));

    strcpy(mSrcVideoPath,srcVideoPath);

    LOGW("Decoder::SetDecoderFile = %s",mSrcVideoPath);

    return 0;
}


int Decoder::SetDecodeBuffer(RingQueue<YUVFrame> *decodeOutputBuffer) {
        mDecodeOutputBuffer = decodeOutputBuffer;
    return 0;
}

}



