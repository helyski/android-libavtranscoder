//
// Created by Administrator on 2021/7/15 0015.
//
#include <jni.h>
#include <string>
#include <string.h>
#include "android/log.h"
#include "Init.h"


extern "C"{

#include "Processor.h"

    using namespace LibTranscode;

#include <libavcodec/jni.h>

static JavaVM *g_vm = NULL;

Processor *processor = NULL;

JNIEXPORT jboolean JNICALL
Java_com_tangjn_libtranscode_JNILibTranscode_StartEngine(JNIEnv *env, jclass clazz) {
    // TODO: implement StartEngine()
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

