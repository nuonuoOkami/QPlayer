#include <jni.h>
#include <string>
#include "player/log4c.h"
extern "C"{
#include <libavutil/avutil.h>
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_leo_qplayer_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello ="版本号为";
    hello.append(av_version_info());
    return env->NewStringUTF(hello.c_str());
}