//
// Created by 橙俊森 on 2018/10/15.
//

#include "video_output.h"
#include "android_log.h"
#include "egl/egl_share_context.h"

extern "C" {
#include <unistd.h>
}

int SaveYuv(unsigned char *buf, int width, int height, char *filename)
{
    FILE *f;
    int i;

    f = fopen(filename, "ab+");
    for (i = 0; i < height; i++)
    {
        fwrite(buf + i * width, 1, width, f);
    }
    fclose(f);
    return 1;
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
    if (!isInited) {
        createRenderHandlerThread();
        isInited = true;
        postMessage(MESSAGE_CREATE_CONTEXT);
    }
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

void VideoOutput::onChangeSize(int screenWidth, int screenHeight) {
    this->screenHeight = screenHeight;
    this->screenWidth = screenWidth;
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

void VideoOutput::output(VideoFrame* frame) {
    postMessage(Message(MESSAGE_RENDER, frame));
    char * mPath = "/storage/emulated/0/meida.yuv";
    SaveYuv(frame->luma, frame->frameWidth, frame->frameHeight, mPath);
    SaveYuv(frame->chromaB, frame->frameWidth/2, frame->frameHeight/2, mPath);
    SaveYuv(frame->chromaR, frame->frameWidth/2, frame->frameHeight/2, mPath);

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
                renderTextureHandler((VideoFrame*)msg.value);
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

void VideoOutput::renderTextureHandler(VideoFrame* videoFrame) {
    LOGE("VideoOutput 屏幕尺寸 %d x %d", screenWidth, screenHeight);
    if (videoFrame != NULL) {
        mEglCore.makeCurrent(mSurface, EglShareContext::getShareContext());
//        mGlRender.onDraw();
        mGlRender.onDraw(videoFrame);
        mEglCore.swapBuffers(mSurface);
//        // 删除textureId
//        GlRenderUtil::deleteTexture(textureFrame->textureId);
//        delete(videoFrame);
    }
}

void VideoOutput::changeSizeHanlder() {
    mEglCore.makeCurrent(mSurface, EglShareContext::getShareContext());
    mGlRender.onChangeSize(screenWidth, screenHeight);
}

bool VideoOutput::isSurfaceValid() {
    return mSurface != EGL_NO_SURFACE;
}
