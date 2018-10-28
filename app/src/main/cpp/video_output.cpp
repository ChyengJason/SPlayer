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
    pthread_cond_init(&mRenderCond, NULL);
    mNativeWindow = NULL;
    mSurface = EGL_NO_SURFACE;
    isThreadInited = false;
    mOutputInterface = callback;
}

VideoOutput::~VideoOutput() {
    pthread_cond_destroy(&mRenderCond);
    pthread_mutex_destroy(&mRenderMutex);
    mOutputInterface = NULL;
}

void VideoOutput::onCreated(ANativeWindow *nativeWindow) {
    if (!isThreadInited) {
        LOGD("VideoOutput onCreated 当前主线程：%lu", (unsigned long)pthread_self());
        mNativeWindow = nativeWindow;
        createRenderHandlerThread();
        postMessage(MESSAGE_CREATE_CONTEXT);
        isThreadInited = true;
    }
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
    if (!isThreadInited) {
        LOGE("VideoOutput output 還未初始化");
        return;
    }
    postMessage(MESSAGE_RENDER);
}

void VideoOutput::output(void* frame) {

}

void *VideoOutput::renderHandlerThread(void *self) {
    VideoOutput* mOutput = (VideoOutput*) self;
    // handler处理消息
    mOutput->processMessages();
    pthread_exit(0);
    return NULL;
}

void VideoOutput::postMessage(VideoOutputMessage msg) {
    mMessageQueue.push(msg);
    pthread_cond_signal(&mRenderCond);
}

void VideoOutput::processMessages() {
    bool isQuited = true;
    while (isQuited) {
        int lockCode = pthread_mutex_lock(&mRenderMutex);
        if (lockCode != 0) {
            LOGE("processMessages 失败");
            return ;
        }
        if (mMessageQueue.isEmpty()) {
            LOGD("VideoOutput MESSAGE 等待");
            pthread_cond_wait(&mRenderCond, &mRenderMutex);
            LOGD("VideoOutput MESSAGE 结束唤醒 %d", mMessageQueue.size());
            pthread_mutex_unlock(&mRenderMutex);
            continue;
        }
        VideoOutputMessage msg = mMessageQueue.pop();
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
    LOGD(" VideoOutput::createEglContextHandler");
    EGLContext context = mEglCore.createGL(EglShareContext::getShareContext());
    EglShareContext::setShareContext(context);
    mSurface = mEglCore.createWindowSurface(mNativeWindow);
    mEglCore.makeCurrent(mSurface, EglShareContext::getShareContext());
    mGlRender.onCreated();
    //LOGD(" VideoOutput::createEglContextHandler finish");
}

void VideoOutput::releaseRenderHanlder() {
    LOGD("VideoOutput destroySurfaceHandler");
    mGlRender.onDestroy();
    mEglCore.destroySurface(mSurface);
    mSurface = EGL_NO_SURFACE;
    mEglCore.destroyGL(EglShareContext::getShareContext());
    EglShareContext::setShareContext(EGL_NO_CONTEXT);
    ANativeWindow_release(mNativeWindow);
    mNativeWindow = NULL;

    while (!mMessageQueue.isEmpty()) {
        VideoOutputMessage message = mMessageQueue.pop();
    }
}

void VideoOutput::renderTextureHandler() {
    TextureFrame* textureFrame = mOutputInterface->getTetureFrame();
    if (textureFrame == NULL) {
        return;
    }
    LOGD("VideoOutput 渲染纹理 %d，屏幕尺寸 %d x %d", textureFrame->textureId, screenWidth, screenHeight);
    mEglCore.makeCurrent(mSurface, EglShareContext::getShareContext());
    mGlRender.onDraw(textureFrame->textureId);
    mEglCore.swapBuffers(mSurface);
    GlRenderUtil::deleteTexture(textureFrame->textureId);
    delete textureFrame;
}

void VideoOutput::changeSizeHanlder() {
    LOGD("VideoOutput changeSizeHanlder");
    mGlRender.onChangeSize(screenWidth, screenHeight);
}

bool VideoOutput::isSurfaceValid() {
    return mSurface != EGL_NO_SURFACE;
}
