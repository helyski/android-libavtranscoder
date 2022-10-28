#include <jni.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <debug.h>
#include <malloc.h>
#include "mediacodec_jni.h"
#include "jni_helper.h"


#define INPUT_BUFFER_TIMEOUT_US (5 * 1000ll)

#define INFO_OUTPUT_BUFFERS_CHANGED -3
#define INFO_OUTPUT_FORMAT_CHANGED  -2
#define INFO_TRY_AGAIN_LATER        -1

#define CLASS_ELEMENT_NUM 			6
#define MEMBER_ELEMENT_NUM 			38


typedef struct __tag_media_static_fedlds
{
	//clolor_format
	int COLOR_FormatYUV420Planar;
	int COLOR_FormatYUV420SemiPlanar;
}media_static_fedlds;



struct jfields
{
    jclass media_codec_list_class, media_codec_class, media_format_class;
    jclass buffer_info_class, byte_buffer_class;
    jmethodID tostring;
    jmethodID get_codec_count, get_codec_info_at, is_encoder, get_capabilities_for_type;
    jfieldID profile_levels_field, profile_field, level_field,color_formats_field;
    jmethodID get_supported_types, get_name;
    jmethodID create_by_codec_type,create_by_codec_name, configure, start, stop, flush, release;
    jmethodID get_output_format;
    jmethodID get_input_buffers, get_input_buffer;
    jmethodID get_output_buffers, get_output_buffer;
    jmethodID dequeue_input_buffer, dequeue_output_buffer, queue_input_buffer;
    jmethodID release_output_buffer;
    jmethodID create_video_format,create_audio_format,set_integer, set_bytebuffer,get_integer;
    jmethodID buffer_info_ctor;
    jfieldID size_field, offset_field, pts_field,flags_field;
};


enum Types
{
    METHOD, STATIC_METHOD, FIELD
};

#define OFF(x) offsetof(struct jfields, x)
struct classname
{
    const char *name;
    int offset;
};


struct member
{
    const char *name;
    const char *sig;
    const char *class;
    int offset;
    int type;
    bool critical;
};


struct __tag_media_codec_context
{
	struct MediaCodecConfigInfo config_info;
	enum MediaCodecStatus codec_status;

    jobject codec;
    jobject buffer_info;
    jobject input_buffers, output_buffers;
    bool b_started;
    char *psz_name;
    media_static_fedlds _media_static_fedlds;

    struct jfields  _jfields;
    struct classname classes[CLASS_ELEMENT_NUM];
    struct member members[MEMBER_ELEMENT_NUM];

    int cur_color_format;
    unsigned char* yuv_buffer;
};

void ConvertPlanarYCbCrToNV21(const unsigned char* aSource, unsigned char* aDest,int width,int height,int switch_uv)
 {
	int i = 0;
	int j = 0;
	// Fill Y plane.
	const unsigned char* y = aSource;
	int ysize = width * height;
	int usize = ((width + 1) >> 1) * ((height + 1) >> 1);

	// Y plane.
	memcpy(aDest, y, ysize);
	aDest += ysize;

	// Fill interleaved UV plane.
	const unsigned char* u = y + ysize;
	const unsigned char* v = u + usize;

	// Subsample to 4:2:0 if source is 4:4:4 or 4:2:2.
	// Y plane width & height should be multiple of U/V plane width & height.

	int uvWidth = width / 2;
	int uvHeight = height / 2;

	for (i = 0; i < uvHeight; i++)
	{
		// 1st pixel per line.
		const unsigned char* uSrc = u;
		const unsigned char* vSrc = v;
        if(switch_uv){
            uSrc = v;
            vSrc = u;
        }
		for ( j = 0; j < uvWidth; j++)
		{
			*aDest++ = *uSrc++;
			*aDest++ = *vSrc++;
		}
		// Pick next source line.
		u += uvWidth;
		v += uvWidth;
	}
}

int ConvertI420PToYV12(const unsigned char* src,int width, int height,int stride)
{
	if(!src)
		return -1;

	if(stride <= 0)
		stride = width;

	register int frame_size_y = stride * height;
	register int frame_size_uv = (stride * height >> 2);
	register unsigned char* dst_u_ptr;
	register unsigned char* dst_v_ptr;
	register unsigned char* src_u_ptr;
	register unsigned char* src_v_ptr;

	src_v_ptr = src + frame_size_y;
	src_u_ptr = src_v_ptr + frame_size_uv;

	dst_u_ptr = src + frame_size_y;
	dst_v_ptr = dst_u_ptr + frame_size_uv;

	{
		int i;
		for (i = 0; i < frame_size_uv; i++)
		{
			unsigned char temp_value = *src_v_ptr++;
			*dst_u_ptr++ = *src_u_ptr++;
			*dst_v_ptr++ = temp_value;
		}
	}

}

static void SetClassElement(media_codec_context* context,const char* name,int offset,int index)
{
	if( !context || index < 0 || index > CLASS_ELEMENT_NUM)
		return;
	context->classes[index].name = name;
	context->classes[index].offset = offset;
}

static void SetMemberElement(media_codec_context* context, const char *name,const char *sig,const char *class,int offset,int type,bool critical,int index)
{
	if( !context || index < 0 || index > MEMBER_ELEMENT_NUM)
		return;

	context->members[index].name = name;
	context->members[index].sig = sig;
	context->members[index].class = class;
	context->members[index].offset = offset;
	context->members[index].type = type;
	context->members[index].critical = critical;

}


