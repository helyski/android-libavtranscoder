//
// Created by Tank on 2022.11.02
//

#ifndef ANDROID_LIBTRANSCODE_ENCVIDEODATABUFFER
#define ANDROID_LIBTRANSCODE_ENCVIDEODATABUFFER

#include "RingQueue.h"
#include "Lock.h"
#include "DataStruct.h"
#include "DataBuffer.h"

#define H264_QUEUE_SIZE 4

class EncVideoDataBuffer : public DataBuffer {
public:
    EncVideoDataBuffer();

    ~EncVideoDataBuffer();

    int Enqueue(const char *h264,int nal_type,int len,int width,int height,int64_t pts,int64_t sys_time,int index);

    H264Frame *GetHeadFrame();

    int FlushBuffer();

};

#endif //ANDROID_LIBTRANSCODE_ENCVIDEODATABUFFER
