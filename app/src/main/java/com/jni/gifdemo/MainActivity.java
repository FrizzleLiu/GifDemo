package com.jni.gifdemo;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private Bitmap bitmap;
    private ImageView imageView;
    private Button btn;
    private GifHandler gifHandler;

    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            int interval = gifHandler.updateFream(bitmap);
            handler.sendEmptyMessageDelayed(1,interval);
            imageView.setImageBitmap(bitmap);
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = findViewById(R.id.image_view);
        btn = findViewById(R.id.btn);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                requestPerms();
            }
        });
    }

    private void requestPerms() {
        //权限,简单处理下
        if (Build.VERSION.SDK_INT>Build.VERSION_CODES.N) {
            String[] perms= {Manifest.permission.WRITE_EXTERNAL_STORAGE};
            if (checkSelfPermission(perms[0]) == PackageManager.PERMISSION_DENIED) {
                requestPermissions(perms,200);
            }else {
                ndkLoadGif();
            }
        }
    }

    public void ndkLoadGif() {
        File file = new File(Environment.getExternalStorageDirectory(), "demo.gif");
        gifHandler = new GifHandler(file.getAbsolutePath());
        int width = gifHandler.getWidth();
        int height = gifHandler.getHeight();
        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        //下一帧的刷新时间
        int interval = gifHandler.updateFream(bitmap);
        handler.sendEmptyMessageDelayed(1, interval);
    }

}
