//
// Created by Tank on 2022/10/8.
//

#include "Processor.h"
extern "C"
{
namespace LibTranscode {

    Processor::Processor(void *vm) {
        hv_set_jvm((JavaVM*)vm);
        init = new Init();
        mDecoder = 0;
        mEncoder = 0;
//        mDecodeBuffer  = new RingQueue<YUVFrame>(4);
//        mEncodeBuffer = new RingQueue<H264Frame>(10);

        mDecodeBuffer = new RawVideoDataBuffer();

        mIsThreadStart = false;
        mExit = false;
    }

    Processor::~Processor() {

    }

    int Processor::Start() {
        LOGD("Processor::Start 1");
        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
        }

        mThread.registerThreadProc(*this);
        mThread.start("MProcessor");
        mIsThreadStart = true;

        LOGD("Processor::Start 2");
        return true;
    }

    int Processor::Stop() {
        LOGD("Processor::Stop 1");

        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
            mExit = true;
        }
        LOGD("Processor::Stop 2");
    }

    void Processor::OpenFFLog() {
        if(init) {
            init->init_ffmpeg_log();
        }
    }



    int Processor::StartTranscode(const char *videoPath, const char *outputName, float seek_seconds, int width,
                                int height, int bitrate, int fps, bool need_audio) {
        AutoLock lock(mProcLock);

//        BasicTranscodingParams *transcoding_param = (BasicTranscodingParams*) malloc(sizeof (BasicTranscodingParams));
//
//        if(!transcoding_param){
//            return 0;
//        }
//
//        memset(transcoding_param,0,sizeof(BasicTranscodingParams));
//
//        transcoding_param->width = width;
//        transcoding_param->height = height;
//        transcoding_param->bitrate = bitrate;
//        transcoding_param->fps = fps;
//        transcoding_param->seek_seconds = seek_seconds;
//
//        transcoding_param->destVideoName = (char *)malloc(sizeof(char *));
//        if(transcoding_param->destVideoName){
//
//        }

        LOGW("srcVideoPath:%s,outputName:%s",videoPath,outputName);
        {
            startMediaCodec(videoPath,seek_seconds);

            if (mDecoder) {
                mDecoder->StopThread();
                delete mDecoder;
                mDecoder = NULL;
            }
            mDecoder = new Decoder();
            mDecoder->SetDecodeFileInfo(videoPath, seek_seconds);
            mDecoder->SetDecodeBuffer(mDecodeBuffer);
            mDecoder->StartThread();


            if (mEncoder) {
                mEncoder->StopThread();
                delete mEncoder;
                mEncoder = NULL;
            }
            mEncoder = new Encoder(mAvcCoder);
            mEncoder->SetInputBuffer(mDecodeBuffer);
//            mEncoder->SetOutputBuffer(NULL);
            mEncoder->StartThread();
        }

        return 1;


    }

    int Processor::StopTranscode() {
        {
            AutoLock lock(mProcLock);


            if (mDecoder) {
                mDecoder->StopThread();
                delete mDecoder;
                mDecoder = NULL;
            }


            if (mEncoder) {
                mEncoder->StopThread();
                delete mEncoder;
                mEncoder = NULL;
            }

            if(mDecodeBuffer){
                mDecodeBuffer->FlushBuffer();
            }

            stopMediaCodec();
        }

        return 0;
    }

    int Processor::startMediaCodec(const char* path,float seek) {
        // start ffmpeg mediacodec
        init_decoder_ctx(path,seek);
        // start jni mediacodec
        mAvcCoder.SetWidth(640);
        mAvcCoder.SetHeight(480);
        mAvcCoder.SetBitrate(2000000,false);
        mAvcCoder.SetFrameRate(20);
        mAvcCoder.SetVideoCodecType(VIDEO_CODEC_TYPE_H264);
        mAvcCoder.SwitchUV(true);
        mAvcCoder.OpenCodec();

        return 0;
    }

    int Processor::stopMediaCodec() {
        // release ffmpeg mediacodec.
        release_decoder_ctx();
        // release jni mediacodec.
        if (mAvcCoder.isOpened()) {
            mAvcCoder.CloseCodec();
        }
        return 0;
    }

    int Processor::OnDestroy() {
        return 0;
    }


    bool Processor::process(int thread_id, void *env) {
        while(!mExit){
            usleep(1000000);
        }
    }


}

}