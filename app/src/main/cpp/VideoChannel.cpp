
#include "VideoChannel.h"

/**
 * 解码
 */
void VideoChannel::decode() {


}

/**
 * 播放
 */
void VideoChannel::play() {


}

/**
 * 设置渲染回调给native-lib.cpp
 * @param renderCallback
 */
void VideoChannel::setRenderCallback(RenderingCallBack renderCallback) {


}

/**
 * 停止
 */
void VideoChannel::stop() {


}

/**
 * 执行解包任务
 * @param args
 */
void *task_decode(void *args) {

}

/**
 * 执行播放任务
 * @param args
 */
void *task_play(void *args) {

    auto *channel = static_cast<VideoChannel *>(args);
    channel->decode();
    
}


/**
 * 任务开始
 * 任务开始 包含解压缩和塞入数据 play只是播放最后的数据
 */
void VideoChannel::start() {

    is_play = true;
    aVPackets.setPlayState(true);
    aVFrames.setPlayState(true);
    //线程执行解包任务
    pthread_create(&p_thread_decode, nullptr, task_decode, this);
    //线程进行播放任务
    pthread_create(&p_thread_play, nullptr, task_play, this);

}


/**
 * 抛弃原始包
 * @param queue
 */
void dumpAvFrames(queue<AVFrame *> &queue) {

    if (!queue.empty()) {
        AVFrame *pFrame = queue.front();
        if (pFrame) {
            av_frame_free(&pFrame);//居然要二级指针 - -！
            pFrame = nullptr;
        }
        queue.pop();//删除

    }
}

/**
 * 丢弃压缩数据 （其实可以不写 因为在塞入ffmpeg的时候就丢弃了 备用）
 * @param queue
 */
void dumpAVPackets(queue<AVPacket *> &queue) {
    while (!queue.empty()) {
        AVPacket *packet = queue.front();
        if (packet->flags != AV_PKT_FLAG_KEY)//不是关键帧可以丢
        {
            av_packet_free(&packet);//释放 二级指针
            packet = nullptr;
            queue.pop();
        } else {
            break;
        }
    }
}

/**
 * 构造函数
 * @param type_index  类型 音频还是视频 
 * @param codecContext  解码器
 * @param time_base  实践基
 * @param fps  帧率
 */
VideoChannel::VideoChannel(int type_index, AVCodecContext *codecContext, AVRational time_base,
                           int fps) : BaseChannel(type_index, codecContext, time_base), fps(fps) {

    aVFrames.setDumpListener(dumpAvFrames);//设置抛弃回调 用于同步
    aVPackets.setDumpListener(dumpAVPackets);//设置抛弃回调 用于同步  其实理论上用不到
}

