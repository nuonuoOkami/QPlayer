
#include "VideoChannel.h"

/**
 * 解码
 */
void VideoChannel::decode() {
    AVPacket *avPacket = nullptr;
    while (is_play) {
        if (is_play && aVFrames.size() > 100) {
            av_usleep(10 * 1000);//ffmpeg 里面使用的是纳秒 睡10ms
            continue;
        }
        int is_ok = aVPackets.take(avPacket);//取出数据包  判断是否成功
        if (!is_play) {
            break;
        }
        if (!is_ok) {//取出失败就继续循环等待添加
            continue;
        }

        //发送数据给ffmpeg 会提供解压后原始包给到我们
        int send = avcodec_send_packet(avCodecContext, avPacket);
        if (send)//0 为成功 其他就失败了
        {
            break;
        }
        //取出 ffmpeg转换完的数据包 个人理解
        AVFrame *pFrame = av_frame_alloc();
        //取出帧包，0成功
        int receive = avcodec_receive_frame(avCodecContext, pFrame);
        if (receive == AVERROR(EAGAIN))//提示继续
        {
            continue;
        } else if (receive != 0)//不成功 得释放了pFrame不然泄露了
        {
            if (pFrame) {//pFrame 还存在记得释放
                av_frame_free(&pFrame);
                pFrame = nullptr;
            }
            break;
        }
        //成功 添加进帧队列
        aVFrames.insert(pFrame);
        //释放掉原始包 av_packet_alloc 和 av_packet_free 必须要配对使用，
        // 否则会造成内存泄漏。av_packet_free 实际是释放AVPacket 的空间
        // av_packet_unref 只是将data置为NULL,size置为0 可以理解为将内部的参数释放了 但是自己还没释放
        av_packet_unref(avPacket);
        av_packet_free(&avPacket);
        avPacket = nullptr;

    }

    //循环结束 还需要释放一次
    av_packet_unref(avPacket);
    av_packet_free(&avPacket);
    avPacket = nullptr;

}

/**
 * 播放
 */
void VideoChannel::play() {

    AVFrame *avFrame = nullptr;
    uint8_t *pointers[4];
    int linesizes[4]; //保存图像每个通道的内存对齐的步长，即一行的对齐内存的宽度，此值大小等于图像宽度
    //申请帧内存 返回负数为失败 因为android 只能显示rgba 而一般是yuv的
    //  align  https://blog.csdn.net/huweijian5/article/details/105832601?utm_medium=distribute.pc_aggpage_search_result.none-task-blog-2~aggregatepage~first_rank_ecpm_v1~rank_v31_ecpm-1-105832601-null-null.pc_agg_new_rank&utm_term=ffmpeg%20%E5%AD%97%E8%8A%82%E5%AF%B9%E9%BD%90&spm=1000.2123.3001.4430
    av_image_alloc(pointers, linesizes, avCodecContext->width, avCodecContext->height,
                   AV_PIX_FMT_RGBA, 1);

    //转换为rgba  输入 宽 高 像素     输出  宽 高  后几个为滤镜
    //sws_scale SWS_BILINEAR 算法 https://blog.csdn.net/leixiaohua1020/article/details/12029505
    SwsContext *swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                            avCodecContext->pix_fmt, avCodecContext->width,
                                            avCodecContext->height,
                                            AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr,
                                            nullptr);

    while (is_play) {
        bool result = aVFrames.take(avFrame);
        if (!is_play) {
            break;
        }
        if (!result) {
            continue;//没取到继续取
        }
        //这个很有意思 大体就是 将帧里面数据从左上角挨个一行行铺下来，铺avCodecContext->height行
        //pointers 里面分别引用的是rgba的地址值
        sws_scale(swsContext, avFrame->data, avFrame->linesize, 0, avCodecContext->height, pointers,
                  linesizes);

        //todo 同步代码
        //数组传递会降级为第0个位置的指针
        renderingCallBack(pointers[0], avCodecContext->width, avCodecContext->height, linesizes[0]);
        //释放

        av_frame_unref(avFrame);
        av_frame_free(&avFrame);
        avFrame = nullptr;


    }
    //释放
    av_frame_unref(avFrame);
    av_frame_free(&avFrame);
    avFrame = nullptr;
    is_play = false;
    av_free(&pointers[0]);
    sws_freeContext(swsContext);


}

/**
 * 设置渲染回调给native-lib.cpp
 * @param renderCallback
 */
void VideoChannel::setRenderCallback(RenderingCallBack renderCallback) {
    this->renderingCallBack = renderCallback;
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
    auto *channel = static_cast<VideoChannel *>(args);
    channel->decode();
    return nullptr;
}

/**
 * 执行播放任务
 * @param args
 */
void *task_play(void *args) {

    auto *channel = static_cast<VideoChannel *>(args);
    channel->play();
    return nullptr;

}

VideoChannel::~VideoChannel() {

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

