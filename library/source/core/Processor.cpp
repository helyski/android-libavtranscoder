//
// Created by HelyskiTank on 2022/10/8.
//

#include "Processor.h"
extern "C"
{
namespace LibTranscode {

    Processor::Processor() {
        init = new Init();
        decoder = new Decoder();

    }

    Processor::~Processor() {

    }

    void Processor::OpenFFLog() {
        if(init) {
            init->init_ffmpeg_log();
        }
    }

    void Processor::StartDecode() {
        if(decoder){
            decoder->StartDecode();
        }
    }
}

}