#include <string.h>
#include <stdlib.h>
#include "tool.h"
#include "mediacodec_jni.h"
#include "OMXCodec.h"
#include "debug.h"


struct __tag_mediacodec_context
{
	void *_video_codec_context;
	enum OmxCodecStatus omx_codec_status;
	char* initial_image;
};

void reset_video_param(OmxCodecContext* context)
{
	if(!context)
		return;

	context->vc.frame_num = 0;
	if( context->vc.sps_pps )
		free(context->vc.sps_pps);
	context->vc.sps_pps = 0;
	context->vc.sps_pps_len = 0;

}

int64_t grow_encode_timestamp(OmxCodecContext* context)
{
	if(!context)
		return 0;

	if(context->vc.fps <= 0)
		context->vc.fps = 15;

	//计算编码时间戳
	int64_t ONE_MICROSECONDS = 1000000;
	int64_t in_timestamp = (++context->vc.frame_num)*ONE_MICROSECONDS/context->vc.fps;

	return in_timestamp;
}

OmxCodecContext* init_omx_codec_context()
{
	LOGD("init_omx_codec_context begin");
	OmxCodecContext *context = 0;

	context = (OmxCodecContext*)malloc(sizeof(OmxCodecContext));
	if(!context)
	{
		LOGE("init_omx_codec_context 分配context失败");
		goto end;
	}

	memset(context,0,sizeof(OmxCodecContext));
	context->vc._mediacodec_context = (mediacodec_context*)malloc(sizeof(mediacodec_context));
	if(0==context->vc._mediacodec_context)
	{
		LOGE("init_omx_codec_context 分配_mediacodec_context失败");
		free(context);
		goto end;
	}

	context->vc._mediacodec_context->omx_codec_status = OMX_Status_INIT;
	context->vc._mediacodec_context->initial_image = 0;
	LOGD("init_omx_codec_context end");
end:
	return context;
}

void omx_switch_uv(OmxCodecContext* context,bool is_switch){
	if(!context)
		return;
	context->switch_uv = is_switch;
}

bool omx_open_video_codec(JNIEnv *env,OmxCodecContext* context,int width,int height,int fps,int bit_rate,bool var_bitrate,const char *codecTypeStr)
{
	LOGD("omx_open_video_codec begin");
	bool ret = false;
	if(!env || !context)
		return ret;

	if( OMX_Status_INIT != context->vc._mediacodec_context->omx_codec_status && OMX_Status_CLOSE != context->vc._mediacodec_context->omx_codec_status)
	{
		LOGE("omx_open_video_codec 状态错误:%d",context->vc._mediacodec_context->omx_codec_status);
		return ret;
	}

	if( width <= 0 || height <= 0 || fps <= 0 || bit_rate <= 0 )
	{
		LOGE("omx_open_video_codec 参数错误");
		goto end;
	}

	//初始化编码参数
	reset_video_param(context);
	struct MediaCodecConfigInfo codec_config_info;
	memset(&codec_config_info,0,sizeof(struct MediaCodecConfigInfo));
	codec_config_info.b_video = true;
	codec_config_info._width = width;
	codec_config_info._height = height;
	codec_config_info._fps = fps;
	codec_config_info._video_bitrate = bit_rate;
	codec_config_info._var_bitrate = var_bitrate;
	codec_config_info.switch_uv = context->switch_uv;
	if(codecTypeStr && strlen(codecTypeStr) > 0)
		strcpy(codec_config_info.videoCodecTypeStr,codecTypeStr);
	else
		strcpy(codec_config_info.videoCodecTypeStr,"video/avc");

	//初始化编码器
	context->vc._mediacodec_context->_video_codec_context = init_media_codec(env);
	if( 0 == context->vc._mediacodec_context->_video_codec_context )
	{
		LOGE("omx_open_video_codec init_media_codec失败");
		goto end;
	}

	//设置编码器参数
	if(!media_codec_set_config_info(env,context->vc._mediacodec_context->_video_codec_context,&codec_config_info))
	{
		LOGE("omx_open_video_codec media_codec_set_config_info失败");
		goto end;
	}

	//启动编码器
	if(!media_codec_start(env,context->vc._mediacodec_context->_video_codec_context) )
	{
		LOGE("omx_open_video_codec media_codec_start失败");
		goto end;
	}

	context->vc.width = width;
	context->vc.height = height;
	context->vc.fps = fps;
	context->is_video = true;
	context->vc._mediacodec_context->omx_codec_status = OMX_Status_OPEN;
	ret = true;
	LOGD("omx_open_video_codec end");
end:
	LOGD("omx_open_video_codec success");
	return ret;
}

