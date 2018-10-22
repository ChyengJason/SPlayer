//
// Created by chengjunsen on 2018/10/22.
//

#include "video_queue.h"
#include "egl/egl_share_context.h"

VideoQueue::VideoQueue() {
    mPbufferSurface = EGL_NO_SURFACE;
    isCreateRendered = false;
    isRunning = false;
    mRenderThread = 0;
    pthread_mutex_init(&mVideoFrameMutex, NULL);
    pthread_mutex_init(&mTextureFrameMutex, NULL);
    pthread_cond_init(&mRenderCond, NULL);
}

VideoQueue::~VideoQueue() {
    isRunning = false;
    isCreateRendered = false;
    pthread_detach(mRenderThread);
    pthread_cond_destroy(&mRenderCond);
    pthread_mutex_destroy(&mTextureFrameMutex);
    pthread_mutex_destroy(&mVideoFrameMutex);
}

void VideoQueue::start(int width, int height) {
    this->width = width;
    this->height = height;
    if (!isCreateRendered) {
        createRenderThread();
    } else {
        signalRender();
    }
}

void VideoQueue::release() {
    isRunning = false;
    isCreateRendered = false;
    pthread_mutex_lock(&mVideoFrameMutex);
    pthread_cond_signal(&mRenderCond);
    pthread_mutex_unlock(&mVideoFrameMutex);
    clear();
    mEglCore.destroySurface(mPbufferSurface);
    mEglCore.destroyGL(EglShareContext::getShareContext());
    mGlRender.onDestroy();
}

//void VideoQueue::changeSize(int width, int height) {
//    this->width = width;
//    this->height = height;
//    mGlRender.onChangeSize(width, height);
//}

void VideoQueue::push(VideoFrame *frame) {
    pthread_mutex_lock(&mVideoFrameMutex);
    mVideoFrameQue.push(frame);
    pthread_mutex_unlock(&mVideoFrameMutex);
    if (!isCreateRendered) {
        createRenderThread();
    } else {
        signalRender();
    }
}

bool VideoQueue::isEmpty() {
    return mTextureFrameQue.empty();
}

TextureFrame *VideoQueue::pop() {
    pthread_mutex_lock(&mTextureFrameMutex);
    TextureFrame* result = NULL;
    if(!mTextureFrameQue.empty()) {
        result = mTextureFrameQue.front();
        mTextureFrameQue.pop();
    }
    pthread_mutex_unlock(&mTextureFrameMutex);
    return result;
}

void VideoQueue::clear() {
    pthread_mutex_lock(&mVideoFrameMutex);
    VideoFrame* videoFrame = mVideoFrameQue.front();
    mVideoFrameQue.pop();
    delete(videoFrame);
    pthread_mutex_unlock(&mVideoFrameMutex);

    pthread_mutex_lock(&mTextureFrameMutex);
    TextureFrame* textrueFrame = mTextureFrameQue.front();
    mVideoFrameQue.pop();
    delete(textrueFrame);
    pthread_mutex_unlock(&mTextureFrameMutex);
}

int VideoQueue::size() {
    return mTextureFrameQue.size();
}

void VideoQueue::createRenderThread() {
    isCreateRendered = true;
    isRunning = true;
    int ret = pthread_create(&mRenderThread, NULL, runRender, this);
    if (ret < 0) {
        LOGE("创建线程失败");
    }
}

void *VideoQueue::runRender(void *self) {
    VideoQueue* videoQueue = (VideoQueue*) self;
    videoQueue->createRender();
    videoQueue->processRender();
    pthread_exit(0);
}

void VideoQueue::signalRender() {
    pthread_mutex_lock(&mVideoFrameMutex);
    pthread_cond_signal(&mRenderCond);
    pthread_mutex_unlock(&mVideoFrameMutex);
}

void VideoQueue::createRender() {
    EGLContext shareContext = mEglCore.createGL(EglShareContext::getShareContext());
    EglShareContext::setShareContext(shareContext);
    mPbufferSurface = mEglCore.createBufferSurface(width, height);
    mEglCore.makeCurrent(shareContext, mPbufferSurface);
    mGlRender.onCreated();
    mGlRender.onChangeSize(width, height);
}

void VideoQueue::processRender() {
    while(isRunning) {
        pthread_mutex_lock(&mVideoFrameMutex);
        VideoFrame* videoFrame = mVideoFrameQue.front();
        if(videoFrame == NULL) {
            LOGE("mVideoFrameQue 等待");
            pthread_cond_wait(&mRenderCond, &mVideoFrameMutex);
            LOGE("mVideoFrameQue 苏醒");
            pthread_mutex_unlock(&mVideoFrameMutex);
            continue;
        }
        mVideoFrameQue.pop();
        pthread_mutex_unlock(&mVideoFrameMutex);
        TextureFrame* textureFrame = textureRender(videoFrame);
        delete videoFrame;
        if(textureFrame != NULL) {
            pthread_mutex_lock(&mTextureFrameMutex);
            mTextureFrameQue.push(textureFrame);
            pthread_mutex_unlock(&mTextureFrameMutex);
        }
    }
}

TextureFrame *VideoQueue::textureRender(VideoFrame *pFrame) {
    mEglCore.makeCurrent(mPbufferSurface, EglShareContext::getShareContext());
    // 创建 fbo 和 texture2D

    return NULL;
}