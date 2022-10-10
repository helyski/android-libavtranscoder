//
// Created by Tank on 2022/10/9.
//




extern "C"
{
#include "Decoder.h"


namespace LibTranscode {


    Decoder::Decoder() {
        mIsThreadStart = false;
        mExit = false;

    }

    Decoder::~Decoder() {
        LOGW("~~~~~~~~~~~~~~~~~Decoder");
    }

    int Decoder::StartDecode(const char* srcVideoPath,float seek_seconds) {
        start_hwdecode();
        return 1;
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

            start_hwdecode();
            break;
        }
        LOGW("Decoder_tid%d::exit loop in process!",thread_id);
        return false;
    }
}

};