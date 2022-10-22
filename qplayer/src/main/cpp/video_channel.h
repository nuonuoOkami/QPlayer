//
// Created by leo on 2022/10/16.
//

#ifndef QPLAYER_VIDEO_CHANNEL_H
#define QPLAYER_VIDEO_CHANNEL_H

#include "base_channel.h"
#include "audio_channel.h"

extern "C"
{
#include <libswscale/swscale.h>//图像转换
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

/**
 * 视频信息流处理
 */
typedef void(*RenderingCallBack)(uint8_t *, int, int, int);//渲染回调
class VideoChannel : public BaseChannel {

private:
    pthread_t p_thread_decode;//解包用线程
    pthread_t p_thread_play;//播放用线程
    double fps;//帧率
    RenderingCallBack renderingCallBack;//渲染回调 因为会用到ANativeWindow
    //传入音频方便同步
    AudioChannel *audio_Channel = 0;



public:
    JniHelper *jniHelper = 0;
    VideoChannel(int type_index, AVCodecContext *codecContext, AVRational time_base, double fps);//构造函数
    //析构函数
    ~VideoChannel();

    void start();//播放
    void stop();//停止播放
    void decode();//解码
    void play();//播放
    void setRenderCallback(RenderingCallBack renderCallback);//设置渲回调
    //设置videoChannel 用于同步
    void putAudio(AudioChannel *);

    //设置jniHelper
    void setJniHelper(JniHelper *);

};


#endif
