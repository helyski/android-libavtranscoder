package com.tank3.examples.libtranscode;
import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

import com.tank3.libtranscode.JNILibTranscode;

import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.util.Locale;
import java.util.concurrent.ArrayBlockingQueue;

public class MainActivity extends AppCompatActivity  implements View.OnClickListener,JNILibTranscode.NativeListener{
    private static final String TAG = "MainActivity";

    /**
     * 设置转码文件的输出目录。
     */
    private static final String OutputDir = "/storage/Tfcard/TranscodeStore";

    private static final String SrcFilePath = "/sdcard/VideosForLibtranscode/00_20221024112518.ts";

    private MainHandler mMainHandler = new MainHandler(this);

    ExeThread t;


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
        findViewById(R.id.btnStopTranscode).setOnClickListener(this);

        JNILibTranscode.Init();
        JNILibTranscode.SetTranscodeOutputType(JNILibTranscode.TRANSCODE_OUTPUT_TYPE.OUTPUT_H264_STREAM_SHARE_TO_JAVA);
//        JNILibTranscode.OpenFFmpegLog();
//        VideoFactory.testFilter();

//        Thread tp = new Thread(hwdecode);
//        tp.start();

        t = new ExeThread(mMainHandler);
        t.start();

        JNILibTranscode.setNativeListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.e(TAG,"onResume");
        PermissionManager.checkGrantPermission(this,getApplicationContext(),true);

    }

    @Override
    protected void onStop() {
        super.onStop();
        JNILibTranscode.StopTranscode();
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View view) {
        switch (view.getId()){

            case R.id.btnStartTranscode:
                t.getHandler().onStart();
                break;

            case R.id.btnStopTranscode:
                t.getHandler().onStop();
                break;
        }
    }




    private static class MainHandler extends Handler {
        public static final int ON_START = 0x33;
        public static final int ON_STOP = 0x34;

        private final WeakReference<MainActivity> target;

        private MainHandler(MainActivity controller) {
            target = new WeakReference<>(controller);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            final MainActivity obj = target.get();
            switch (msg.what) {
                case ON_START:
//                    obj.startTranscode();
                    break;
                case ON_STOP:
//                    obj.stopTranscode();
                    break;
                default:

                    break;
            }
        }
    }


    private class ExeThread extends Thread{
        private boolean exit = false;
        private int count  = 0;
        private volatile  ExeHandler thisHandler;
        private MainHandler mainHandler;
        private ArrayBlockingQueue<CMD> cmdQ = new ArrayBlockingQueue<>(11);
        public ExeThread(MainHandler handler){
            this.mainHandler = handler;
            thisHandler = new ExeHandler(this);
        }
        public void finish(){
            exit = true;
        }
        public ExeHandler getHandler(){
            return thisHandler;
        }
        private void addStartCMD(){
            cmdQ.offer(new CMD(1));
        }
        private void addStopCMD(){
            cmdQ.offer(new CMD(2));
        }

        private void startTranscode(){
            Log.e(TAG,"hello");
            Log.e(TAG,"AAAAAAAAAAAAAAAAAAAAAAAAAA");
//                VideoFactory.testFilter();
            String newFileName = String.format(Locale.CHINA,"transcode%d_%s",System.currentTimeMillis(),
                    SrcFilePath.substring(SrcFilePath.lastIndexOf("/")+1));
            Log.e(TAG,newFileName);
            JNILibTranscode.StartTranscode(SrcFilePath,newFileName,0,640,480,2000,15,true);
        }

        private void stopTranscode(){
            Log.e(TAG,"FFFFFFFFFFFFFFFFFFFFFFFFFF");
            JNILibTranscode.StopTranscode();
        }
        @Override
        public void run() {
            while(!exit){
                try {
                    CMD cmd = cmdQ.take();
                    switch (cmd.cmd_type){
                        case 1:
                            startTranscode();
                            break;

                        case 2:
                            stopTranscode();
                            break;

                        default:

                            break;
                    }
                }catch (Exception e){
                    Log.e(TAG,"ExeThread Run catch exception:"+Log.getStackTraceString(e));
                }

            }
        }


    };

    private static class ExeHandler extends Handler {
        public static final int ON_START = 0x33;
        public static final int ON_STOP = 0x34;
        private final WeakReference<ExeThread> target;
        private ExeHandler(ExeThread controller) {
            target = new WeakReference<>(controller);
        }
        public void onStart(){
            sendEmptyMessage(ON_START);
        }
        public void onStop(){
            sendEmptyMessage(ON_STOP);
        }
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            final ExeThread obj = target.get();
            switch (msg.what) {
                case ON_START:
                    obj.addStartCMD();
                    break;
                case ON_STOP:
                    obj.addStopCMD();
                    break;
                default:

                    break;
            }
        }
    }

    public class CMD{
        int cmd_type;

        public CMD(int cmd_type) {
            this.cmd_type = cmd_type;
        }

        public int getCmd_type() {
            return cmd_type;
        }

        public void setCmd_type(int cmd_type) {
            this.cmd_type = cmd_type;
        }
    }


    @Override
    public void onH264CallBack(int dataIndex, ByteBuffer data, int len, long pts, int isKeyFrame, int nal_type) {
        Log.d(TAG,"onH264CallBack dataIndex"+dataIndex+", len:"+len);
    }
}