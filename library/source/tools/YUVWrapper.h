//
// Created by Tank on 2022/10/31.
//
// Notice: Not original code.

#ifndef ANDROID_LIBTRANSCODE_YUVWRAPPER_H
#define ANDROID_LIBTRANSCODE_YUVWRAPPER_H

#include "libyuv.h"

void NV21ToYUV420P(const unsigned char *srcData, unsigned char *destData, int srcWidth,
                   int srcHeight);

void ScaleI420(const unsigned char *srcData, int srcWidth, int srcHeight,
               unsigned char *destData, int destWidth, int destHeight);



#endif //ANDROID_LIBTRANSCODE_YUVWRAPPER_H
