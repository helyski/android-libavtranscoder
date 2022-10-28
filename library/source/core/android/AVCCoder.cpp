//
// Created by Administrator on 2018/4/9 0009.
//
#include <debug.h>
#include <hvjava.h>
#include <OMXCodec.h>
#include <tool.h>
#include "AVCCoder.h"


    CAVCCoder::CAVCCoder(){
        mOpened = false;
        mWidth = 0;
        mHeight = 0;
        mFrameRate = 0;
        mBitrate = 0;
        mVarVideoBR = false;
        mAVCCoder = NULL;
        mSwitchUV = false;
        mVideoCodecType = VIDEO_CODEC_TYPE_MPEG4;
    }

    CAVCCoder::CAVCCoder(int aWidth,int aHeight,int aFrameRate,int aBitrate,bool aVarVideoBR){
        mOpened = false;
        mWidth = aWidth;
        mHeight = aHeight;
        mFrameRate = aFrameRate;
        mBitrate = aBitrate;
        mVarVideoBR = aVarVideoBR;
    }

    void CAVCCoder::SetWidth(int aWidth){
        mWidth = aWidth;
    }

    void CAVCCoder::SetHeight(int aHeight){
        mHeight = aHeight;
    }

    void CAVCCoder::SetFrameRate(int aFrameRate){
        mFrameRate = aFrameRate;
    }

    void CAVCCoder::SetBitrate(int aBitrate,bool aVarVideoBR){
        mBitrate = aBitrate;
        mVarVideoBR = aVarVideoBR;
    }

    bool CAVCCoder::SwitchUV(bool bSwitch){
        mSwitchUV = bSwitch;
        return true;
    }

    void CAVCCoder::SetVideoCodecType(int videoCodecType){
        mVideoCodecType = videoCodecType;
    }

    int CAVCCoder::GetWidth( ){
        return mWidth;
    }

    int CAVCCoder::GetHeight(){
        return mHeight;
    }

    int CAVCCoder::GetFrameRate(){
        return mFrameRate;
    }

    int CAVCCoder::GetBitrate(){
        return mBitrate;
    }

    int CAVCCoder::GetSupportedColourFormat(){
        JNIEnv *env = NULL;
        int nead_detach = 0;
        env = hv_get_jni_env(&nead_detach);
        if (!env){
            return -1;
        }

        int format = -1;
        if(mOpened){
            format = omx_get_color_format(env,mAVCCoder);
        }else{

        }

        if(nead_detach > 0)
            hv_detach_jni_env(env);

        return format;
    }


    int CAVCCoder::GetSupportedColourFormat2(const char *szMime){
        int colour_format = -1;
        JNIEnv *env = NULL;
        int nead_detach = 0;
        env = hv_get_jni_env(&nead_detach);
        if (!env){
            return -1;
        }
        colour_format = omx_get_supported_colourformat(env,szMime);

        if(nead_detach > 0)
            hv_detach_jni_env(env);

        return colour_format;
    }

    bool CAVCCoder::OpenCodec(){
        LOGD(" %s begin",__FUNCTION__);

        if(mOpened){
            //关闭编码器
            CloseCodec();
        }

        mOpened = false;

        char codecTypeStr[64] = "video/avc";
        JNIEnv *env = NULL;
        int nead_detach = 0;
        env = hv_get_jni_env(&nead_detach);
        if (!env){
            return mOpened;
        }
        LOGE("CAVCCoder::OpenCodec 22222222");
        mAVCCoder = init_omx_codec_context();
        if(NULL == mAVCCoder){
            LOGE(" %s 初始化H264硬编码器失败",__FUNCTION__);
            goto end;
        }

        omx_switch_uv(mAVCCoder,mSwitchUV);

        LOGE("CAVCCoder::OpenCodec 333333333");
        if(VIDEO_CODEC_TYPE_H264 == mVideoCodecType)
            strcpy(codecTypeStr,"video/avc");
        if(VIDEO_CODEC_TYPE_H265 == mVideoCodecType)
            strcpy(codecTypeStr,"video/hevc");
        if(VIDEO_CODEC_TYPE_MPEG4 == mVideoCodecType)
            strcpy(codecTypeStr,"video/mp4v-es");
        if (!omx_open_video_codec(env, mAVCCoder,mWidth,mHeight,mFrameRate,mBitrate,mVarVideoBR,codecTypeStr)){
            LOGE("CAVCCoder::OpenCodec %s 打开H264硬编码器失败",__FUNCTION__);
            uninit_omx_codec_context(mAVCCoder);
            goto end;
        }

        mOpened = true;

        LOGD(" %s end",__FUNCTION__);
end:
        if(nead_detach > 0)
            hv_detach_jni_env(env);
        return mOpened;
    }

    bool CAVCCoder::CloseCodec(){
        if(mOpened) {
            JNIEnv *env = NULL;
            int nead_detach = 0;
            env = hv_get_jni_env(&nead_detach);
            if (!env){
                return false;
            }

            omx_close_codec(env,mAVCCoder);
            uninit_omx_codec_context(mAVCCoder);
            mAVCCoder = NULL;
            mOpened = false;

            if(nead_detach > 0)
                hv_detach_jni_env(env);
        }
        return true;
    }

    bool CAVCCoder::GetAVCSequencePrefEncode(unsigned char* Output,int* OutputLen){
        LOGD(" %s begin",__FUNCTION__);

        if(mOpened)
            return false;

        char codecTypeStr[64] = "video/avc";
        JNIEnv *env = NULL;
        int nead_detach = 0;
        env = hv_get_jni_env(&nead_detach);
        if (!env){
            return false;
        }


        if(nead_detach > 0)
            hv_detach_jni_env(env);

        if(VIDEO_CODEC_TYPE_H264 == mVideoCodecType)
            strcpy(codecTypeStr,"video/avc");
        if(VIDEO_CODEC_TYPE_MPEG4 == mVideoCodecType)
            strcpy(codecTypeStr,"video/mp4v-es");
        if( !omx_search_sps_pps(env,mWidth,mHeight,mFrameRate,mBitrate,codecTypeStr,Output,OutputLen) > 0 ){
            return false;
        }

        LOGD(" %s end",__FUNCTION__);

        return true;
    }

    int CAVCCoder::GetSPSPPSLen(){
        return omx_get_sps_pps_len(mAVCCoder);
    }

    const unsigned char* CAVCCoder::GetSPSPPSData(){
        return omx_get_sps_pps_data(mAVCCoder);
    }

    int CAVCCoder::GetCodecFrames(){
        if(!mOpened)
            return 0;

        return omx_get_encode_frame_num(mAVCCoder);
    }

    bool CAVCCoder::Encode(JNIEnv *env,const unsigned char *Data,int Len){
        LOGE("CAVCCoder::Encode 22222222 is open:%d",mOpened);
        if(!mOpened)
            return false;
        LOGE("CAVCCoder::Encode 333333333");
        if( NULL == env)
            return false;
        LOGE("CAVCCoder::Encode 4444444");
        if( NULL == Data || Len <= 0 )
            return false;

        LOGE("CAVCCoder::Encode 555555555");
        if (!omx_encode(env,mAVCCoder,Data,Len,-1)){
            LOGW("CAVCCoder::Encode %s 编码YUV数据失败",__FUNCTION__);
            return false;
        }

        return true;
    }

    bool CAVCCoder::GetFrame(JNIEnv *env,unsigned char** Output,int* OutputLen,int64_t* outTimestamp){
        bool ret = false;
        if(!mOpened)
            return ret;

        if( NULL == env)
            return ret;
        if( NULL == Output || NULL == OutputLen )
            return ret;

        *OutputLen = 0;
        ret = omx_get_encoded_frame(env,mAVCCoder,Output,OutputLen,outTimestamp);
        if(!ret || *OutputLen <= 0)
            return false;

        return true;
    }

    bool CAVCCoder::isOpened(){
        return mOpened;
    }
