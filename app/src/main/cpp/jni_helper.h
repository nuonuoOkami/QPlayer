//
// Created by leo on 2022/10/17.
//

#ifndef QPLAYER_JNI_HELPER_H
#define QPLAYER_JNI_HELPER_H

#include "util.h"
#include <jni.h>

class JniHelper {
private:
    JavaVM *vm = 0;//跨线程使用
    JNIEnv *env = 0;//主线程
    jobject job=0;//承载播放器实例

    jmethodID jmd_prepared;//预加载
    jmethodID jmd_onError;//异常
    jmethodID jmd_onProgress; // 播放音频的时间回调

public:
    JniHelper(JavaVM *vm, JNIEnv *env, jobject j);

    ~JniHelper();

    void onPrepared(int thread_mode);

    void onError(int thread_mode, int error_code, char *ffmpegError);

    void onProgress(int thread_mode, int time);


};


#endif //QPLAYER_JNI_HELPER_H
