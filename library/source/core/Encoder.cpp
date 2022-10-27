//
// Created by Tank on 2022/10/9.
//
#include "Encoder.h"
extern "C"
{

    Encoder::Encoder(){
        mIsThreadStart = false;
        mExit = false;

    }

    Encoder::~Encoder() {
        LOGW("~Encoder");
    }

    int Encoder::StartEncode() {

        return 1;
    }

    bool Encoder::StartThread() {
        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
        }
        mThread.registerThreadProc(*this);
        mThread.start(mThreadName);
        mIsThreadStart = true;
        LOGW("Encoder::StartThread");
        return true;
    }

    void Encoder::StopThread() {
        mExit = true;
        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
        }
        LOGW("Encoder::StopThread");
    }

    bool Encoder::process(int thread_id, void *env) {
        const char* path = "/sdcard/videos/encoder/123.yuv";
        FILE * file = 0;

        file = open_file_c(path);

        LOGW("Encoder_tid%d::start loop in process!",thread_id);
        int temp = 0;
        while(!mExit){
            usleep(50000);
            LOGW("Encoder_tid%d::%d!",thread_id,temp++);
            // TODO encode here
//            break;
            if(mYuvBuffer){
                YUVFrame frame = mYuvBuffer->getHeadElement();
                LOGW("Encoder_tid%d::yuv len",thread_id,frame.yuvlen);
                if(file){
                    fwrite(frame.yuv,1,frame.yuvlen,file);
                }
            }
        }
        LOGW("Encoder_tid%d::exit loop in process!",thread_id);
        return false;
    }


int Encoder::SetInputBuffer(RingQueue<YUVFrame> *yuvBuffer) {
    mYuvBuffer = yuvBuffer;
    return 0;
}

int Encoder::SetOutputBuffer(RingQueue<H264Frame> *h264Buffer) {
    mH264Buffer = h264Buffer;
    return 0;
} ;

}