// 初始化所有 jni fields
static bool InitJNIFields (JNIEnv *env,media_codec_context* context)
{
	if (!env || !context)
		return false;

	bool ret = false;
	int i = 0;

	//初始化所有类
	for (i = 0; context->classes[i].name; i++)
	{
		jclass clazz = (*env)->FindClass(env, context->classes[i].name);
		if (CHECK_EXCEPTION())
		{
			//LOGE("InitJNIFields Unable to find class %s",context->classes[i].name);
			goto end;
		}
		*(jclass*) ((uint8_t*) &context->_jfields + context->classes[i].offset) = (jclass) (*env)->NewGlobalRef(env, clazz);
		SAFE_RELEASE_LOCAL_REF_C(env, clazz)
	}

	//初始化所有类公共成员
	jclass last_class = 0;
	for (i = 0; context->members[i].name; i++)
	{
		last_class = (*env)->FindClass(env, context->members[i].class);
		if (CHECK_EXCEPTION())
		{
			LOGE("InitJNIFields Unable to find class %s",
					context->members[i].class);
			goto end;
		}

		switch (context->members[i].type)
		{
		case METHOD:
			*(jmethodID*) ((uint8_t*) &context->_jfields
					+ context->members[i].offset) = (*env)->GetMethodID(env,
					last_class, context->members[i].name,
					context->members[i].sig);
			break;
		case STATIC_METHOD:
			*(jmethodID*) ((uint8_t*) &context->_jfields
					+ context->members[i].offset) = (*env)->GetStaticMethodID(
					env, last_class, context->members[i].name,
					context->members[i].sig);
			break;
		case FIELD:
			*(jfieldID*) ((uint8_t*) &context->_jfields
					+ context->members[i].offset) = (*env)->GetFieldID(env,
					last_class, context->members[i].name,
					context->members[i].sig);
			break;
		}
		if (CHECK_EXCEPTION())
		{
			LOGW("InitJNIFields Unable to find the member %s in %s",
					context->members[i].name, context->members[i].class);
			if (context->members[i].critical)
				goto end;
		}

		SAFE_RELEASE_LOCAL_REF_C(env, last_class)

	}

	/* getInputBuffers and getOutputBuffers are deprecated if API >= 21
	 * use getInputBuffer and getOutputBuffer instead. */

	if (context->_jfields.get_input_buffer
			&& context->_jfields.get_output_buffer) {
		context->_jfields.get_output_buffers = NULL;
		context->_jfields.get_input_buffers = NULL;
	} else if (!context->_jfields.get_output_buffers
			&& !context->_jfields.get_input_buffers) {
		LOGE("InitJNIFields Unable to find get Output/Input Buffer/Buffers");
		goto end;
	}

	ret = true;

end:
	if (!ret)
		LOGE("InitJNIFields jni init failed");

	return ret;
}

static void UninitJNIFields (JNIEnv *env,media_codec_context* context)
{
	int i;
	for (i = 0; context->classes[i].name; i++)
	{
		SAFE_RELEASE_GLOBAL_REF_C(env,*(jclass*)((uint8_t*)&context->_jfields + context->classes[i].offset))
	}
}

//MediaFormat函数
static jobject media_format_create_video_format(JNIEnv *env,media_codec_context* context,const char *psz_mime,int width,int height)
{
	jobject jformat = NULL;
	jstring jmime = (*env)->NewStringUTF(env, psz_mime);
	jformat = (*env)->CallStaticObjectMethod(env, context->_jfields.media_format_class,context->_jfields.create_video_format, jmime, width, height);
	jformat = (*env)->NewGlobalRef(env, jformat);
	(*env)->DeleteLocalRef(env, jmime);

	return jformat;
}

static void media_format_delete_video_format(JNIEnv *env,jobject obj)
{
	if(!obj)
		return;
	(*env)->DeleteGlobalRef(env, obj);
}

static int media_format_get_integer(JNIEnv *env,media_codec_context* context, jobject obj, const char *psz_name)
{
    int i_ret;
    jstring jname = (*env)->NewStringUTF(env, psz_name);
    i_ret = (*env)->CallIntMethod(env, obj, context->_jfields.get_integer, jname);
    (*env)->DeleteLocalRef(env, jname);

    return i_ret;
}

static void media_format_set_integer(JNIEnv *env,media_codec_context* context, jobject obj, const char *psz_name,int value)
 {
	jstring jname = (*env)->NewStringUTF(env, psz_name);
	(*env)->CallVoidMethod(env, obj, context->_jfields.set_integer, jname, value);
	(*env)->DeleteLocalRef(env, jname);
}

static int jstrcmp(JNIEnv* env, jobject str, const char* str2)
{
    jsize len = (*env)->GetStringUTFLength(env, str);
    if (len != (jsize) strlen(str2))
        return -1;
    const char *ptr = (*env)->GetStringUTFChars(env, str, NULL);
    int ret = memcmp(ptr, str2, len);
    (*env)->ReleaseStringUTFChars(env, str, ptr);
    return ret;
}

static bool populate_static_fields(JNIEnv* env,media_codec_context* context)
{
	if(!context)
		return false;
	jclass cls;
	jfieldID fid;
	const char* class_name = "android/media/MediaCodecInfo$CodecCapabilities";


	cls = (*env)->FindClass(env,class_name);
	if (CHECK_EXCEPTION())
	{
	    LOGE("populate_static_fields Unable to find class %s", class_name);
	    return false;
	}

	fid = (*env)->GetStaticFieldID(env,cls, "COLOR_FormatYUV420Planar","I");
	context->_media_static_fedlds.COLOR_FormatYUV420Planar = (int)(*env)->GetStaticIntField(env,cls, fid);

	fid = (*env)->GetStaticFieldID(env,cls, "COLOR_FormatYUV420SemiPlanar","I");
	context->_media_static_fedlds.COLOR_FormatYUV420SemiPlanar = (int)(*env)->GetStaticIntField(env,cls, fid);

	SAFE_RELEASE_LOCAL_REF_C(env,cls)

	return true;
}

