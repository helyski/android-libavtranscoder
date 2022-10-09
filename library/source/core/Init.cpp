//
// Created by HelyskiTank on 2022/10/8.
//

#include "Init.h"

namespace LibTranscode {

    Init::Init() {

    }

    Init::~Init() {

    }

    void Init::ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vl) {
        va_list vl2;
        char *line = static_cast<char *>(malloc(128 * sizeof(char)));
        static int print_prefix = 1;
        va_copy(vl2, vl);
        av_log_format_line(ptr, level, fmt, vl2, line, 128, &print_prefix);
        va_end(vl2);
        line[127] = '\0';
        LOGW("ffmpeg_log ->%s\n", line);
        free(line);
    }

    void Init::init_ffmpeg_log() {
        av_log_set_level(AV_LOG_TRACE);
        av_log_set_callback(ffmpeg_log_callback);
    }

}
