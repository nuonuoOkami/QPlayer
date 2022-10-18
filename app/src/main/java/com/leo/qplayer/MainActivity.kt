package com.leo.qplayer

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.leo.qplayer.databinding.ActivityMainBinding


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val player = QPlayer()

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.sampleText.text = stringFromJNI()
        player.setSurface(binding.surface)
        player.setPath("rtmp://media3.scctv.net/live/scctv_800")

        lifecycle.addObserver(player)


        val wm1 = this.windowManager
        val width1 = wm1.defaultDisplay.width
    }


    external fun stringFromJNI(): String
    external fun player(path: String): Void


}