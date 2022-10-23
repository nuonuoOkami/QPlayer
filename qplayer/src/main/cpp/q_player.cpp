//
// Created by leo on 2022/10/16.
//

#include "q_player.h"
#include "log4c.h"

/**
 * 子线程执行预加载
 * @param args 
 * @return 
 */
void *thread_prepare(void *args) {
    auto *pPlayer = static_cast<QPlayer *>(args);
    pPlayer->start_prepare();
    return nullptr;//切记返回 坑死了
}

/**
 * 准备
 */
void QPlayer::prepare() {
    //将任务切换到子线程 因为是io操作
    pthread_create(&p_thread_prepare, nullptr, thread_prepare, this);
}


/**
 * 子线程执行加载任务
 */
void QPlayer::start_prepare() {
    LOGE("start_prepare")
    //初始化上下文 ，老少爷们全靠你了
    avFormatContext = avformat_alloc_context();
    //字典  可以理解为一个设置的map
    AVDictionary *avDictionary = nullptr;
    //给字典老哥上嘴脸 超时 5s 都是纳秒
    av_dict_set(&avDictionary, "timeout", "5000000", 0);

    //设置给上下文 路径 字典 所有资源都要从上下文获取 返回0 成功
    int result = avformat_open_input(&avFormatContext, source, nullptr, &avDictionary);
    //释放字典老哥
    av_dict_free(&avDictionary);
    if (result) {

        if (jniHelper) {
            jniHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL, av_err2str(result));
        }
        //释放掉上下文
        avformat_close_input(&avFormatContext);
        return;

    }
    //查看信息流 大于0 ok  其他就失败了
    result = avformat_find_stream_info(avFormatContext, nullptr);

    if (result < 0) { //异常
        //释放掉上下文
        if (jniHelper) {
            jniHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL, av_err2str(result));
        }
        avformat_close_input(&avFormatContext);
        return;
    }

    // 当前资源长度 要进行实践基转换
    this->duration = avFormatContext->duration / AV_TIME_BASE;
    //http://blog.csdn.net/leixiaohua1020/article/details/14214859
    AVCodecContext *avCodecContext = nullptr;

    //遍历流 一般 是2个  音频 视频 字幕
    for (int index = 0; index < avFormatContext->nb_streams; ++index) {
        //获取到流信息   AVStream https://blog.csdn.net/leixiaohua1020/article/details/14215821
        AVStream *avStream = avFormatContext->streams[index];
        //获取流信息的编码器参数啥的
        AVCodecParameters *parameters = avStream->codecpar;

        //获取解析器 或者叫编解码器 null失败 https://blog.csdn.net/leixiaohua1020/article/details/14215833
        AVCodec *avCodec = avcodec_find_decoder(parameters->codec_id);

        if (!avCodec) {

            if (jniHelper) {
                jniHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL, av_err2str(result));
            }
            //释放上下文
            avformat_close_input(&avFormatContext);
            return;
        }

        //干活大佬 null 失败
        avCodecContext = avcodec_alloc_context3(avCodec);

        if (!avCodecContext) {
            if (jniHelper) {
                jniHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL, av_err2str(result));
            }
            //释放avCodecContext
            avcodec_free_context(&avCodecContext);
            //释放上下文
            avformat_close_input(&avFormatContext);
            return;
        }

        //给干活大佬上参数 >= 0 on success
        result = avcodec_parameters_to_context(avCodecContext, parameters);

        //失败处理
        if (result < 0) {
            if (jniHelper) {
                jniHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL, av_err2str(result));
            }
            //释放avCodecContext
            avcodec_free_context(&avCodecContext);
            //释放上下文
            avformat_close_input(&avFormatContext);
            return;
        }

        //打开解码器 zero on success
        result = avcodec_open2(avCodecContext, avCodec, nullptr);
        if (result) {
            if (jniHelper) {
                jniHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL, av_err2str(result));
            }
            //释放avCodecContext
            avcodec_free_context(&avCodecContext);
            //释放上下文
            avformat_close_input(&avFormatContext);
            return;
        }
        //获取时间基  这玩意就是处理转换时间的 得告诉你一秒多少帧啥的
        AVRational time_base = avStream->time_base;

        //是音频
        if (parameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            // 虽然是视频类型，但是只有一帧封面   stream->attached_pic 封面
            if (avStream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                continue;
            }
            ////播放的秒 =pts*av_q2d(fps_rational)
            //求出每个刻度是多少秒   avg_frame_rate->平均帧率 但是这个东西是{1,25}这种类型 要转成int类型要用 av_q2d
            double fps = av_q2d(avStream->avg_frame_rate);
            //Index 为了区分是音频还是视频
            //构造传参
            video_channel = new VideoChannel(index, avCodecContext, time_base, fps);
            video_channel->setRenderCallback(renderingCallBack);
            if (duration != 0) {
                video_channel->setJniHelper(jniHelper);
            }


        } else if (parameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_channel = new AudioChannel(index, avCodecContext, time_base);
            if (duration != 0) {//直播不会有时长
                audio_channel->setJniHelper(jniHelper);
            }
        }

    }

    if (!audio_channel && !video_channel) {
        if (jniHelper) {
            jniHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA, av_err2str(result));
        }
        //关闭
        if (avFormatContext) {
            avcodec_free_context(&avCodecContext);
        }
        avformat_close_input(&avFormatContext);
        return;
    }

    if (jniHelper) {
        jniHelper->onPrepared(THREAD_CHILD);
    }


}

void *thread_start(void *args) {
    auto *player = static_cast<QPlayer *>(args);
    player->start_play();
    return nullptr;

}

