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
    mNativeWindow = NULL;
    mSurface = EGL_NO_SURFACE;
    mContext = EGL_NO_CONTEXT;
    isCreated = false;
    isChangeSized = false;
    isDestroy = false;
    mOutputInterface = callback;
    screenWidth = 0;
    screenHeight = 0;
}

VideoOutput::~VideoOutput() {
    mOutputInterface = NULL;
}

void VideoOutput::onCreated(ANativeWindow *nativeWindow) {
    mNativeWindow = nativeWindow;
    createRunThread();
    isCreated = true;
    isDestroy = false;
}

void VideoOutput::onChangeSize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    isChangeSized = true;
}

void VideoOutput::onDestroy() {
    isDestroy = true;
    isCreated = false;
}

bool VideoOutput::isRunning() {
    return isCreated && !isDestroy;
}

void VideoOutput::createRunThread() {
    int ret = pthread_create(&mRenderHandlerThread, NULL, runHandler, this);
    if (ret < 0) {
        LOGE("VideoOutput createRunThread fail");
    }
}

void *VideoOutput::runHandler(void *self) {
    VideoOutput* mOutput = (VideoOutput*) self;
    mOutput->runHandlerImpl();
    pthread_exit(0);
    return NULL;
}

void VideoOutput::runHandlerImpl() {
    createContextHandler();
    changeSizeHandler();
    while (!isDestroy) {
        VideoFrame* videoFrame = mOutputInterface->getVideoFrame(); // 会阻塞
        renderVideoFrameHandler(videoFrame);
    }
    releaseContextHandler();
    pthread_exit(0);
}

void VideoOutput::createContextHandler() {
    mContext = mEglCore.createGL(EGL_NO_CONTEXT);
    mSurface = mEglCore.createWindowSurface(mNativeWindow);
    mEglCore.makeCurrent(mSurface, mContext);
    mGlRender.onCreated();
}

void VideoOutput::releaseContextHandler() {
    mGlRender.onDestroy();
    mEglCore.destroySurface(mSurface);
    mSurface = EGL_NO_SURFACE;
    mEglCore.destroyGL(mContext);
    ANativeWindow_release(mNativeWindow);
    mNativeWindow = NULL;
    isDestroy = true;
    isCreated = false;
}

void VideoOutput::renderVideoFrameHandler(VideoFrame *videoFrame) {
    if (videoFrame == NULL) {
        LOGE("VideoOutput renderVideoHandler 为空");
        return;
    }
    mEglCore.makeCurrent(mSurface, mContext);
    if (!videoFrame->isSkip) {
        mGlRender.onDraw(videoFrame);
        mEglCore.swapBuffers(mSurface);
        LOGE("VideoOutput 渲染videoFrame完成: %lf %lu", videoFrame->timestamp, (unsigned long)pthread_self());
    } else {
        LOGE("VideoOutput 跳帧");
    }
    delete videoFrame;
}

void VideoOutput::changeSizeHandler() {
    screenWidth = ANativeWindow_getWidth(mNativeWindow);
    screenHeight = ANativeWindow_getHeight(mNativeWindow);
    mEglCore.makeCurrent(mSurface, mContext);
    mGlRender.onChangeSize(screenWidth, screenHeight);
    LOGD("VideoOutput changeSizeHanlder %dx%d", screenWidth, screenHeight);
}





