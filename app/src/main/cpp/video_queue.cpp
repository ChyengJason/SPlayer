//
// Created by chengjunsen on 2018/10/22.
//

#include "video_queue.h"
#include "egl/egl_share_context.h"

VideoQueue::VideoQueue() {
    mPbufferSurface = EGL_NO_SURFACE;
    mRenderThread = 0;
    isThreadInited = false;
    mContext = EGL_NO_CONTEXT;
    pthread_mutex_init(&mRenderMutex, NULL);
    pthread_mutex_init(&mTextureFrameMutex, NULL);
    pthread_cond_init(&mRenderCond, NULL);
}

VideoQueue::~VideoQueue() {
    pthread_detach(mRenderThread);
    pthread_cond_destroy(&mRenderCond);
    pthread_mutex_destroy(&mTextureFrameMutex);
    pthread_mutex_destroy(&mRenderMutex);
}

void VideoQueue::start(int width, int height) {
    this->frameWidth = width;
    this->frameHeight = height;
    isThreadInited = true;
    LOGE("videoqueue start: %d x %d", frameWidth, frameHeight);
    createRenderThread();
    postMessage(VIDEOQUEUE_MESSAGE_CREATE);
}

void VideoQueue::finish()  {
    LOGE("VideoQueue::finish");
    isThreadInited = false;
    postMessage(VIDEOQUEUE_MESSAGE_QUIT);
    //pthread_detach(mRenderThread);
}

void VideoQueue::postMessage(VideoQueueMessage msg) {
    pthread_mutex_lock(&mRenderMutex);
    mHandlerMessageQueue.push(msg);
    pthread_cond_signal(&mRenderCond);
    pthread_mutex_unlock(&mRenderMutex);
}

void VideoQueue::postMessage(std::vector<VideoQueueMessage> msgs) {
    LOGE("postMessage msgs");
    pthread_mutex_lock(&mRenderMutex);
    for (int i = 0; i < msgs.size(); ++i) {
        mHandlerMessageQueue.push(msgs[i]);
    }
    pthread_cond_signal(&mRenderCond);
    pthread_mutex_unlock(&mRenderMutex);
}

void VideoQueue::push(std::vector<VideoFrame*> frames) {
    if (!isThreadInited) {
        return;
    }
    std::vector<VideoQueueMessage> msgs;
    for (int i = 0; i < frames.size(); ++i) {
        msgs.push_back(VideoQueueMessage(VIDEOQUEUE_MESSAGE_PUSH, frames[i]));
    }
    postMessage(msgs);
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
    postMessage(VIDEOQUEUE_MESSAGE_CLEAR);
}

int VideoQueue::size() {
    return mTextureFrameQue.size();
}

void VideoQueue::createRenderThread() {
    LOGE("videoQue createRenderThread");
    int ret = pthread_create(&mRenderThread, NULL, runRender, this);
    if (ret < 0) {
        LOGE("创建线程失败");
    }
}

void *VideoQueue::runRender(void *self) {
    VideoQueue* videoQueue = (VideoQueue*) self;
    videoQueue->processMessages();
}

void VideoQueue::processMessages() {
    bool isQuited = true;
    while (isQuited) {
        int lockCode = pthread_mutex_lock(&mRenderMutex);
        if (lockCode != 0) {
            LOGE("VideoQueue processMessages 失败");
            return ;
        }
        if (mHandlerMessageQueue.empty()) {
            LOGE("VideoQueue MESSAGE 等待");
            pthread_cond_wait(&mRenderCond, &mRenderMutex);
            LOGE("VideoQueue MESSAGE 结束唤醒");
            pthread_mutex_unlock(&mRenderMutex);
            continue;
        }
        VideoQueueMessage msg = mHandlerMessageQueue.front();
        mHandlerMessageQueue.pop();
        LOGE("VideoQueue MESSAGE %d", msg.msgType);
        switch (msg.msgType) {
            case VIDEOQUEUE_MESSAGE_CREATE:
                createHandler();
                break;
            case VIDEOQUEUE_MESSAGE_PUSH:
                renderHandler(msg.value);
                break;
            case VIDEOQUEUE_MESSAGE_CLEAR:
                clearHandler();
                break;
            case VIDEOQUEUE_MESSAGE_QUIT:
                //releaseHandler();
                isQuited = false;
                break;
            default:
                break;
        }
        pthread_mutex_unlock(&mRenderMutex);
    }
    pthread_exit(0);
    LOGE("VideoQueue processMessages 结束");
}

void VideoQueue::createHandler() {
    LOGE("videoQueue createHandler");
    if (EglShareContext::getShareContext() == EGL_NO_CONTEXT) {
        LOGE("EglShareContext::getShareContext() == EGL_NO_CONTEXT error");
        return;
    }
    mContext = mEglCore.createGL(EglShareContext::getShareContext());
    mPbufferSurface = mEglCore.createBufferSurface(frameWidth, frameHeight);
    mEglCore.makeCurrent(mPbufferSurface, mContext);
    mGlRender.onCreated();
    mGlRender.onChangeSize(frameWidth, frameHeight);
    mFbo = GlRenderUtil::createFrameBuffer();
}

void VideoQueue::releaseHandler() {
    LOGE("videoQueue releaseHandler");
    mEglCore.makeCurrent(mPbufferSurface, mContext);
    GlRenderUtil::deleteFrameBuffer(mFbo);
    mGlRender.onDestroy();
    mEglCore.destroySurface(mPbufferSurface);
    mEglCore.destroyGL(mContext);
    mContext = EGL_NO_CONTEXT;
    clearHandler();
}

void VideoQueue::renderHandler(void* frame) {
    LOGE("videoQueue renderHandler");
    VideoFrame * videoFrame = (VideoFrame*) frame;
    mEglCore.makeCurrent(mPbufferSurface, mContext);
    // 创建texture2D
    int outTexture = GlRenderUtil::createTexture(frameWidth, frameHeight);
    // 绑定到fbo
    LOGE("videoQueue drawTexture %d mFbo %d onDraw %d x %d", outTexture, mFbo, videoFrame->frameWidth, videoFrame->frameHeight);
    GlRenderUtil::bindFrameTexture(mFbo, outTexture);
    // 绘制VideoFrame 到 fbo中
    mGlRender.onDraw(videoFrame);
    // 解绑 fbo
    GlRenderUtil::unBindFrameTexture();
    delete videoFrame;
    TextureFrame* textureFrame = new TextureFrame;
    textureFrame->screenHeight = videoFrame->frameHeight;
    textureFrame->screenWidth = videoFrame->frameWidth;
    textureFrame->duration = videoFrame->duration;
    textureFrame->timestamp = videoFrame->timestamp;
    textureFrame->textureId = outTexture;

    pthread_mutex_lock(&mTextureFrameMutex);
    mTextureFrameQue.push(textureFrame);
    pthread_mutex_unlock(&mTextureFrameMutex);
}

void VideoQueue::clearHandler() {
    LOGE("VideoQueue::clearHandler");
    while (!mHandlerMessageQueue.empty()) {
        VideoQueueMessage message = mHandlerMessageQueue.front();
        mHandlerMessageQueue.pop();
        if (message.msgType == VIDEOQUEUE_MESSAGE_PUSH) {
            delete(message.value);
        }
    }

    //pthread_mutex_lock(&mTextureFrameMutex);
    while(!mTextureFrameQue.empty()) {
        TextureFrame* textrueFrame = mTextureFrameQue.front();
        mTextureFrameQue.pop();
        delete(textrueFrame);
    }
    //pthread_mutex_unlock(&mTextureFrameMutex);
}
