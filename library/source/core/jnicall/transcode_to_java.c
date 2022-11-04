//
// Created by Tank on 2022/11/4.
//

#include "transcode_to_java.h"

JavaCallBackContext jcb_ctx = {0};
JavaCallBackContext *javactx = &jcb_ctx;

JavaByteBufferStd  byteBuffer = {0};
JavaByteBufferStd *bytebuffer = &byteBuffer;

int init_all_jni_api(){
    init_transcode_java_api();
    init_bytebuffer();
    return 0;
}

int unInit_all_jni_api(){
    unInit_transcode_java_api();
    unInit_bytebuffer();
    return 0;
}


int init_transcode_java_api(){
    LOGD("init_transcode_jni start...");
    int ret=0;
    memset(javactx,0,sizeof(JavaCallBackContext));


    int need_detach = 0;
    JNIEnv *env = hv_get_jni_env(&need_detach);

    if(env) {
        jclass java_clazz = NULL;
        java_clazz = (*env)->FindClass(env, "com/tangjn/libtranscode/JNILibTranscode");
        if(java_clazz){
            javactx->java_api_clazz = (jclass) (*env)->NewGlobalRef(env, java_clazz);
            if(javactx->java_api_clazz){
                javactx->on_h264_call_back_method =
                        (*env)->GetStaticMethodID(env,
                                                  javactx->java_api_clazz,
                                                  "OnH264CallBack",
                                                  "(ILjava/nio/ByteBuffer;IJII)V");

                if(!javactx->on_h264_call_back_method){
                    // TODO
                    LOGE("init_transcode_jni :: find on_h264_call_back_method error");
                    ret = -1;
                }
            }

            (*env)->DeleteLocalRef(env,java_clazz);
        }else{
            LOGE("init_transcode_jni :: find java class error");
            ret = -1;
        }

    }

    if(need_detach){
        hv_detach_jni_env(env);
    }

    LOGD("init_transcode_jni end...");
    return ret;
}

int unInit_transcode_java_api(){
    int need_detach = 0;
    JNIEnv *env = hv_get_jni_env(&need_detach);
    if(env){
        (*env)->DeleteGlobalRef(env,javactx->java_api_clazz);
        if(need_detach){
            hv_detach_jni_env(env);
        }
    }
    memset(javactx, 0, sizeof(JavaCallBackContext));
    return 0;
}


int init_bytebuffer() {
    LOGD("init_bytebuffer start...");
    int ret = 0;
    int need_detach = 0;
    JNIEnv *env = hv_get_jni_env(&need_detach);
    if (!env)
        return -1;
    memset(bytebuffer, 0, sizeof(JavaByteBufferStd));

    jclass local_class;
    local_class = (*env)->FindClass(env, "java/nio/ByteBuffer");
    if (local_class)
        bytebuffer->byte_buffer_class = (*env)->NewGlobalRef(env,
                                                             local_class);
    if (!bytebuffer->byte_buffer_class) {
        LOGE("init_bytebuffer java.lang.ByteBuffer not found");
        ret = -1;
        goto end;
    }

    bytebuffer->allocate_direct_method = (*env)->GetStaticMethodID(env,
                                                                   bytebuffer->byte_buffer_class,
                                                                   "allocateDirect",
                                                                   "(I)Ljava/nio/ByteBuffer;");
    if (!bytebuffer->allocate_direct_method) {
        LOGE("init_bytebuffer java.lang.ByteBuffer.allocateDirect(int) not found.");
        ret = -1;
        goto end;
    }

    bytebuffer->clear_method = (*env)->GetMethodID(env,
                                                   bytebuffer->byte_buffer_class,
                                                   "clear",
                                                   "()Ljava/nio/Buffer;");
    if (!bytebuffer->clear_method) {
        LOGE("init_bytebuffer java.lang.ByteBuffer.clear() not found.");
        ret = -1;
        goto end;
    }

    bytebuffer->limit_method = (*env)->GetMethodID(env,
                                                   bytebuffer->byte_buffer_class,
                                                   "limit",
                                                   "(I)Ljava/nio/Buffer;");
    if (bytebuffer->limit_method == NULL) {
        LOGE("init_bytebuffer java.lang.ByteBuffer.limit() not found.");
        ret = -1;
        goto end;
    }

    bytebuffer->put = (*env)->GetMethodID(env,
                                          bytebuffer->byte_buffer_class,
                                          "put",
                                          "([B)Ljava/nio/ByteBuffer;");
    if (bytebuffer->put == NULL) {
        LOGE("init_bytebuffer java.lang.ByteBuffer.put() not found.");
        ret = -1;
        goto end;
    }
    LOGE("init_bytebuffer finish success!!!");
    end:
    if (need_detach) {
        hv_detach_jni_env(env);
    }
    return ret;
}

void bytebuffer_alloc(JNIEnv *env,int size) {
    LOGD("bytebuffer_alloc size:%d",size);

    if (!bytebuffer->byte_buffer_class || !bytebuffer->allocate_direct_method) {
        LOGD("bytebuffer_alloc error, can not found clazz!");
        goto end;
    }
    bytebuffer->buffer = (*env)->NewGlobalRef(env,(*env)->CallStaticObjectMethod(env,
                                                                                 bytebuffer->byte_buffer_class,
                                                                                 bytebuffer->allocate_direct_method,
                                                                                 size));
    end:
    if(bytebuffer->buffer)
        LOGD("bytebuffer_alloc success, alloc size:%d ok",size);
}

int unInit_bytebuffer(){
    int need_detach = 0;
    JNIEnv *env = hv_get_jni_env(&need_detach);
    if (!env)
        return -1;
    (*env)->DeleteGlobalRef(env, bytebuffer->buffer);
    (*env)->DeleteGlobalRef(env, bytebuffer->byte_buffer_class);
    if (need_detach) {
        hv_detach_jni_env(env);
    }
    memset(bytebuffer, 0, sizeof(JavaByteBufferStd));

    return 0;
}


void video_encode_frame_call_back(int index,void *data,int len,unsigned long long pts, int is_key_frame, int nal_type){
    // todo
    LOGE("video_encode_frame_call_back len:%d pts:%lld nal_type:%d",len,pts,nal_type);
    int nead_detach = 0;
    JNIEnv *env = hv_get_jni_env(&nead_detach);
    if(!env){
        LOGE("video_encode_frame_call_back get JNIEnv failed");
        return;
    }
    if(!bytebuffer->buffer){
        bytebuffer->allocLen = len + 10;
        bytebuffer_alloc(env,bytebuffer->allocLen);
    }else{
        if(len > bytebuffer->allocLen){
            bytebuffer->allocLen = len + 10;
            (*env)->DeleteGlobalRef(env,bytebuffer->buffer);
            bytebuffer_alloc(env,bytebuffer->allocLen);
        }
    }

    if(!bytebuffer->byte_buffer_class || !bytebuffer->allocate_direct_method || !bytebuffer->buffer)
        return;

    int size = len;
    int capacity = 0;
    uint8_t *ptr = 0;
    capacity = (*env)->GetDirectBufferCapacity(env,bytebuffer->buffer);
    ptr = (*env)->GetDirectBufferAddress(env, bytebuffer->buffer);

    if(ptr){
        if(size > capacity)
            size = capacity;
        memcpy(ptr,data,size);
        if(javactx->on_h264_call_back_method){
            (*env)->CallStaticVoidMethod(env, javactx->java_api_clazz,
                                         javactx->on_h264_call_back_method,
                                         index,bytebuffer->buffer,size,pts,is_key_frame,nal_type);
            LOGE("video_encode_frame_call_back OK!!!");
        }
    }
}

