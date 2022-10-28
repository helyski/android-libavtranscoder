/*******************************************************************************
 * Software Name :safe
 *
 * Copyright (C) 2014 HanVideo.
 *
 * author : 梁智游
 ******************************************************************************/
#ifndef _TOOL_H
#define _TOOL_H


#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if 1
#define DEBUG_SLEEP(x)sleep(x);
#else
#define DEBUG_SLEEP(x)
#endif

#define LOG_TIME_BEGIN(c)  \
    unsigned long long logStartTime##c = GetCurSysTime();
#define LOG_TIME_END(c,log) \
    LOGD("%s time:%d",log,GetCurSysTime()-logStartTime##c);


enum{
    NAL_TYPE_P = 1,
    NAL_TYPE_I = 5,
    NAL_TYPE_SEI = 6,
    NAL_TYPE_SPS = 7,
    NAL_TYPE_PPS = 8
};

int find_nal_units(const unsigned char* buf,int buf_size,int* nal_start, int* nal_end);

/* 十六进制转十进制
 *
 */
int hex2int(int hexValue);

/* 获取绝对时间(单位毫秒)
 *
 */
unsigned long long GetCurSysTime();
void mysleep(unsigned int milliseconds);
int get_nal_type( void *p, int len );
int get_hevc_nal_type( void *p, int len );

/*
 *  返回:
 *  0 :关键帧 -1 非关键帧
 *
 */
int is_h264_keyframe(void *data,int len);
int is_h265_keyframe(void *data,int len);



inline unsigned char  get1(const unsigned char *data,size_t i);
inline unsigned int get2(const unsigned char *data,size_t i);
inline unsigned int get3(const unsigned char *data,size_t i);
inline unsigned int get4(const unsigned char *data,size_t i);
inline uint64_t get8(const unsigned char *data,size_t i);

inline void set1(unsigned char *data,size_t i,unsigned char val);
inline void set2(unsigned char *data,size_t i,unsigned int val);
inline void set3(unsigned char *data,size_t i,unsigned int val);
inline void set4(unsigned char *data,size_t i,unsigned int val);

inline void set6(unsigned char *data,size_t i,uint64_t val);

inline void set8(unsigned char *data,size_t i,uint64_t val);

uint64_t get_file_size(const char *path);


int count_str_same(const char * p,char ch);
int max_len_for_line(char * str);
char *my_strtok_dump(const char *str,const char *ch,int index);
void my_strtok_free(char *str);
long long getFreeMemoryImpl(const char* const sums[], const int sumsLen[], int num);
float get_pcm_time(int sample_rate,int channel,int len);
void mkdirs(char *muldir);

#ifdef __cplusplus
}
#endif

#endif //_TOOL_H
