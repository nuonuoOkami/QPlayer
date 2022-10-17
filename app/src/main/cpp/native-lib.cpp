#include <jni.h>
#include <string>
#include "log4c.h"

extern "C" {
#include <libavutil/avutil.h>
}

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

/**
 * 设置资源路径
 */
extern "C"
JNIEXPORT jlong JNICALL
Java_com_leo_qplayer_QPlayer_prepareNative(JNIEnv *env, jobject thiz, jstring data_source) {

     const char *path = env->GetStringUTFChars(data_source, nullptr);
     new QPlayer()
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