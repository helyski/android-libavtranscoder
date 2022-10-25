//
// Created by Tank on 2022/10/24.
//

#ifndef ANDROID_LIBTRANSCODE_DECODE_TOOL_H
#define ANDROID_LIBTRANSCODE_DECODE_TOOL_H

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "debug.h"
#include "DataStruct.h"

int _4_debugger_show_stream_info(const struct AVFormatContext * const input_ctx,const char * log_tag);

int get_decode_info_from_input(AVFormatContext *input_ctx,VideoInfo *pVideoInfo,AudioInfo *pAudioInfo);


#endif //ANDROID_LIBTRANSCODE_DECODE_TOOL_H
