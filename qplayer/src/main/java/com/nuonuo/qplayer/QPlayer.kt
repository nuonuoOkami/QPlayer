package com.nuonuo.qplayer

import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.lifecycle.LifecycleObserver

/**

 * @author ：leo
 * @创建时间 on 2022/10/17 22:02
 * @version 1  测试 地址 rtmp://media3.scctv.net/live/scctv_800
 * @描述
 */
class QPlayer : SurfaceHolder.Callback, LifecycleObserver {
    private val TAG = "QPlayer"

    //播放资源
    private var resources: String? = null


    //c++层回调的播放器对象地址
    private var nativeQPlayer: Long? = null
    private var surfaceHolder: SurfaceHolder? = null

    //监听
    private var playerListener: PlayerListener? = null
    override fun surfaceCreated(holder: SurfaceHolder) {
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        if (nativeQPlayer != null) {
            setSurfaceNative(holder.surface, nativeQPlayer!!)
        }
    }


    /**
     *获取时长
     */
    fun getDuration(): Int {
        if (nativeQPlayer != null) {
            return getDurationNative(nativeQPlayer!!)
        }
        return -1;
    }

    /**
     * 设置监听
     */
    fun setPlayerListener(listener: PlayerListener) {
        this.playerListener = listener;
    }


    override fun surfaceDestroyed(holder: SurfaceHolder) {

    }

    /**
     * 开始播放
     */
    fun start() {

        if (nativeQPlayer != null) {
            startNative(nativeQPlayer!!)
            playerListener?.onPlayStart()
        }

    }

    fun prepare() {
        if (resources != null) {
            nativeQPlayer = prepareNative(resources!!)
        }
    }

    //回调预加载
  private  fun onPrepared() {
        playerListener?.onPrepared(this)
    }

    //回调进度
  private  fun onProgress(progress: Int) {
        playerListener?.onProgress(progress)
        if (progress >= getDuration()) {
            playerListener?.onPlayEnd()
        }
    }


    /**
     * 传递外部播放控件
     * @param surfaceView SurfaceView
     */
    fun setSurface(surfaceView: SurfaceView) {
        if (surfaceHolder != null) {
            surfaceHolder?.removeCallback(this)
        }
        surfaceHolder = surfaceView.holder
        surfaceHolder?.addCallback(this)
    }


    /**
     * 错误处理
     */
   private fun onError(errorCode: Int, ffmpegError: String) {
        val title = "\nFFmpeg给出的错误如下:\n"
        if (null != playerListener) {
            var msg: String? = null
            when (errorCode) {
                FFMPGE.FFMPEG_CAN_NOT_OPEN_URL -> msg = "打不开视频$title$ffmpegError"
                FFMPGE.FFMPEG_CAN_NOT_FIND_STREAMS -> msg = "找不到流媒体$title$ffmpegError"
                FFMPGE.FFMPEG_FIND_DECODER_FAIL -> msg = "找不到解码器$title$ffmpegError"
                FFMPGE.FFMPEG_ALLOC_CODEC_CONTEXT_FAIL -> msg = "无法根据解码器创建上下文$title$ffmpegError"
                FFMPGE.FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL -> msg =
                    "根据流信息 配置上下文参数失败$title$ffmpegError"
                FFMPGE.FFMPEG_OPEN_DECODER_FAIL -> msg = "打开解码器失败$title$ffmpegError"
                FFMPGE.FFMPEG_NOMEDIA -> msg = "没有音视频$title$ffmpegError"
            }
            playerListener?.onError(msg)
        }
    }

    /**
     * 设置播放资源路径
     */
    fun setPath(path: String) {
        this.resources = path
    }


    companion object {
        init {
            System.loadLibrary("qplayer")
        }
    }

    //进度条拖动
    fun seek(playProgress: Int) {
        if (nativeQPlayer != null) {
            seekNative(playProgress, nativeQPlayer!!)
        }
    }


    //获取到C++层player的对象地址
    private external fun prepareNative(dataSource: String): Long

    //开启播放
    private external fun startNative(nativeObj: Long)

    //停止播放
    private external fun stopNative(nativeObj: Long)

    //设置surfaceView
    private external fun setSurfaceNative(surface: Surface, nativeObj: Long)

    //进度条拖动
    private external fun seekNative(playValue: Int, nativeObj: Long)

    //释放
    private external fun releaseNative(nativeObj: Long)

    private external fun getDurationNative(nativeObj: Long): Int
}