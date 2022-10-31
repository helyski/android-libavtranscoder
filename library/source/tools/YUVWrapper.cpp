//
// Created by Tank on 2022/10/31.
//
// Notice: Not original code.

#include "YUVWrapper.h"

#include "libyuv.h"

using namespace libyuv;

void NV21ToYUV420P(const unsigned char *srcData,
                            unsigned char *destData, int srcWidth,
                            int srcHeight) {

    int src_size_y = srcWidth * srcHeight;
    int src_size_vu = (srcWidth * srcHeight >> 2);
    const unsigned char *src_y_ptr = srcData;
    const unsigned char *src_vu_ptr = srcData + src_size_y;

    unsigned char *dest_i420_y_data = destData;

    unsigned char *dest_i420_u_data = dest_i420_y_data + src_size_y;

    unsigned char *dest_i420_v_data = dest_i420_y_data + src_size_y + src_size_vu;


    libyuv::NV21ToI420(src_y_ptr, srcWidth,
                       src_vu_ptr, srcWidth,
                       dest_i420_y_data, srcWidth,
                       dest_i420_u_data, srcWidth >> 1,
                       dest_i420_v_data, srcWidth >> 1,
                       srcWidth, srcHeight);

}

void ScaleI420(const unsigned char *srcData, int srcWidth, int srcHeight,
                        unsigned char *destData, int destWidth, int destHeight) {
    int ysize, usize, yscalesize, uscalesize;
    int ystride = srcWidth;
    int uvstride = (srcWidth + 1) / 2;
    int scaledystride = destWidth;
    int scaleduvstride = (destWidth + 1) / 2;
    const unsigned char *y, *u, *v;
    unsigned char *scale_y, *scale_u, *scale_v;

    ysize = srcWidth * srcHeight;
    usize = ((srcWidth + 1) >> 1) * ((srcHeight + 1) >> 1);
    y = (unsigned char *) srcData;
    u = y + ysize;
    v = u + usize;

    yscalesize = destWidth * destHeight;
    uscalesize = ((destWidth + 1) >> 1) * ((destHeight + 1) >> 1);
    scale_y = destData;
    scale_u = scale_y + yscalesize;
    scale_v = scale_u + uscalesize;


    I420Scale(y, ystride,
              u, uvstride,
              v, uvstride,
              srcWidth, srcHeight,
              scale_y, scaledystride,
              scale_u, scaleduvstride,
              scale_v, scaleduvstride,
              destWidth, destHeight,
              kFilterNone);

}