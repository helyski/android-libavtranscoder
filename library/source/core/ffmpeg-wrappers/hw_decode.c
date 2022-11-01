//
// Created by Tank on 2022/9/16.
//

#include "hw_decode.h"

const char * TAG = "hw_decode";

AVBufferRef *hw_device_ctx = NULL;
enum AVPixelFormat hw_pix_fmt;
char is_stop = 0;
enum DecodeState decode_state = DECODE_NOT_START;

VideoInfo  *videoInfo;
AudioInfo *audioInfo;

// For test.
FILE *output_file = NULL;

AVFormatContext *input_ctx = NULL;
AVStream *video = NULL;
AVCodecContext *decoder_ctx = NULL;
AVCodec *decoder = NULL;
AVPacket packet;
enum AVHWDeviceType type;
int video_stream;


// For test.
int write_head_yuv = 0;

#if 1
const char * output_file_name = "/sdcard/VideosForLibtranscode/hello.yuv";
#endif

enum DecodeState get_decode_state(){

    return decode_state;
}

int stop_hw_decode(){
    is_stop = 1;
    return 0;
}

int start_hw_decode(const char* input_file_path,float seek_seconds){
    return 0;
}


int release_decoder_ctx(){
    LOGW("release_decoder_ctx 111");

    packet.data = NULL;
    packet.size = 0;

    // TODO need to fix this,av_packet_unref will crash in google nexus5 android11.
//    ret = decode_write(decoder_ctx, &packet);
//    av_packet_unref(&packet);
    LOGW("release_decoder_ctx 222");
//    if (output_file)
//        fclose(output_file);

    LOGW("release_decoder_ctx 333");

    if(decoder_ctx) {
        avcodec_free_context(&decoder_ctx);
        decoder_ctx = NULL;
    }

    LOGW("release_decoder_ctx 444-2");
    if(hw_device_ctx) {
        av_buffer_unref(&hw_device_ctx);
        hw_device_ctx = NULL;
    }

    LOGW("release_decoder_ctx 444-1");
    if(input_ctx) {
        avformat_close_input(&input_ctx);
        input_ctx = NULL;
    }

    LOGW("release_decoder_ctx 444-3");
    if(videoInfo){
        if(videoInfo->decode_info){
            free(videoInfo->decode_info);
            videoInfo->decode_info = NULL;
        }
        free(videoInfo);
        videoInfo = NULL;
    }
    LOGW("release_decoder_ctx 555");

    if(audioInfo){
        if(audioInfo->decode_info){
            free(audioInfo->decode_info);
            audioInfo->decode_info = NULL;
        }
        free(audioInfo);
        audioInfo = NULL;
    }
    LOGW("release_decoder_ctx 666");

    decode_state = DECODE_FINISH;

    return 0;
}