int get_supported_colourformat(JNIEnv *env,const char *psz_mime) {

    if (!env || !psz_mime)
        return -1;

    int colour_format = -1;
    jstring jmime = (*env)->NewStringUTF(env, psz_mime);

    int i,j,k;
    int num_codecs;


    jclass media_codec_list_class = NULL;
    //jclass codec_info_class = NULL;
    jmethodID get_codec_count = NULL;
    jmethodID get_codec_info_at = NULL;
    jmethodID get_name = NULL;

    //MediaCodecList
    media_codec_list_class = (*env)->FindClass(env, "android/media/MediaCodecList");
    if (!media_codec_list_class) {
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        goto end;
    }

    get_codec_count = (*env)->GetStaticMethodID(env, media_codec_list_class, "getCodecCount", "()I");
    get_codec_info_at = (*env)->GetStaticMethodID(env, media_codec_list_class, "getCodecInfoAt","(I)Landroid/media/MediaCodecInfo;");
    if (!get_codec_count || !get_codec_info_at) {
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        goto end;
    }


    //得到编码器数 MediaCodecList.getCodecCount()
    num_codecs = (*env)->CallStaticIntMethod(env,media_codec_list_class,get_codec_count);
    //LOGD("get_media_codec_name_and_color_formats num_codecs:%d",num_codecs);

    for ( i = 0; i < num_codecs; i++)
    {
        jclass codec_info_class = NULL;
        jobject codec_capabilities = NULL;
        jobject color_formats = NULL;
        jobject info = NULL;
        jobject name = NULL;
        jobject types = NULL;
        jmethodID is_encoder,get_capabilities_for_type,get_supported_types;
        jint *color_formats_elems = NULL;
        jsize name_len = 0;
        int profile_levels_len = 0, color_formats_len = 0,num_types = 0;
        const char *name_ptr = NULL;
        bool found = false;



        //得到编码信息 MediaCodecList.getCodecInfoAt
        info = (*env)->CallStaticObjectMethod(env, media_codec_list_class,get_codec_info_at, i);
        codec_info_class = (*env)->GetObjectClass(env, info);
        get_name = (*env)->GetMethodID(env, codec_info_class, "getName","()Ljava/lang/String;");
        name = (*env)->CallObjectMethod(env, info,get_name);
        name_len = (*env)->GetStringUTFLength(env, name);
        name_ptr = (*env)->GetStringUTFChars(env, name, NULL);

        is_encoder = (*env)->GetMethodID(env, codec_info_class, "isEncoder", "()Z");
        get_capabilities_for_type = (*env)->GetMethodID(env, codec_info_class, "getCapabilitiesForType",
                                                           "(Ljava/lang/String;)Landroid/media/MediaCodecInfo$CodecCapabilities;");
        get_supported_types = (*env)->GetMethodID(env, codec_info_class, "getSupportedTypes","()[Ljava/lang/String;");

        //查看是否编码器
        if(! (*env)->CallBooleanMethod(env, info,is_encoder) )
        {
            //LOGD("get_media_codec_name_and_color_formats not encoder");
            goto loopclean;
        }

        //得到编码能力
        codec_capabilities = (*env)->CallObjectMethod(env, info,get_capabilities_for_type,jmime);
        if (CHECK_EXCEPTION()){
            goto loopclean;
        } else if (codec_capabilities){
           // profile_levels = (*env)->GetObjectField(env, codec_capabilities,context->_jfields.profile_levels_field);
          //  if (profile_levels)
          //      profile_levels_len = (*env)->GetArrayLength(env,profile_levels);
        }
        //LOGD("get_media_codec_name_and_color_formats Number of profile levels: %d", profile_levels_len);

        //得到支持的类型字符数组 codecInfo.getSupportedTypes()
        types = (*env)->CallObjectMethod(env, info,get_supported_types);
        num_types = (*env)->GetArrayLength(env, types);
        found = false;

        //LOGD("get_media_codec_name_and_color_formats num_types:%d",num_types);
        for (j = 0; j < num_types && !found; j++)
        {
            jobject type = (*env)->GetObjectArrayElement(env, types, j);
            if (!jstrcmp(env, type, psz_mime))
            {
                found = true;
            }
            (*env)->DeleteLocalRef(env, type);
        }

        if (found){
            jmethodID color_formats_field;
            jclass capabilities_class = NULL;

            capabilities_class = (*env)->GetObjectClass(env, codec_capabilities);
            color_formats_field =	(*env)->GetFieldID(env, capabilities_class, "colorFormats", "[I");

            //获取支持的颜色格式
            color_formats = (*env)->GetObjectField(env, codec_capabilities,color_formats_field);
            if (color_formats)
            {
                color_formats_len = (*env)->GetArrayLength(env,color_formats);

                color_formats_elems = (*env)->GetIntArrayElements (env, color_formats, NULL);

                for( k = 0; k < color_formats_len; k++ )
                {
                    if( 19 == color_formats_elems[k] || 21 == color_formats_elems[k] )
                    {
                        colour_format = color_formats_elems[k];
                        //LOGD("get_media_codec_name_and_color_formats out_color_formats:%d",color_formats_elems[k]);
                        break;
                    }
                }


            }

            if (capabilities_class)
                (*env)->DeleteLocalRef(env, capabilities_class);

        }

        loopclean:
		if (codec_info_class)
			(*env)->DeleteLocalRef(env, codec_info_class);
		if (codec_capabilities)
			(*env)->DeleteLocalRef(env, codec_capabilities);
		if(color_formats)
			(*env)->DeleteLocalRef(env, color_formats);
		if (info)
			(*env)->DeleteLocalRef(env, info);
        if (name)
            (*env)->ReleaseStringUTFChars(env, name, name_ptr);

        if (types)
            (*env)->DeleteLocalRef(env, types);


        if (found)
            break;
    }

end:
    if(jmime)
        SAFE_RELEASE_LOCAL_REF_C(env,jmime)
    if (media_codec_list_class)
        (*env)->DeleteLocalRef (env, media_codec_list_class);

    return colour_format;

}

