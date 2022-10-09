#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>

#define LOG_DEBUG_OUT 1
#define LOG_TAG   "android-libtranscode"


#ifdef __cplusplus
extern "C" {
#endif



#define ANDROID_LOG_DEBUG   3
#define ANDROID_LOG_INFO    4
#define ANDROID_LOG_WARN    5
#define ANDROID_LOG_ERROR   6

void print_logcat(int log_priority,const char *format,...);

#if LOG_DEBUG_OUT

#define LOGD(...) print_logcat(ANDROID_LOG_DEBUG,__VA_ARGS__)
#define LOGI(...) print_logcat(ANDROID_LOG_INFO,__VA_ARGS__)
#define LOGW(...) print_logcat(ANDROID_LOG_WARN,__VA_ARGS__)
#define LOGE(...) print_logcat(ANDROID_LOG_ERROR,__VA_ARGS__)

#else
#define LOGI(...)
#define LOGW(...)
#define LOGD(...)
#define LOGE(...)
#endif


#ifdef __cplusplus
}
#endif

#endif //__DEBUG_H__
