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


    //初始化
    swr_init(swrContext);

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
 * 子线程解包
 * @param args
 * @return
 */
void *to_decode(void *args) {
    auto *pChannel = static_cast<AudioChannel *>(args);
    pChannel->audio_decode();
    return nullptr;
}


/**
 *  子线程播放
 * @param args
 * @return
 */
void *to_play(void *args) {
    auto *pChannel = static_cast<AudioChannel *>(args);
    pChannel->audio_play();
    return nullptr;
}

/**
 * 播放
 */
void AudioChannel::audio_play() {

    //接收结果
    SLresult result;

    //创建引擎  https://www.jianshu.com/p/82da5f87314f
    //上下文，选择项数量，具体的选择项，支持的接口的数量，具体的要支持的接口，是枚举的数组
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
        //异常了
        return;
    }
    //引擎初始化 延迟初始化
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        //异常了
        return;
    }
    //取接口
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (result != SL_RESULT_SUCCESS) {
        //异常了
        return;
    }
    if (engineInterface) {
        //接口成功
    } else {
        //接口失败
        return;
    }
    //todo 混音器其实不用可以不创建 当学习了
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, 0, 0);
    if (result != SL_RESULT_SUCCESS) {
        //异常了
        return;
    }
    //混音器初始化
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        //异常了
        return;
    }

    //播放器创建
    //创建buffer缓存类型的队列  2的队列大小 其实写10和2一样
    SLDataLocator_AndroidSimpleBufferQueue simpleBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};

    // pcm数据格式 == PCM是不能直接播放，mp3可以直接播放(参数集)，人家不知道PCM的参数
    //  SL_DATAFORMAT_PCM：数据格式为pcm格式
    //  2：双声道
    //  SL_SAMPLINGRATE_44_1：采样率为44100
    //  SL_PCMSAMPLEFORMAT_FIXED_16：采样格式为16bit
    // SL_PCMSAMPLEFORMAT_FIXED_16：数据大小为16bit
    // SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT：左右声道（双声道）
    // SL_BYTEORDER_LITTLEENDIAN：小端模式
    SLDataFormat_PCM slDataFormatPcm = {
            SL_DATAFORMAT_PCM,//格式 pcm
            2,//声道2
            SL_SAMPLINGRATE_44_1,//44100
            SL_PCMSAMPLEFORMAT_FIXED_16, // 每秒采样样本 存放大小 16bit 集成声卡都是16
            SL_PCMSAMPLEFORMAT_FIXED_16,// 每个样本位数 16bit
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//前左右声道
            SL_BYTEORDER_LITTLEENDIAN//小端 android 一般都是小端

    };
    //配置数据源 告诉 就这个样式的  前面是对象 后面是格式
    SLDataSource source = {&simpleBufferQueue, &slDataFormatPcm};

    //混音器

    //音轨
    SLDataLocator_OutputMix slDataLocatorOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSource sourceOutputMix = {&slDataLocatorOutputMix, NULL};

    // 接口配置
    SLInterfaceID id1[1] = {SL_IID_BUFFERQUEUE};
    SLboolean req[1] = {SL_BOOLEAN_TRUE};

    (*engineInterface)->CreateAudioPlayer(engineInterface, bqPlayerObject, source, sourceOutputMix);


}

/**
 * 开始
 */
void AudioChannel::start() {
    //开始工作 设置工作状态为true
    is_play = true;
    aVPackets.setPlayState(is_play);
    aVFrames.setPlayState(is_play);
    //开启线程 解包和播放
    pthread_create(&p_thread_decode, nullptr, to_decode, this);
    pthread_create(&p_thread_play, nullptr, to_play, this);


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

    AVPacket *avPacket = nullptr;
    while (is_play) {
        //解压出来的包超过100 就休息一下
        if (is_play && aVPackets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }

        //是否成功
        bool isSuccess = aVPackets.take(avPacket);
        //判断下是否播放
        if (!is_play) {
            break;
        }

        //获取失败就继续取新的
        if (!isSuccess) {
            continue;
        }

        //0成功 发送给avCodecContext 记得释放avPacket  提供原始数据包数据作为解码器的输入
        int result = avcodec_send_packet(avCodecContext, avPacket);
        if (result) {//true就失败了
            break;
        }

        //开辟内存 申请AVFrame
        AVFrame *pFrame = av_frame_alloc();
        //上一次frame，然后重新赋值，可以给同一个frame  0成功
        result = avcodec_receive_frame(avCodecContext, pFrame);
        if (result == AVERROR(EAGAIN)) {//遇到让你再试一次的错误就继续试一下
            continue;
        } else if (result)//又异常了
        {
            //释放 AVFrame  av_frame_unref把自身对buf的引用释放掉，数据的引用计数-1。
            //av_frame_free内部还是调用了unref,只是把传入的frame也置空。
            av_frame_unref(pFrame);
            av_frame_free(&pFrame);
            pFrame = nullptr;
            break;
        }
        //插入队列
        aVFrames.insert(pFrame);
        //各种释放
        av_packet_unref(avPacket);
        av_packet_free(&avPacket);
        avPacket = nullptr;
    }

}

/**
 *
 * @return
 */
int AudioChannel::getPcm() {

}