static char *get_media_codec_name_and_color_formats(media_codec_context* context, JNIEnv *env, const char *psz_mime,int* out_color_formats)
{
    LOGD("get_media_codec_name_and_color_formats begin");

	if(!context)
		NULL;

    if(!psz_mime)
        return 0;

	int i,j,k;
	int num_codecs;

    jstring jmime = (*env)->NewStringUTF(env,psz_mime);

	//得到编码器数 MediaCodecList.getCodecCount()
	num_codecs = (*env)->CallStaticIntMethod(env,context->_jfields.media_codec_list_class,context->_jfields.get_codec_count);
	//LOGD("get_media_codec_name_and_color_formats num_codecs:%d",num_codecs);
	for ( i = 0; i < num_codecs; i++) {
        jobject codec_capabilities = NULL;
        jobject profile_levels = NULL;
        jobject color_formats = NULL;
        jobject info = NULL;
        jobject name = NULL;
        jobject types = NULL;
        jint *color_formats_elems = NULL;
        const char *name_ptr = NULL;
        jsize name_len = 0;
        int profile_levels_len = 0, color_formats_len = 0, num_types = 0;

        bool found = false;

        //得到编码信息 MediaCodecList.getCodecInfoAt
        info = (*env)->CallStaticObjectMethod(env, context->_jfields.media_codec_list_class,
                                              context->_jfields.get_codec_info_at, i);
        if (CHECK_EXCEPTION()) {
            goto loopclean;
        }

        //查看是否编码器
        if (!(*env)->CallBooleanMethod(env, info, context->_jfields.is_encoder)) {
            //LOGD("get_media_codec_name_and_color_formats not encoder");
            goto loopclean;
        }


        //得到支持的类型字符数组 codecInfo.getSupportedTypes()
        types = (*env)->CallObjectMethod(env, info, context->_jfields.get_supported_types);
        if (CHECK_EXCEPTION()) {
            goto loopclean;
        }
        num_types = (*env)->GetArrayLength(env, types);
        found = false;

        //LOGD("get_media_codec_name_and_color_formats num_types:%d",num_types);
        for (j = 0; j < num_types; j++) {
            jobject type = (*env)->GetObjectArrayElement(env, types, j);
            if (0 == jstrcmp(env, type, psz_mime)) {
                found = true;
                (*env)->DeleteLocalRef(env, type);
                break;
            }
            (*env)->DeleteLocalRef(env, type);
        }

        if (found) {
            //获得编码器名字
            name = (*env)->CallObjectMethod(env, info, context->_jfields.get_name);
            if (CHECK_EXCEPTION()) {
                goto loopclean;
            }

            name_len = (*env)->GetStringUTFLength(env, name);
            name_ptr = (*env)->GetStringUTFChars(env, name, NULL);
            if (CHECK_EXCEPTION()) {
                goto loopclean;
            }
            LOGD("get_media_codec_name_and_color_formats coede name:%s", name_ptr);

            //得到编码能力
            codec_capabilities = (*env)->CallObjectMethod(env, info,
                                                          context->_jfields.get_capabilities_for_type,
                                                          jmime);
            if (CHECK_EXCEPTION()) {
                //LOGD("get_media_codec_name_and_color_formats Exception occurred in MediaCodecInfo.getCapabilitiesForType");
                goto loopclean;
            } else if (codec_capabilities) {
                profile_levels = (*env)->GetObjectField(env, codec_capabilities,context->_jfields.profile_levels_field);
                if (profile_levels)
                    profile_levels_len = (*env)->GetArrayLength(env,profile_levels);
            }
            //LOGD("get_media_codec_name_and_color_formats Number of profile levels: %d", profile_levels_len);


            LOGD("get_media_codec_name_and_color_formats using %.*s", name_len, name_ptr);
            if(context->psz_name)
                free(context->psz_name);
            context->psz_name = malloc(name_len + 1);
            memset(context->psz_name,0,name_len + 1);
            memcpy(context->psz_name, name_ptr, name_len);



            //获取支持的颜色格式
            if (context->config_info.b_video) {
                color_formats = (*env)->GetObjectField(env, codec_capabilities,
                                                       context->_jfields.color_formats_field);
                if (color_formats) {
                    color_formats_len = (*env)->GetArrayLength(env, color_formats);

                    color_formats_elems = (*env)->GetIntArrayElements(env, color_formats, NULL);

                    for (k = 0; k < color_formats_len && color_formats_elems; k++) {
                        if (context->_media_static_fedlds.COLOR_FormatYUV420Planar ==
                            color_formats_elems[k] ||
                            context->_media_static_fedlds.COLOR_FormatYUV420SemiPlanar ==
                            color_formats_elems[k]) {
                            *out_color_formats = color_formats_elems[k];
                            //LOGD("get_media_codec_name_and_color_formats out_color_formats:%d",color_formats_elems[k]);
                            break;
                        }
                    }
                    if(color_formats_elems)
                        (*env)->ReleaseIntArrayElements(env,color_formats,color_formats_elems,JNI_ABORT);

                }
            }

        }

        loopclean:

        if(name_ptr)
            (*env)->ReleaseStringUTFChars(env, name, name_ptr);
        SAFE_RELEASE_LOCAL_REF_C(env,name)
        SAFE_RELEASE_LOCAL_REF_C(env,types)
        SAFE_RELEASE_LOCAL_REF_C(env,codec_capabilities)
        SAFE_RELEASE_LOCAL_REF_C(env,profile_levels)
        SAFE_RELEASE_LOCAL_REF_C(env,color_formats)
        SAFE_RELEASE_LOCAL_REF_C(env,info)

        if (found)
            break;
    }

    SAFE_RELEASE_LOCAL_REF_C(env,jmime)

    LOGD("get_media_codec_name_and_color_formats end");
	return context->psz_name;
}

