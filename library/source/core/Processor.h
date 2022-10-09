//
// Created by HelyskiTank on 2022/10/8.
//

#ifndef ANDROID_LIBTRANSCODE_PROCESSOR_H
#define ANDROID_LIBTRANSCODE_PROCESSOR_H

#include "DataStruct.h"
#include "RingQueue.h"

extern "C"
{
#include "Init.h"
#include "Decoder.h"
#include "ffmpeg-wrappers/hw_decode.h"


namespace LibTranscode {

    class Processor {
    public:
        Processor();

        ~Processor();

        void OpenFFLog();

        void StartDecode();

    private:
        Init *init = NULL;
        Decoder *decoder = NULL;
        RingQueue<YUVFrame> *yuvRingQueue  = new RingQueue<YUVFrame>(4);
    };

}


}
#endif //ANDROID_LIBTRANSCODE_PROCESSOR_H
