#ifndef MEDIACODEC_JNI_H_
#define MEDIACODEC_JNI_H_


#include <jni.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __tag_media_codec_context media_codec_context;

enum MediaCodecStatus{
	CodecStatus_INIT,
	CodecStatus_CONFIG,
	CodecStatus_START,
	CodecStatus_STOP,
	CodecStatus_UNINIT
};

struct MediaCodecConfigInfo
{
	bool b_video;

	//video param
	int _width;
	int _height;
	float _fps;
	int _video_bitrate;
	bool _var_bitrate;
	bool switch_uv;
	char videoCodecTypeStr[64];

	//audio param
	int _sample_rate;
	int _channel;
	int _bits;
	int _audio_bitrate;
	int _buffer_size_in_bytes;
};

media_codec_context* init_media_codec(JNIEnv *env);
bool media_codec_set_config_info(JNIEnv *env,media_codec_context* context,const struct MediaCodecConfigInfo *config_info);
bool media_codec_start(JNIEnv *env,media_codec_context* context);
int media_codec_get_color_format(media_codec_context* context);
bool media_codec_encode(JNIEnv *env,media_codec_context* context,const unsigned char* data, int len,int64_t timestamp);
int media_codec_get_frame(JNIEnv *env,media_codec_context* context,unsigned char** p_out,int* out_len,int64_t* out_timestamp, int* output_flags,int64_t timeout);
bool media_codec_release_output(JNIEnv *env,media_codec_context* context, int i_index);
bool media_codec_stop(JNIEnv *env,media_codec_context* context);
enum MediaCodecStatus media_codec_get_status(JNIEnv *env,media_codec_context* context);
void uninit_media_codec(JNIEnv *env,media_codec_context* context);
int get_supported_colourformat(JNIEnv *env,const char *psz_mime);
#ifdef __cplusplus
}
#endif

#endif //MEDIACODEC_JNI_H_