int init_decoder_ctx(const char* input_file_path,float seek_seconds) {
    is_stop = 0;
    decode_state = DECODING;

    int ret;

    int i;



    /**
     * Find a hw device , Android mostly is mediacodec.If can not find hw device,process will exit.
     */
    const char *hw_device_name = "mediacodec";
    type = av_hwdevice_find_type_by_name(hw_device_name);

    LOGW("%s :: av_hwdevice_find_type_by_name(%s) type=%d ",TAG,hw_device_name,type);
    LOGW("%s :: av_hwdevice_get_type_name(%d) name=%s ",TAG,type,av_hwdevice_get_type_name(type));

    if(type == AV_HWDEVICE_TYPE_NONE){
        LOGW("%s :: can not found 'mediacodec',encoder will exit.Support hw_device list:",TAG);
        while((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
            LOGW("%s :: hw_device:",TAG,av_hwdevice_get_type_name(type));
        }
        return -1;
    }

    /**
     * Open input file, if open success AVFormatContext will be init.
     */
    if(avformat_open_input(&input_ctx,input_file_path,NULL,NULL) !=0){
        LOGW("%s :: avformat_open_input can not open input file '%s'",TAG,input_file_path);
        return -1;
    }
    LOGW("%s :: avformat_open_input success!",TAG);

    /**
     * VideoInfo and AudioInfo
     */
    videoInfo = (VideoInfo*)malloc(sizeof (VideoInfo));
    memset(videoInfo,0,sizeof(VideoInfo));
    audioInfo = (AudioInfo*)malloc(sizeof (AudioInfo));
    memset(audioInfo,0,sizeof (AudioInfo));

    /**
     * Find Stream Info,Get Video Info from video stream and audio stream.
     */
    if(avformat_find_stream_info(input_ctx,NULL) <0){
        LOGW("%s :: avformat_find_stream_info error...",TAG);
        goto end;
    }
    LOGW("%s :: avformat_find_stream_info success!",TAG);

    /**
     * FIXME only for debug
     */
    _4_debugger_show_stream_info(input_ctx,TAG);

    get_decode_info_from_input(input_ctx,videoInfo,audioInfo);

    if(videoInfo){
        LOGW("%s :: videoInfo fps=%d",TAG,videoInfo->fps);
        LOGW("%s :: videoInfo bitrate=%d",TAG,videoInfo->bitrate);
        LOGW("%s :: videoInfo width=%d",TAG,videoInfo->width);
        LOGW("%s :: videoInfo height=%d",TAG,videoInfo->height);

        int k;
        for (k = 0; k < videoInfo->decode_info_len; k++) {
            LOGW("%s :: decode_info [%d]:  %X",TAG, k, videoInfo->decode_info[k]);
        }
    }
    /**
     * Get video and audio codec info from AVFormatContext,AVStream,AVCodecContext.
     *
     */
//     get_decode_info_from_input(input_ctx,&videoInfo,&audioInfo);


    ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    LOGW("%s :: avformat_find_stream_info video_bitrate:%lld, width:%d, height:%d",TAG,
         input_ctx->bit_rate,input_ctx->streams[ret]->codecpar->width, input_ctx->streams[ret]->codecpar->height);
    if (ret < 0) {
        LOGW("%s :: av_find_best_stream error...",TAG);
        goto end;
    }
    video_stream = ret;

//    _4_debugger_show_stream_info(input_ctx,TAG);

    /**
     * Find decoder.
     */
    decoder = avcodec_find_decoder_by_name("h264_mediacodec");
    if(!decoder){
        LOGD("%s :: avcodec_find_decoder_by_name can not find 'h264_mediacodec'",TAG);
        goto end;
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

    int64_t jump_seconds = 0;

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


    if(!decoder_ctx || !decoder){
        LOGW("avcodec_open2 error , try it again later !");
        return -1;
    }

    if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0) {
        LOGW("avcodec_open2: %d(%s)", ret, av_err2str(ret));
        return -1;
    }

    LOGW("avcodec_open2 success !");
    end:
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

int decode_frames = 0;

int decode(AVCodecContext *avctx, AVPacket *packet,unsigned char **output_buffer,int* buffer_size) {
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
            LOGW("avcodec_receive_frame erro !!!!!!!!!!!\n");
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            return REC_END;
        } else if (ret < 0) {
            LOGW("Error while decoding\n");
            goto fail;
        }

        LOGW("receive video frame success: pts=%lld , frame_counts:%d",frame->pts,++decode_frames);

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

        if(*output_buffer){
            free(*output_buffer);
        }
        *output_buffer = malloc(size);
        if(!*output_buffer){
            goto fail;
        }

        *buffer_size = size;
        memcpy(*output_buffer,buffer,size);

        fail:
        av_frame_free(&frame);
        av_frame_free(&sw_frame);
        av_freep(&buffer);
        if (ret < 0)
            return ret;
    }
}

int get_frame(unsigned char **output_buffer,int* buffer_size){

    int ret = 0;
    int is_get_frame = 0;
    do{
        LOGW("decodeYUV is_stop:%d.....",is_stop);
        if(is_stop){
            break;
        }
        LOGW("decodeYUV start read H264 frame.....");
        if ((ret = av_read_frame(input_ctx, &packet)) < 0) {
            LOGW("decodeYUV finish_ERROR read H264 frame.....#");
            return DEC_INPUT_END_ERR;
//            break;
        }

        LOGW("decodeYUV finish_SUCCESS read H264 frame.....#, index=%d",packet.stream_index);
        if(video_stream != packet.stream_index){
            continue;
        }

        if (video_stream == packet.stream_index)
            ret = decode(decoder_ctx, &packet,output_buffer,buffer_size);

        if(ret == REC_END){
            is_get_frame = 1;
        }

        LOGW("decodeYUV decode result -> %d",ret);

        av_packet_unref(&packet);

//        usleep(50000);

    }while(is_get_frame==0);

    LOGW("get_frame finish end");
    return 0;
}



VideoInfo* get_video_info(){
    return videoInfo;
}

AudioInfo* get_audio_info(){
    return audioInfo;
}
