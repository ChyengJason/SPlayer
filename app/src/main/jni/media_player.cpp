//
// Created by chengjunsen on 2018/10/12.
//
#include <jni.h>
#include <string>
#include "../cpp/android_log.h"
#include "../cpp/media_decoder.h"

MediaDecoder* mVideoDecoder = NULL;

extern "C"
JNIEXPORT void JNICALL Java_com_jscheng_splayer_player_VideoPlayer_printConfig(JNIEnv *env, jobject instance) {
    std::string config = avcodec_configuration();
    LOGE(" 测试打印 配置信息 %s", config.c_str());
}

extern "C"
JNIEXPORT void JNICALL Java_com_jscheng_splayer_player_VideoPlayer_prepare(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    mVideoDecoder = new MediaDecoder(path);
    mVideoDecoder->start();

    env->ReleaseStringUTFChars(path_, path);
}