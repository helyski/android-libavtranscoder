//
// Created by Tank on 2022/9/16.
//

#include "hw_decode.h"



AVBufferRef *hw_device_ctx = NULL;
enum AVPixelFormat hw_pix_fmt;
FILE *output_file = NULL;
int write_head_yuv = 0;


int start_hwdecode() {

    AVFormatContext *input_ctx = NULL;
    int video_stream, ret;
    AVStream *video = NULL;
    AVCodecContext *decoder_ctx = NULL;
    AVCodec *decoder = NULL;
    AVPacket packet;
    enum AVHWDeviceType type;
    int i;

    const char * input_file_name = "/storage/sdcard0/DVR/front/loop/00_20220819125342.ts";
    const char * output_file_name = "/sdcard/mapgoologcat/hello.yuv";

    __android_log_print(5, "AVFILTER_TEST", "testHWDecode");


    const char* hw_device_name = "mediacodec";
    type = av_hwdevice_find_type_by_name(hw_device_name);
    __android_log_print(5, "AVFILTER_TEST", "AVHWDeviceType:%d",type);
    __android_log_print(5, "AVFILTER_TEST", "av_hwdevice_get_type_name:%s",av_hwdevice_get_type_name(type));
    if(type == AV_HWDEVICE_TYPE_NONE){
        __android_log_print(5,"AVFILTER_TEST","hw_decode_init Can not found av_hwdevice by name:%s", hw_device_name);
        __android_log_print(5,"AVFILTER_TEST","hw_decode_init Support av_hwdevice list:");
        while((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
            __android_log_print(5,"AVFILTER_TEST","--%s",av_hwdevice_get_type_name(type));
        }
        return -1;
    }

    if(avformat_open_input(&input_ctx,input_file_name,NULL,NULL) !=0){
        __android_log_print(5,"AVFILTER_TEST","Cannot open input file '%s'",input_file_name);
        return -1;
    }

    LOGW("avformat_open_input success!");



    if(avformat_find_stream_info(input_ctx,NULL) <0){
        LOGW("avformat_find_stream_info failed!!!");
        return -1;
    }

    LOGW("avformat_find_stream_info success!");


    ret = av_find_best_stream(input_ctx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);

    LOGW("avformat_find_stream_info video_bitrate:%lld, width:%d, height:%d",input_ctx->bit_rate,input_ctx->streams[ret]->codecpar->width,input_ctx->streams[ret]->codecpar->height);


    if(ret<0){
        LOGW("av_find_best_stream failed!");
        return -1;
    }




    video_stream = ret;

    decoder = avcodec_find_decoder_by_name("h264_mediacodec");

    if(!decoder){
        LOGD("can not find ‘h264_mediacodec’");
        return -1;
    }


    LOGW("av_find_best_stream success! Show decoder info: name=%s, type=%d,hw_config=%d",decoder->name,decoder->type,decoder->hw_configs);

    for (i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i);
        if (!config) {
            LOGW("Decoder %s does not support device type %s.\n",
                 decoder->name, av_hwdevice_get_type_name(type));
            return -1;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
            config->device_type == type) {
            hw_pix_fmt = config->pix_fmt;
            break;
        }
    }

    LOGW("hw_pix_fmt = %d",hw_pix_fmt);

    int64_t jump_seconds = 10;

    int seek_result = av_seek_frame(input_ctx,video_stream,jump_seconds/av_q2d(input_ctx->streams[video_stream]->time_base),AVSEEK_FLAG_FRAME);

    LOGW("av_seek_frame result = %d",seek_result);

    decoder_ctx = avcodec_alloc_context3(decoder);

    if(!decoder_ctx){
        LOGW("avcodec_alloc_context3 failed");
        return -1;
    }

    ret = avcodec_parameters_to_context(decoder_ctx,input_ctx->streams[video_stream]->codecpar);

    if(ret < 0){
        LOGW("avcodec_parameters_to_context failed");
        return -1;
    }
    LOGW("avcodec_parameters_to_context success!");

    decoder_ctx->get_format = get_hw_format;

    if (hw_decoder_init(decoder_ctx, type) < 0) {
        LOGW("hw_decoder_init failed");
        return -1;
    }

    LOGW("hw_decoder_init success !");


    if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0) {
        LOGW("avcodec_open2: %d(%s)", ret, av_err2str(ret));
        return -1;
    }

    LOGW("avcodec_open2 success !");

    remove(output_file_name);
    output_file = fopen(output_file_name,"wb+");

    while(ret>=0){
        LOGW("decodeYUV start read H264 frame.....");
        if ((ret = av_read_frame(input_ctx, &packet)) < 0) {
            LOGW("decodeYUV finish_ERROR read H264 frame.....#");
            break;
        }

        LOGW("decodeYUV finish_SUCCESS read H264 frame.....#");

        if (video_stream == packet.stream_index)
            ret = decode_write(decoder_ctx, &packet);

        av_packet_unref(&packet);
    }


    packet.data = NULL;
    packet.size = 0;
    ret = decode_write(decoder_ctx, &packet);
    av_packet_unref(&packet);

    if (output_file)
        fclose(output_file);
    avcodec_free_context(&decoder_ctx);
    avformat_close_input(&input_ctx);
    av_buffer_unref(&hw_device_ctx);


    return 0;

}

enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                 const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    LOGW("Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type) {
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
                                      NULL, NULL, 0)) < 0) {
        LOGW("Failed to create specified HW device.\n");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    return err;
}

int decode_write(AVCodecContext *avctx, AVPacket *packet) {
    AVFrame *frame = NULL, *sw_frame = NULL;
    AVFrame *tmp_frame = NULL;
    uint8_t *buffer = NULL;
    int size;
    int ret = 0;

    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0) {
        LOGW("avcodec_send_packet: %d(%s)", ret, av_err2str(ret));
        if(ret != AVERROR(EAGAIN)){
            return ret;
        }
    }

    while (1) {
        if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
            LOGW("Can not alloc frame\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        ret = avcodec_receive_frame(avctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            return 0;
        } else if (ret < 0) {
            LOGW("Error while decoding\n");
            goto fail;
        }

        LOGW("receive video frame success: pts=%lld",frame->pts);

        if (frame->format == hw_pix_fmt) {
            if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
                LOGW("Error transferring the data to system memory\n");
                goto fail;
            }
            tmp_frame = sw_frame;
        } else
            tmp_frame = frame;

        size = av_image_get_buffer_size(tmp_frame->format,
                                        tmp_frame->width,
                                        tmp_frame->height, 1);
        buffer = av_malloc(size);
        if (!buffer) {
            LOGW("Can not alloc buffer\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        //本次测试所得：AV_PIX_FMT_NV12
        ret = av_image_copy_to_buffer(buffer, size,
                                      (const uint8_t *const *) tmp_frame->data,
                                      (const int *) tmp_frame->linesize,
                                      tmp_frame->format,
                                      tmp_frame->width, tmp_frame->height, 1);
        if (ret < 0) {
            LOGW("Can not copy image to buffer\n");
            goto fail;
        }

//        if ((ret = fwrite(buffer, 1, size, output_file)) < 0) {
//            LOGW("Failed to dump raw data.\n");
//            goto fail;
//        }

        if(!write_head_yuv){
            fwrite(buffer, 1, size, output_file);
            fclose(output_file);
            write_head_yuv = 1;
        }

        fail:
        av_frame_free(&frame);
        av_frame_free(&sw_frame);
        av_freep(&buffer);
        if (ret < 0)
            return ret;
    }
}
