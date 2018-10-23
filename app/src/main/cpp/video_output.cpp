//
// Created by 橙俊森 on 2018/10/15.
//

#include "video_output.h"
#include "android_log.h"
#include "egl/egl_share_context.h"

extern "C" {
#include <unistd.h>
}

VideoOutput::VideoOutput() {
    mNativeWindow = NULL;
    mSurface = EGL_NO_SURFACE;
    isInited = false;
}

VideoOutput::~VideoOutput() {
    pthread_cond_destroy(&mRenderHandlerCond);
    pthread_mutex_destroy(&mRenderHandlerMutex);
}

void VideoOutput::start() {
    LOGE("当前主线程：%lu", (unsigned long)pthread_self());
    createRenderHandlerThread();
    isInited = true;
    postMessage(MESSAGE_CREATE_CONTEXT);
}

void VideoOutput::finish() {
    postMessage(MESSAGE_QUIT);
    isInited = false;
}

void VideoOutput::onCreated(ANativeWindow *nativeWindow) {
    if (!isInited) {
        start();
    }
    mNativeWindow = nativeWindow;
    mSurface = EGL_NO_SURFACE;
    postMessage(MESSAGE_CREATE_SURFACE);
}

void VideoOutput::onUpdated(ANativeWindow *nativeWindow) {
    mNativeWindow = nativeWindow;
    postMessage(MESSAGE_UPDATE_SURFACE);
}

void VideoOutput::onChangeSize(int width, int height) {
    screenHeight = height;
    screenWidth = width;
    postMessage(MESSAGE_CHANGE_SIZE);
}

void VideoOutput::onDestroy() {
    postMessage(MESSAGE_DESTROY_SURFACE);
}

void VideoOutput::createRenderHandlerThread() {
    pthread_mutex_init(&mRenderHandlerMutex, NULL);
    pthread_cond_init(&mRenderHandlerCond, NULL);
    int ret = pthread_create(&mRenderHandlerThread, NULL, renderHandlerThread, this);
    if (ret < 0) {
        LOGE("createRenderHandlerThread fail");
    }
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
            case MESSAGE_CREATE_SURFACE:
                createSurfaceHandler();
                break;
            case MESSAGE_UPDATE_SURFACE:
                updateSurfaceHandler();
                break;
            case MESSAGE_DESTROY_SURFACE:
                destroySurfaceHandler();
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
    EGLContext context = mEglCore.createGL(EglShareContext::getShareContext());
    EglShareContext::setShareContext(context);
}

void VideoOutput::createSurfaceHandler() {
    mSurface = mEglCore.createWindowSurface(mNativeWindow);
    mEglCore.makeCurrent(mSurface, EglShareContext::getShareContext());
    mGlRender.onCreated();
}

void VideoOutput::updateSurfaceHandler() {
    mGlRender.onDestroy();
    mEglCore.destroySurface(mSurface);
    mSurface = EGL_NO_SURFACE;
    mSurface = mEglCore.createWindowSurface(mNativeWindow);
    mEglCore.makeCurrent(mSurface, EglShareContext::getShareContext());
    mGlRender.onCreated();
}

void VideoOutput::destroySurfaceHandler() {
    mGlRender.onDestroy();
    mEglCore.destroySurface(mSurface);
    mSurface = EGL_NO_SURFACE;
    ANativeWindow_release(mNativeWindow);
}

void VideoOutput::releaseRenderHanlder() {
    mEglCore.destroyGL(EglShareContext::getShareContext());
}

void VideoOutput::renderTextureHandler(int textureId) {
    //mEglCore.makeCurrent(mSurface, EglShareContext::getShareContext());
    LOGE("渲染 %d", textureId);
    //int textureId = 0;
    //mGlRender.draw(textureId);
    //mEglCore.swapBuffers(mSurface);
}

void VideoOutput::changeSizeHanlder() {
    mGlRender.onChangeSize(screenWidth, screenHeight);
}

bool VideoOutput::isSurfaceValid() {
    return mSurface != EGL_NO_SURFACE;
}
