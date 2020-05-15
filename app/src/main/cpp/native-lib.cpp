#include <jni.h>
#include <string>
#include <malloc.h>
#include <cstring>
#include "gif_lib.h"
#include <android/log.h>
#include <android/bitmap.h>
#define  LOG_TAG    "wangyi"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)

typedef struct GifBean{
    //当前帧
    int currnt_frame;
    //总帧数
    int total_frame;
    //延迟时间数组,长度不确定,根据gif帧数计算
    int *delays;
}GifBean;


//将gif的帧绘制到bitmap
void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo bitmapInfo, void *pixels) {
    //获取当前帧
    SavedImage savedImage=gifFileType->SavedImages[gifBean->currnt_frame];
    //初始化指向图片首地址
    int *px=(int *)pixels;
    int pointPixels;
    //Gif编码中有边界
    //遍历的时候从边界开始,不是从0开始
    GifImageDesc imageDesc=savedImage.ImageDesc;

    //字典,存放的是gif压缩rgb数据
    ColorMapObject *colorMap=imageDesc.ColorMap;
    //部分图片某些帧的ColorMapObject取到为null
    if(colorMap==NULL)
    {
        colorMap=gifFileType->SColorMap;
    }
    GifByteType gifByteType;//压缩数据
    //指向第一行首地址
    px=(int *)((char *)px +bitmapInfo.stride * imageDesc.Top);
    //每一行的首地址
    int *line;
    for (int y = imageDesc.Top; y <imageDesc.Top+imageDesc.Height ; ++y) {
        line=px;
        for (int x = imageDesc.Left; x <imageDesc.Left+imageDesc.Width ; ++x) {
            //拿到位置坐标的索引
            pointPixels=(y-imageDesc.Top)*imageDesc.Width+(x-imageDesc.Left);
            //gif中为了节省内存rgb采用lzw压缩,所以取rgb信息需要解压
            gifByteType = savedImage.RasterBits[pointPixels];
            //拿到解压后的rgb数据
            GifColorType gifColorType=colorMap->Colors[gifByteType];
            line[x] = argb(255,gifColorType.Red,gifColorType.Green,gifColorType.Blue);
        }
        //指向下一行首地址
        px=(int *)((char *)px +bitmapInfo.stride);
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_jni_gifdemo_GifHandler_loadPath(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    int err;
    GifFileType *gifFileType= DGifOpenFileName(path,&err);
     DGifSlurp(gifFileType);
    //malloc用于开辟内存空间,可以理解为Java中的new一个对象,但是需要先清空内存
    GifBean *gifBean=(GifBean *)malloc(sizeof(GifBean));
    //先清空内存
    memset(gifBean,0, sizeof(GifBean));
    //绑定内存地址,这里类似于Java中的View.setTag(new Object())
    //UserData; 的类型是void * 无符号类型,相当于Java中的Object
    gifFileType->UserData=gifBean;

    //通过gif帧数计算延迟时间数组的长度,清除空内存
    gifBean->delays=(int *) malloc(sizeof(int)* gifFileType->ImageCount);
    //绑定内存地址
    memset(gifBean->delays,0, sizeof(sizeof(int)* gifFileType->ImageCount));
    gifFileType->UserData=gifBean;
    //初始化当前帧和总帧数
    gifBean->currnt_frame=0;
    gifBean->total_frame=gifFileType->ImageCount;

    ExtensionBlock *extensionBlock;
    //遍历每一帧
    for (int i = 0; i <gifFileType->ImageCount ; ++i) {
        //遍历每一帧中的扩展块(度娘Gif编码)
        SavedImage frame= gifFileType->SavedImages[i];
        for (int j = 0; j <frame.ExtensionBlockCount ; ++j) {
            //取图形控制扩展块,其中包含延迟时间
            if (frame.ExtensionBlocks[j].Function==GRAPHICS_EXT_FUNC_CODE){
                extensionBlock=&frame.ExtensionBlocks[j];
                break;
            }
        }

        //获取延迟时间,extensionBlock的第二,三个元素一起存放延迟时间低8位和高8位向左偏移8位,进行或运算
        //乘10因为编码的时间单位是1/100秒 乘10换算为毫秒
        if (extensionBlock){
            int frame_delay=10*(extensionBlock->Bytes[1]|(extensionBlock->Bytes[2]<<8));
            gifBean->delays[i]=frame_delay;
             LOGE("时间  %d   ",frame_delay);
        }
    }
    env->ReleaseStringUTFChars(path_,path);
    return (jlong)gifFileType;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jni_gifdemo_GifHandler_getWidth(JNIEnv *env, jobject instance, jlong ndkGif) {
    GifFileType* gifFileType= (GifFileType*)ndkGif;
    return gifFileType->SWidth;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jni_gifdemo_GifHandler_getHeight(JNIEnv *env, jobject instance, jlong ndkGif) {

    GifFileType* gifFileType= (GifFileType*)ndkGif;
    return gifFileType->SHeight;

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jni_gifdemo_GifHandler_updateFrame(JNIEnv *env, jobject instance, jlong ndkGif,
                                             jobject bitmap) {
    GifFileType* gifFileType= (GifFileType*)ndkGif;
    GifBean *gifBean= ( GifBean * )gifFileType->UserData;

    AndroidBitmapInfo bitmapInfo;
    AndroidBitmap_getInfo(env,bitmap,&bitmapInfo);

    //对bitmap加锁,然后取缓冲区数据
    void *pixels;
    AndroidBitmap_lockPixels(env,bitmap,&pixels);

    drawFrame(gifFileType,gifBean,bitmapInfo,pixels);
    gifBean->currnt_frame+=1;
    if (gifBean->currnt_frame >= gifBean->total_frame-1){
        gifBean->currnt_frame=0;
          LOGE("重复播放  %d  ",gifBean->currnt_frame);
    }
    AndroidBitmap_unlockPixels(env,bitmap);
    //返回bitmap给java层
    return gifBean->delays[gifBean->currnt_frame];
}