media_codec_context* init_media_codec(JNIEnv *env)
{
	int class_element = 0,member_element = 0;
	media_codec_context* _media_codec_context = (media_codec_context*)malloc(sizeof(media_codec_context));
	if(!_media_codec_context)
		return NULL;

	memset(_media_codec_context,0,sizeof(media_codec_context));

	//为classes赋值
	SetClassElement(_media_codec_context,"android/media/MediaCodecList", OFF(media_codec_list_class),class_element++);
	SetClassElement(_media_codec_context,"android/media/MediaCodec",     OFF(media_codec_class),class_element++);
	SetClassElement(_media_codec_context,"android/media/MediaFormat", 	OFF(media_format_class),class_element++);
	SetClassElement(_media_codec_context,"android/media/MediaCodec$BufferInfo", OFF(buffer_info_class),class_element++);
	SetClassElement(_media_codec_context,"java/nio/ByteBuffer", 			OFF(byte_buffer_class),class_element++);
	SetClassElement(_media_codec_context,NULL, 0,class_element++);

	//为member赋值
	SetMemberElement(_media_codec_context,"toString", "()Ljava/lang/String;", "java/lang/Object", OFF(tostring), METHOD, true,member_element++);

	SetMemberElement(_media_codec_context,"getCodecCount", "()I", "android/media/MediaCodecList", OFF(get_codec_count), STATIC_METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"getCodecInfoAt", "(I)Landroid/media/MediaCodecInfo;", "android/media/MediaCodecList", OFF(get_codec_info_at), STATIC_METHOD, true ,member_element++);

	SetMemberElement(_media_codec_context,"isEncoder", "()Z", "android/media/MediaCodecInfo", OFF(is_encoder), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"getSupportedTypes", "()[Ljava/lang/String;", "android/media/MediaCodecInfo", OFF(get_supported_types), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"getName", "()Ljava/lang/String;", "android/media/MediaCodecInfo", OFF(get_name), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"getCapabilitiesForType", "(Ljava/lang/String;)Landroid/media/MediaCodecInfo$CodecCapabilities;", "android/media/MediaCodecInfo", OFF(get_capabilities_for_type), METHOD, true ,member_element++);

	SetMemberElement(_media_codec_context,"profileLevels", "[Landroid/media/MediaCodecInfo$CodecProfileLevel;", "android/media/MediaCodecInfo$CodecCapabilities", OFF(profile_levels_field), FIELD, true ,member_element++);
	SetMemberElement(_media_codec_context,"profile", "I", "android/media/MediaCodecInfo$CodecProfileLevel", OFF(profile_field), FIELD, true ,member_element++);
	SetMemberElement(_media_codec_context,"level", "I", "android/media/MediaCodecInfo$CodecProfileLevel", OFF(level_field), FIELD, true ,member_element++);
	SetMemberElement(_media_codec_context,"colorFormats", "[I", "android/media/MediaCodecInfo$CodecCapabilities", OFF(color_formats_field), FIELD, true ,member_element++);


	SetMemberElement(_media_codec_context,"createDecoderByType", "(Ljava/lang/String;)Landroid/media/MediaCodec;", "android/media/MediaCodec", OFF(create_by_codec_type), STATIC_METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"createByCodecName", "(Ljava/lang/String;)Landroid/media/MediaCodec;", "android/media/MediaCodec", OFF(create_by_codec_name), STATIC_METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"configure", "(Landroid/media/MediaFormat;Landroid/view/Surface;Landroid/media/MediaCrypto;I)V", "android/media/MediaCodec", OFF(configure), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"start", "()V", "android/media/MediaCodec", OFF(start), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"stop", "()V", "android/media/MediaCodec", OFF(stop), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"flush", "()V", "android/media/MediaCodec", OFF(flush), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"release", "()V", "android/media/MediaCodec", OFF(release), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"getOutputFormat", "()Landroid/media/MediaFormat;", "android/media/MediaCodec", OFF(get_output_format), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"getInputBuffers", "()[Ljava/nio/ByteBuffer;", "android/media/MediaCodec", OFF(get_input_buffers), METHOD, false ,member_element++);
	SetMemberElement(_media_codec_context,"getInputBuffer", "(I)Ljava/nio/ByteBuffer;", "android/media/MediaCodec", OFF(get_input_buffer), METHOD, false ,member_element++);
	SetMemberElement(_media_codec_context,"getOutputBuffers", "()[Ljava/nio/ByteBuffer;", "android/media/MediaCodec", OFF(get_output_buffers), METHOD, false ,member_element++);
	SetMemberElement(_media_codec_context,"getOutputBuffer", "(I)Ljava/nio/ByteBuffer;", "android/media/MediaCodec", OFF(get_output_buffer), METHOD, false ,member_element++);
	SetMemberElement(_media_codec_context,"dequeueInputBuffer", "(J)I", "android/media/MediaCodec", OFF(dequeue_input_buffer), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"dequeueOutputBuffer", "(Landroid/media/MediaCodec$BufferInfo;J)I", "android/media/MediaCodec", OFF(dequeue_output_buffer), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"queueInputBuffer", "(IIIJI)V", "android/media/MediaCodec", OFF(queue_input_buffer), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"releaseOutputBuffer", "(IZ)V", "android/media/MediaCodec", OFF(release_output_buffer), METHOD, true ,member_element++);

	SetMemberElement(_media_codec_context,"createVideoFormat", "(Ljava/lang/String;II)Landroid/media/MediaFormat;", "android/media/MediaFormat", OFF(create_video_format), STATIC_METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"createAudioFormat", "(Ljava/lang/String;II)Landroid/media/MediaFormat;", "android/media/MediaFormat", OFF(create_audio_format), STATIC_METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"setInteger", "(Ljava/lang/String;I)V", "android/media/MediaFormat", OFF(set_integer), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"getInteger", "(Ljava/lang/String;)I", "android/media/MediaFormat", OFF(get_integer), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"setByteBuffer", "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V", "android/media/MediaFormat", OFF(set_bytebuffer), METHOD, true ,member_element++);

	SetMemberElement(_media_codec_context, "<init>", "()V", "android/media/MediaCodec$BufferInfo", OFF(buffer_info_ctor), METHOD, true ,member_element++);
	SetMemberElement(_media_codec_context,"size", "I", "android/media/MediaCodec$BufferInfo", OFF(size_field), FIELD, true ,member_element++);
	SetMemberElement(_media_codec_context,"offset", "I", "android/media/MediaCodec$BufferInfo", OFF(offset_field), FIELD, true ,member_element++);
	SetMemberElement(_media_codec_context,"presentationTimeUs", "J", "android/media/MediaCodec$BufferInfo", OFF(pts_field), FIELD, true ,member_element++);
	SetMemberElement(_media_codec_context,"flags", "I", "android/media/MediaCodec$BufferInfo", OFF(flags_field), FIELD, true ,member_element++);
	SetMemberElement(_media_codec_context,NULL, NULL, NULL, 0, 0, false ,member_element++);

	if(!InitJNIFields(env,_media_codec_context))
	{
		LOGE("init_media_codec_context InitJNIFields failed");
		return NULL;
	}
	if(!populate_static_fields(env,_media_codec_context))
	{
		LOGE("init_media_codec_context InitJNIFields populate_static_fields");
		return NULL;
	}

	_media_codec_context->codec_status = CodecStatus_INIT;

	return _media_codec_context;
}

