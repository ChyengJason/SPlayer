//
// Created by chengjunsen on 2018/10/12.
//
#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/bitmap.h>
#include "../cpp/media_player_controller.h"
#include "../cpp/util/android_log.h"

MediaPlayerController* mPlayerController = new MediaPlayerController;

extern "C"
JNIEXPORT void JNICALL Java_com_jscheng_splayer_player_VideoPlayer_printConfig(JNIEnv *env, jobject instance) {
    std::string config = avcodec_configuration();
    LOGD(" 测试打印 配置信息 %s", config.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_onSurfaceCreated(JNIEnv *env, jobject instance,
                                                             jobject surface) {
    LOGD("jni onSurfaceCreated");
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    mPlayerController->onSurfaceCreated(nativeWindow);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_onSurfaceSizeChanged(JNIEnv *env, jobject instance,
                                                                 jint screenWidth, jint screenHeight) {
    LOGD("jni onSurfaceSizeChanged");
    mPlayerController->onSurfaceSizeChanged(screenWidth, screenHeight);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_onSurfaceDestroy(JNIEnv *env, jobject instance) {
    LOGD("jni onSurfaceDestroy");
    mPlayerController->onSurfaceDestroy();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_start(JNIEnv *env, jobject instance, jstring path_) {
    LOGD("jni start");
    const char *path = env->GetStringUTFChars(path_, 0);
    mPlayerController->start(path);
    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_stop(JNIEnv *env, jobject instance) {
    LOGD("jni stop");
    mPlayerController->stop();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_seek(JNIEnv *env, jobject instance, jdouble position) {
    LOGD("jni seek");
    mPlayerController->seek(position);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_pause(JNIEnv *env, jobject instance) {
    LOGD("jni pause");
    mPlayerController->pause();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_resume(JNIEnv *env, jobject instance) {
    LOGD("jni resume");
    mPlayerController->resume();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_getDuration(JNIEnv *env, jobject instance) {
    LOGD("jni getDuration");
    return mPlayerController->getDuration();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_getProgress(JNIEnv *env, jobject instance) {
    LOGD("jni getProgress");
    return mPlayerController->getProgress();
}