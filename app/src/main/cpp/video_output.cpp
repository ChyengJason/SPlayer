//
// Created by 橙俊森 on 2018/10/15.
//

#include "video_output.h"
extern "C" {
#include "android_log.h"
}

extern "C" {
#include <unistd.h>
}

VideoOutput::VideoOutput() {
    mNativeWindow = NULL;
    mEglCore = NULL;
    mRenderHandlerMutex = NULL;
    mRenderHandlerCond = NULL;
    mSurface = NULL;
}

VideoOutput::~VideoOutput() {
    if (mEglCore) {
        delete(mEglCore);
        mEglCore = NULL;
    }
    pthread_cond_destroy(&mRenderHandlerCond);
    pthread_mutex_destroy(&mRenderHandlerMutex);
}

void VideoOutput::onSurfaceCreated(ANativeWindow *nativeWindow, int screenHeight, int screenWidth) {
    mNativeWindow = nativeWindow;
    createRenderHandlerThread();
}

void VideoOutput::onSurfaceDestroy() {
    stop();
}

void VideoOutput::stop() {
    postMessage(MESSAGE_QUIT);
}

void VideoOutput::createRenderHandlerThread() {
    pthread_mutex_init(&mRenderHandlerMutex, NULL);
    pthread_cond_init(&mRenderHandlerCond, NULL);
    pthread_create(&mRenderHandlerThread, 0, renderHandlerThread, this);
}

void VideoOutput::output(const VideoFrame &videoFrame) {
    if (videoFrame.width <= 0 || videoFrame.height <= 0) {
        return;
    }
}

void *VideoOutput::renderHandlerThread(void *self) {
    VideoOutput* mOutput = (VideoOutput*) self;
    // handler处理消息
    mOutput->processMessages();
    pthread_exit(0);
    return NULL;
}

bool VideoOutput::postMessage(Message msg) {
    int lockCode = pthread_mutex_lock(&mRenderHandlerMutex);
    if (lockCode != 0) {
        LOGE("postMessage lock 失败");
        return false;
    }
    mHandlerMessageQueue.push(msg);
    pthread_cond_signal(&mRenderHandlerCond);
    pthread_mutex_unlock(&mRenderHandlerMutex);
    return true;
}

void VideoOutput::processMessages() {
    bool isQuited = true;
    while (isQuited) {
        int lockCode = pthread_mutex_lock(&mRenderHandlerMutex);
        if (lockCode != 0) {
            LOGE("processMessages 失败");
            return ;
        }
        Message msg = dequeueHandlerMessage();
        if (msg == MESSAGE_NONE) {
            LOGE("MESSAGE_NONE 等待");
            pthread_cond_wait(&mRenderHandlerCond, &mRenderHandlerMutex);
            LOGE("MESSAGE_NONE 结束唤醒");
        } else if (msg == MESSAGE_CREATE_CONTEXT) {
            LOGE("MESSAGE_CREATE_CONTEXT");
            createHandlerEglContext();
        } else if (msg == MESSAGE_QUIT) {
            LOGE("MESSAGE_QUIT");
            releaseRenderHanlder();
            isQuited = true;
        }
        pthread_mutex_unlock(&mRenderHandlerMutex);
    }
}

Message VideoOutput::dequeueHandlerMessage() {
    int lockCode = pthread_mutex_lock(&mRenderHandlerMutex);
    if (lockCode != 0) {
        LOGE("postMessage lock 失败");
        return MESSAGE_NONE;
    }
    Message msg;
    if (mHandlerMessageQueue.empty()) {
        msg = MESSAGE_NONE;
    } else {
        msg = mHandlerMessageQueue.front();
        mHandlerMessageQueue.pop();
    }
    pthread_mutex_unlock(&mRenderHandlerMutex);
    return msg;
}

void VideoOutput::createHandlerEglContext() {
    mEglCore = new EglCore;
    mEglCore->createGL();
    mSurface = mEglCore->createWindowSurface(mNativeWindow);
    mEglCore->makeCurrent(mSurface);
}

void VideoOutput::releaseRenderHanlder() {
    mEglCore->destroySurface(mSurface);
    mEglCore->destroyGL();
    delete mEglCore ;
    mEglCore = NULL;
    ANativeWindow_release(mNativeWindow);
}

