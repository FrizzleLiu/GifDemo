package com.jni.gifdemo;

import android.graphics.Bitmap;

/**
 * author : LiuWeiJun
 * e-mail : 463866506@qq.com
 * date   : 2020/5/13 19:19
 * desc   : GifHandler
 */
public class GifHandler {

    static {
        System.loadLibrary("native-lib");
    }

    //native 结构体地址,方便传参
    public long gifAddress;

    //加载gif的时候拿到结构体地址
    public GifHandler(String path) {
        this.gifAddress = loadPath(path);
    }

    public int getWidth(){
        return getWidth(gifAddress);
    }

    public int getHeight(){
        return getHeight(gifAddress);
    }

    public int updateFream(Bitmap bitmap){
       return updateFrame(gifAddress,bitmap);
    }

    //通过路径加载gif图片(这里使用的是本地图片,源码中的gif加载是支持流的格式的)
    public native long loadPath(String path);
    //获取gif的宽,long类型的ndkGif表示的是native 结构体的地址
    public native int getWidth(long ndkGif);
    //获取gif的高,long类型的ndkGif表示的是native 结构体的地址
    public native int getHeight(long ndkGif);
    //每隔一段时间刷新一次,返回的int值表示下次刷新的时间间隔,long类型的ndkGif表示的是native 结构体的地址
    public native int updateFrame(long ndkGif, Bitmap bitmap);
}
