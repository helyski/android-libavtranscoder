//
// Created by Tank on 2022/10/8.
//

#include "Processor.h"
extern "C"
{
namespace LibTranscode {

    Processor::Processor() {
        init = new Init();
        mDecoder = 0;
        mEncoder = 0;
        mDecodeRingBuffer  = new RingQueue<YUVFrame>(4);
        mEncodeRingBuffer = new RingQueue<H264Frame>(10);

    }

    Processor::~Processor() {

    }

    void Processor::OpenFFLog() {
        if(init) {
            init->init_ffmpeg_log();
        }
    }



    int Processor::StartTranscode(const char *videoPath, const char *outputName, float seek_seconds, int width,
                                int height, int bitrate, int fps, bool need_audio) {

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


        if(mDecoder){
            mDecoder->StopThread();
            delete mDecoder;
            mDecoder = NULL;
        }
        mDecoder = new Decoder();
        mDecoder->StartThread();


        if(mEncoder){
            mEncoder->StopThread();
            delete mEncoder;
            mEncoder = NULL;
        }
        mEncoder = new Encoder();
        mEncoder->StartThread();

        return 1;


    }

    int Processor::StopTranscode() {
        if(mDecoder){
            mDecoder->StopThread();
            delete mDecoder;
            mDecoder = NULL;
        }
        return 0;
    }

    int Processor::OnDestroy() {
        return 0;
    }


}

}