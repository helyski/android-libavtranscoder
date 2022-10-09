package com.tangjn.libtranscode;

/**
 * @author HelyskiTank
 * @description
 */
public class JNILibTranscode {

    static {
        System.loadLibrary("x265");
        System.loadLibrary("ffmpeg");
        System.loadLibrary("avtranscoder");
    }
    public static native boolean StartEngine();

    public static native void OpenFFmpegLog();

    public static native int StartHWDecode();
}