void QPlayer::start() {

    is_play = true;
    //传入audio 用于同步
    if (video_channel) {
        //判断audio_channel是否为null
        if (audio_channel) {
            video_channel->putAudio(audio_channel);
        }
        video_channel->start();
    }
    if (audio_channel) {
        audio_channel->start();
    }
    pthread_create(&p_thread_start, nullptr, thread_start, this); // this == DerryPlayer的实例
}


/**
 * 开始播放
 */

QPlayer::QPlayer(const char *source, JniHelper *helper) {
    this->source = new char[strlen(source) + 1];
    strcpy(this->source, source); // 把源 Copy给成员
    this->jniHelper = helper;
    pthread_mutex_init(&p_thread_seek_mutex, nullptr);
}

QPlayer::~QPlayer() {
    //释放播放资源路径 释放jniHelper
    if (source) {
        delete source;
        source = nullptr;
    }
    if (jniHelper) {
        delete jniHelper;
        jniHelper = nullptr;
    }
}

void QPlayer::start_play() {
    while (is_play) {
        //避免内存爆炸
        if (video_channel && video_channel->aVPackets.size() > 100) {
            av_usleep(10 * 1000); // 单位：microseconds 微妙 10毫秒
            continue;
        }
        if (audio_channel && audio_channel->aVPackets.size() > 100) {
            av_usleep(10 * 1000); // 单位：microseconds 微妙 10毫秒
            continue;
        }

        //申请 AVPacket
        AVPacket *avPacket = av_packet_alloc();

        // 读包 0 if OK, < 0 on error
        int result = av_read_frame(avFormatContext, avPacket);
        if (!result)//没失败
        {
            //匹配一下 如果videoChannel 不是null 而且对上号
            if (video_channel && video_channel->type_index == avPacket->stream_index) {
                video_channel->aVPackets.insert(avPacket);
            } else if (audio_channel && audio_channel->type_index == avPacket->stream_index) {
                audio_channel->aVPackets.insert(avPacket);
            }

        } else if (result == AVERROR_EOF) {//读到末尾
            //包都解析完了停止

            if (video_channel && video_channel->aVPackets.empty() && audio_channel &&
                audio_channel->aVPackets.empty()) {
                break;
            }
        } else {
            break;
        }

    }
    //停止播放
    is_play = false;
    if (audio_channel) {
        audio_channel->stop();
    }
    if (video_channel) {
        video_channel->stop();
    }


}

/**
 * 设置渲染接口
 * @param renderingCallBack 渲染回调
 */
void QPlayer::setRenderCallback(RenderingCallBack callBack) {
    this->renderingCallBack = callBack;
}

/**
 * 线程停止任务
 * @param args
 * @return
 */
void *thread_stop(void *args) {
    auto *player = static_cast<QPlayer *>(args);
    player->start_stop(player);
    return nullptr;
}

/**
 * 停止
 */
void QPlayer::stop() {

    jniHelper = nullptr;
    if (audio_channel) {
        audio_channel->jniHelper = nullptr;
    }
    if (video_channel) {
        video_channel->jniHelper = nullptr;
    }
    pthread_create(&p_thread_stop, nullptr, thread_stop, this);

}

void QPlayer::seek(int progress) {
    if (!audio_channel && !video_channel) {
        return;
    }
    if (progress < 0 || progress > duration) {
        return;
    }
    if (!avFormatContext) {
        return;
    }

    //锁
    pthread_mutex_lock(&p_thread_seek_mutex);
    LOGE("seek lock%d", progress)
    //  AVSEEK_FLAG_FRAME -->https://blog.csdn.net/weiyuefei/article/details/51719735
//https://zhuanlan.zhihu.com/p/331503824
//截取视频  花屏幕 使用 AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME

   //这里的一个错误  TIMER_ABSTIME   和 AV_TIME_BASE  前者说的是 使用绝对时间。 坑啊
   //https://blog.csdn.net/luotuo44/article/details/39374759
    int result = av_seek_frame(avFormatContext, -1, progress * AV_TIME_BASE, AVSEEK_FLAG_FRAME);
    if (result < 0) {//>= 0 on success
        LOGE("seek ERROR")
        jniHelper->onError(THREAD_MAIN, FFMPEG_CAN_NOT_OPEN_URL, av_err2str(result));
        return;
    }
    //seek的时候 停止播放 清掉所有队列 然后继续开始
    if (audio_channel) {
        audio_channel->aVPackets.setPlayState(false);
        audio_channel->aVFrames.setPlayState(false);
        audio_channel->aVPackets.clear();
        audio_channel->aVFrames.clear();
        audio_channel->aVPackets.setPlayState(true);
        audio_channel->aVFrames.setPlayState(true);
    }
    if (video_channel) {
        video_channel->aVPackets.setPlayState(false);
        video_channel->aVFrames.setPlayState(false);
        video_channel->aVPackets.clear();
        video_channel->aVFrames.clear();
        video_channel->aVPackets.setPlayState(true);
        video_channel->aVFrames.setPlayState(true);
    }

    //解锁
    pthread_mutex_unlock(&p_thread_seek_mutex);
    LOGE("seek unlock ")
}

/**
 * 返回时长
 * @return
 */
int64_t QPlayer::getDuration() {
    return duration;

}


/**
 * 停止工作
 */
void QPlayer::start_stop(QPlayer *player) {
    is_play = false;
    pthread_join(p_thread_start, nullptr);
    pthread_join(p_thread_prepare, nullptr);
    //释放 卡住 等他们都干完了 我再干这活  不然崩了
    if (avFormatContext) {
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
        avFormatContext = nullptr;
    }

    delete audio_channel;
    audio_channel = nullptr;
    delete video_channel;
    video_channel = nullptr;
    delete player;

}
