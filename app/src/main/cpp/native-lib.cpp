#include <jni.h>
#include <string>
#include "log4c.h"

extern "C" {
#include <libavutil/avutil.h>
}

#include <android/native_window_jni.h>

#include "safe_queue.h"
#include "QPlayer.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_leo_qplayer_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "版本号为";
    hello.append(av_version_info());

    SafeQueue<int> safeQueue = SafeQueue<int>();
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_leo_qplayer_MainActivity_player(JNIEnv *env, jobject thiz, jstring path) {

}

JavaVM *vm = nullptr;
ANativeWindow *window = nullptr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 静态初始化 锁


void renderFrame(uint8_t *src_data, int width, int height, int src_linesize) {

}

/**
 * 初始化vm
 * @param vm
 * @param args
 * @return
 */
jint JNI_OnLoad(JavaVM * vm, void * args) {
    ::vm = vm;
    return JNI_VERSION_1_6;
}

/**
 * 设置资源路径
 */
extern "C"
JNIEXPORT jlong JNICALL
Java_com_leo_qplayer_QPlayer_prepareNative(JNIEnv *env, jobject job, jstring data_source) {

    const char *path = env->GetStringUTFChars(data_source, nullptr);
    auto *helper = new JniHelper(vm, env, job);
    auto *pPlayer = new QPlayer(path, helper);
    pPlayer->setRenderCallback(renderFrame);
    pPlayer->prepare();
    //释放掉资源
    env->ReleaseStringUTFChars(data_source,path);
    return reinterpret_cast<jlong>(pPlayer);
}

/**
 * 开始播放
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_leo_qplayer_QPlayer_startNative(JNIEnv *env, jobject thiz, jlong native_obj) {
    // TODO: implement startNative()
}


/**
 * 停止播放
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_leo_qplayer_QPlayer_stopNative(JNIEnv *env, jobject thiz, jlong native_obj) {
    // TODO: implement stopNative()
}

/**
 * 设置surface 到cpp
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_leo_qplayer_QPlayer_setSurfaceNative(JNIEnv *env, jobject thiz, jobject surface,
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