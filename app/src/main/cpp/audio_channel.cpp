//
// Created by 于海 on 2022/10/19.
//

#include "audio_channel.h"
#include "audio_channel.h"
#include "log4c.h"

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
    out_buffer_size = out_channels * out_sample_rate * out_sample_size;

    LOGE("out_sample_rate%  d",out_sample_rate)
    LOGE("out_buffer_size%  d",out_buffer_size)
    LOGE("out_channels%  d",out_channels)
    LOGE("out_sample_size%  d",out_sample_size)

    //堆开辟记得释放
    out_buffers = static_cast<uint8_t *> (malloc(out_buffer_size));
    //https://blog.csdn.net/Explorer_day/article/details/76332556
    //如果第一个参数指向为NULL,则创建一个新的SwrContext，否则对其进行参数配置。
    swrContext = swr_alloc_set_opts(0,
                                    AV_CH_LAYOUT_STEREO,
                                    AV_SAMPLE_FMT_S16P,
                                    out_sample_rate,
                                    codecContext->channel_layout,
                                    codecContext->sample_fmt, codecContext->sample_rate, 0,
                                    0);


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


void playBufferQueueCallBack(SLAndroidSimpleBufferQueueItf sl, void *args) {
    auto *pChannel = static_cast<AudioChannel *>(args);
    int pcm_size = pChannel->getPcm();
    (*sl)->Enqueue(sl, pChannel->out_buffers, pcm_size);

}

/**
 * 播放
 */
void AudioChannel::audio_play() {

    //接收结果
    SLresult result;

    //创建引擎  https://www.jianshu.com/p/82da5f87314f
    //上下文，选择项数量，具体的选择项，支持的接口的数量，具体的要支持的接口，是枚举的数组
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    if (result != SL_RESULT_SUCCESS) {
        //异常了
        LOGE("创建引擎 slCreateEngine error");
        return;
    }
    //引擎初始化 延迟初始化
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        //异常了
        LOGE("创建引擎 Realize error");
        return;
    }
    //取接口
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("创建引擎接口 Realize error");
        return;
    }
    if (engineInterface) {
        //接口成功
        LOGD("创建引擎接口 create success");
    } else {
        //接口失败
        LOGD("创建引擎接口 create error");
        return;
    }
    //todo 混音器其实不用可以不创建 当学习了
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, 0,
                                                 0);
    if (result != SL_RESULT_SUCCESS) {
        //异常了
        LOGD("创建混音器 CreateOutputMix failed");
        return;
    }
    //混音器初始化
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        //异常了
        LOGD("初始化混音器 (*outputMixObject)->Realize failed");
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
            SL_DATAFORMAT_PCM, // PCM数据格式
            2, // 声道数
            SL_SAMPLINGRATE_44_1, // 采样率（每秒44100个点）
            SL_PCMSAMPLEFORMAT_FIXED_16, // 每秒采样样本 存放大小 16bit
            SL_PCMSAMPLEFORMAT_FIXED_16, // 每个样本位数 16bit
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 前左声道  前右声道
            SL_BYTEORDER_LITTLEENDIAN

    };
    //配置数据源 告诉 就这个样式的  前面是对象 后面是格式  音频格式
    SLDataSource audioSrc = {&simpleBufferQueue, &slDataFormatPcm};

    //混音器

    //音轨
    SLDataLocator_OutputMix slDataLocatorOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
   // SLDataSink audioSnk = {&slDataLocatorOutputMix, NULL};
    SLDataSink audioSnk = {&slDataLocatorOutputMix, NULL};

    // 接口配置
    const  SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const  SLboolean req[1] = {SL_BOOLEAN_TRUE};

    //创建播放器 data source 代表着输入源的信息，即数据从哪儿来、输入的数据参数是怎样的；
    // 而 data sink 则代表着输出的信息，即数据输出到哪儿、以什么样的参数来输出。
    result = (*engineInterface)->CreateAudioPlayer(engineInterface//参数1：引擎接口
            , &bqPlayerObject,//参数2：播放器
                                                   &audioSrc,//参数3：音频配置信息
                                                   &audioSnk,// 参数4：混音器
                                                   1, //参数5：开放的参数的个数
                                                   ids,// 参数6：代表我们需要 Buff
                                                   req);// 参数7：代表我们上面的Buff 需要开放出去
    if (result != SL_RESULT_SUCCESS) {
        //异常了
        LOGD("创建播放器 CreateAudioPlayer failed!");
        return;
    }
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);//等待成功
    if (SL_RESULT_SUCCESS != result) {
        //异常
        LOGD("实例化播放器 CreateAudioPlayer failed!");
        return;
    }

    // SL_IID_PLAY:播放接口 == iplayer
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (SL_RESULT_SUCCESS != result) {
        //异常
        LOGD("获取播放接口 GetInterface SL_IID_PLAY failed!");
        return;
    }
    //获取播放队列
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &playBufferQueue);

    if (SL_RESULT_SUCCESS != result) {
        //异常
        LOGD("获取播放队列 GetInterface SL_IID_BUFFERQUEUE failed!");
        return;
    }
    //回调接口
    result = (*playBufferQueue)->RegisterCallback(playBufferQueue, playBufferQueueCallBack, this);
    if (SL_RESULT_SUCCESS != result) {
        //异常
        return;
    }
    //播放器接口设置播放
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    //回调自己
    playBufferQueueCallBack(playBufferQueue, this);
}


