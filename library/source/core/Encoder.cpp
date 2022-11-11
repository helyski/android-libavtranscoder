//
// Created by Tank on 2022/10/9.
//
#include "Encoder.h"
#include "libyuv.h"

using namespace libyuv;
extern "C"
{

    Encoder::Encoder(CAVCCoder coder,Decoder *decoder,int width,int height,int bitrate){
        mIsThreadStart = false;
        mExit = false;

        avcCoder = coder;
        mDecoder = decoder;


        /*
         * Default settings.
         */
        mSrcWidth = 0;
        mSrcHeight = 0;

        mDestWidth = width;
        mDestHeight = height;
        mDestBitrate = bitrate;

        mFPS = 20;

    }

    Encoder::~Encoder() {
        LOGW("~Encoder");
    }

    int Encoder::StartEncode() {

        return 1;
    }

    bool Encoder::StartThread() {
//        AutoLock lock(mEncoderLock);
        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
        }

        AutoLock lock(mEncoderLock);
//
//        avcCoder.SetWidth(mDestWidth);
//        avcCoder.SetHeight(mDestHeight);
//        avcCoder.SetBitrate(mDestBitrate,false);
//        avcCoder.SetFrameRate(mFPS);
//        avcCoder.SetVideoCodecType(VIDEO_CODEC_TYPE_H264);
//        avcCoder.SwitchUV(true);
//
//        avcCoder.OpenCodec();

        mThread.registerThreadProc(*this);
        mThread.start(mThreadName);
        mIsThreadStart = true;
        LOGW("Encoder::StartThread");
        return true;
    }

    void Encoder::StopThread() {
        mExit = true;
        LOGW("Encoder::StopThread start ...");
        {
            AutoLock lock(mEncoderLock);
            if (mIsThreadStart) {
                mThread.stop();
                mIsThreadStart = false;
            }
//            if (avcCoder.isOpened()) {
//                avcCoder.CloseCodec();
//            }
        }
        LOGW("Encoder::StopThread end ...");
    }

    bool Encoder::process(int thread_id, void *env) {
        const char* path = "/sdcard/videos/encoder/123.yuv";
        FILE * file = 0;
        int yuv_index = 0;
        unsigned long long last_frame_time = 0;

        file = open_file_c(path);

        unsigned long long wait_lock_start_time;
        unsigned long long wait_lock_finish_time;

        unsigned long long write_start_time;
        unsigned long long write_finish_time;

        unsigned long long cpy_finish_time;

//        void* yuv=0;
//        int len=0;

        unsigned char *pI420 = nullptr;
        unsigned char *pScaleI420 = nullptr;

        LOGW("Encoder_tid%d::start loop in process!",thread_id);
        int temp = 0;

        int encode_frames = 0;

        int getDecodeInfo = 0;

        LOGW("Encoder_tid%d::get codec info start!",thread_id);
        while (!getDecodeInfo && !mExit){
            if(mDecoder->GetVideoCodecInfo()){
                LOGW("Encoder_tid%d::get from decoder");
                mSrcWidth = mDecoder->GetVideoCodecInfo()->width;
                mSrcHeight = mDecoder->GetVideoCodecInfo()->height;
                getDecodeInfo = 1;
            }else{
                usleep(1000);
            }
        }
        LOGW("Encoder_tid%d::got codec info finished! srcWidth:%d,srcHeight;%d",thread_id,mSrcWidth,mSrcHeight);

        if(mSrcWidth<=0 || mSrcHeight<=0){
            mSrcWidth = DEFAULT_WIDTH;
            mSrcHeight = DEFAULT_HEIGHT;
        }

        LOGW("Encoder_tid%d::encode target width:%d, height:%d, bitrate:%d",thread_id,mDestWidth,mDestHeight,mDestBitrate);
        if(mDestWidth<=0 || mDestHeight<=0){
            mDestWidth = DEFAULT_TARGET_WIDTH;
            mDestHeight = DEFAULT_TARGET_HEIGHT;
            mDestBitrate = DEFAULT_TARGET_BITRATE;
        }

//        int nead_detach = 0;
//        JNIEnv *mEnv = hv_get_jni_env(&nead_detach);
//        if (!mEnv){
//            return false;
//        }

if(!env){
    LOGE("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC");
    return false;
}


        while(!mExit){
            LOGW("Encoder_tid%d::%d!",thread_id,temp++);
            // TODO encode here
//            break;

            {
                wait_lock_start_time = get_system_current_time_millis();
                LOGW("Encoder wait_lock_start_time:%lld",wait_lock_start_time);
                AutoLock lock(mYuvBuffer->GetLock());
                wait_lock_finish_time = get_system_current_time_millis();
                LOGW("Encoder wait_lock_finish_time:%lld, wait_lock_time:%lld",wait_lock_finish_time,wait_lock_finish_time - wait_lock_start_time);

                if (mYuvBuffer) {
                    YUVFrame *frame = mYuvBuffer->GetHeadFrame();
                    if(!frame){
                        usleep(5000);
                        continue;
                    }
                    LOGW("Encoder_tid%d::get YUV index:%d", thread_id, frame->index);
                    if (frame->index > yuv_index) {
                        yuv_index = frame->index;
                        last_frame_time = get_system_current_time_millis();
//                        len = frame->yuvlen;
//                        if(!yuv){
//                            yuv = malloc(len);
//                        }
//                        memcpy(yuv,frame.yuv,len);
//                        yuv = frame->yuv;

                        if(frame->yuv && frame->yuvlen >0) {

                            if(!pI420) {
                                pI420 = (unsigned char *) malloc(frame->yuvlen);
                            }
                            if(pI420) {
                                NV21ToYUV420P((const unsigned char*)frame->yuv,pI420,mSrcWidth,mSrcHeight);
                            }

                            if(mDestWidth!=mSrcWidth || mDestHeight!=mSrcHeight){
                                if(!pScaleI420){
                                    pScaleI420 = (unsigned char*) malloc(mDestWidth * mDestHeight *3 /2);
                                }
                                if(pScaleI420){
                                    ScaleI420(pI420,mSrcWidth,mSrcHeight,pScaleI420,mDestWidth,mDestHeight);
                                }
                            }


                            bool encode_stat = false;
                            encode_stat = avcCoder.Encode((JNIEnv*)env, (const unsigned char *) pScaleI420, mDestWidth * mDestHeight *3 /2);

                            LOGW("Encoder :: encode stat:%d", encode_stat);


                            bool bGetOneNal = false;
                            unsigned char *pH264 = NULL;
                            int len = 0;
                            int64_t pts;

                            int get_times = 0;

                            while (1) {
                                bGetOneNal = avcCoder.GetFrame((JNIEnv*)env, &pH264, &len,
                                                               &pts);
//                                LOGW("Encoder :: GetFrame len:%d,timestamp:%llu", iH264Len, Timestamp);
                                if (bGetOneNal && pH264 && len > 0) {
//                                    LOGW("Encoder :: GetFrame ret:%d, frame_size:%d,timestamps:%d,encode_frames:%d", bGetOneNal, len,Timestamp,encode_frames);
                                    {
                                        AutoLock lock(mH264Buffer->GetLock());

                                        mH264Buffer->Enqueue((const char *) pH264,
                                                             get_nal_type(pH264, len), len,
                                                             mDestWidth, mDestHeight, pts,
                                                             get_system_current_time_millis(),
                                                             ++encode_frames);
                                        LOGW("Encoder :: GetFrame ret:%d, frame_size:%d,timestamps:%lld,encode_frames:%d",
                                             bGetOneNal, len, pts, encode_frames);
                                    }

//                                    if (file && len > 0 && pH264) {
                                        // todo convert yuv format here for next test.Current encoded frame color space is not correct.
//                                        write_start_time = get_system_current_time_millis();
//                                        fwrite(pH264, 1, iH264Len, file);
//                                        write_finish_time = get_system_current_time_millis();
//                                        LOGW("Encoder :: GetFrame write index:%d,write_use_time:%lld",encode_frames,
//                                             write_finish_time - write_start_time);
//                                    }
                                    usleep(1000); // just wait a moment.
                                    free(pH264);
                                    pH264 = NULL;
                                    get_times=0;
                                    continue;
                                }

                                if(++get_times>2){
                                    break;
                                }
                                usleep(2000);
                            }
                        }
                    }
                }
                cpy_finish_time = get_system_current_time_millis();
                LOGW("Encoder getHeadElement and CPY finish at:%lld, use_time:%lld",cpy_finish_time,cpy_finish_time - wait_lock_finish_time);

            }
            // FIXME DEBUG ONLY
            // Write a 1280x720 size yuv to file may spend a hundred milliseconds.
            // So 2 things should be noticed:
            //      1. Write YUV to file is a time-consuming operation.
            //      2. RingQueue lock use in this Encoder may cause Decoder has a low efficiency.
//            if (file && len>0 && yuv) {
//                write_start_time = get_system_current_time_millis();
//                fwrite(yuv, 1, len, file);
//                write_finish_time = get_system_current_time_millis();
//                LOGW("Encoder write_use_time:%lld",write_finish_time-write_start_time);
//            }
            usleep(10000);

            if(get_system_current_time_millis() - last_frame_time > 3000 && last_frame_time>0){
                break;
            }
        }
//
//        if(file){
//            fclose(file);
//        }

        if(pI420){
            free(pI420);
            pI420 = NULL;
        }
        if(pScaleI420){
            free(pScaleI420);
            pScaleI420 = NULL;
        }

//        avcCoder.CloseCodec();

//usleep(1000000);

        LOGW("Encoder_tid%d::exit loop in process!",thread_id);

//        if(nead_detach>0){
//            hv_detach_jni_env(mEnv);
//        }
        return false;
    }


int Encoder::SetInputBuffer(RawVideoDataBuffer *yuvBuffer) {
    mYuvBuffer = yuvBuffer;
    return 0;
}

int Encoder::SetOutputBuffer(EncVideoDataBuffer *h264Buffer) {
    mH264Buffer = h264Buffer;
    return 0;
}

}