void uninit_media_codec(JNIEnv *env,media_codec_context* context)
{
	if(!context)
		return;

	UninitJNIFields(env,context);

	context->codec_status = CodecStatus_UNINIT;

	free(context);
}

bool media_codec_set_config_info(JNIEnv *env,media_codec_context* context,const struct MediaCodecConfigInfo *config_info)
{

	if(!context || !config_info)
		return false;

	if( context->codec_status != CodecStatus_INIT )
		return false;


	memcpy(&context->config_info,config_info,sizeof(struct MediaCodecConfigInfo));

	context->codec_status = CodecStatus_CONFIG;

	return true;
}


bool media_codec_start(JNIEnv *env,media_codec_context* context)
{
    LOGD("media_codec_start begin");
	if(!context)
		return false;

	if( context->codec_status != CodecStatus_CONFIG )
		return false;

	bool ret = false;
	bool b_direct_rendering;
	jstring jmime = NULL;
	jstring jcodec_name = NULL;
	jobject jcodec = NULL;
	jobject jformat = NULL;
	jobject jinput_buffers = NULL;
	jobject joutput_buffers = NULL;
	jobject jbuffer_info = NULL;
	char sz_mime[32] = {0};

	if(context->config_info.b_video)
	{
		context->cur_color_format = context->_media_static_fedlds.COLOR_FormatYUV420Planar;
		strcpy(sz_mime,context->config_info.videoCodecTypeStr);
		jmime = (*env)->NewStringUTF(env,sz_mime);
	}
	else
	{
		strcpy(sz_mime,"audio/mp4a-latm");
		jmime = (*env)->NewStringUTF(env,sz_mime);
	}
	if (!jmime)
	     return ret;

    LOGD("media_codec_start get color formats");
	 if(!get_media_codec_name_and_color_formats(context,env,sz_mime,&context->cur_color_format))
	 {
		 LOGE("media_codec_start No suitable codec matching %s was found", sz_mime);
	     goto error;
	 }


    jcodec_name = (*env)->NewStringUTF(env,context->psz_name);

	 if( context->config_info.b_video && context->_media_static_fedlds.COLOR_FormatYUV420SemiPlanar == context->cur_color_format)
	 {
		 //分配yuv buffer
		 context->yuv_buffer = (unsigned char*)malloc( context->config_info._width*context->config_info._height*3/2);
		 if(0 ==  context->yuv_buffer)
		 {
			 LOGE("media_codec_start 分配yuv_buffer失败");
			 goto error;
		 }
	 }


	 //创建编码对象 MediaCodec.createByCodecName
    LOGD("media_codec_start create_by_codec_name");
	jcodec = (*env)->CallStaticObjectMethod(env, context->_jfields.media_codec_class,
			context->_jfields.create_by_codec_name, jcodec_name);
	if (CHECK_EXCEPTION())
	{
		LOGE("media_codec_start Exception occurred in MediaCodec.createByCodecName");
		goto error;
	}
	context->codec = (*env)->NewGlobalRef(env, jcodec);

	 if(context->config_info.b_video)
	 {
		 //创建视频格式 MediaFormat.createVideoFormat
		jformat = (*env)->CallStaticObjectMethod(env,
				context->_jfields.media_format_class,
				context->_jfields.create_video_format, jmime,
				context->config_info._width, context->config_info._height);
	 }
	 else
	 {
		jformat = (*env)->CallStaticObjectMethod(env,
				context->_jfields.media_format_class,
				context->_jfields.create_audio_format, jmime, context->config_info._sample_rate,
				context->config_info._channel);
	 }

	SAFE_RELEASE_LOCAL_REF_C(env,jmime)
	SAFE_RELEASE_LOCAL_REF_C(env,jcodec)
	SAFE_RELEASE_LOCAL_REF_C(env,jcodec_name)

    LOGD("media_codec_start config codec");
	if (context->config_info.b_video)
	{
		//配置视频编码器
		if(context->config_info._var_bitrate)
			media_format_set_integer(env, context, jformat, "bitrate-mode",1); //BITRATE_MODE_CQ = 0;BITRATE_MODE_VBR = 1;BITRATE_MODE_CBR = 2;
		else
			media_format_set_integer(env, context, jformat, "bitrate-mode",2); //BITRATE_MODE_CQ = 0;BITRATE_MODE_VBR = 1;BITRATE_MODE_CBR = 2;
		media_format_set_integer(env, context, jformat, "bitrate", context->config_info._video_bitrate);
		media_format_set_integer(env, context, jformat, "frame-rate", context->config_info._fps);
		media_format_set_integer(env, context, jformat, "color-format",context->cur_color_format); //COLOR_FormatYUV420Planar
		media_format_set_integer(env, context, jformat, "i-frame-interval",1);
	}
	else
	{
		media_format_set_integer(env, context, jformat, "bitrate", context->config_info._audio_bitrate);
		media_format_set_integer(env, context, jformat, "aac-profile", 2);//AACObjectLC
		media_format_set_integer(env, context, jformat, "max-input-size",context->config_info._buffer_size_in_bytes);
	}
	(*env)->CallVoidMethod(env, context->codec, context->_jfields.configure, jformat,NULL,NULL, 1);
	if (CHECK_EXCEPTION())
	{
		LOGE("media_codec_start Exception occurred in MediaCodec.configure");
		goto error;
	}

    SAFE_RELEASE_LOCAL_REF_C(env,jformat)

	//开始
    LOGD("media_codec_start start codec");
	(*env)->CallVoidMethod(env, context->codec, context->_jfields.start);
	if (CHECK_EXCEPTION())
	{
		LOGE("media_codec_start Exception occurred in MediaCodec.start");
	    goto error;
	}
	context->b_started = true;

	//获得输入输出缓存
    LOGD("media_codec_start get codec input and out buffers");
	 if (context->_jfields.get_input_buffers && context->_jfields.get_output_buffers)
	 {

	        jinput_buffers = (*env)->CallObjectMethod(env, context->codec,
	                                                  context->_jfields.get_input_buffers);
	        if (CHECK_EXCEPTION())
	        {
	        	LOGE("media_codec_start Exception in MediaCodec.getInputBuffers");
	            goto error;
	        }
	        context->input_buffers = (*env)->NewGlobalRef(env, jinput_buffers);
	        SAFE_RELEASE_LOCAL_REF_C(env,jinput_buffers)


	        joutput_buffers = (*env)->CallObjectMethod(env, context->codec,
	                                                   context->_jfields.get_output_buffers);
	        if (CHECK_EXCEPTION())
	        {
	        	LOGE("media_codec_start Exception in MediaCodec.getOutputBuffers");
	            goto error;
	        }
	        context->output_buffers = (*env)->NewGlobalRef(env, joutput_buffers);
	        SAFE_RELEASE_LOCAL_REF_C(env,joutput_buffers)
	 }
	 jbuffer_info = (*env)->NewObject(env, context->_jfields.buffer_info_class,context->_jfields.buffer_info_ctor);
	 context->buffer_info = (*env)->NewGlobalRef(env, jbuffer_info);
	 SAFE_RELEASE_LOCAL_REF_C(env,jbuffer_info)

	 context->codec_status = CodecStatus_START;
    LOGD("media_codec_start end");
	return true;

error:
	    if (jmime)
	        (*env)->DeleteLocalRef(env, jmime);
	    if (jcodec_name)
	        (*env)->DeleteLocalRef(env, jcodec_name);
	    if (jcodec)
	        (*env)->DeleteLocalRef(env, jcodec);
	    if (jformat)
	        (*env)->DeleteLocalRef(env, jformat);
	    if (jinput_buffers)
	        (*env)->DeleteLocalRef(env, jinput_buffers);
	    if (joutput_buffers)
	        (*env)->DeleteLocalRef(env, joutput_buffers);
	    if (jbuffer_info)
	        (*env)->DeleteLocalRef(env, jbuffer_info);

	    if (!ret)
	    	media_codec_stop(env,context);

	    if(context->yuv_buffer)
	    {
	    	free(context->yuv_buffer);
	    	context->yuv_buffer = 0;
	    }

	    LOGE("media_codec_start error end");
	    return ret;
}

