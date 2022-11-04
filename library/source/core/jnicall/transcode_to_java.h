//
// Created by Tank on 2022/11/4.
//

#ifndef ANDROID_LIBTRANSCODE_TRANSCODE_TO_JAVA_H
#define ANDROID_LIBTRANSCODE_TRANSCODE_TO_JAVA_H

#include "jni.h"
#include "string.h"
#include "hvjava.h"
#include "debug.h"

typedef struct _JavaCallBackContext{
    jclass java_api_clazz;
    jmethodID on_h264_call_back_method;
} JavaCallBackContext;

typedef struct _JavaByteBufferStd{
    jclass byte_buffer_class;
    jmethodID allocate_direct_method;
    jmethodID clear_method;
    jmethodID limit_method;
    jmethodID put;
    jobject buffer;
    int allocLen;
    int width;
    int height;
}JavaByteBufferStd;

int init_all_jni_api();
int init_transcode_java_api();
int init_bytebuffer();

void bytebuffer_alloc(JNIEnv *env,int size);

int unInit_all_jni_api();
int unInit_transcode_java_api();
int unInit_bytebuffer();

void video_encode_frame_call_back(int index,void *data,int len,unsigned long long pts, int is_key_frame, int nal_type);






#endif //ANDROID_LIBTRANSCODE_TRANSCODE_TO_JAVA_H