/**
 * 开始
 */
void AudioChannel::start() {
    //开始工作 设置工作状态为true
    is_play = true;
    aVPackets.setPlayState(true);
    aVFrames.setPlayState(true);
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
        if (is_play && aVFrames.size() > 100) {
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
        } else if (result != 0)//又异常了
        {
            //释放 AVFrame  av_frame_unref把自身对buf的引用释放掉，数据的引用计数-1。
            //av_frame_free内部还是调用了unref,只是把传入的frame也置空。
            if (pFrame) {
                av_frame_unref(pFrame);
                av_frame_free(&pFrame);
                pFrame = nullptr;
            }

            break;
        }
        //插入队列
        aVFrames.insert(pFrame);

        //各种释放
        av_packet_unref(avPacket);
        av_packet_free(&avPacket);
        avPacket = nullptr;
    }
    av_packet_unref(avPacket);
    av_packet_free(&avPacket);
    avPacket = nullptr;

}

/**
 *
 * @return
 */
int AudioChannel::getPcm() {

    int pcm_size = 0;
    AVFrame *avFrame = nullptr;

    while (is_play) {
        bool isSuccess = aVFrames.take(avFrame);
        if (!is_play) {
            break;
        }
        if (!isSuccess) {
            continue;
        }

        //swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples ?
        //获取转换的采样率 进来可能是6000，出去就是44100这种 其实48000更好

        //单通道样本数
        int dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swrContext, avFrame->sample_rate) + avFrame->nb_samples,
                // 获取下一个输入样本相对于下一个输出样本将经历的延迟
                out_sample_rate, //输出采样 44100
                avFrame->sample_rate,//输入采样
                AV_ROUND_UP//向上取整
        );



        //通道输出样本数   采样数据转换
        //参数1：音频重采样的上下文
        // 参数2：输出的指针。传递的输出的数组
        // 参数3：输出的样本数量，不是字节数。单通道的样本数量。
        // 参数4：输入的数组，AVFrame解码出来的DATA
        // 参数5：输入的单通道的样本数量。
        int samples_per_channel = swr_convert(swrContext, &out_buffers, dst_nb_samples,
                                              (const uint8_t **) avFrame->data,
                                              avFrame->nb_samples);
        pcm_size = samples_per_channel * out_channels * out_sample_size; //样本数 *声道*样本字节数


        //todo 进度

        break;
    }
    av_frame_unref(avFrame);
    av_frame_free(&avFrame);
    avFrame = nullptr;
    return pcm_size;

}

