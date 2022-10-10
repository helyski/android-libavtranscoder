//
// Created by Tank on 2022/10/8.
//

#ifndef ANDROID_LIBTRANSCODE_PROCESSOR_H
#define ANDROID_LIBTRANSCODE_PROCESSOR_H

#include "DataStruct.h"
#include "RingQueue.h"

extern "C"
{
#include "Init.h"
#include "Decoder.h"
#include "Encoder.h"
#include "ffmpeg-wrappers/hw_decode.h"


namespace LibTranscode {

    class Processor {
    public:
        Processor();

        ~Processor();

        void OpenFFLog();

        int StartTranscode(const char* videoPath,const char* outputName,float seek_seconds,int width,int height,int bitrate,int fps,bool need_audio);

        int StopTranscode();

    private:

        int OnDestroy();

    private:
        Init *init = NULL;
        RingQueue<YUVFrame> *mDecodeRingBuffer = NULL;
        RingQueue<H264Frame> *mEncodeRingBuffer = NULL;
        Decoder *mDecoder;
        Encoder *mEncoder;
    };

}


}
#endif //ANDROID_LIBTRANSCODE_PROCESSOR_H
