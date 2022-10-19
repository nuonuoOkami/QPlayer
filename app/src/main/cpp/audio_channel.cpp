//
// Created by 于海 on 2022/10/19.
//

#include "audio_channel.h"
#include "audio_channel.h"

/**
 * 构造参数初始化
 * @param type_index  音频/视频
 * @param codecContext  解码器
 * @param time_base  时间基
 */
AudioChannel::AudioChannel(int type_index, AVCodecContext *codecContext, AVRational time_base) :
        BaseChannel(type_index, codecContext, time_base) {
    //音频三要素  采样率  通道 和采样格式
    //声道数量2 其实写2没啥问题 问就是专业 根据通道的layout返回通道的个数
    //https://blog.csdn.net/huweijian5/article/details/105605197
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    //采样率44100
    out_sample_rate = 44100;

    //通道*采样率*采样格式
    out_buffer_size = out_channels * out_sample_rate * out_sample_rate;

    //堆开辟记得释放
    out_buffers = static_cast<uint8_t *> (malloc(out_buffer_size));
    //https://blog.csdn.net/Explorer_day/article/details/76332556
    //如果第一个参数指向为NULL,则创建一个新的SwrContext，否则对其进行参数配置。
    swrContext = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16P, out_sample_rate,
                                    avCodecContext->channel_layout,
                                    avCodecContext->sample_fmt, avCodecContext->sample_fmt, 0,
                                    NULL);


}

AudioChannel::~AudioChannel() noexcept {
    if (swrContext) {//释放
        swr_free(&swrContext);
    }
    //删除内存开辟的空间 设置为null指针
    delete out_buffers;
    out_buffers = nullptr;
}

/**
 * 开始
 */
void AudioChannel::start() {

}

/**
 * 停止
 */
void AudioChannel::stop() {

}

/**
 * 解码
 */
void AudioChannel::audio_decode() {


}

/**
 *
 * @return
 */
int AudioChannel::getPcm() {

}

