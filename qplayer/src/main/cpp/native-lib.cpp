#include <jni.h>
#include <string>
#include "log4c.h"


extern "C" {
#include <libavutil/avutil.h>
}

#include <android/native_window_jni.h>

#include "safe_queue.h"
#include "q_player.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_nuonuo_qplayer_QPlayer__stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "版本号为";
    hello.append(av_version_info());
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_nuonuo_qplayer_QPlayer__player(JNIEnv *env, jobject thiz, jstring path) {

    return thiz;
}

JavaVM *vm = nullptr;
ANativeWindow *window = nullptr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 静态初始化 锁


/**
 * 渲染界面
 * @param src_data
 * @param width
 * @param height
 * @param src_linesize
 */
void renderFrame(uint8_t *src_data, int width, int height, int src_linesize) {


    //src_linesize 保存图像每个通道的内存对齐的步长，即一行的对齐内存的宽度，此值大小等于图像宽度
    //加锁
    pthread_mutex_lock(&mutex);

    if (!window) {
        //window出了问题要解锁 不然卡死了
        pthread_mutex_unlock(&mutex);
    }

    //设置属性 宽高行数  window  宽高，样式 rgba 888
    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer window_buffer;
    //如果是锁住的就释放一下
    if (ANativeWindow_lock(window, &window_buffer, nullptr)) {
        ANativeWindow_release(window);
        window = nullptr;
        pthread_mutex_unlock(&mutex);
        return;
    }

    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    //要*4 因为rgba 各自占了一个字节 stride 可能大于像素宽度
    int32_t dst_linesize = window_buffer.stride * 4;


    for (int i = 0; i < window_buffer.height; ++i) {

        // 复制到哪里  源头  长度  src_linesize 是步长
//        memcpy(dst_data + i * dst_linesize, src_data + i * src_linesize, src_linesize);
        memcpy(dst_data + i * dst_linesize, src_data + i * src_linesize, src_linesize);
    }

    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);


}

/**
 * 初始化vm
 * @param vm
 * @param args
 * @return
 */
jint JNI_OnLoad(JavaVM *vm, void *args) {
    ::vm = vm;
    return JNI_VERSION_1_6;
}

/**
 * 设置资源路径
 */
extern "C"
JNIEXPORT jlong JNICALL
Java_com_nuonuo_qplayer_QPlayer_prepareNative(JNIEnv *env, jobject job, jstring data_source) {

    const char *path = env->GetStringUTFChars(data_source, nullptr);
    auto *helper = new JniHelper(vm, env, job);
    auto *pPlayer = new QPlayer(path, helper);
    pPlayer->setRenderCallback(renderFrame);
    pPlayer->prepare();
    //释放掉资源
    env->ReleaseStringUTFChars(data_source, path);
    return reinterpret_cast<jlong>(pPlayer);
}

/**
 * 开始播放
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_nuonuo_qplayer_QPlayer_startNative(JNIEnv *env, jobject thiz, jlong native_obj) {
    auto *pPlayer = reinterpret_cast<QPlayer *>(native_obj);
    if (pPlayer) {
        pPlayer->start();
    }
}


/**
 * 停止播放
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_nuonuo_qplayer_QPlayer_stopNative(JNIEnv *env, jobject thiz, jlong native_obj) {
    auto *player = reinterpret_cast<QPlayer *>(thiz);
    if (player) {
        player->stop();
    }
}

/**
 * 设置surface 到cpp
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_nuonuo_qplayer_QPlayer_setSurfaceNative(JNIEnv *env, jobject thiz, jobject surface,
                                                 jlong native_obj) {
    pthread_mutex_lock(&mutex);

    // 先释放之前的显示窗口
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }

    // 创建新的窗口用于视频显示
    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_nuonuo_qplayer_QPlayer_seekNative(JNIEnv *env, jobject thiz, jint play_value,jlong native_obj) {
    auto *player = reinterpret_cast<QPlayer *>(native_obj);
    if (player) {
        player->seek(play_value);
    }
}

//释放
extern "C"
JNIEXPORT void JNICALL
Java_com_nuonuo_qplayer_QPlayer_releaseNative(JNIEnv *env, jobject thiz, jlong native_obj) {
    auto *player = reinterpret_cast<QPlayer *>(native_obj);
    pthread_mutex_lock(&mutex);
    // 先释放之前的显示窗口
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }

    pthread_mutex_unlock(&mutex);
    // 释放工作
    delete player;
    player = nullptr;
    delete vm;
    vm = nullptr;
    delete window;
    window = nullptr;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_nuonuo_qplayer_QPlayer_getDurationNative(JNIEnv *env, jobject thiz, jlong native_obj) {
    auto * player = reinterpret_cast<QPlayer *>(native_obj);
    if (player) {
        return player->getDuration();
    }
    return 0;
}