//
// Created by yuhai on 2022/10/19.
//

#ifndef QPLAYER_AUDIO_CHANNEL_H
#define QPLAYER_AUDIO_CHANNEL_H

#include "base_channel.h"
//导入 OpenSLES 相关 这块我也不懂
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C"
{
#include "libswresample/swresample.h"
};

class AudioChannel : public BaseChannel {

private:
    //线程解码
    pthread_t p_thread_decode;
    //线程播放
    pthread_t p_thread_play;

public:
    //输出的频道 2
    int out_channels = 0;
    // 2字节 16
    int out_sample_size = 0;

    //采样率44100*2*2
    int out_sample_rate = 0;
    //buffer大小
    int out_buffer_size = 0;
    // 重采样需要 uint8_t类型
    uint8_t *out_buffers = 0;
    // 重采样 结构体   https://blog.csdn.net/Jammg/article/details/52688506
    // https://zhuanlan.zhihu.com/p/545279669
    SwrContext *swrContext = 0;

    //音频时长
    double audio_time;

    //jniHelper
    JniHelper *jniHelper = 0;



    //OpenSLES 相关
public:
    //引擎 SLObjectItf 是很重要的接口,基本上想要的东西都通过id可以拿到
    //这玩意居然是个指针
    SLObjectItf engineObject = 0;
    //引擎接口
    SLEngineItf engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf bqPlayerPlay = 0;
    //播放器队列
    SLAndroidSimpleBufferQueueItf playBufferQueue = 0;

public:
    AudioChannel(int type_index, AVCodecContext *codecContext, AVRational time_base);

    ~AudioChannel();

    void start();

    void stop();

    void audio_decode();

    void audio_play();

    int getPcm();

    void setJniHelper(JniHelper *jniHelper);

};


#endif //QPLAYER_AUDIO_CHANNEL_H
