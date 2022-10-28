#ifndef _OMX_CODEC_H_
#define _OMX_CODEC_H_

#include <jni.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __tag_mediacodec_context mediacodec_context;

enum OmxCodecStatus{
	OMX_Status_INIT,
	OMX_Status_OPEN,
	OMX_Status_CLOSE,
	OMX_Status_UNINIT
};


typedef struct __tag_OmxVideoContext
{
	//in_put param
	int width;
	int height;
	int fps;
	int64_t frame_num;
	unsigned long long start_time;

	//out_put param
	int64_t output_timestamp;
	int output_flags;
	unsigned char* sps_pps;
	int sps_pps_len;

	//private
	mediacodec_context* _mediacodec_context;
}OmxVideoContext;

typedef struct __tag_OmxCodecContext
{
	bool is_video;
	bool switch_uv;
	OmxVideoContext vc;
}OmxCodecContext;

OmxCodecContext* init_omx_codec_context();
void omx_switch_uv(OmxCodecContext* context,bool is_switch);
bool omx_open_video_codec(JNIEnv *env,OmxCodecContext* context,int width,int height,int fps,int bit_rate,bool var_bitrate,const char *codecTypeStr);
bool omx_open_audio_codec(JNIEnv *env,OmxCodecContext* context,int sample_rate,int channel,int format,int bit_rate);
int omx_get_color_format(JNIEnv *env,OmxCodecContext* context);
int omx_search_sps_pps(JNIEnv *env,int width,int height,int fps,int bit_rate,const char *codecTypeStr,unsigned char* output_buf,int* output_buf_len);
int omx_get_sps_pps_len(OmxCodecContext* context);
const unsigned char *omx_get_sps_pps_data(OmxCodecContext* context);
bool omx_encode(JNIEnv *env,OmxCodecContext* context,const unsigned char* in_data,int in_len,long long in_timestamp_ms);
bool omx_get_encoded_frame(JNIEnv *env,OmxCodecContext* context,unsigned char** output_buf,int* output_buf_len,int64_t* out_timestamp);
enum OmxCodecStatus omx_get_encode_status(OmxCodecContext* context);
int64_t omx_get_encode_frame_num(OmxCodecContext* context);
bool omx_close_codec(JNIEnv *env,OmxCodecContext* context);
void uninit_omx_codec_context(OmxCodecContext* context);
int omx_get_supported_colourformat(JNIEnv *env,const char *psz_mime);
#ifdef __cplusplus
}
#endif

#endif //_OMX_CODEC_H_
