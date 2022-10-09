//
// Created by Administrator on 2021/7/15 0015.
//
#include <jni.h>
#include <string>
#include <string.h>
#include "android/log.h"
#include "Init.h"
#include "Processor.h"


extern "C"{



    using namespace LibTranscode;

#include <libavcodec/jni.h>

static JavaVM *g_vm = NULL;

Processor *processor = NULL;

JNIEXPORT jboolean JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_Init(JNIEnv *env, jclass clazz) {
    processor = new Processor();
    return JNI_TRUE;
}


JNIEXPORT void JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_OpenFFmpegLog(
        JNIEnv *env,
        jclass clazz){
    processor->OpenFFLog();
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_StartHWDecode(JNIEnv *env, jclass clazz) {

    processor->StartDecode();
    return 1;
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_EnableMediaCodec(JNIEnv *env, jclass clazz,
                                                              jboolean enable) {
    // TODO: implement EnableMediaCodec()
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_SetOutputFileDirection(JNIEnv *env, jclass clazz,
                                                                    jstring dir) {
    // TODO: implement SetOutputFileDirection()
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_SetTranscodeOutputType(JNIEnv *env, jclass clazz,
                                                                    jint out_put_type) {
    // TODO: implement SetTranscodeOutputType()
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_StartTranscode(JNIEnv *env, jclass clazz,
                                                            jstring video_path) {
    // TODO: implement StartTranscode()
}

JNIEXPORT jint JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_StopTranscode(JNIEnv *env, jclass clazz) {
    // TODO: implement StopTranscode()
}



}

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


