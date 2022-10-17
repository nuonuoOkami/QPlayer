//
// Created by leo on 2022/10/16.
//

#ifndef QPLAYER_QPLAYER_H
#define QPLAYER_QPLAYER_H

#include <pthread.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};

class QPlayer {
private:

    //路径
    char *source = 0;
    //准备线程
    pthread_t p_thread_prepare;
    //开始线程
    pthread_t p_thread_start;
    //媒体上下文
    AVFormatContext *avFormatContext = 0;




};


#endif //QPLAYER_QPLAYER_H
