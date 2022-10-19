//
// Created by leo on 2022/10/16.
//

#ifndef QPLAYER_QPLAYER_H
#define QPLAYER_QPLAYER_H

#include <pthread.h>
#include "VideoChannel.h"
#include "jni_helper.h"

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
    //媒体上下文
    //https://blog.csdn.net/leixiaohua1020/article/details/14214705
    AVFormatContext *avFormatContext = 0;
    //视频播放流
    VideoChannel *videoChannel=0;
    //总时长
    int duration;
    //是否播放
    bool is_play;
    RenderingCallBack renderingCallBack;


public:
    QPlayer(const char *source , JniHelper * jniHelper);
    ~QPlayer();
    void prepare();
    void prepare_();

    void start();
    void start_();


    void setRenderCallback(RenderingCallBack renderingCallBack);

    int getDuration();

    void seek(int play_value);

    void stop();

    void stop_(QPlayer *);



};


#endif //QPLAYER_QPLAYER_H
