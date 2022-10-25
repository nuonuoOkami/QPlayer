package com.leo.qplayer

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.leo.qplayer.databinding.ActivityMainBinding
import com.nuonuo.qplayer.PlayerListenerAdapter
import com.nuonuo.qplayer.QPlayer


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val TAG = "MainActivity"
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val player = QPlayer(this)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        player.setSurface(binding.surface)
        //player.setSurface(null)
        //player.setPath("rtmp://media3.scctv.net/live/scctv_800")
        //player.setPath(File(filesDir, "1.mp4").absolutePath)
        player.setPath("http://vfx.mtime.cn/Video/2019/02/04/mp4/190204084208765161.mp4")
        //player.setPath("rtmp://void.75755.com/liverepeater/500106?wsSecret=fbc187bda06c3af2f6bda001d339277d&wsTime=634fbba9")
        // player.setPath("https://stream7.iqilu.com/10339/upload_transcode/202002/18/20200218025702PSiVKDB5ap.mp4")
        lifecycle.addObserver(player)
        player.setPlayerListener(object : PlayerListenerAdapter() {

            override fun onPlayEnd() {
                super.onPlayEnd()
            }

            override fun onPlayStart() {
                super.onPlayStart()
            }

            override fun onPrepared(player: QPlayer) {
                super.onPrepared(player)
                player.start()


            }

            override fun onError(errorCode: String?) {
                super.onError(errorCode)


            }

        })
        player.prepare()
        //player.stop()



        binding.start.setOnClickListener {
            player.seek(25)
        }
    }


}