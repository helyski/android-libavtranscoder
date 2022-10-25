//
// Created by Tank on 2022/10/24.
//

#include "decode_tool.h"

int _4_debugger_show_stream_info(const struct AVFormatContext * const input_ctx,const char* TAG){
    int i=0;
    LOGW("%s :: AVFormatContext->bitrate:%lld",TAG, input_ctx->bit_rate);
    for(i=0;i<input_ctx->nb_streams;i++){
        AVStream *st = input_ctx->streams[i];
        struct AVCodecContext *avc_ctx = st->codec;
        AVCodecParameters *codecpar = st->codecpar;

        LOGW("%s :: analysis stream[%d] .......................",TAG, i);
        // From AVStream
        LOGW("%s :: ... ... AVStream ... ...",TAG);
        LOGW("%s :: AVStream->start_time:%lld",TAG, st->start_time);
        LOGW("%s :: AVStream->duration:%lld",TAG, st->duration);
        LOGW("%s :: AVStream->time_base.num:%d",TAG, st->time_base.num);
        LOGW("%s :: AVStream->time_base.den:%d",TAG, st->time_base.den);
        LOGW("%s :: AVStream->avg_frame_rate.num:%d",TAG, st->avg_frame_rate.num);
        LOGW("%s :: AVStream->avg_frame_rate.den:%d",TAG, st->avg_frame_rate.den);
        LOGW("%s :: AVStream->r_frame_rate.num:%d",TAG, st->r_frame_rate.num);
        LOGW("%s :: AVStream->r_frame_rate.den:%d",TAG, st->r_frame_rate.den);
        // From AVCodexContext
        LOGW("%s ::  ... ... AVCodexContext ... ...",TAG);
        LOGW("%s :: AVCodecContext->time_base.num:%d",TAG, avc_ctx->time_base.num);
        LOGW("%s :: AVCodecContext->time_base.den:%d",TAG, avc_ctx->time_base.den);
        LOGW("%s :: AVCodecContext->framerate.den:%d",TAG, avc_ctx->framerate.den);
        LOGW("%s :: AVCodecContext->framerate.num:%d",TAG, avc_ctx->framerate.num);
        LOGW("%s :: AVCodecContext->codec_id:%d",TAG, avc_ctx->codec_id);
        LOGW("%s :: AVCodecContext->codec_type:%d",TAG, avc_ctx->codec_type);
        LOGW("%s :: AVCodecContext->width:%d",TAG, avc_ctx->width);
        LOGW("%s :: AVCodecContext->height:%d",TAG, avc_ctx->height);
        LOGW("%s :: AVCodecContext->gop_size:%d",TAG, avc_ctx->gop_size);
        LOGW("%s :: AVCodecContext->bitrate:%lld",TAG, avc_ctx->bit_rate);
        LOGW("%s :: AVCodecContext->extradata_size:%d",TAG, avc_ctx->extradata_size);
        // From AVFormatContext
        LOGW("%s ::  ... ... AVFormatContext ... ...",TAG);
        LOGW("%s :: AVFormatContext->bitrate:%lld",TAG, input_ctx->bit_rate);
        // From AVCodecParameters
        LOGW("%s ::  ... ... AVCodecParameters ... ...",TAG);
        LOGW("%s :: AVCodecParameters->bit_rate:%lld",TAG, codecpar->bit_rate);
        LOGW("%s :: AVCodecParameters->codec_type:%d",TAG, codecpar->codec_type);
        LOGW("%s :: AVCodecParameters->width:%d",TAG, codecpar->width);
        LOGW("%s :: AVCodecParameters->height:%d",TAG, codecpar->height);
        LOGW("%s :: AVCodecParameters->format:%d",TAG, codecpar->format);
        LOGW("%s :: AVCodecParameters->sample_aspect_ratio.num:%d",TAG, codecpar->sample_aspect_ratio.num);
        LOGW("%s :: AVCodecParameters->sample_aspect_ratio.den:%d",TAG, codecpar->sample_aspect_ratio.den);
        LOGW("%s :: AVCodecParameters->extradata_size:%d",TAG, codecpar->extradata_size);
        LOGW("%s :: AVCodecParameters->color_space:%d",TAG, codecpar->color_space);

        if (avc_ctx->extradata_size > 0) {
            LOGW("%s :: AVCodecContext->extradata:",TAG);
            unsigned int k;
            for (k = 0; k < avc_ctx->extradata_size; k++) {
                LOGW("%s :: extradata[%d]:  %X",TAG, k, avc_ctx->extradata[k]);
            }
        }

        if (codecpar->extradata_size > 0) {
            LOGW("%s :: AVCodecParameters->extradata:",TAG);
            unsigned int k;
            for (k = 0; k < codecpar->extradata_size; k++) {
                LOGW("%s :: extradata[%d]:  %X",TAG, k, codecpar->extradata[k]);
            }
        }



        if (avc_ctx->codec_type == AVMEDIA_TYPE_VIDEO){
            LOGW("%s :: stream type:%s", TAG,"VIDEO");
        }

        if (avc_ctx->codec_type == AVMEDIA_TYPE_AUDIO){
            LOGW("%s :: stream type:%s", TAG,"AUDIO");
        }
    }

    return 0;
};

/**
 * Get Codec parameter from AVFormatContext.
 *  * Some parameters in AVFormatContext.
 *      - bitrate
 *  * Some in AVFormatContext->AVStream.
 *      - fps
 *  * Some in AVFormatContext->AVStream->AVCodecContext (attribute_deprecated)
 *      or AVFormatContext->AVStream->AVCodecParameters
 *      - width
 *      - height
 * @param input_ctx
 * @param pVideoInfo
 * @param pAudioInfo
 * @return
 */
int get_decode_info_from_input(AVFormatContext *input_ctx,VideoInfo *pVideoInfo,AudioInfo *pAudioInfo){
    if(!input_ctx){
        return -1;
    }
    int i;
    for(i=0;i<input_ctx->nb_streams;i++) {
        AVStream *st = input_ctx->streams[i];
        struct AVCodecParameters * codecpar = st->codecpar;
        // Analysis video
       if(AVMEDIA_TYPE_VIDEO == codecpar->codec_type){
           if(pVideoInfo){
               pVideoInfo->bitrate = input_ctx->bit_rate;
               pVideoInfo->width = codecpar->width;
               pVideoInfo->height = codecpar->height;
               if(st->avg_frame_rate.den > 0) {
                   pVideoInfo->fps = st->avg_frame_rate.num / st->avg_frame_rate.den;
               }
               if(pVideoInfo->fps <=0) {
                   if (st->codec->framerate.den > 0) {
                       pVideoInfo->fps = st->codec->framerate.num / st->codec->framerate.den;
                   }
               }

               if(codecpar->extradata_size > 0){
                   pVideoInfo->decode_info = malloc(sizeof(codecpar->extradata_size));
                   if(pVideoInfo->decode_info){
                       memcpy(pVideoInfo->decode_info,codecpar->extradata,codecpar->extradata_size);
                       pVideoInfo->decode_info_len = codecpar->extradata_size;
                   }
               }
           }
       }
    }


    return 0;
}
