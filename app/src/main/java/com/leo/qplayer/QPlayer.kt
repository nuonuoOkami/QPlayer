package com.leo.qplayer

import android.view.SurfaceHolder
import androidx.lifecycle.LifecycleObserver

/**

 * @author ：leo
 * @创建时间 on 2022/10/17 22:02
 * @version 1  测试 地址 rtmp://media3.scctv.net/live/scctv_800
 * @描述
 */
class QPlayer : SurfaceHolder.Callback, LifecycleObserver {
    private var path: String? = null;
    override fun surfaceCreated(holder: SurfaceHolder) {
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {

    }

    /**
     * 设置播放资源路径
     */
    fun setPath(path: String) {
        this.path = path
    }

    companion object {
        init {
            System.loadLibrary("qplayer")
        }
    }

    //获取到C++层player的对象地址
    private external fun prepareNative(dataSource: String): Long

    private external fun startNative(nativeObj: Long)

    private external fun stopNative(nativeObj: Long)
}