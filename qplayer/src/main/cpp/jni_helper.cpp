//
// Created by leo on 2022/10/17.
//

#include "jni_helper.h"
#include "log4c.h"

void JniHelper::onError(int thread_mode, int error_code, char *ffmpegError) {

    //主线程直接回调
    if (thread_mode == THREAD_MAIN) {
        jstring ffmpegError_ = env->NewStringUTF(ffmpegError);
        env->CallVoidMethod(job, jmd_onError, error_code, ffmpegError_);
    } else{
        //子线程要使用自己的env;
        JNIEnv *env_child;
        vm->AttachCurrentThread(&env_child, nullptr);
        jstring ffmpegError_ = env_child->NewStringUTF(ffmpegError);
        env_child->CallVoidMethod(job, jmd_onError, error_code, ffmpegError_);
        vm->DetachCurrentThread();
    }

}

void JniHelper::onPrepared(int thread_mode) {
    if (thread_mode == THREAD_MAIN) {
        // 主线程：直接调用即可
        env->CallVoidMethod(job, jmd_prepared);
    } else if (thread_mode == THREAD_CHILD) {
        // 子线程 env也不可以跨线程吧 对的   全新的env   子线程 必须用 JavaVM 子线程中附加出来的新 子线程专用env
        JNIEnv * env_child;
        vm->AttachCurrentThread(&env_child, nullptr);
        env_child->CallVoidMethod(job, jmd_prepared);
        vm->DetachCurrentThread();
    }
}

void JniHelper::onProgress(int thread_mode, int time) {
    if (thread_mode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(job, jmd_onProgress, time);
    } else {
        //子线程
        //当前子线程的 JNIEnv
        JNIEnv *env_child;
        vm->AttachCurrentThread(&env_child, nullptr);
        env_child->CallVoidMethod(job, jmd_onProgress, time);
        vm->DetachCurrentThread();
    }
}

JniHelper::JniHelper(JavaVM *vm, JNIEnv *env, jobject j) {
    this->vm = vm;
    this->env = env;
    this->job = env->NewGlobalRef(j);//提升全局
    jclass player = env->GetObjectClass(j);
    //准备方法
    jmd_prepared = env->GetMethodID(player, "onPrepared", "()V");
    //进度监听
    jmd_onProgress = env->GetMethodID(player, "onProgress", "(I)V");
    //报错监听
    jmd_onError = env->GetMethodID(player, "onError", "(ILjava/lang/String;)V");

}

JniHelper::~JniHelper() {

    vm = nullptr;
    env->DeleteGlobalRef(job);
    env = nullptr;
    job = nullptr;


}