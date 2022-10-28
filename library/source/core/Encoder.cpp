//
// Created by Tank on 2022/10/9.
//
#include "Encoder.h"
extern "C"
{

    Encoder::Encoder(){
        mIsThreadStart = false;
        mExit = false;

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

        LOGW("Encoder_tid%d::start loop in process!",thread_id);
        int temp = 0;


        avcCoder.SetWidth(1280);
        avcCoder.SetHeight(720);
        avcCoder.SetBitrate(3000000,false);
        avcCoder.SetFrameRate(15);
        avcCoder.SetVideoCodecType(VIDEO_CODEC_TYPE_H264);
        avcCoder.SwitchUV(true);

        avcCoder.OpenCodec();

        int nead_detach = 0;
        JNIEnv *mEnv = hv_get_jni_env(&nead_detach);
        if (!mEnv){
            return false;
        }

        bool encode_stat = false;
        bool bGetOneNal = false;
        unsigned char *pH264 = NULL;
        int iH264Len = 0;
        int64_t Timestamp;

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
                        if(!yuv){
                            yuv = malloc(len);
                        }
                        memcpy(yuv,frame.yuv,len);

                        encode_stat = avcCoder.Encode(mEnv,(const unsigned char*)yuv,len);

                        LOGW("Encoder :: encode stat:%d", encode_stat);
                        bGetOneNal = avcCoder.GetFrame((JNIEnv *) env, &pH264, &iH264Len, &Timestamp);

                        if(bGetOneNal && pH264 && iH264Len >0){
                            LOGW("Encoder :: GetFrame ret:%d, h264_size:%d", bGetOneNal,len);
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
} ;

}

