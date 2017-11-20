#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include  <android/log.h>
#include "opus.h"

#define TAG "myDemo-jni" // 这个是自定义的LOG的标识   
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型   
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型   
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型   
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型   
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型  

OpusEncoder *enc;      /*编码*/
OpusDecoder *dec;     /*解码*/

/*****************************************************************************
 * 函数名：OpusEncInit(lSamplingRate, lChannels, lApplication)
 * 功  能：创建编码器状态
 * 参  数：lSamplingRate：采样率， lChannels:通道数， lApplication: 编码模式
 * 返回值：0 为成功，-1 为失败
 * ***************************************************************************/
JNIEXPORT jint JNICALL Java_com_example_hellojni_OpusTools_OpusEncInit(JNIEnv *evn, jobject obj, jint lSamplingRate, jint lChannels, jint lBitRate, jint lCodedChannel, jint lApplication)
{   
    int lRet;
    int lSts;
    int size;
    int error;
    int complexity;   /*编码复杂度*/
    complexity = 1;   /*默认最低复杂度*/

    lRet = 0;
    lSts = 0;
    size = 0;
    error = 0;

    size = opus_encoder_get_size(lChannels);
    enc = malloc(size);
    error = opus_encoder_init(enc, lSamplingRate, lChannels, lApplication);
    /*设置编码波特率*/
    opus_encoder_ctl(enc, OPUS_SET_BITRATE(lBitRate));
    /*设置带宽*/
    opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(OPUS_AUTO));
    /*设置编码器的复杂度*/
    opus_encoder_ctl(enc, OPUS_SET_VBR(complexity));
    /*禁用可变比特率*/
    opus_encoder_ctl(enc, OPUS_SET_VBR_CONSTRAINT(1));
    opus_encoder_ctl(enc, OPUS_SET_VBR(0)); 
    /*编码声道强制*/
    opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(lCodedChannel));
    /*使用40ms帧*/
    opus_encoder_ctl(enc, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_40_MS));
    return lRet;
}

/****************************************************************
 *
 * 函数名：OpusEncoderDestroy()
 * 功  能：编码器释放内存
 * 参  数：无
 * 返回值：无
 * **************************************************************/
JNIEXPORT void JNICALL Java_com_example_hellojni_OpusTools_OpusEncoderDestroy(JNIEnv *env, jobject obj)
{
    if (NULL !=enc)
    {
        free(enc);
    }
}

/*****************************************************************************
 * 函数名：OpusEncoder(jshortArray szpcmData, jint lFrame_size, jint  max_data_bytes)
 * 功  能：编码
 * 参  数：szpcmData:输入信号，就是原始的PCM流，为short类型数组，
 * lFrame_size: 每个通道数的采样数，用来计算采样时间（个人理解），
 * ByData:保存编码后的数据
 * max_data_bytes:输出有效载荷的分配内存大小 建议大小：4000
 * 返回值：编码后的数组
 * ***************************************************************************/
JNIEXPORT jbyteArray JNICALL Java_com_example_hellojni_OpusTools_OpusEncode(JNIEnv *env, jobject obj, jshortArray szPcmData, jint lFrame_size, jint max_data_bytes)
{
    int lRet;
    int lSts;
    int i;

    lRet = 0;
    lSts = 0;
    i = 0;
    unsigned char buf[7680];
    memset(buf, 0, sizeof(buf));
    
    /*音频流数组转化为本地数组*/
    jshort *PcmData = (*env)->GetShortArrayElements(env, szPcmData, NULL);
    i = (*env)->GetArrayLength(env, szPcmData);
    /*编码*/
    lSts = opus_encode(enc, (short *)PcmData, lFrame_size, buf, max_data_bytes);
    /*创建一个数组*/
    jbyteArray byPcmData = (*env)->NewByteArray(env, lSts);
    
    (*env)->SetByteArrayRegion(env, byPcmData, 0, lSts, (jbyte *)buf);
    /*释放数组*/
    (*env)->ReleaseShortArrayElements(env, szPcmData, PcmData, 0);
    return byPcmData;
}

