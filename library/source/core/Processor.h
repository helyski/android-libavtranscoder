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

extern "C"
{
#include "Init.h"
#include "Decoder.h"
#include "Encoder.h"
#include "ffmpeg-wrappers/hw_decode.h"


namespace LibTranscode {

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

    private:
        bool process(int thread_id, void *env);
        int OnDestroy();

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

        int mDataDest;

        Lock mProcLock;
    };

}


}
#endif //ANDROID_LIBTRANSCODE_PROCESSOR_H
