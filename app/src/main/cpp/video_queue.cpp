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

void VideoQueue::start(int frameWidth, int frameHeight) {
    LOGE("videoqueue start: %d x %d", frameWidth, frameHeight);
    this->frameWidth = frameWidth;
    this->frameHeight = frameHeight;
    if (!isCreateRendered) {
        LOGE("videoqueue start: createRenderThread");
        createRenderThread();
    } else {
        LOGE("videoqueue start: signalRender");
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
}

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
    LOGE("videoQue createRenderThread");
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
    mPbufferSurface = mEglCore.createBufferSurface(frameWidth, frameHeight);
    mEglCore.makeCurrent(mPbufferSurface, shareContext);
    mGlRender.onCreated();
    mGlRender.onChangeSize(frameWidth, frameHeight);
    createFboRender();
}

void VideoQueue::processRender() {
    while(isRunning) {
        pthread_mutex_lock(&mVideoFrameMutex);
        if(mVideoFrameQue.empty()) {
            LOGE("mVideoFrameQue 等待");
            pthread_cond_wait(&mRenderCond, &mVideoFrameMutex);
            LOGE("mVideoFrameQue 苏醒");
            pthread_mutex_unlock(&mVideoFrameMutex);
            continue;
        }
        VideoFrame* videoFrame = mVideoFrameQue.front();
        LOGE("processRender textureRender timestamp %lf,  %d x %d", videoFrame->timestamp, videoFrame->frameWidth, videoFrame->frameHeight);
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
    destroyRender();
}

TextureFrame *VideoQueue::textureRender(const VideoFrame *pFrame) {
    LOGE("videoQueue textureRender %lf", pFrame->timestamp);
    mEglCore.makeCurrent(mPbufferSurface, EglShareContext::getShareContext());
    // 创建texture2D
    int outTexture = GlRenderUtil::createTexture(frameWidth, frameHeight);
    // 绑定到fbo
    LOGE("videoQueue createTexture %d mFbo %d", outTexture, mFbo);
    GlRenderUtil::bindFrameTexture(mFbo, outTexture);
    // 绘制VideoFrame 到 fbo中
    LOGE("videoQueue onDraw %d x %d", pFrame->frameWidth, pFrame->frameHeight);

    mGlRender.onDraw(pFrame->luma, pFrame->chromaB, pFrame->chromaR);
    // 解绑 fbo
    GlRenderUtil::unBindFrameTexture();
    TextureFrame* textureFrame = new TextureFrame;
    textureFrame->screenHeight = pFrame->frameHeight;
    textureFrame->screenWidth = pFrame->frameWidth;
    textureFrame->duration = pFrame->duration;
    textureFrame->timestamp = pFrame->timestamp;
    textureFrame->textureId = outTexture;
    return textureFrame;
}

void VideoQueue::createFboRender() {
    // 创建 fbo
    LOGE("videoQueue createFboRender");
    mEglCore.makeCurrent(mPbufferSurface, EglShareContext::getShareContext());
    mFbo = GlRenderUtil::createFrameBuffer();
}

void VideoQueue::destroyRender() {
    mEglCore.makeCurrent(mPbufferSurface, EglShareContext::getShareContext());
    if (mFbo) GlRenderUtil::deleteFrameBuffer(mFbo);
    mGlRender.onDestroy();
    mEglCore.destroySurface(mPbufferSurface);
    mEglCore.destroyGL(EglShareContext::getShareContext());
}
