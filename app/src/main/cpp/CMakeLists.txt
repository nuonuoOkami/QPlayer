
cmake_minimum_required(VERSION 3.18.1)
set(FFMPEG ${CMAKE_SOURCE_DIR}/ffmpeg) # ffmpeg的路径 CMAKE_SOURCE_DIR 就是和cmakeLists 是同一路径
set(RTMP ${CMAKE_SOURCE_DIR}/rtmp) # rtmp的路径
include_directories(${FFMPEG}/include) # 导入ffmpeg的头文件 避免挨个ffmpeg/xxx/那么导入了
include_directories(${FFMPEG}/include) # 导入ffmpeg的头文件 ${} 和kt里面一样就是引用上面的路径
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${FFMPEG}/libs/${CMAKE_ANDROID_ARCH_ABI}") # ffmpeg库指定
#这里我查了很久 我只能理解为把这个路径让编译的时候 知道路径 个人理解
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${RTMP}/libs/${CMAKE_ANDROID_ARCH_ABI}") # rtmp库指定
# 批量导入 源文件
file(GLOB src_files *.cpp)
#project("qplayer") 我个人测试自己项目这里加包会报错

add_library( # Sets the name of the library.
        qplayer
        SHARED
        ${src_files}
        )
target_link_libraries(
        qplayer
        # 忽略顺序的方式，导入 静态库 有的库依赖其他的如果那个库还没加载会崩
        -Wl,--start-group
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group
        log # 日志库，打印日志用的
        z # libz.so库，是FFmpeg需要用ndk的z库，FFMpeg需要额外支持  libz.so
        rtmp # rtmp
        android # android 为了引用 ANativeWindow
        OpenSLES # OpenSLES  播放声音使用
        jnigraphics

)