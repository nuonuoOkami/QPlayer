package com.leo.qplayer

import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleObserver
import androidx.lifecycle.OnLifecycleEvent
import kotlin.math.log

/**

 * @author ：leo
 * @创建时间 on 2022/10/17 22:02
 * @version 1  测试 地址 rtmp://media3.scctv.net/live/scctv_800
 * @描述
 */
class QPlayer : SurfaceHolder.Callback, LifecycleObserver {
    private var path: String? = null
    private var nativeQPlayer: Long? = null
    private var surfaceHolder: SurfaceHolder? = null
    override fun surfaceCreated(holder: SurfaceHolder) {
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        if (nativeQPlayer != null) {
            setSurfaceNative(holder.surface, nativeQPlayer!!)
        }

    }



    override fun surfaceDestroyed(holder: SurfaceHolder) {

    }

    /**
     * 开始播放
     */
    fun start() {
        if (nativeQPlayer != null) {
            startNative(nativeQPlayer!!)
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
     * 设置播放资源路径
     */
    fun setPath(path: String) {
        this.path = path
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    fun prepare() { // 我们的准备工作：触发

        path="rtmp://media3.scctv.net/live/scctv_800"

      nativeQPlayer = prepareNative(path!!)


       //startNative(nativeQPlayer!!)
    }

    companion object {
        init {
            System.loadLibrary("qplayer")
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
}