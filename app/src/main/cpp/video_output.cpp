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
    isThreadInited = false;
}

VideoOutput::~VideoOutput() {
    pthread_cond_destroy(&mRenderHandlerCond);
    pthread_mutex_destroy(&mRenderHandlerMutex);
}

void VideoOutput::start() {
    LOGE("当前主线程：%lu", (unsigned long)pthread_self());
    createRenderHandlerThread();
    isThreadInited = true;
    postMessage(MESSAGE_CREATE_CONTEXT);
}

void VideoOutput::finish() {
    postMessage(MESSAGE_QUIT);
    isThreadInited = false;
}

void VideoOutput::onCreated(ANativeWindow *nativeWindow) {
    if (!isThreadInited) {
        start();
    }
    mNativeWindow = nativeWindow;
    mSurface = EGL_NO_SURFACE;
    postMessage(MESSAGE_CREATE_SURFACE);
}

void VideoOutput::onChangeSize(int screenWidth, int screenHeight) {
    if (!isThreadInited) {
        return;
    }
    this->screenHeight = screenHeight;
    this->screenWidth = screenWidth;
    postMessage(MESSAGE_CHANGE_SIZE);
}

void VideoOutput::onDestroy() {
    if (!isThreadInited) {
        return;
    }
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

void VideoOutput::output(void* frame) {
    postMessage(Message(MESSAGE_RENDER, frame));
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
            LOGE("VideoOutput MESSAGE 等待");
            pthread_cond_wait(&mRenderHandlerCond, &mRenderHandlerMutex);
            LOGE("VideoOutput MESSAGE 结束唤醒");
            pthread_mutex_unlock(&mRenderHandlerMutex);
            continue;
        }
        Message msg = mHandlerMessageQueue.front();
        mHandlerMessageQueue.pop();
        LOGE("VideoOutput MESSAGE %d", msg.msgType);
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
    mNativeWindow = NULL;
}

void VideoOutput::releaseRenderHanlder() {
    mEglCore.destroyGL(EglShareContext::getShareContext());
}

void VideoOutput::renderTextureHandler(void* frame) {
//    LOGE("VideoOutput 渲染纹理 %d，屏幕尺寸 %d x %d", textureFrame->textureId, screenWidth, screenHeight);
//    if (textureFrame->textureId >= 0) {
//        mEglCore.makeCurrent(mSurface, EglShareContext::getShareContext());
//        mGlRender.onDraw(textureFrame->textureId);
//        mEglCore.swapBuffers(mSurface);
        // 删除textureId
//        GlRenderUtil::deleteTexture(textureFrame->textureId);
//        delete(textureFrame);
//    }
    LOGE("renderTextureHandler");
    mEglCore.makeCurrent(mSurface, EglShareContext::getShareContext());
    mGlRender.onDraw(((TextureFrame*)frame)->textureId);
    mEglCore.swapBuffers(mSurface);
}

void VideoOutput::changeSizeHanlder() {
    mGlRender.onChangeSize(screenWidth, screenHeight);
}

bool VideoOutput::isSurfaceValid() {
    return mSurface != EGL_NO_SURFACE;
}
