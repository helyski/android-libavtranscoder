//
// Created by Tank on 2022/10/8.
//

#ifndef ANDROID_LIBTRANSCODE_PROCESSOR_H
#define ANDROID_LIBTRANSCODE_PROCESSOR_H

#include "DataStruct.h"
#include "RingQueue.h"
#include "RawVideoDataBuffer.h"
#include "AVCCoder.h"
#include "Dispatcher.h"
#include "common.h"
#include "functional"

extern "C"
{
#include "Init.h"
#include "Decoder.h"
#include "Encoder.h"
#include "ffmpeg-wrappers/hw_decode.h"
#include "transcode_to_java.h"

//typedef void (*video_frame_call_back)(int index,void *data,int len,unsigned long long pts,int is_key_frame,int nal_type);



    class Processor:public SingleThread::ThreadProc {
    public:
        Processor(void * vm);

        ~Processor();

        int Start();
        int Stop();

        void OpenFFLog();

        int StartTranscode(const char* videoPath,const char* outputName,float seek_seconds,int width,int height,int bitrate,int fps,bool need_audio);

        int StopTranscode();

        int startMediaCodec(const char* videoPath,float seek);

        int stopMediaCodec();

        int SetVideoOutputType(int type,video_frame_call_back cb);

        void OnDecoderStatChanged(int stat);

        int OnEncoderStatChanged(enum CODEC_STAT stat);

    private:
        bool process(int thread_id, void *env);
        int OnDestroy();
        int SetVideoFrameCallBack(video_frame_call_back cb);

    private:
        //工作线程
        bool mIsThreadStart;
        SingleThread mThread;
        bool mExit;


        Init *init = NULL;
//        RingQueue<YUVFrame> *mDecodeBuffer = NULL;
//        RingQueue<H264Frame> *mEncodeBuffer = NULL;
        Decoder *mDecoder;
        Encoder *mEncoder;
        Dispatcher *mDispatcher;

        CAVCCoder mAvcCoder;

        RawVideoDataBuffer *mDecodeBuffer;
        EncVideoDataBuffer *mEncodeBuffer;

        video_frame_call_back mFrameCallBack;

        int mDataDest;

        Lock mProcLock;
    };




}
#endif //ANDROID_LIBTRANSCODE_PROCESSOR_H
