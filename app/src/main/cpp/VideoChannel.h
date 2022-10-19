//
// Created by leo on 2022/10/16.
//

#ifndef QPLAYER_VIDEOCHANNEL_H
#define QPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"

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
    int fps;//帧率
    RenderingCallBack renderingCallBack;//渲染回调 因为会用到ANativeWindow


public:
    VideoChannel(int type_index, AVCodecContext *codecContext, AVRational time_base, int fps);//构造函数
    //析构函数
    ~VideoChannel();

    void  start();//播放
    void  stop();//停止播放
    void  decode();//解码
     void  play();//播放
    void setRenderCallback(RenderingCallBack renderCallback);//设置渲回调


};


#endif
