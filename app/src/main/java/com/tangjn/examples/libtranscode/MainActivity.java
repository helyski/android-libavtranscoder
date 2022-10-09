package com.tangjn.examples.libtranscode;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.tangjn.libtranscode.JNILibTranscode;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    Button btnTest;

    Runnable hwdecode = new Runnable() {
        @Override
        public void run() {
            JNILibTranscode.StartHWDecode();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        init();
        JNILibTranscode.StartEngine();
//        JNILibTranscode.OpenFFmpegLog();
//        VideoFactory.testFilter();

        Thread tp = new Thread(hwdecode);
        tp.start();
    }

    @Override
    protected void onResume() {
        super.onResume();
        PermissionManager.checkGrantPermission(this,getApplicationContext(),true);

    }


    void init(){
        btnTest = findViewById(R.id.btn_testFilter);
        btnTest.setOnClickListener(this);
    }


    @Override
    public void onClick(View view) {
        switch (view.getId()){
            case R.id.btn_testFilter:
//                VideoFactory.testFilter();
                break;
        }
    }


}