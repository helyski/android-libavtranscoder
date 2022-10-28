//
// Created by Administrator on 2018/4/9 0009.
//

#ifndef MGMEDIAENGINE_AVCCODER_H
#define MGMEDIAENGINE_AVCCODER_H

#include <OMXCodec.h>

enum{
    VIDEO_CODEC_TYPE_MPEG4 = 0,     //mpeg-4
    VIDEO_CODEC_TYPE_H264 = 1,       //h264
    VIDEO_CODEC_TYPE_H265 = 2       //h265
};

namespace mgvideo {
    class CAVCCoder{
    public:
        CAVCCoder();
        CAVCCoder(int aWidth,int aHeight,int aFrameRate,int aBitrate,bool aVarVideoBR);

        /* 设置 */
        void SetWidth(int aWidth );
        void SetHeight(int aHeight);
        void SetFrameRate(int aFrameRate);
        void SetBitrate(int aBitrate,bool aVarVideoBR);
        bool SwitchUV(bool bSwitch);
        void SetVideoCodecType(int videoCodecType);

        /* 获取 */
        int GetWidth( );
        int GetHeight();
        int GetFrameRate();
        int GetBitrate();


        /* 获得系统支持的颜色格式*/
        int GetSupportedColourFormat();
        static int GetSupportedColourFormat2(const char *szMime);


        /* 打开编码器 */
        bool OpenCodec();
        /* 关闭编码器 */
        bool CloseCodec();
        /* 预先获得sps/pps */
        bool GetAVCSequencePrefEncode(unsigned char* Output,int* OutputLen);
        /* 获得sps pps长度*/
        int GetSPSPPSLen();
        /* 获得sps pps数据指针*/
        const unsigned char* GetSPSPPSData();

        /* 获得编码总的帧数 */
        int GetCodecFrames();
        /* 异步方式编码YUV数据 */
        bool Encode(JNIEnv *env,const unsigned char *Data,int Len);
        /* 获得编码的H264数据 */
        bool GetFrame(JNIEnv *env,unsigned char** Output,int* OutputLen,int64_t* outTimestamp);
        /* 是否打开了编码器 */
        bool isOpened();

    private:
        OmxCodecContext *mAVCCoder;
        bool mOpened;
        int mWidth;
        int mHeight;
        int mFrameRate;
        int mBitrate;
        bool mVarVideoBR;
        bool mSwitchUV;
        int mVideoCodecType;    //视频编码类型
    };
}


#endif //MGMEDIAENGINE_AVCCODER_H
