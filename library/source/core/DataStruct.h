//
// Created by Tank on 2022/10/9.
//

#ifndef ANDROID_LIBTRANSCODE_DATASTRUCT_H
#define ANDROID_LIBTRANSCODE_DATASTRUCT_H

#include "stdint.h"

typedef struct _basicTranscodingParams{
    int width;
    int height;
    int bitrate;
    int fps;
    float seek_seconds;
    char *originVideoPath;
    char *destVideoName;
}BasicTranscodingParams;

struct YUVFrame {
    void * yuv;
    int yuvlen;
    int width;              //YUV宽
    int height;             //YUV高
    int format_type;        //YUV颜色格式
    long long duration;
    unsigned long long start_time;  //帧系统时间
};

struct H264Frame{
    void * H264;
    int width;
    int height;
    int len;
};


struct _VideoInfo {
    int width;
    int height;
    int b_bnnex_b;
    int bitrate;
    int fps;
    int gop;
    unsigned char *decode_info;
    int decode_info_len;
    int64_t durationus;
};

struct _AudioInfo {
    int sample_rate;
    int sample_size;
    int frame_size;
    int channels;
    int bitrate;
    unsigned char *decode_info;
    int decode_info_len;
    int64_t durationus;
};


typedef struct _VideoInfo VideoInfo;
typedef struct _AudioInfo AudioInfo;

#endif //ANDROID_LIBTRANSCODE_DATASTRUCT_H
