//
// Created by Tank on 2022/11/4.
//

#include "Dispatcher.h"

Dispatcher::Dispatcher() {
    mIsThreadStart = false;
    mExit = false;
    mVideoBuffer = 0;
    mLastIndex = 0;
}

Dispatcher::~Dispatcher(){

}

bool Dispatcher::StartThread() {
    if (mIsThreadStart) {
        mThread.stop();
        mIsThreadStart = false;
    }
    mThread.registerThreadProc(*this);
    mThread.start(mThreadName);
    mIsThreadStart = true;
    return true;
}

void Dispatcher::StopThread() {
    mExit = true;
    {
        AutoLock lock(mThreadLock);
        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
        }
    }
}

void Dispatcher::SetDateBuffer(EncVideoDataBuffer *dataBuffer) {
    mVideoBuffer = dataBuffer;
}

void Dispatcher::SetVideoFrameCallBack(video_frame_call_back callBack) {
    mVideoCallBack = callBack;
}


bool Dispatcher::process(int thread_id, void *env) {
    LOGD("Dispatcher start loop.");
    while(!mExit){
        if(mVideoBuffer){
            AutoLock lock(mVideoBuffer->GetLock());
            H264Frame *frame = mVideoBuffer->GetHeadFrame();
            if(!frame)
                goto end;
            if(frame->index>mLastIndex) {
                LOGD("Dispatcher getFrame len:%d, nal_type:%d, width:%d,height:%d,sys_time:%lld,pts:%lld,index:%d",
                     frame->len, frame->nal_type, frame->width, frame->height, frame->system_time,
                     frame->pts, frame->index);
                if(frame->index - mLastIndex >1){
                    LOGE("Dispatcher lost ..........................................");
                }

                mLastIndex = frame->index;

                if(mVideoCallBack){
                    mVideoCallBack(0,frame->h264,frame->len,frame->pts,
                                   is_h264_keyframe(frame->h264,frame->len),
                                   get_nal_type(frame->h264,frame->len));
                }

                // TODO next.
            }

        }

        end:
        usleep(1000);
    }

    LOGD("Dispatcher stop loop.");

    return false;

}
