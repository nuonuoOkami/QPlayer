# QPlayer

![Image text](https://github.com/nuonuoOkami/images/blob/main/qplayer.png.png)
### 依赖库
    ffmpeg 4.2.1 
    OpenSlES 

### 支持
    本地音视频/rtmp流
### native占用
#### 本地mp4
![Image text](https://github.com/nuonuoOkami/images/blob/main/mp4_native.png)
####  rtmp
![Image text](https://github.com/nuonuoOkami/images/blob/main/rtmp.png)

### 依赖使用
    implementation 'io.github.nuonuoOkami:QPlayer:1.1.0'

### 使用方式
    
    //初始化
    val player = QPlayer()
    player.setSurface(binding.surface)
    //设置链接地址 本地或者rmtp
    player.setPath("rtmp://void.75755.com/liverepeater/500106?wsSecret=fbc187bda06c3af2f6bda001d339277d&wsTime=634fbba9")
    //添加生命周期关联
     lifecycle.addObserver(player)
    //添加加载监听
    player.setPlayerListener(object : PlayerListenerAdapter() {

            override fun onPlayEnd() {
                super.onPlayEnd()
               //播放结束 rmtp 不会有结束
            }

            override fun onPlayStart() {
                super.onPlayStart()
               //开始播放
            }

            override fun onPrepared(player: QPlayer) {
                super.onPrepared(player)
              // 加载完成可以播放
                //开始播放
                player.start()

            }
        })
        //初始化
        player.prepare()
        
        //关闭
     player.stop()

### 写在最后
    1.2版本正在编写中,预开发内容
    * 支持https 
    * 封装生命周期组件
    
    
    

