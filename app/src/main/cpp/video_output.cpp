//
// Created by 橙俊森 on 2018/10/15.
//

#include "video_output.h"
#include "util/android_log.h"
#include "egl/egl_share_context.h"

extern "C" {
#include <unistd.h>
}

VideoOutput::VideoOutput(IVideoOutput *callback) {
    pthread_mutex_init(&mRenderMutex, NULL);
    pthread_mutex_init(&mMessageMutex, NULL);
    pthread_cond_init(&mRenderCond, NULL);
    mNativeWindow = NULL;
    mSurface = EGL_NO_SURFACE;
    mContext = EGL_NO_CONTEXT;
    isThreadInited = false;
    mOutputInterface = callback;
}

VideoOutput::~VideoOutput() {
    pthread_cond_destroy(&mRenderCond);
    pthread_mutex_destroy(&mRenderMutex);
    pthread_mutex_destroy(&mMessageMutex);
    mOutputInterface = NULL;
}

void VideoOutput::onCreated(ANativeWindow *nativeWindow) {
    if (!isThreadInited) {
        LOGD("VideoOutput onCreated 当前主线程：%lu size %d", (unsigned long)pthread_self(), mMessageQueue.size());
        mNativeWindow = nativeWindow;
        createRenderHandlerThread();
        isThreadInited = true;
        postMessage(MESSAGE_CREATE_CONTEXT);
        postMessage(MESSAGE_RENDER);
    }
}

void VideoOutput::onChangeSize(int screenWidth, int screenHeight) {
    LOGE("VideoOutput chagesize");
    if (!isThreadInited) {
        return;
    }
    this->screenHeight = screenHeight;
    this->screenWidth = screenWidth;
    postMessage(MESSAGE_CHANGE_SIZE);
}

void VideoOutput::onDestroy() {
    LOGE("VideoOutput onDestroy");
    if (!isThreadInited) {
        return;
    }
    isThreadInited = false;
    postMessage(MESSAGE_QUIT);
    pthread_detach(mRenderHandlerThread);
}

void VideoOutput::createRenderHandlerThread() {
    int ret = pthread_create(&mRenderHandlerThread, NULL, renderHandlerThread, this);
    if (ret < 0) {
        LOGE("createRenderHandlerThread fail");
    }
}

void VideoOutput::signalRenderFrame() {
    postMessage(MESSAGE_RENDER);
}

void *VideoOutput::renderHandlerThread(void *self) {
    VideoOutput* mOutput = (VideoOutput*) self;
    // handler处理消息
    mOutput->processMessages();
    pthread_exit(0);
    return NULL;
}

void VideoOutput::postMessage(VideoOutputMessage msg) {
    pthread_mutex_lock(&mMessageMutex);
    mMessageQueue.push(msg);
    LOGE("mMessageQueue.push %d",msg);
    pthread_mutex_unlock(&mMessageMutex);
    pthread_cond_signal(&mRenderCond);
}

void VideoOutput::processMessages() {
    bool isQuited = true;
    while (isQuited) {
        int lockCode = pthread_mutex_lock(&mRenderMutex);
        if (lockCode != 0) {
            LOGE("VideoOutput processMessages 失败");
            return ;
        }

        if (mMessageQueue.empty()) {
            LOGE("VideoOutput MESSAGE 等待");
            pthread_cond_wait(&mRenderCond, &mRenderMutex);
            LOGE("VideoOutput MESSAGE 结束唤醒 %d", mMessageQueue.size());
            pthread_mutex_unlock(&mRenderMutex);
            continue;
        }

        pthread_mutex_lock(&mMessageMutex);
        VideoOutputMessage msg = mMessageQueue.front();
        mMessageQueue.pop();
        pthread_mutex_unlock(&mMessageMutex);
        LOGE("VideoOutput message: %d", msg);
        switch (msg) {
            case MESSAGE_CREATE_CONTEXT:
                createContextHandler();
                break;
            case MESSAGE_RENDER:
                renderTextureHandler();
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
        pthread_mutex_unlock(&mRenderMutex);
    }
    LOGD("VideoOutput processMessages 结束");
    pthread_exit(0);
}

void VideoOutput::createContextHandler() {
    EglShareContext::getInstance().lock();
    mContext = mEglCore.createGL(EglShareContext::getInstance().getShareContext());
    EglShareContext::getInstance().setShareContext(mContext);
    EglShareContext::getInstance().unlock();
    mSurface = mEglCore.createWindowSurface(mNativeWindow);
    mEglCore.makeCurrent(mSurface, mContext);
    mGlRender.onCreated();
    LOGD(" VideoOutput::createEglContextHandler finish");
}

void VideoOutput::releaseRenderHanlder() {
    LOGD("VideoOutput destroySurfaceHandler");
    mGlRender.onDestroy();
    mEglCore.destroySurface(mSurface);
    mSurface = EGL_NO_SURFACE;
    mEglCore.destroyGL(mContext);
    EglShareContext::getInstance().clearShareContext();
    ANativeWindow_release(mNativeWindow);
    mNativeWindow = NULL;

    pthread_mutex_lock(&mMessageMutex);
    while (!mMessageQueue.empty()) {
        mMessageQueue.pop();
    }
    pthread_mutex_unlock(&mMessageMutex);
}

void VideoOutput::renderTextureHandler() {
    TextureFrame* textureFrame = mOutputInterface->getTetureFrame();
    if (textureFrame == NULL) {
        LOGE("renderTextureHandler 为空");
        return;
    }
    mEglCore.makeCurrent(mSurface, mContext);
    if (!textureFrame->isSkip) {
        LOGD("VideoOutput 渲染纹理 %d，屏幕尺寸 %d x %d", textureFrame->textureId, screenWidth, screenHeight);
        mGlRender.onDraw(textureFrame->textureId);
        mEglCore.swapBuffers(mSurface);
    } else {
        LOGE("videouOutput 跳帧");
    }
    GlRenderUtil::deleteTexture(textureFrame->textureId);
    delete textureFrame;
}

void VideoOutput::changeSizeHanlder() {
    LOGD("VideoOutput changeSizeHanlder");
    mGlRender.onChangeSize(screenWidth, screenHeight);
}

bool VideoOutput::isRunning() {
    return isThreadInited;
}

