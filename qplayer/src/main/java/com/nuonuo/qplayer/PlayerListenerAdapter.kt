package com.nuonuo.qplayer

/**

 * @author ：leo
 * @创建时间 on 2022/10/23 21:24
 * @version 1
 * @描述 适配器 避免覆盖所有方法
 */
open class PlayerListenerAdapter : PlayerListener {
    override fun onProgress(progress: Int) {

    }

    override fun onPrepared(player: QPlayer) {

    }

    override fun onError(errorCode: String?) {

    }

    override fun onPlayEnd() {

    }

    override fun onPlayStart() {
    }

    override fun frameParams(frameWidth: Int, frameHeight: Int) {

    }

}