//
// Created by Tank on 2022.11.02
//
#include "RawVideoDataBuffer.h"

RawVideoDataBuffer::RawVideoDataBuffer():DataBuffer(YUV_QUEUE_SIZE) {

}

RawVideoDataBuffer::~RawVideoDataBuffer() {

}

int RawVideoDataBuffer::Enqueue(const char *inputYUV, int width, int height, int len,unsigned long long timestamp,int yuv_format,int index) {
    AutoLock lock(mQueueLock);
    if(!inputYUV || len<=0)
        return -1;
    bool ret;
    LOGD("Enqueue YUV start...");
    YUVFrame * frame = (YUVFrame*)CreateItem(sizeof (YUVFrame));
    frame->yuvlen = len;
    frame->width = width;
    frame->height = height;
    frame->index = index;
    frame->start_time = timestamp;
    frame->format_type = yuv_format;
    frame->yuv = malloc(len);
    memcpy(frame->yuv,inputYUV,len);

    if(IsFull()){
        LOGD("Enqueue when queue is full! Queue Capacity:%d, Queue size:%d",GetQueueMaxDepth(),GetQueueDepth());
        auto *popFrame = (YUVFrame*)Pop();
        LOGD("Enqueue POP head, now Queue size:%d",GetQueueDepth());
        if(!popFrame){
            LOGD("Enqueue POP head and free item end");
        }
    }

    ret = Push((QueueItem*)frame);

    LOGD("Enqueue POP finish,ret=%d",ret);
    return ret;
}


int RawVideoDataBuffer::ClearFrames() {

}

YUVFrame * RawVideoDataBuffer::GetHeadFrame() {
    AutoLock lock(mQueueLock);
    return (YUVFrame*)GetFront();
}

int RawVideoDataBuffer::FlushBuffer() {
    AutoLock lock(mQueueLock);
    EraseQueue();
    return 0;
}