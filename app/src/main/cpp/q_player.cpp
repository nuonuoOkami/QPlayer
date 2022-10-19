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

    auto *pPlayer = static_cast<q_player *>(args);
    pPlayer->prepare_();
    return nullptr;//切记返回 坑死了
}

/**
 * 准备
 */
void q_player::prepare() {
    //将任务切换到子线程 因为是io操作
    pthread_create(&p_thread_prepare, nullptr, thread_prepare, this);
}


/**
 * 子线程执行加载任务
 */
void q_player::prepare_() {


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
        //异常 todo 暂时不处理
        //释放掉上下文
        avformat_close_input(&avFormatContext);

        return;

    }
    //查看信息流 大于0 ok  其他就失败了
    result = avformat_find_stream_info(avFormatContext, nullptr);

    if (result < 0) { //异常 todo 暂时不处理
        //释放掉上下文
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

            //todo 异常处理
            //释放上下文
            avformat_close_input(&avFormatContext);

            return;
        }

        //干活大佬 null 失败
        avCodecContext = avcodec_alloc_context3(avCodec);

        if (!avCodecContext) {
            //todo 异常处理

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
            //todo 异常处理

            //释放avCodecContext
            avcodec_free_context(&avCodecContext);
            //释放上下文
            avformat_close_input(&avFormatContext);
            return;
        }


        //打开解码器 zero on success
        result = avcodec_open2(avCodecContext, avCodec, nullptr);
        if (result) {
            //todo 异常处理
            //释放avCodecContext
            avcodec_free_context(&avCodecContext);
            //释放上下文
            avformat_close_input(&avFormatContext);
            return;
        }
        //todo  音视频同步代码 

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
            int fps = av_q2d(avStream->avg_frame_rate);
            //Index 为了区分是音频还是视频
            //构造传参
            videoChannel = new video_channel(index, avCodecContext, time_base, fps);
            videoChannel->setRenderCallback(renderingCallBack);

        }


    }


}

void *to_start(void *args) {
    auto *player = static_cast<q_player *>(args);
    player->start_();
    return nullptr;

}

void q_player::start() {

    is_play = true;
    if (videoChannel) {
        videoChannel->start();

    }
    pthread_create(&p_thread_start, nullptr, to_start, this); // this == DerryPlayer的实例
}


/**
 * 开始播放
 */

q_player::q_player(const char *source, JniHelper *helper) {
    this->source = new char[strlen(source) + 1];
    strcpy(this->source, source); // 把源 Copy给成员

}

q_player::~q_player() {

}

void q_player::start_() {


    while (is_play) {
        //todo 延迟代码

        //申请 AVPacket
        AVPacket *avPacket = av_packet_alloc();

        // 读包 0 if OK, < 0 on error
        int result = av_read_frame(avFormatContext, avPacket);
        if (!result)//没失败
        {

            //匹配一下 如果videoChannel 不是null 而且对上号
            if (videoChannel && videoChannel->type_index == avPacket->stream_index) {
                videoChannel->aVPackets.insert(avPacket);
            }


        } else if (result == AVERROR_EOF) {//读到末尾

        } else {
            break;
        }

    }
}

/**
 * 设置渲染接口
 * @param renderingCallBack
 */
void q_player::setRenderCallback(RenderingCallBack renderingCallBack) {
    this->renderingCallBack = renderingCallBack;
}