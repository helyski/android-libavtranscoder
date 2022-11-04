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

struct _YUVFrame {
    void * yuv;
    int yuvlen;
    int width;              //YUV宽
    int height;             //YUV高
    int format_type;        //YUV颜色格式
    long long duration;
    unsigned long long start_time;  //帧系统时间
    int index;
};

struct _H264Frame{
    void * h264;
    int nal_type;
    int width;
    int height;
    int len;
    int index;
    int64_t pts;
    int64_t system_time;
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
typedef struct _YUVFrame YUVFrame;
typedef struct _H264Frame H264Frame;

enum TRANSCODE_OUTPUT_TYPE{
    OUTPUT_VIDEO_FILE = 0,
    OUTPUT_H264_STREAM_SHARE_TO_JAVA ,
    OUTPUT_H264_STREAM_PUSH_TO_RTMP_SERVER
};

#endif //ANDROID_LIBTRANSCODE_DATASTRUCT_H
