//
// Created by Tank on 2022.11.02
//

#ifndef ANDROID_LIBTRANSCODE_RAWVIDEODATABUFFER
#define ANDROID_LIBTRANSCODE_RAWVIDEODATABUFFER

#include "RingQueue.h"
#include "Lock.h"
#include "DataStruct.h"
#include "DataBuffer.h"

#define YUV_QUEUE_SIZE 4

class RawVideoDataBuffer : public DataBuffer {
public:
    RawVideoDataBuffer();

    ~RawVideoDataBuffer();

    int Enqueue(const char *inputYUV, int width, int height, int len,unsigned long long timestamp,int yuv_format,int index);

    YUVFrame *GetHeadFrame();

    int ClearFrames();

    int FlushBuffer();

private:
    Lock mQueueLock;

};

#endif //ANDROID_LIBTRANSCODE_RAWVIDEODATABUFFER