int media_codec_get_color_format(media_codec_context* context){
    return context->cur_color_format;
}

bool media_codec_stop(JNIEnv *env,media_codec_context* context)
{
	if (!context)
		return false;

	if( context->codec_status != CodecStatus_START  )
			return false;

	if (!context->b_started)
		return false;
	if(context->psz_name)
		free(context->psz_name);

	if (context->input_buffers)
	{
		(*env)->DeleteGlobalRef(env, context->input_buffers);
		context->input_buffers = NULL;
	}
	if (context->output_buffers)
	{
		(*env)->DeleteGlobalRef(env, context->output_buffers);
		context->output_buffers = NULL;
	}
	if (context->codec)
	{
		if (context->b_started)
		{
			(*env)->CallVoidMethod(env, context->codec, context->_jfields.stop);
			if (CHECK_EXCEPTION())
				LOGE("stop_media_codec Exception in MediaCodec.stop");
			context->b_started = false;
		}

		(*env)->CallVoidMethod(env, context->codec, context->_jfields.release);
		if (CHECK_EXCEPTION())
			LOGE("stop_media_codec Exception in MediaCodec.release");
		(*env)->DeleteGlobalRef(env, context->codec);
		context->codec = NULL;
	}
	if (context->buffer_info)
	{
		(*env)->DeleteGlobalRef(env, context->buffer_info);
		context->buffer_info = NULL;
	}

    if(context->yuv_buffer)
    {
    	free(context->yuv_buffer);
    	context->yuv_buffer = 0;
    }

	context->codec_status = CodecStatus_STOP;

	return true;
}

enum MediaCodecStatus media_codec_get_status(JNIEnv *env,media_codec_context* context)
{
	if (!context)
		return CodecStatus_INIT;

	return context->codec_status;
}


bool media_codec_encode(JNIEnv *env,media_codec_context* context,const unsigned char* data, int len,int64_t timestamp)
 {
	if (!context || !data || len <= 0)
		return false;

	if( context->codec_status != CodecStatus_START  )
		return false;

	bool ret = false;

	int index;
	const unsigned char* p_temp = 0;
	uint8_t *p_mc_buf;
	jobject j_mc_buf;
	jsize j_mc_size;
	jint jflags = 0;


	//出队列
	index = (*env)->CallIntMethod(env, context->codec,context->_jfields.dequeue_input_buffer, INPUT_BUFFER_TIMEOUT_US);
	if (CHECK_EXCEPTION())
	{
		LOGE("media_codec_encode Exception occurred in MediaCodec.dequeueInputBuffer");
		return ret;
	}


	if (index < 0)
	{
		LOGW("media_codec_encode index小于0 ");
		return ret;
	}


	if (context->_jfields.get_input_buffers)
		j_mc_buf = (*env)->GetObjectArrayElement(env, context->input_buffers,index);
	else
		j_mc_buf = (*env)->CallObjectMethod(env, context->codec,context->_jfields.get_input_buffer, index);

	j_mc_size = (*env)->GetDirectBufferCapacity(env, j_mc_buf);
	p_mc_buf = (*env)->GetDirectBufferAddress(env, j_mc_buf);
	if (j_mc_size < 0)
	{
		LOGE("media_codec_encode Java buffer has invalid size");
		(*env)->DeleteLocalRef(env, j_mc_buf);
		return false;
	}

	if (j_mc_size < len)
	{
		LOGE("media_codec_encode Java buffer j_mc_size < len");
		(*env)->DeleteLocalRef(env, j_mc_buf);
		return false;
	}

	p_temp = data;

	if( context->config_info.b_video )
	{
		if(context->_media_static_fedlds.COLOR_FormatYUV420SemiPlanar == context->cur_color_format && context->yuv_buffer) {
			ConvertPlanarYCbCrToNV21(p_temp, context->yuv_buffer, context->config_info._width,
									 context->config_info._height,context->config_info.switch_uv);
			p_temp = context->yuv_buffer;
		}

		if(context->_media_static_fedlds.COLOR_FormatYUV420Planar == context->cur_color_format  && context->config_info.switch_uv ){
			ConvertI420PToYV12(data,context->config_info._width,context->config_info._height,context->config_info._width);
		}

	}



	memcpy(p_mc_buf, p_temp, len);

	(*env)->CallVoidMethod(env, context->codec, context->_jfields.queue_input_buffer,index, 0, j_mc_size, timestamp, jflags);
	(*env)->DeleteLocalRef(env, j_mc_buf);
	if (CHECK_EXCEPTION())
	{
		LOGE("media_codec_encode Exception in MediaCodec.queueInputBuffer");
		return false;
	}


	return true;
}

