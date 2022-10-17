//
// Created by leo on 2022/10/17.
//

#include "jni_helper.h"

void JniHelper::onError(int thread_mode, int error_code, char *ffmpegError) {


}

void JniHelper::onPrepared(int thread_mode) {

}

void JniHelper::onProgress(int thread_mode, int time) {}

JniHelper::JniHelper(JavaVM *vm, JNIEnv *env, jobject j) {
    this->vm = vm;
    this->env = env;
    this->job = env->NewGlobalRef(j);
    jclass player = env->GetObjectClass(j);
    //todo 回调方法 暂时没开发
}

JniHelper::~JniHelper() {

    vm = nullptr;
    env->DeleteGlobalRef(job);
    env = nullptr;
    job = nullptr;


}