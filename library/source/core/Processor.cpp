//
// Created by Tank on 2022/10/8.
//

#include "Processor.h"
extern "C"
{






    Processor::Processor(void *vm) {
        hv_set_jvm((JavaVM*)vm);
//        init_all_jni_api();

        init = new Init();
        mDecoder = 0;
        mEncoder = 0;
//        mDecodeBuffer  = new RingQueue<YUVFrame>(4);
//        mEncodeBuffer = new RingQueue<H264Frame>(10);

        mDecodeBuffer = new RawVideoDataBuffer();
        mEncodeBuffer = new EncVideoDataBuffer();

        mIsThreadStart = false;
        mExit = false;

        mEncoder = 0;
        mDecoder = 0;
        mDispatcher = 0;

        mDataDest = 0;
    }

    Processor::~Processor() {
//        unInit_all_jni_api();
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
        return 0;
    }

    int Processor::Stop() {
        LOGD("Processor::Stop 1");

        if (mIsThreadStart) {
            mThread.stop();
            mIsThreadStart = false;
            mExit = true;
        }
        LOGD("Processor::Stop 2");
        return 0;
    }

    void Processor::OpenFFLog() {
        if(init) {
            init->init_ffmpeg_log();
        }
    }



    int Processor::StartTranscode(const char *videoPath, const char *outputName, float seek_seconds, int width,
                                int height, int bitrate, int fps, bool need_audio) {
        AutoLock lock(mProcLock);

        LOGW("srcVideoPath:%s,outputName:%s", videoPath, outputName);

        startMediaCodec(videoPath, seek_seconds);

        /**
         * Decoder
         */
        if (mDecoder) {
            mDecoder->StopThread();
            delete mDecoder;
            mDecoder = NULL;
        }
        mDecoder = new Decoder();
        mDecoder->SetStateListener([this](int state) {
            return this->OnDecoderStatChanged(state);
        });
//            mDecoder->SetStateListener(std::bind(&Processor::OnDecoderStatChanged, this, std::placeholders::_1));
        mDecoder->SetDecodeFileInfo(videoPath, seek_seconds);
        mDecoder->SetDecodeBuffer(mDecodeBuffer);
        mDecoder->StartThread();

        /**
         * Encoder
         */
        if (mEncoder) {
            mEncoder->StopThread();
            delete mEncoder;
            mEncoder = NULL;
        }
        mEncoder = new Encoder(mAvcCoder);
        mEncoder->SetInputBuffer(mDecodeBuffer);
        mEncoder->SetOutputBuffer(mEncodeBuffer);
        mEncoder->StartThread();

        /**
         * Dispatcher
         */
        if (mDispatcher) {
            mDispatcher->StopThread();
            delete mDispatcher;
            mDispatcher = NULL;
        }
        if (mDataDest == OUTPUT_H264_STREAM_SHARE_TO_JAVA) {
            mDispatcher = new Dispatcher();
            mDispatcher->SetDateBuffer(mEncodeBuffer);
            mDispatcher->StartThread();
            SetVideoFrameCallBack(mFrameCallBack);
        }
        return 1;
    }

    int Processor::StopTranscode() {
        {
            LOGD("StopTranscode start...");
            AutoLock lock(mProcLock);


            if (mDecoder) {
                mDecoder->StopThread();
                delete mDecoder;
                mDecoder = NULL;
                //Release ffmpeg_mediacodec decoder.
                //FIXME When lib stable ,you can move this to Decoder.
                release_decoder_ctx();
            }


            if (mEncoder) {
                mEncoder->StopThread();
                delete mEncoder;
                mEncoder = NULL;
                //Release mediacodec encoder.
                //FIXME When lib stable ,you can move this to Encoder.
                if (mAvcCoder.isOpened()) {
                    mAvcCoder.CloseCodec();
                }
            }

            if(mDispatcher){
                mDispatcher->StopThread();
                delete mDispatcher;
                mDispatcher = NULL;
            }

            if(mDecodeBuffer){
                mDecodeBuffer->FlushBuffer();
            }
            if(mEncodeBuffer){
                mEncodeBuffer->FlushBuffer();
            }
        }

        LOGD("StopTranscode finish...");
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

        return 0;
    }

    int Processor::OnDestroy() {
        return 0;
    }

    int Processor::SetVideoOutputType(int type,video_frame_call_back cb) {
        LOGD("SetVideoOutputType type = %d",type);
        mDataDest = type;
        mFrameCallBack = cb;
        return 0;
    }

    int Processor::SetVideoFrameCallBack(video_frame_call_back cb) {
        if(mDispatcher){
            LOGD("SetVideoFrameCallBack");
            mDispatcher->SetVideoFrameCallBack(cb);
        }
        return 0;
    }

    void Processor::OnDecoderStatChanged(int stat) {
        LOGD("Processor::OnDecoderStatChanged stat:%d",stat);
        if(CODEC_EXIT_AUTO == stat){
            StopTranscode();
        }
    }

    int Processor::OnEncoderStatChanged(enum CODEC_STAT stat) {
        return 0;
    }


    bool Processor::process(int thread_id, void *env) {
        while(!mExit){
            usleep(1000000);
        }
        return false;
    }






}