
extern "C"//ffmpeg 需要extern c
{
#include "libavcodec/avcodec.h"
#include "libavutil/time.h"
}

#include "safe_queue.h"//导入安全队列

#ifndef QPLAYER_BASECHANNEL_H
#define QPLAYER_BASECHANNEL_H


/**
 * 封装基础类给视频音频使用
 */
class BaseChannel {
public:
    int type_index;//根据 AVFormatContext.nb_streams 获取自己音视频类型
    bool is_play;//播放状态
    AVRational time_base;//时间基
    AVCodecContext *avCodecContext = 0;//解码器 初始化为0
    SafeQueue<AVFrame *> aVFrames;//从信息源里获取出来的原始压缩数据包
    SafeQueue<AVPacket *> aVPackets;//解析出来的原始包 还需要音频/视频的再次解析
public:
    BaseChannel(int typeIndex, AVCodecContext *avCodecContext, AVRational timeBase,)
            : avCodecContext(avCodecContext),
              type_index(typeIndex), time_base(timeBase) {
    };

    virtual ~BaseChannel() {
        aVFrames.clear();
        aVPackets.clear();
    }
};


#endif
