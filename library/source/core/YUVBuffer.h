//
// Created by Tank on 2022/11/2.
//

#ifndef ANDROID_LIBTRANSCODE_YUVBUFFER_H
#define ANDROID_LIBTRANSCODE_YUVBUFFER_H


#include "RingQueue.h"
#include "Lock.h"
#include "DataStruct.h"

class YUVBuffer{
public:
    YUVBuffer(int capacity);
    ~YUVBuffer();

    int Enqueue(const char* yuv,int width,int height,int len);

    YUVFrame* GetHeadFrame();

    int ClearFrames();

private:
    Lock mQueueLock;
    int mCapacity = 3;


    YUVFrame * frames[3];

    RingQueue<YUVFrame> *mRingBuffer;

};


#endif //ANDROID_LIBTRANSCODE_YUVBUFFER_H
