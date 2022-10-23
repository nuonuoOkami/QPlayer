package com.leo.qplayer

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.leo.qplayer.databinding.ActivityMainBinding
import com.nuonuo.qplayer.PlayerListenerAdapter
import com.nuonuo.qplayer.QPlayer


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val TAG = "MainActivity"
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val player = QPlayer()

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        player.setSurface(binding.surface)
        //player.setPath("rtmp://media3.scctv.net/live/scctv_800")
        //player.setPath(File(filesDir, "1.mp4").absolutePath)
        player.setPath("rtmp://void.75755.com/liverepeater/500106?wsSecret=fbc187bda06c3af2f6bda001d339277d&wsTime=634fbba9")
        lifecycle.addObserver(player)
        player.setPlayerListener(object : PlayerListenerAdapter() {

            override fun onPlayEnd() {
                super.onPlayEnd()
                Log.e(TAG, "onPlayEnd: ")
            }

            override fun onPlayStart() {
                super.onPlayStart()
                Log.e(TAG, "onPlayStart: ")
            }

            override fun onPrepared(player: QPlayer) {
                super.onPrepared(player)
                player.start()


            }
        })
        player.prepare()


        binding.start.setOnClickListener {
            player.seek(25)
        }
    }


}