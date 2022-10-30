package com.tangjn.libtranscode;

/**
 * @author Tank
 *
 * More about this project please visit <a href="helyski.github.io">avtranscoder</> home page.
 */
public class JNILibTranscode {

    public interface TRANSCODE_OUTPUT_TYPE{
        int OUTPUT_VIDEO_FILE = 0;
        int OUTPUT_H264_STREAM_SHARE_TO_JAVA = 1;
        int OUTPUT_H264_STREAM_PUSH_TO_RTMP_SERVER = 2;
    }



    /**
     * Init AvTranscoder.
     * @return JNI_TRUE(1) success.
     */
    public static native int Init();

    /**
     * Destroy AvTranscoder and release all resources.
     * @return JNI_TRUE(1) success.
     */
    public static native int UnInit();

    /**
     * Open ffmpeg log and redirect to android logcat.
     * @return JNI_TRUE(1) success.
     */
    public static native int OpenFFmpegLog();

    /**
     * FFMPEG support Android Mediacodec to hw_decode video,but do not support hw_encode.
     * So this only effect in decode work flow.
     *
     * Default will auto detect mediacodec,if it has been supported, choose it to decode.
     * Otherwise choose a sw_decode way.
     *
     * @param enable if true,decoder will try to use h264_mediacodec.
     *               if false,decoder use ffmpeg sw_decode.
     * @return JNI_TRUE(1) success.
     */
    public static native int SetEnableMediaCodec(boolean enable);

    /**
     * Set Transcode Output type.All the types define in {@link TRANSCODE_OUTPUT_TYPE}
     *
     * Default will transcode to Video File.
     * 
     * @param outPutType the output type.
     * @return JNI_TRUE(1) success.
     */
    public static native int SetTranscodeOutputType(int outPutType);

    /**
     * Set Folder Direction for output file,Not a file full path.
     *
     * Default dir as the same as src file in.
     *
     * Only need to set this when {@link this#SetTranscodeOutputType(int)}
     * with {@link TRANSCODE_OUTPUT_TYPE#OUTPUT_VIDEO_FILE}
     *
     * @param dir Folder path for Output Video File. Can not NULL.
     * @return JNI_TRUE(1) success.
     */
    public static native int SetOutputFileDirection(String dir);

    /**
     * Start transcode a video by given {@code srcVideoPath}.If
     *
     * @param srcVideoPath The Source Video File Full Path.
     * @param destVideoName The Output File Name,but do not contain the folder dir.
     * @param seekSeconds Seek seconds in this file.
     * @param destWidth Output video resolution Width.
     * @param destHeight Output video resolution Height.
     * @param destBitrate Output video bitrate.
     * @param destFPS Output video FPS.If this value larger than origin video fps,it will not work.
     * @param needAudio If true,output video has audio.
     * @return JNI_TRUE(1) success.
     */
    public static native int StartTranscode(String srcVideoPath,String destVideoName,
                                            float seekSeconds,int destWidth,int destHeight,
                                            int destBitrate,int destFPS,boolean needAudio);

    /**
     * Stop transcode at any time with out waiting for transcoding to the end of the video.
     *
     * If processor does not in transcoding,it will return a negative value.
     * If it stop successfully,the status of the processor will notify app throw a call back.
     *
     * @return JNI_TRUE(1) success.
     */
    public static native int StopTranscode();




    static {
        System.loadLibrary("x265");
        System.loadLibrary("yuv");
        System.loadLibrary("ffmpeg");
        System.loadLibrary("avtranscoder");
    }
}
