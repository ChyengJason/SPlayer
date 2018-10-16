//
// Created by chengjunsen on 2018/10/12.
//
#include <jni.h>
#include <string>
#include "../cpp/media_synchronizer.h"
#include "../cpp/video_output.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include "../cpp/android_log.h"
}

extern "C"
JNIEXPORT void JNICALL Java_com_jscheng_splayer_player_VideoPlayer_printConfig(JNIEnv *env, jobject instance) {
    std::string config = avcodec_configuration();
    LOGE(" 测试打印 配置信息 %s", config.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jscheng_splayer_player_VideoPlayer_prepare(JNIEnv *env, jobject instance, jstring path_,
                                                    jobject surface) {
    const char *path = env->GetStringUTFChars(path_, 0);
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    MediaSynchronizer *mSychronizer = new MediaSynchronizer;
    VideoOutput* mVideoOutput = new VideoOutput;
    mSychronizer->prepare(path);
    mVideoOutput->onSurfaceCreated(nativeWindow, 1920, 1080);
    mSychronizer->setVideoOutput(mVideoOutput);
    mSychronizer->start();
    //mSychronizer->finish();
    //delete mSychronizer;
    //delete mVideoOutput;
    env->ReleaseStringUTFChars(path_, path);

}