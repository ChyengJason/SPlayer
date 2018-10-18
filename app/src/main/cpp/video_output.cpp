//
// Created by 橙俊森 on 2018/10/15.
//

#include "video_output.h"
#include "android_log.h"

extern "C" {
#include <unistd.h>
}

VideoOutput::VideoOutput() {
    mNativeWindow = NULL;
}

VideoOutput::~VideoOutput() {
    pthread_cond_destroy(&mRenderHandlerCond);
    pthread_mutex_destroy(&mRenderHandlerMutex);
}

void VideoOutput::onCreated(ANativeWindow *nativeWindow) {
    mNativeWindow = nativeWindow;
    LOGE("当前主线程：%lu", (unsigned long)pthread_self());
    createRenderHandlerThread();
    postMessage(MESSAGE_CREATE_CONTEXT);
}

void VideoOutput::onChangeSize(int width, int height) {
    screenHeight = height;
    screenWidth = width;
    postMessage(MESSAGE_CHANGE_SIZE);
}

void VideoOutput::onDestroy() {
    postMessage(MESSAGE_QUIT);
}

void VideoOutput::createRenderHandlerThread() {
    pthread_mutex_init(&mRenderHandlerMutex, NULL);
    pthread_cond_init(&mRenderHandlerCond, NULL);
    pthread_create(&mRenderHandlerThread, NULL, renderHandlerThread, this);
}

void VideoOutput::output(VideoFrame &videoFrame) {
    if (videoFrame.width <= 0 || videoFrame.height <= 0) {
        return;
    }
    postMessage(MESSAGE_RENDER);
}

void *VideoOutput::renderHandlerThread(void *self) {
    LOGE("当前Handler线程：%lu", (unsigned long)pthread_self());
    VideoOutput* mOutput = (VideoOutput*) self;
    // handler处理消息
    mOutput->processMessages();
    pthread_exit(0);
    return NULL;
}

bool VideoOutput::postMessage(Message msg) {
    int lockCode = pthread_mutex_lock(&mRenderHandlerMutex);
    if (lockCode != 0) {
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
        if (mHandlerMessageQueue.empty()) {
            LOGE("MESSAGE 等待");
            pthread_cond_wait(&mRenderHandlerCond, &mRenderHandlerMutex);
            LOGE("MESSAGE 结束唤醒");
            pthread_mutex_unlock(&mRenderHandlerMutex);
            continue;
        }
        Message msg = mHandlerMessageQueue.front();
        mHandlerMessageQueue.pop();
        LOGE("MESSAGE %d", msg.msgType);
        switch (msg.msgType) {
            case MESSAGE_CREATE_CONTEXT:
                createEglContextHandler();
                break;
            case MESSAGE_RENDER:
                renderTextureHandler(msg.value);
                break;
            case MESSAGE_CHANGE_SIZE:
                changeSizeHanlder();
                break;
            case MESSAGE_QUIT:
                releaseRenderHanlder();
                isQuited = false;
                break;
            default:
                break;
        }
        pthread_mutex_unlock(&mRenderHandlerMutex);
    }
    LOGE("processMessages 结束");
}

void VideoOutput::createEglContextHandler() {
    mEglCore.createGL();
    mSurface = mEglCore.createWindowSurface(mNativeWindow);
    mEglCore.makeCurrent(mSurface);
    mGlRender.onCreated();

}

void VideoOutput::releaseRenderHanlder() {
    mGlRender.onDestroy();
    mEglCore.destroySurface(mSurface);
    mEglCore.destroyGL();
    ANativeWindow_release(mNativeWindow);
}

void VideoOutput::renderTextureHandler(int textureId) {
    mEglCore.makeCurrent(mSurface);
    LOGE("渲染 %d", textureId);
    //int textureId = 0;
    //mGlRender.draw(textureId);
    mEglCore.swapBuffers(mSurface);
}

void VideoOutput::changeSizeHanlder() {
    mGlRender.onChangeSize(screenWidth, screenHeight);
}

EGLContext VideoOutput::getShareContext() {
    return mEglCore.getShareContext();
}