/************************************************************************************************************
 *函数名：PcmLeftOrRight(JNIEnv *env, jobject obj, jbyteArray szPcmData)
 *功  能：PCM数据保留左声道或者右声道，另外一个声道填0,整体数据大小不变,只适用16bit量化位数
 *参  数：szPcmData:输入信号，就是原始的PCM流，为byte类型数组, lChannels:0表示只保留左声道，1表示只保留右声道
 *返回值：修改后的数组
 * **********************************************************************************************************/
JNIEXPORT jbyteArray JNICALL Java_com_example_hellojni_OpusTools_PcmLeftOrRight(JNIEnv *env, jobject obj, jbyteArray szPcmData, jint lChannels)
{
   int lRet;
   int lSts;
   int i;
   unsigned char LeftBuffer[7680];         //保存左声道
   unsigned char RightBuffer[7680];        //保存右声道
   unsigned char StereoBuffer[7680];       //返回值
   unsigned char buf[2];                  //空数组

   lRet = 0;
   lSts = 0;
   i = 0;
   memset(LeftBuffer, 0, sizeof(LeftBuffer));
   memset(RightBuffer, 0, sizeof(RightBuffer));
   memset(StereoBuffer, 0, sizeof(StereoBuffer));
   memset(buf, 0, sizeof(buf));

   /*转化为本地数组*/
   jbyte *byt = (*env)->GetByteArrayElements(env, szPcmData, NULL);

   while(i < sizeof(StereoBuffer))
   {
       /*保留左声道，清空右声道*/
       memcpy(LeftBuffer + i, byt + i, 2);   //左声道保留到左声道数组中
       memcpy(LeftBuffer + i + 2, buf, sizeof(buf) );//右声道清空
    
       /*保留右声道，清空左声道*/
       memcpy(RightBuffer + i, buf, sizeof(buf)); //左声道清空
       memcpy(RightBuffer + i + 2, byt + i + 2, 2);//保留右声道
       i = i + 4;
   }
   if (0 == lChannels)       /*左声道*/
   {
        memcpy(StereoBuffer, LeftBuffer, sizeof(LeftBuffer));
   }
   else if (1 == lChannels) /*右声道*/
   {
        memcpy(StereoBuffer, RightBuffer, sizeof(RightBuffer));
   }
   else                     /*不处理*/
   {
       memcpy(StereoBuffer, byt, sizeof(StereoBuffer));
   }
   /*创建一个数组*/
   jbyteArray pcmByte = (*env)->NewByteArray(env, sizeof(StereoBuffer));
   /*设置内容*/
   (*env)->SetByteArrayRegion(env, pcmByte, 0, sizeof(StereoBuffer), (jbyte *)StereoBuffer);
   /*释放本地数组*/
   (*env)->ReleaseByteArrayElements(env, szPcmData, byt, 0);
    
   return  pcmByte;
}

/*****************************************************************************
 * 函数名：OpusDecInit(lSamplingRate, lChannels, lApplication)
 * 功  能：创建解码器状态
 * 参  数：lSamplingRate：采样率， lChannels:通道数
 * 返回值：0 为成功，-1 为失败
 * ***************************************************************************/
JNIEXPORT jint JNICALL  Java_com_example_hellojni_OpusTools_OpusDecInit(JNIEnv *env, jobject obj, jint lSamplingRate, jint lChannels)
{
    int lRet;
    int error;
    int size;
    
    lRet = 0;
    error = 0;
    size = 0;

    size = opus_decoder_get_size(lChannels);
    dec = malloc(size);
    if (NULL == dec)
    {
        lRet = -1;
        return lRet;
    }
    error = opus_decoder_init(dec,lSamplingRate,lChannels);
    if (OPUS_OK != error)
    {
        return error;
    }
    return lRet;
}

/****************************************************************
 *
 * 函数名：OpusDecoderDestroy()
 * 功  能：解码器释放内存
 * 参  数：无
 * 返回值：无
 * **************************************************************/
JNIEXPORT void JNICALL Java_com_example_hellojni_OpusTools_OpusDecoderDestroy(JNIEnv *env, jobject obj)
{
    if (NULL != dec)
    {
        free(dec);
    }
}