bool omx_open_audio_codec(JNIEnv *env,OmxCodecContext* context,int sample_rate,int channel,int format,int bit_rate){
	LOGD("omx_open_audio_codec begin");
	bool ret = false;
	if(!env || !context)
		return ret;

	if( OMX_Status_INIT != context->vc._mediacodec_context->omx_codec_status && OMX_Status_CLOSE != context->vc._mediacodec_context->omx_codec_status)
	{
		LOGE("omx_open_audio_codec 状态错误:%d",context->vc._mediacodec_context->omx_codec_status);
		return ret;
	}

	if( sample_rate <= 0 || channel <= 0 || format <= 0 || bit_rate <= 0)
	{
		LOGE("omx_open_codec 参数错误");
		goto end;
	}

	//初始化编码参数
	reset_video_param(context);
	struct MediaCodecConfigInfo codec_config_info;
	memset(&codec_config_info,0,sizeof(struct MediaCodecConfigInfo));
	codec_config_info.b_video = false;
	codec_config_info._sample_rate = sample_rate;
	codec_config_info._channel = channel;
	codec_config_info._bits = format;
	codec_config_info._audio_bitrate = bit_rate;
	//codec_config_info._buffer_size_in_bytes = 100 * 1024;

	//初始化编码器
	context->vc._mediacodec_context->_video_codec_context = init_media_codec(env);
	if( 0 == context->vc._mediacodec_context->_video_codec_context )
	{
		LOGE("omx_open_audio_codec init_media_codec失败");
		goto end;
	}

	//设置编码器参数
	if(!media_codec_set_config_info(env,context->vc._mediacodec_context->_video_codec_context,&codec_config_info))
	{
		LOGE("omx_open_audio_codec media_codec_set_config_info失败");
		goto end;
	}

	//启动编码器
	if(!media_codec_start(env,context->vc._mediacodec_context->_video_codec_context) )
	{
		LOGE("omx_open_audio_codec media_codec_start失败");
		goto end;
	}
	context->is_video = false;
	context->vc._mediacodec_context->omx_codec_status = OMX_Status_OPEN;
	ret = true;
	LOGD("omx_open_audio_codec end");
	end:
	return ret;
}

int omx_get_color_format(JNIEnv *env,OmxCodecContext* context){
	int ret = -1;
	if(!env || !context)
		return ret;

    ret = media_codec_get_color_format(context->vc._mediacodec_context->_video_codec_context);

	return ret;
}

bool omx_close_codec(JNIEnv *env,OmxCodecContext* context)
{
	bool ret = false;
	if(!env || !context)
		return ret;

	if( OMX_Status_INIT != context->vc._mediacodec_context->omx_codec_status && OMX_Status_OPEN != context->vc._mediacodec_context->omx_codec_status)
		return ret;

	if( 0 == context->vc._mediacodec_context->_video_codec_context)
	{
		LOGE("omx_close_codec context->vc._mediacodec_context->_video_codec_context 为NULL");
		goto end;
	}

	if(!media_codec_stop(env,context->vc._mediacodec_context->_video_codec_context))
	{
		LOGE("omx_close_codec media_codec_stop失败");
		goto end;
	}

	uninit_media_codec(env,context->vc._mediacodec_context->_video_codec_context);


	context->vc._mediacodec_context->omx_codec_status = OMX_Status_CLOSE;
	ret = true;

end:
	return ret;
}

bool omx_create_test_image(OmxCodecContext* context,int width,int height)
{
	if(!context)
		return false;

	int size = width*height;
	if( context->vc._mediacodec_context->initial_image )
		free(context->vc._mediacodec_context->initial_image);
	context->vc._mediacodec_context->initial_image = 0;

	context->vc._mediacodec_context->initial_image = (char*)malloc(3*size/2);
	if(!context->vc._mediacodec_context->initial_image)
		return false;

	int i = 0;
	char* initial_image = context->vc._mediacodec_context->initial_image;

	for ( i=0;i<size;i++)
	{
		initial_image[i] = (char) (40+i%199);
	}

	for ( i=size;i<3*size/2;i+=2)
	{
		initial_image[i] = (char) (40+i%200);
		initial_image[i+1] = (char) (40+(i+99)%200);
	}

	return true;
}

