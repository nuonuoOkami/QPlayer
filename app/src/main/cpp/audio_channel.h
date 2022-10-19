//
// Created by 于海 on 2022/10/19.
//

#ifndef QPLAYER_AUDIO_CHANNEL_H
#define QPLAYER_AUDIO_CHANNEL_H

#include "BaseChannel.h"

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
    //堆区开辟大小
    uint8_t *out_buffers = 0;
    //转换上下文
    SwrContext *swrContext = nullptr;


};


#endif //QPLAYER_AUDIO_CHANNEL_H
