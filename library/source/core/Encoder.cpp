//
// Created by Tank on 2022/10/9.
//
#include "Encoder.h"
#include "libyuv.h"

using namespace libyuv;
extern "C"
{

    Encoder::Encoder(){
        mIsThreadStart = false;
        mExit = false;


        /*
         * Default settings.
         */
        mSrcWidth = 1280;
        mSrcHeight = 720;

        mDestWidth = 640;
        mDestHeight = 480;

        mDestBitrate = 2000000;
        mFPS = 20;

    }

    Encoder::~Encoder() {
        LOGW("~Encoder");
    }

    int Encoder::StartEncode() {

        return 1;
    }

    bool Encoder::StartThread() {
        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
        }
        mThread.registerThreadProc(*this);
        mThread.start(mThreadName);
        mIsThreadStart = true;
        LOGW("Encoder::StartThread");
        return true;
    }

    void Encoder::StopThread() {
        mExit = true;
        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
        }
        LOGW("Encoder::StopThread");
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

        void* yuv=0;
        int len=0;

        unsigned char *pI420 = nullptr;
        unsigned char *pScaleI420 = nullptr;

        LOGW("Encoder_tid%d::start loop in process!",thread_id);
        int temp = 0;

        int encode_frames = 0;


        avcCoder.SetWidth(mDestWidth);
        avcCoder.SetHeight(mDestHeight);
        avcCoder.SetBitrate(mDestBitrate,false);
        avcCoder.SetFrameRate(mFPS);
        avcCoder.SetVideoCodecType(VIDEO_CODEC_TYPE_H264);
        avcCoder.SwitchUV(true);

        avcCoder.OpenCodec();

        int nead_detach = 0;
        JNIEnv *mEnv = hv_get_jni_env(&nead_detach);
        if (!mEnv){
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
                    YUVFrame frame = mYuvBuffer->getHeadElement();
                    LOGW("Encoder_tid%d::get YUV index:%d", thread_id, frame.index);
                    if (frame.index > yuv_index) {
                        yuv_index = frame.index;
                        last_frame_time = get_system_current_time_millis();
                        len = frame.yuvlen;
//                        if(!yuv){
//                            yuv = malloc(len);
//                        }
//                        memcpy(yuv,frame.yuv,len);
                        yuv = frame.yuv;

                        if(yuv && len >0) {

                            if(!pI420) {
                                pI420 = (unsigned char *) malloc(len);
                            }
                            if(pI420) {
                                NV21ToYUV420P((const unsigned char*)yuv,pI420,mSrcWidth,mSrcHeight);
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
                            encode_stat = avcCoder.Encode(mEnv, (const unsigned char *) pScaleI420, mDestWidth * mDestHeight *3 /2);

                            LOGW("Encoder :: encode stat:%d", encode_stat);


                            bool bGetOneNal = false;
                            unsigned char *pH264 = NULL;
                            int iH264Len = 0;
                            int64_t Timestamp;

                            int get_times = 0;

                            while (1) {
                                bGetOneNal = avcCoder.GetFrame(mEnv, &pH264, &iH264Len,
                                                               &Timestamp);
//                                LOGW("Encoder :: GetFrame len:%d,timestamp:%llu", iH264Len, Timestamp);
                                if (bGetOneNal && pH264 && iH264Len > 0) {
//                                    LOGW("Encoder :: GetFrame ret:%d, frame_size:%d,timestamps:%d,encode_frames:%d", bGetOneNal, len,Timestamp,encode_frames);
                                    LOGW("Encoder :: GetFrame ret:%d, frame_size:%d,timestamps:%lld,encode_frames:%d", bGetOneNal, iH264Len,Timestamp,++encode_frames);

                                    if (file && iH264Len > 0 && pH264) {
                                        // todo convert yuv format here for next test.Current encoded frame color space is not correct.


                                        write_start_time = get_system_current_time_millis();
                                        fwrite(pH264, 1, iH264Len, file);
                                        write_finish_time = get_system_current_time_millis();
                                        LOGW("Encoder :: GetFrame write index:%d,write_use_time:%lld",encode_frames,
                                             write_finish_time - write_start_time);
                                    }
                                    usleep(1000); // just wait a moment.
                                    continue;
                                }

                                if(++get_times>2){
                                    break;
                                }
                                usleep(2000);
                            }
                        }
                    }else{
                        len = 0;
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

        if(file){
            fclose(file);
        }
        if(yuv){
            free(yuv);
            yuv = NULL;
        }
        if(pI420){
            free(pI420);
            pI420 = NULL;
        }
        if(pScaleI420){
            free(pScaleI420);
            pScaleI420 = NULL;
        }

        LOGW("Encoder_tid%d::exit loop in process!",thread_id);
        return false;
    }


int Encoder::SetInputBuffer(RingQueue<YUVFrame> *yuvBuffer) {
    mYuvBuffer = yuvBuffer;
    return 0;
}

int Encoder::SetOutputBuffer(RingQueue<H264Frame> *h264Buffer) {
    mH264Buffer = h264Buffer;
    return 0;
}

}

