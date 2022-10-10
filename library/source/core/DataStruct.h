//
// Created by Tank on 2022/10/9.
//

#ifndef ANDROID_LIBTRANSCODE_DATASTRUCT_H
#define ANDROID_LIBTRANSCODE_DATASTRUCT_H

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

#endif //ANDROID_LIBTRANSCODE_DATASTRUCT_H