int copy_sps_and_pps(const unsigned char* h264_buff,int h264_buff_len,unsigned char* sps_pps)
{
	if( !h264_buff || !sps_pps )
		return 0;

	const unsigned char* p = NULL;
	int nal_start;
    int nal_end;
	int len;
	int total_len;

	p = h264_buff;
    nal_start = 0;
    nal_end = 0;
	len = 0;
	total_len = 0;

	//sps
	len =  find_nal_units(h264_buff,h264_buff_len,&nal_start,&nal_end);
	if ( (nal_end-nal_start) > 0 )
	{
		memcpy(sps_pps,h264_buff+nal_start,len);
		total_len += len;
		p += len;
	}



	//pps
	nal_start = 0;
	nal_end = 0;
	len =  find_nal_units(p,h264_buff_len-len,&nal_start,&nal_end);
	if ( len < 0 )
		len = nal_end-nal_start;
	if ( len > 0 )
	{
		memcpy(sps_pps+total_len,p,len);
		total_len += len;
	}

	return total_len;
}

int omx_search_sps_pps(JNIEnv *env,int width,int height,int fps,int bit_rate,const char *codecTypeStr,unsigned char* output_buf,int* output_buf_len)
{
	int ret = 0;
	if(!env  || !output_buf || !output_buf_len)
		return ret;

	bool is_init = false,is_open = false;


	OmxCodecContext * omx_context = init_omx_codec_context();
	if(!omx_context)
		return ret;
	else
		is_init = true;

	if (!omx_open_video_codec(env, omx_context,width, height,fps,bit_rate,false,codecTypeStr))
	{
		goto end;
	}
	else
		is_open = true;

	if(!omx_create_test_image(omx_context,width,height))
		goto end;

	int yuv_len = width*height*3/2;
	unsigned char sps[256] = {0};
	int sps_len = 0,pps_len = 0;
	unsigned char pps[256] = {0};
	long elapsed = 0;
	int counter = 0;
	unsigned long long now = GetCurSysTime();

	int h264_output_buf_len = 0;
	unsigned char* h264_output_buf = 0;

	while ( (elapsed < 500 || counter < 25 ) && (omx_context->vc.sps_pps_len <= 0))
	{
        elapsed = GetCurSysTime()-now;
		counter ++;
		if(!media_codec_encode(env,omx_context->vc._mediacodec_context->_video_codec_context,omx_context->vc._mediacodec_context->initial_image,yuv_len,grow_encode_timestamp(omx_context)))
		{
			mysleep(5);
			continue;
		}

		int index = -1;
		index = media_codec_get_frame(env,omx_context->vc._mediacodec_context->_video_codec_context,&h264_output_buf,&h264_output_buf_len,&omx_context->vc.output_timestamp,&omx_context->vc.output_flags,1000*1000);
		if( index < 0 )
		{
			LOGD("omx_get_encoded_frame media_codec_get_frame 此时没有有效的视频帧");
			continue;
		}


		if ( h264_output_buf_len > 0 && h264_output_buf_len <= 1024)
		{
			*output_buf_len = copy_sps_and_pps(h264_output_buf,h264_output_buf_len,output_buf);
			ret = *output_buf_len;
			LOGD("omx_get_encoded_frame media_codec_get_frame get sps and pps len:%d",h264_output_buf_len);

			media_codec_release_output(env,omx_context->vc._mediacodec_context->_video_codec_context,index);
			break;
		}
		else
		{
			media_codec_release_output(env,omx_context->vc._mediacodec_context->_video_codec_context,index);
			goto end;
		}

	}

end:
	if(is_open)
		omx_close_codec(env,omx_context);
	if(is_init)
		uninit_omx_codec_context(omx_context);

	if(h264_output_buf)
		free(h264_output_buf);
	return ret;
}

int omx_get_sps_pps_len(OmxCodecContext* context){
	if(!context)
		return 0;

	return context->vc.sps_pps_len;
}

const  unsigned char *omx_get_sps_pps_data(OmxCodecContext* context){
	if(!context)
		return 0;

	return context->vc.sps_pps;
}