void media_codec_add_adts(media_codec_context* context,unsigned char* packet, int packetLen)
 {
	if (!context)
		return;

	int profile = 2;  //AAC LC
					  //39=MediaCodecInfo.CodecProfileLevel.AACObjectELD;
	int freqIdx = 8;  //16KHz
	int chanCfg = context->config_info._channel;  //CPE

	switch (context->config_info._sample_rate)
	{
	case 8000:
		freqIdx = 11;
		break;
	case 16000:
		freqIdx = 8;
		break;
	case 22050:
		freqIdx = 7;
		break;
	case 24000:
		freqIdx = 6;
		break;
	case 32000:
		freqIdx = 5;
		break;
	case 44100:
		freqIdx = 4;
		break;
	case 48000:
		freqIdx = 3;
		break;
	default:
		freqIdx = 8;
	}

	// fill in ADTS data
	packet[0] = (unsigned char) 0xFF;
	packet[1] = (unsigned char) 0xF9;
	packet[2] = (unsigned char) (((profile) << 6) + (freqIdx << 2)
			+ (chanCfg >> 2));
	packet[3] = (unsigned char) (((chanCfg & 3) << 6) + (packetLen >> 11));
	packet[4] = (unsigned char) ((packetLen & 0x7FF) >> 3);
	packet[5] = (unsigned char) (((packetLen & 7) << 5) + 0x1F);
	packet[6] = (unsigned char) 0xFC;
}

int media_codec_get_frame(JNIEnv *env,media_codec_context* context,unsigned char** p_out,int* out_len,int64_t* out_timestamp, int* output_flags,int64_t timeout)
{
	if (!context || !p_out || !out_len )
		return -1;

	if( context->codec_status != CodecStatus_START )
		return 0;

	int i_index;
	unsigned char* p_data = 0;

	i_index = (*env)->CallIntMethod(env, context->codec,context->_jfields.dequeue_output_buffer, context->buffer_info, timeout);
	if (CHECK_EXCEPTION())
	{
		LOGE("media_codec_get_frame Exception in MediaCodec.dequeueOutputBuffer");
		return -1;
	}

	if (i_index >= 0)
	{
		jobject buf;
		uint8_t *ptr;
		int offset;
		if (context->_jfields.get_output_buffers)
			buf = (*env)->GetObjectArrayElement(env, context->output_buffers,i_index);
		else
			buf = (*env)->CallObjectMethod(env, context->codec,context->_jfields.get_output_buffer, i_index);

		 ptr = (*env)->GetDirectBufferAddress(env, buf);

		*out_timestamp = (*env)->GetLongField(env, context->buffer_info,context->_jfields.pts_field);
        *output_flags = (*env)->GetIntField(env, context->buffer_info,context->_jfields.flags_field);
		 offset = (*env)->GetIntField(env, context->buffer_info,context->_jfields.offset_field);
		 p_data =  ptr + offset;
		 *out_len = (*env)->GetIntField(env, context->buffer_info,context->_jfields.size_field);

		 if(context->config_info.b_video){
			 *p_out = (unsigned char*)malloc(*out_len);
			 if(*p_out)
			 	memcpy(*p_out,p_data,*out_len);
			 else
				 *out_len = 0;
		 }else{
			 int packetLen = *out_len+7;
             *p_out = (unsigned char*)malloc(packetLen);
             if(*p_out) {
                 media_codec_add_adts(context, *p_out, packetLen);
                 memcpy(*p_out + 7, p_data, *out_len);
				 *out_len = packetLen;
             } else
				 *out_len = 0;
		 }

		 (*env)->DeleteLocalRef(env, buf);


	}
	else if (i_index == INFO_OUTPUT_FORMAT_CHANGED)
	{
		jobject format = NULL;
		jobject format_string = NULL;
		jsize format_len;
		const char *format_ptr;
        LOGD("media_codec_get_frame output format changed");
		format = (*env)->CallObjectMethod(env, context->codec,context->_jfields.get_output_format);
		if (CHECK_EXCEPTION())
		{
			LOGE("media_codec_get_frame Exception in MediaCodec.getOutputFormat");
			return i_index;
		}
		format_string = (*env)->CallObjectMethod(env, format, context->_jfields.tostring);
		format_len = (*env)->GetStringUTFLength(env, format_string);
		format_ptr = (*env)->GetStringUTFChars(env, format_string, NULL);

        LOGD("media_codec_get_frame format_string:%s", format_ptr);
		(*env)->ReleaseStringUTFChars(env, format_string, format_ptr);

		(*env)->DeleteLocalRef(env, format);

	} else if (i_index == INFO_OUTPUT_BUFFERS_CHANGED)
	{
		jobject joutput_buffers;

        LOGD("media_codec_get_frame output buffers changed");
		if (!context->_jfields.get_output_buffers)
			return i_index;
		(*env)->DeleteGlobalRef(env, context->output_buffers);

		joutput_buffers = (*env)->CallObjectMethod(env, context->codec,context->_jfields.get_output_buffers);
		if (CHECK_EXCEPTION())
		{
			LOGE("media_codec_get_frame Exception in MediaCodec.getOutputBuffer");
			context->output_buffers = NULL;
			return i_index;
		}
		context->output_buffers = (*env)->NewGlobalRef(env, joutput_buffers);
		(*env)->DeleteLocalRef(env, joutput_buffers);
	}


	return i_index;
}

bool media_codec_release_output(JNIEnv *env,media_codec_context* context, int i_index)
{
    (*env)->CallVoidMethod(env,context->codec, context->_jfields.release_output_buffer,i_index,0);
    if (CHECK_EXCEPTION())
    {
    	LOGE("Exception in MediaCodec.releaseOutputBuffer");
        return false;
    }

    return true;
}




