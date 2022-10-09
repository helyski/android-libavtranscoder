package com.tangjn.libtranscode;

/**
 * @author HelyskiTank
 * @description
 */
public class JNILibTranscode {

    public interface TRANSCODE_OUTPUT_TYPE{
        int OUTPUT_VIDEO_FILE = 0;
        int OUTPUT_H264_STREAM_SHARE_TO_JAVA = 1;
        int OUTPUT_H264_STREAM_PUSH_TO_RTMP_SERVER = 2;
    }

    static {
        System.loadLibrary("x265");
        System.loadLibrary("ffmpeg");
        System.loadLibrary("avtranscoder");
    }
    public static native boolean Init();

    public static native void OpenFFmpegLog();


    /**
     * 
     * @param enable if true,decoder will try to use h264_mediacodec.
     *               if false,decoder use ffmpeg sw_decode.
     * @return >=0 success.
     */
    public static native int EnableMediaCodec(boolean enable);

    /**
     * Set Transcode Output type.
     * Default will transcode to Video File.
     * 
     * @param outPutType see this {@link TRANSCODE_OUTPUT_TYPE}
     * @return >=0 success.
     */
    public static native int SetTranscodeOutputType(int outPutType);

    /**
     * Set Folder Direction for output file,Not a file full name.
     * Default in the same dir as the src file.
     *
     * Only need to set this when {@link this#SetTranscodeOutputType(int)}
     * with {@link TRANSCODE_OUTPUT_TYPE#OUTPUT_VIDEO_FILE}
     *
     * @param dir Folder path for Output Video File. Can not NULL.
     * @return >=0 success.
     */
    public static native int SetOutputFileDirection(String dir);

    public static native int StartHWDecode();

    public static native int StartTranscode(String srcVideoPath,String destVideoName,float seekSeconds,int destWidth,int destHeight,int destBitrate,int destFPS,boolean needAudio);

    public static native int StopTranscode();
}