/*
int omx_search_sps_pps(JNIEnv *env,OmxCodecContext* context,const unsigned char* in_data,int in_len,unsigned char* output_buf,int* output_buf_len)
{
	int ret = 0;
	if(!env || !context || !in_data || !output_buf || !output_buf_len)
		return ret;

	if(OMX_Status_OPEN != context->vc._mediacodec_context->omx_codec_status)
	{
		LOGE("omx_encode 编码器尚未打开");
		goto end;
	}

	if( in_len <= 0 )
	{
		LOGE("omx_encode 参数错误");
		goto end;
	}

	unsigned char sps[256] = {0};
	int sps_len = 0,pps_len = 0;
	unsigned char pps[256] = {0};
	long elapsed = 0;
	unsigned long long now = GetCurSysTime();

	while (elapsed < 1000*60 && (context->vc.sps_pps_len <= 0))
	{
		if(!media_codec_encode(env,context->vc._mediacodec_context->_video_codec_context,in_data,in_len,grow_encode_timestamp(context)))
		{
			mysleep(30);
			continue;
		}

		int index = -1;
		index = media_codec_get_frame(env,context->vc._mediacodec_context->_video_codec_context,&output_buf,output_buf_len,&context->vc.output_timestamp,&context->vc.output_flags,1000*1000);
		if( index < 0 )
		{
			LOGD("omx_get_encoded_frame media_codec_get_frame 此时没有有效的视频帧");
			continue;
		}

		if ( *output_buf_len > 0 && *output_buf_len < 128)
		{
			context->vc.sps_pps = (unsigned char*)malloc(*output_buf_len);
			if(context->vc.sps_pps)
			{
				context->vc.sps_pps_len = *output_buf_len;
				memcpy(context->vc.sps_pps,output_buf,context->vc.sps_pps_len);
				ret = context->vc.sps_pps_len;
			}
		}
		else
		{
			LOGD("omx_get_encoded_frame media_codec_get_frame 获取sps、pps失败");
			goto end;
		}

		media_codec_release_output(env,context->vc._mediacodec_context->_video_codec_context,index);
	}

end:
		return ret;
}
*/
bool omx_encode(JNIEnv *env,OmxCodecContext* context,const unsigned char* in_data,int in_len,long long in_timestamp_ms)
{
	bool ret = false;

	if(!env || !context || !in_data)
		return ret;

	if(OMX_Status_OPEN != context->vc._mediacodec_context->omx_codec_status)
	{
		LOGE("omx_encode 编码器尚未打开");
		goto end;
	}

	if( in_len <= 0 )
	{
		LOGE("omx_encode 参数错误");
		goto end;
	}

	if(0 == context->vc.start_time)
		context->vc.start_time = GetCurSysTime();

	int64_t timestamp =  (GetCurSysTime()-context->vc.start_time)*1000;
	if(in_timestamp_ms <= 0)
		in_timestamp_ms = grow_encode_timestamp(context);

	if(!media_codec_encode(env,context->vc._mediacodec_context->_video_codec_context,in_data,in_len,in_timestamp_ms))
	{
		LOGE("omx_encode media_codec_encode 失败");
		goto end;

	}

	ret = true;

end:
	return ret;
}


bool omx_get_encoded_frame(JNIEnv *env,OmxCodecContext* context,unsigned char** output_buf,int* output_buf_len,int64_t* out_timestamp)
{
	bool ret = false;

	if(!env || !context || !output_buf || !output_buf_len )
		return ret;

	if(OMX_Status_OPEN != context->vc._mediacodec_context->omx_codec_status)
	{
		LOGE("omx_get_encoded_frame 编码器尚未打开");
		goto end;
	}


	int index = -1;
	index = media_codec_get_frame(env,context->vc._mediacodec_context->_video_codec_context,output_buf,output_buf_len,&context->vc.output_timestamp,&context->vc.output_flags,0/*1000*10*/);
	if( index < 0 )
	{
		//LOGD("omx_get_encoded_frame media_codec_get_frame 此时没有有效的视频帧");
		goto end;
	}
	*out_timestamp = context->vc.output_timestamp;


	//编码出来的配置信息包含sps和pps
	if( 0  == context->vc.sps_pps && context->is_video )
	{
		if( *output_buf_len > 0 && 2 == context->vc.output_flags)
		{
			context->vc.sps_pps = (unsigned char*)malloc(*output_buf_len);
			if(context->vc.sps_pps)
			{
				context->vc.sps_pps_len = *output_buf_len;
				memcpy(context->vc.sps_pps,*output_buf,context->vc.sps_pps_len);
			}
		}
	}

	media_codec_release_output(env,context->vc._mediacodec_context->_video_codec_context,index);

	ret = true;

end:
	return ret;
}

enum OmxCodecStatus omx_get_encode_status(OmxCodecContext* context)
{
	if(!context)
		return 0;

	return context->vc._mediacodec_context->omx_codec_status;
}

int64_t omx_get_encode_frame_num(OmxCodecContext* context)
{
	if(!context)
		return 0;

	if(OMX_Status_OPEN != context->vc._mediacodec_context->omx_codec_status)
		return 0;

	return context->vc.frame_num;
}

void uninit_omx_codec_context(OmxCodecContext* context)
{
	if(!context)
		return;

	if(context->vc.sps_pps)
		free(context->vc.sps_pps);

	if(context->vc._mediacodec_context->initial_image)
		free(context->vc._mediacodec_context->initial_image);

	context->vc._mediacodec_context->omx_codec_status = OMX_Status_UNINIT;

	free(context->vc._mediacodec_context);
	free(context);
}

int omx_get_supported_colourformat(JNIEnv *env,const char *psz_mime){
	return get_supported_colourformat(env,psz_mime);
}
