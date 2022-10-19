//
// Created by leo on 2022/10/16.
//

#ifndef QPLAYER_Q_PLAYER_H
#define QPLAYER_Q_PLAYER_H

#include <pthread.h>
#include "video_channel.h"
#include "jni_helper.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

class q_player {
private:

    //路径
    char *source = 0;
    //准备线程
    pthread_t p_thread_prepare;
    //开始线程
    pthread_t p_thread_start;
    //媒体上下文
    //https://blog.csdn.net/leixiaohua1020/article/details/14214705
    AVFormatContext *avFormatContext = 0;
    //视频播放流
    video_channel *videoChannel=0;
    //总时长
    int duration;
    //是否播放
    bool is_play;
    RenderingCallBack renderingCallBack;


public:
    q_player(const char *source , JniHelper * jniHelper);
    ~q_player();
    void prepare();
    void prepare_();

    void start();
    void start_();


    void setRenderCallback(RenderingCallBack renderingCallBack);

    int getDuration();

    void seek(int play_value);

    void stop();

    void stop_(q_player *);



};


#endif //QPLAYER_Q_PLAYER_H
