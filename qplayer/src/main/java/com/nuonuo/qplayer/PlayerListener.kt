package com.nuonuo.qplayer

/**

 * @author ：leo
 * @创建时间 on 2022/10/23 19:33
 * @version 1
 * @描述  回调监听
 */
interface PlayerListener {
    /**
     * 进度回调
     */
    fun onProgress(progress: Int)

    /**
     * 准备完成
     */
    fun onPrepared(player: QPlayer)

    /**
     * 错误回调
     */
    fun onError(errorCode: String?)

    /**
     * 播放结束
     */
    fun onPlayEnd();

    /**
     * 开始播放
     */
    fun onPlayStart();

    /**
     * 视频宽高参数
     * @param frameWidth Int
     * @param frameHeight Int
     */
    fun frameParams(frameWidth: Int, frameHeight: Int)
}