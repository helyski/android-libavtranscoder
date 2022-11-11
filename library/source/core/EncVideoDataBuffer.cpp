//
// Created by Tank on 2022.11.02
//
#include "EncVideoDataBuffer.h"

EncVideoDataBuffer::EncVideoDataBuffer():DataBuffer(H264_QUEUE_SIZE) {

}

EncVideoDataBuffer::~EncVideoDataBuffer() {

}

int EncVideoDataBuffer::Enqueue(const char *h264,int nal_type,int len,int width,int height,int64_t pts,int64_t sys_time,int index) {
    AutoLock lock(mBufferLock);
    if(!h264 || len<=0)
        return -1;
    bool ret;
    LOGD("EncVideoDataBuffer::Enqueue H264 start...");
    H264Frame * frame = (H264Frame*)CreateItem(sizeof (H264Frame));
    frame->len = len;
    frame->nal_type = nal_type;
    frame->index = index;
    frame->pts = pts;
    frame->system_time = sys_time;
    frame->width = width;
    frame->height = height;
    frame->h264 = malloc(len);
    memcpy(frame->h264,h264,len);

    if(IsFull()){
        LOGD("EncVideoDataBuffer::Enqueue when queue is full! Queue Capacity:%d, Queue size:%d",GetQueueMaxDepth(),GetQueueDepth());
        auto *popFrame = (H264Frame *)Pop();
        LOGD("EncVideoDataBuffer::Enqueue POP head, now Queue size:%d",GetQueueDepth());
        if(!popFrame){
            LOGD("EncVideoDataBuffer::Enqueue POP head and free item end");
        }
    }

    ret = Push((QueueItem*)frame);

    LOGD("EncVideoDataBuffer::Enqueue finish,ret=%d",ret);
    return ret;
}

H264Frame * EncVideoDataBuffer::GetHeadFrame() {
    AutoLock lock(mBufferLock);
    return (H264Frame*)GetFront();
}

int EncVideoDataBuffer::FlushBuffer() {
    AutoLock lock(mBufferLock);
    EraseQueue();
    return 0;
}