//
// Created by Tank on 2022/9/16.
//

#ifndef  ANDROID_LIBTRANSCODE_HW_DECODE_H
#define  ANDROID_LIBTRANSCODE_HW_DECODE_H

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>

#include "debug.h"

enum DecodeState{
    DECODE_NOT_START = 0,
    DECODING,
    DECODE_FINISH
};

int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type);

enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);

int decode_write(AVCodecContext *avctx, AVPacket *packet);

int start_hw_decode();

int stop_hw_decode();

//enum DcodeState get_decode_state();

#endif //ANDROID_LIBTRANSCODE_HW_DECODE_H
