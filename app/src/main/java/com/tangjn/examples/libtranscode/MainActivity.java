package com.tangjn.examples.libtranscode;
import android.annotation.SuppressLint;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

import com.tangjn.libtranscode.JNILibTranscode;

import java.util.Locale;

public class MainActivity extends AppCompatActivity  implements View.OnClickListener{
    private static final String TAG = "MainActivity";

    /**
     * 设置转码文件的输出目录。
     */
    private static final String OutputDir = "/storage/Tfcard/TranscodeStore";

    private static final String SrcFilePath = "/storage/sdcard0/DVR/front/loop/00_20220819125342.ts";


    Runnable hwdecode = new Runnable() {
        @Override
        public void run() {

            String newFileName = String.format(Locale.CHINA,"transcode%d_%s",System.currentTimeMillis(),
                    SrcFilePath.substring(SrcFilePath.lastIndexOf("/")+1));
            Log.e(TAG,newFileName);

            // Test Transcode to file.

            JNILibTranscode.SetEnableMediaCodec(true);
            JNILibTranscode.SetTranscodeOutputType(JNILibTranscode.TRANSCODE_OUTPUT_TYPE.OUTPUT_VIDEO_FILE);
            JNILibTranscode.SetOutputFileDirection(OutputDir);
            JNILibTranscode.StartTranscode(SrcFilePath,newFileName,12,640,480,2000,15,true);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        findViewById(R.id.btnStartTranscode).setOnClickListener(this);
        findViewById(R.id.btn_testFilter).setOnClickListener(this);

        JNILibTranscode.Init();
//        JNILibTranscode.OpenFFmpegLog();
//        VideoFactory.testFilter();

//        Thread tp = new Thread(hwdecode);
//        tp.start();
    }

    @Override
    protected void onResume() {
        super.onResume();
//        PermissionManager.checkGrantPermission(this,getApplicationContext(),true);

    }

    @Override
    public void onClick(View view) {
        switch (view.getId()){
            case R.id.btn_testFilter:
//                Log.e(TAG,"AAAAAAAAAAAAAAAAAAAAAAAAAA");
////                VideoFactory.testFilter();
//                String newFileName = String.format(Locale.CHINA,"transcode%d_%s",System.currentTimeMillis(),
//                        SrcFilePath.substring(SrcFilePath.lastIndexOf("/")+1));
//                Log.e(TAG,newFileName);
//                JNILibTranscode.StartTranscode(SrcFilePath,newFileName,12,640,480,2000,15,true);
                break;

            case R.id.btnStartTranscode:
                Log.e(TAG,"hello");
                Log.e(TAG,"AAAAAAAAAAAAAAAAAAAAAAAAAA");
//                VideoFactory.testFilter();
                String newFileName = String.format(Locale.CHINA,"transcode%d_%s",System.currentTimeMillis(),
                        SrcFilePath.substring(SrcFilePath.lastIndexOf("/")+1));
                Log.e(TAG,newFileName);
                JNILibTranscode.StartTranscode(SrcFilePath,newFileName,12,640,480,2000,15,true);
                break;
        }
    }


}