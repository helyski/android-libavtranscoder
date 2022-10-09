//
// Created by mapgoo1426 on 2022/10/9.
//

#ifndef ANDROID_LIBTRANSCODE_DATASTRUCT_H
#define ANDROID_LIBTRANSCODE_DATASTRUCT_H


struct YUVFrame {
    void * yuv;
    int yuvlen;
    int width;              //YUV宽
    int height;             //YUV高
    int format_type;        //YUV颜色格式
    long long duration;
    unsigned long long start_time;  //帧系统时间
};

#endif //ANDROID_LIBTRANSCODE_DATASTRUCT_H