/*****************************************************************************
 * 函数名：OpusDecode(JNIEnv *env, jobject obj, jbyteArray szByData, jint llenth, jint lFrame_size, jint lDecode_fec)
 * 功  能：解码                                                                                                  
 * 参  数：szByData :待解码的数据， llenth:待解码的数据的长度，szpcmData:接收解码后数据的缓冲区，lFrame_size:采样数，lDecode_fec:标志（0或1），
 * 要求对带内前向纠错数据进行解码.
 * 返回值：解码后的数据
 * ***************************************************************************/
JNIEXPORT jshortArray JNICALL Java_com_example_hellojni_OpusTools_OpusDecode(JNIEnv *env, jobject obj, jbyteArray szByData, jint llenth, jint lChannels, jint lFrame_size, jint lDecode_fec)
{
    int lRet;
    int lSts;
    short buf[7680];
    int i;

    lRet = 0;
    lSts = 0;
    memset(buf, 0, sizeof(buf));
    i = 0;

    /*将待解码数据转化为本地数组*/
    jbyte *byt = (*env)->GetByteArrayElements(env, szByData, NULL);
    i = (*env)->GetArrayLength(env, szByData);
    /*解码*/
    lSts = opus_decode(dec, (unsigned char *)byt, llenth, buf, lFrame_size, lDecode_fec);
    /*创建一个数组*/
    jshortArray shPcmData = (*env)->NewShortArray(env, lSts * lChannels);
    /*设置内容*/
    (*env)->SetShortArrayRegion(env, shPcmData, 0, lSts * lChannels, (jshort*)buf);       
    /*释放本地数组*/
    (*env)->ReleaseByteArrayElements(env, szByData, byt, 0);
    return shPcmData;
}


/*******************************************************************************
 *函数名：GetChannels(JNIEnv *env, jobject obj, jbyteArray szByData)
 *功  能：获取音频数据的通道数
 *参  数：szByData:音频数据
 *返回值：音频数据的通道数
 * ****************************************************************************/

JNIEXPORT jint JNICALL Java_com_example_hellojni_OpusTools_GetChannels(JNIEnv *env, jobject obj, jbyteArray szByData)
{
    int lChannels;
    lChannels = 0;
    /*获得本地数组*/
    jbyte *byt = (*env)->GetByteArrayElements(env, szByData, NULL);
    /*获得通道数*/
    lChannels = opus_packet_get_nb_channels(byt); 
    /*释放本地数组*/
    (*env)->ReleaseByteArrayElements(env, szByData, byt, 0);
    return lChannels;
}

/******************************************************************************
 *函数名：GetBandWith(JNIEnv *env, jobject cls, jbyteArray szByData)
 *功  能：获取音频数据的带宽
 *参  数：szByData:音频数据
 *返回值：音频数据数据带宽
 *****************************************************************************/
JNIEXPORT jint JNICALL Java_com_example_hellojni_OpusTools_GetBandWidth(JNIEnv *env, jobject cls, jbyteArray szByData)
{
    int lBandWidth;
    lBandWidth = 0;
    /*获得本地数组*/
    jbyte *byt = (*env)->GetByteArrayElements(env, szByData, NULL);
    /*获得音频带宽*/
    lBandWidth = opus_packet_get_bandwidth(byt);
    /*释放本地数组*/
    (*env)->ReleaseByteArrayElements(env, szByData, byt, 0);
    return lBandWidth;
}

/******************************************************************************
 *函数名：GetDataModGetSamplesPerFrame(JNIEnv *env, jobject obj, jbyteArray szByData, jint lSamplingRate)
 *功  能：获取音频数据的采样数
 *参  数：szByData:音频数据，lSamplingRate:采样率
 *返回值：音频数据采样数
 *****************************************************************************/
JNIEXPORT jint JNICALL Java_com_example_hellojni_OpusTools_GetSamplesPerFrame(JNIEnv *env, jobject obj, jbyteArray szByData, jint lSamplingRate)
{
    int lSampleNumber;
    lSampleNumber = 0;
    /*获得本地数组*/
    jbyte *byt = (*env)->GetByteArrayElements(env, szByData, NULL);
    lSampleNumber = opus_packet_get_samples_per_frame(byt, lSamplingRate);
    /*释放本地数组*/
    (*env)->ReleaseByteArrayElements(env, szByData, byt, 0);
    return lSampleNumber;
}


