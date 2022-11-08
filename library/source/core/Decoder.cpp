//
// Created by Tank on 2022/10/9.
//

#include "Decoder.h"
#include "RingQueue.h"

extern "C"
{

    Decoder::Decoder(){
        mIsThreadStart = false;
        mExit = false;
        mDecodeOutputBuffer = 0;
        mStatListener = 0;
    }

    Decoder::~Decoder() {
        LOGW("~Decoder");

//        if(mDecodeOutputBuffer){
//            LOGW("mDecodeOutputBuffer delete .......");
//            delete mDecodeOutputBuffer;
//            mDecodeOutputBuffer = NULL;
//        }
        LOGW("Decoder~");
    }

    bool Decoder::StartThread() {
        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
        }
        mThread.registerThreadProc(*this);
        mThread.start(mThreadName);
        mIsThreadStart = true;
        LOGW("Decoder::StartThread");
        return true;
    }

    void Decoder::StopThread() {
        mExit = true;
        LOGW("Decoder::StopThread start ~");
        {
            AutoLock lock(mDecoderLock);
            if (mIsThreadStart) {
                mThread.stop();
                mIsThreadStart = false;
            }
        }
        LOGW("Decoder::StopThread end ~");
    }

    bool Decoder::process(int thread_id, void *env) {
        LOGW("Decoder_tid%d::start loop in process!", thread_id);
//        uint32_t max_handle_time, ret = 0;
//        uint64_t last_decode_time;

        // FIXME todo
        FILE *debug_file = NULL;
        const char * path = "/sdcard/debugYUV/1027.yuv";

        int ret = 0,frame_index = 0;
        unsigned char *yuv_buffer=0;
        int yuv_size;
        VideoInfo *videoInfo;
        unsigned long long sleep_time ,start_decode_time,finish_decode_time,used_time,max_handle_time;
        sleep_time = 0;
        start_decode_time = 0;
        finish_decode_time = 0;
        {
            AutoLock lock(mDecoderLock);
            // Init decode context first.If has error,proc will exit.
//            ret = init_decoder_ctx(mSrcVideoPath, mDecodeSeekSeconds);
//            if (ret < 0) {
//                goto proc_end;
//            }
            // Check video codec info.If info not found,proc will exit.
            videoInfo = get_video_info();
            if (!videoInfo || videoInfo->width <= 0 || videoInfo->height <= 0) {
                goto proc_end;
            }
            // Calculating loop frequency.
            max_handle_time = GetFrameMaxHandleTimeMS(videoInfo->fps);
            if (max_handle_time <= 0) {
                goto proc_end;
            }

//        remove(path);
//        debug_file = open_file_c(path);

            unsigned long long lock_start_time;
            unsigned long long lock_finish_time;

            unsigned long long add_queue_finish_time;
            unsigned long long last_finish_time;

            unsigned long long one_frame_time;

            while (!mExit) {
                lock_start_time = get_system_current_time_millis();
                LOGW("Decoder loop start at:%lld, frame_max_handle_time:%lld", lock_start_time,
                     max_handle_time);

//            ret = start_hw_decode(mSrcVideoPath,10);
//            if(ret<0){
//                break;
//            }
                {
                    LOGW("Decoder wait_lock start at:%lld", lock_start_time);
                    AutoLock lock(mDecodeOutputBuffer->GetLock());
                    lock_finish_time = get_system_current_time_millis();
                    LOGW("Decoder wait_lock finish at:%lld, wait_time:%lld", lock_finish_time,
                         lock_finish_time - lock_start_time);

                    start_decode_time = get_system_current_time_millis();
                    ret = get_frame(&yuv_buffer, &yuv_size);
                    finish_decode_time = get_system_current_time_millis();
                    used_time = finish_decode_time - start_decode_time;
                    LOGW("Decoder decode finish at:%lld, decode use_time:%lld", finish_decode_time,
                         used_time);

                    if (mExit)
                        goto proc_end;

                    //            if(debug_file&&yuv_buffer){
                    //
                    //                fwrite(yuv_buffer,1,yuv_size,debug_file);
                    //            }




//            ret = get_frame(&yuv_buffer,&yuv_size);
                    if (yuv_buffer && yuv_size > 0) {
                        if (nullptr == yuv_buffer) {
                            LOGW("yuv_buffer is null");
                            break;
                        }
                        LOGW("get frame-> %d, yuv_size:%d", &yuv_buffer, yuv_size);
//                        auto *yuvFrame = (YUVFrame *) malloc(sizeof(YUVFrame));
//                        if (yuvFrame == nullptr) {
//                            break;
//                        }
//                        yuvFrame->width = videoInfo->width;
//                        yuvFrame->height = videoInfo->height;
//                        yuvFrame->duration = videoInfo->durationus;
//                        yuvFrame->start_time = get_system_current_time_millis();
//                        yuvFrame->format_type = 1; // TODO need to add YUV_FORMAT
//                        yuvFrame->yuv = malloc(yuv_size);
//                        yuvFrame->yuvlen = yuv_size;
//                        yuvFrame->index = ++frame_index;
//                        memcpy(yuvFrame->yuv, yuv_buffer, yuv_size);
//
//                        if (mDecodeOutputBuffer->isFull()) {
//                            YUVFrame topFrame = mDecodeOutputBuffer->pop();
//                            free(topFrame.yuv);
//                        }
//                        mDecodeOutputBuffer->push(*yuvFrame);

                        mDecodeOutputBuffer->Enqueue((const char*)yuv_buffer,videoInfo->width,videoInfo->height,yuv_size,get_system_current_time_millis(),1,++frame_index);
                    }

                    add_queue_finish_time = get_system_current_time_millis();
                    LOGW("Decoder add queue finish at:%lld, push use_time:%lld",
                         add_queue_finish_time, add_queue_finish_time - finish_decode_time);


                    if (yuv_buffer) {
                        free(yuv_buffer);
                        yuv_buffer = NULL;
                    }
                }

                one_frame_time = get_system_current_time_millis() - lock_start_time;

                LOGW("Decoder decode one frame total use_time:%lld",one_frame_time);
                if (ret == DEC_INPUT_END_ERR) {
                    break;
                }

                // Sleep remaining time on a frame handling period.
//            uint64_t this_time = get_system_current_time_millis();
//            uint32_t used_time = this_time - last_decode_time;
                if (one_frame_time > max_handle_time) {
                    sleep_time = 1;
                } else {
                    sleep_time = max_handle_time - one_frame_time;
                }
//            last_decode_time = this_time;
                LOGW("Decoder sleep_ms:%lld,start_time:%lld,end_time:%lld,max_time:%lld",
                     sleep_time, start_decode_time, finish_decode_time, max_handle_time);
                usleep(sleep_time * 1000);

//usleep(20000);

//usleep(1000000);

//break;

                LOGW("get frame end~~~~~~~~~~~~");
            }
            LOGW("Decoder_tid%d::exit loop in process!", thread_id);

            proc_end:
//        if(debug_file){
//            fclose(debug_file);
//        }


//            release_decoder_ctx();

            if(yuv_buffer){
                free(yuv_buffer);
                yuv_buffer = NULL;
            }
        }

        int stat = 0;
        if(mIsThreadStart==false || mExit){
            stat = CODEC_EXIT_CALL;
        }else{
            stat = CODEC_EXIT_AUTO;
        }
        LOGW("Decoder_tid%d::process stat = %d",thread_id,stat);
        if(mStatListener){
            mStatListener(stat);
        }
        LOGW("Decoder_tid%d::process exit 0!",thread_id);

        return false;
    }

    int Decoder::SetDecodeFileInfo(const char *srcVideoPath, float seek_seconds) {
        mDecodeSeekSeconds = seek_seconds;
    //    if(!srcVideoPath){
    //        return -1;
    //    }
    //    if(!mSrcVideoPath){
    //        mSrcVideoPath = (unsigned char*)malloc(sizeof(srcVideoPath));
    //    }
    //    if(!mSrcVideoPath){
    //        return -1;
    //    }
    //    memcpy(mSrcVideoPath,srcVideoPath,strlen(srcVideoPath));

        strcpy(mSrcVideoPath,srcVideoPath);

        LOGW("Decoder::SetDecoderFile = %s",mSrcVideoPath);

        return 0;
    }


    int Decoder::SetDecodeBuffer(RawVideoDataBuffer *decodeOutputBuffer) {
            mDecodeOutputBuffer = decodeOutputBuffer;
        return 0;
    }

    int Decoder::GetFrameMaxHandleTimeMS(int fps) {
        int time_ms = 0;
        if (fps > 0) {
            time_ms = 1000 / fps;
        }
        return time_ms;
    }

//    int Decoder::SetStateListener(encoder_state_call_back listener) {
//        mStatListener = listener;
//        return 0;
//    }

}



