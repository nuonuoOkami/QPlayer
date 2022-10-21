//
// Created by leo on 2022/10/16.
//

#ifndef QPLAYER_Q_PLAYER_H
#define QPLAYER_Q_PLAYER_H

#include <pthread.h>
#include "video_channel.h"
#include "jni_helper.h"
#include "audio_channel.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

class QPlayer {
private:


    //路径
    char *source = 0;
    //准备线程
    pthread_t p_thread_prepare;
    //开始线程
    pthread_t p_thread_start;
    //停止线程
    pthread_t p_thread_stop;
    //seek 锁
    pthread_mutex_t p_thread_seek_mutex;

    //媒体上下文
    //https://blog.csdn.net/leixiaohua1020/article/details/14214705
    AVFormatContext *avFormatContext = 0;
    //视频播放流
    VideoChannel *video_channel = 0;
    //音频流
    AudioChannel *audio_channel = 0;
    //总时长
    int64_t duration;
    //是否播放
    bool is_play;
    RenderingCallBack renderingCallBack;

    //JNI层回调
    JniHelper *jniHelper;


public:
    QPlayer(const char *source, JniHelper *jniHelper);

    ~QPlayer();

    void prepare();

    void start_prepare();

    void start();

    void start_play();


    void setRenderCallback(RenderingCallBack renderingCallBack);

    int getDuration();

    void seek(int play_value);

    void stop();

    void start_stop(QPlayer *);


};


#endif //QPLAYER_Q_PLAYER_H
