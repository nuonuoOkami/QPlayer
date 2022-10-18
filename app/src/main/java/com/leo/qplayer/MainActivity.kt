package com.leo.qplayer

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
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
    }


    external fun stringFromJNI(): String
    external fun player(path: String): Void


}