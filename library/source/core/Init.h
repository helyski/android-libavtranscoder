//
// Created by HelyskiTank on 2022/10/8.
//

#ifndef ANDROID_LIBTRANSCODE_INIT_H
#define ANDROID_LIBTRANSCODE_INIT_H


#ifdef __cplusplus
extern "C"
{
#endif

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

#ifdef __cplusplus
};
#endif

namespace LibTranscode {
    class Init {
    public:
        Init();
        ~Init();

        static void init_ffmpeg_log();


    private:
        static void ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vl);

    };
}


#endif //ANDROID_LIBTRANSCODE_INIT_H
