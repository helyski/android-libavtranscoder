//
// Created and modified by Tank on 2022/10/10.
//
#include <jni.h>
#include <string>
#include <string.h>
#include "android/log.h"
#include "Init.h"
#include "Processor.h"
#include "hvjava.h"




#ifdef __cplusplus
extern "C"
{
#endif

#include "transcode_to_java.h"
#include <libavcodec/jni.h>
#include "DataStruct.h"



static JavaVM *g_vm = NULL;

Processor *processor = NULL;


/**
 * Init processor.
 * @param env
 * @param clazz
 * @return JNI_TRUE if init success;
 *          JNI_FALSE if init error;
 */
JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_Init(JNIEnv *env, jclass clazz) {
    int ret = JNI_FALSE;
    processor = new Processor(g_vm);
    processor->Start();
    if (processor) {
        ret = JNI_TRUE;
    }
    init_all_jni_api();
    return ret;
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_UnInit(JNIEnv *env, jclass clazz) {
    // TODO: implement UnInit()
    unInit_all_jni_api();
    int ret = JNI_FALSE;
//    processor = new Processor();
//    if (processor) {
//        ret = JNI_TRUE;
//    }
    return ret;
}


JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_OpenFFmpegLog(JNIEnv *env, jclass clazz) {
    int ret = JNI_FALSE;
    if (processor) {
        processor->OpenFFLog();
        ret = JNI_TRUE;
    }
    return ret;
}


JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_SetEnableMediaCodec(JNIEnv *env, jclass clazz,
                                                                 jboolean enable) {
    // TODO: implement SetEnableMediaCodec()
    int ret = JNI_FALSE;
    if (processor) {
        // TODO next
        ret = JNI_TRUE;
    }
    return ret;
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_SetOutputFileDirection(JNIEnv *env, jclass clazz,
                                                                    jstring dir) {
    // TODO: implement SetOutputFileDirection()
    int ret = JNI_FALSE;
    if (processor) {
        // TODO next
        ret = JNI_TRUE;
    }
    return ret;
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_SetTranscodeOutputType(JNIEnv *env, jclass clazz,
                                                                    jint out_put_type) {
    // TODO: implement SetTranscodeOutputType()
    int ret = JNI_FALSE;
    if (processor) {
        // TODO next

        processor->SetVideoOutputType(out_put_type,video_encode_frame_call_back);
        ret = JNI_TRUE;
    }
    return ret;
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_StartTranscode(JNIEnv *env, jclass clazz,
                                                            jstring src_video_path,
                                                            jstring dest_video_name,
                                                            jfloat seek_seconds, jint dest_width,
                                                            jint dest_height, jint dest_bitrate,
                                                            jint dest_fps, jboolean need_audio) {
    int ret = JNI_FALSE;

    char *mSrcVideoPath = 0;
    char *mOutputFileName = 0;

    jboolean isCopy = false;
    mSrcVideoPath = (char *)env->GetStringUTFChars(src_video_path,&isCopy);
    mOutputFileName = (char *)env->GetStringUTFChars(dest_video_name,&isCopy);

    if(!mSrcVideoPath || strlen(mSrcVideoPath)<=0){
        goto end;
    }



    if (processor) {
        processor->StartTranscode(mSrcVideoPath,mOutputFileName,seek_seconds,dest_width,dest_height,dest_bitrate,dest_fps,need_audio);
        ret = JNI_TRUE;
    }

    end:
    if(mSrcVideoPath){
        env->ReleaseStringUTFChars(src_video_path,mSrcVideoPath);
    }

    if(mOutputFileName){
        env->ReleaseStringUTFChars(dest_video_name,mOutputFileName);
    }

    return ret;
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_StopTranscode(JNIEnv *env, jclass clazz) {
    // TODO: implement StopTranscode()
    int ret = JNI_FALSE;
    if(processor){
        // TODO next
        processor->StopTranscode();
        ret = JNI_TRUE;
    }
    return ret;
}

#ifdef __cplusplus
};
#endif

JNIEXPORT jint JNICALL  JNI_OnLoad(JavaVM* vm, void* reserved) {

    av_jni_set_java_vm(vm, reserved);

    JNIEnv* env = 0;
    int result = -1;

    g_vm = vm;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }


    result = JNI_VERSION_1_6;

    return result;
}