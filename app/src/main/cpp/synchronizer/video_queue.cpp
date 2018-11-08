//
// Created by chengjunsen on 2018/10/22.
//

#include "video_queue.h"
#include "../egl/egl_share_context.h"

VideoQueue::VideoQueue() {
    mPbufferSurface = EGL_NO_SURFACE;
    mRenderThread = 0;
    isThreadInited = false;
    mContext = EGL_NO_CONTEXT;
    mAllDuration = 0;
    pthread_mutex_init(&mRenderMutex, NULL);
    pthread_mutex_init(&mMessageQueMutex, NULL);
    pthread_mutex_init(&mTextureQueMutex, NULL);
    pthread_cond_init(&mRenderCond, NULL);
}

VideoQueue::~VideoQueue() {
    pthread_detach(mRenderThread);
    pthread_cond_destroy(&mRenderCond);
    pthread_mutex_destroy(&mRenderMutex);
    pthread_mutex_destroy(&mMessageQueMutex);
}

void VideoQueue::start() {
    if (!isThreadInited) {
        LOGD("VideoQueue start 当前主线程：%lu", (unsigned long)pthread_self());
        this->mAllDuration = 0;
        createRenderThread();
        postMessage(VIDEOQUEUE_MESSAGE_CREATE);
        isThreadInited = true;
        isClearing = false;
    }
}

void VideoQueue::finish()  {
    LOGD("VideoQueue::finish");
    if (!isThreadInited) {
        return;
    }
    isThreadInited = false;
    postMessage(VIDEOQUEUE_MESSAGE_QUIT);
    //pthread_detach(mRenderThread);
}

bool VideoQueue::isRunning() {
    return isThreadInited;
}

void VideoQueue::postMessage(VideoQueueMessage msg) {
    pthread_mutex_lock(&mMessageQueMutex);
    mHandlerMessageQueue.push(msg);
    pthread_mutex_unlock(&mMessageQueMutex);
    pthread_cond_signal(&mRenderCond);
    //LOGD("VideoQueue pthread_cond_signal %d", mHandlerMessageQueue.size());
}

void VideoQueue::postMessage(std::vector<VideoQueueMessage> msgs) {
    pthread_mutex_lock(&mMessageQueMutex);
    for (VideoQueueMessage message : msgs) {
        mHandlerMessageQueue.push(message);
    }
    pthread_mutex_unlock(&mMessageQueMutex);
    pthread_cond_signal(&mRenderCond);
    //LOGD("VideoQueue pthread_cond_signal %d", mHandlerMessageQueue.size());
}

void VideoQueue::push(std::vector<VideoFrame*> frames) {
    //LOGD("VideoQueue pushFrames %d", frames.size());
    if (frames.empty()) {
        LOGE("frames为空");
        return;
    }
    std::vector<VideoQueueMessage> msgs;
    for (int i = 0; i < frames.size(); ++i) {
        mAllDuration += frames[i]->duration;
        msgs.push_back(VideoQueueMessage(VIDEOQUEUE_MESSAGE_PUSH, frames[i]));
    }
    postMessage(msgs);
}

bool VideoQueue::isEmpty() {
    return mTextureFrameQue.empty();
}

TextureFrame *VideoQueue::pop() {
    pthread_mutex_lock(&mTextureQueMutex);
    TextureFrame* frame = NULL;
    if (!isClearing && !mTextureFrameQue.empty()) {
        frame = mTextureFrameQue.front();
        mTextureFrameQue.pop();
        mAllDuration -= frame->duration;
    }
    pthread_mutex_unlock(&mTextureQueMutex);
    return frame;
}

void VideoQueue::clear() {
    mAllDuration = 0;
    isClearing = true;
    postMessage(VIDEOQUEUE_MESSAGE_CLEAR);
}

int VideoQueue::size() {
    return mTextureFrameQue.size();
}

void VideoQueue::createRenderThread() {
    LOGD("videoQue createRenderThread");
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
            LOGE("videoQue processMessages 失败");
            return ;
        }
        if (mHandlerMessageQueue.empty()) {
            //LOGD("VideoQueue MESSAGE 等待");
            pthread_cond_wait(&mRenderCond, &mRenderMutex);
            //LOGD("VideoQueue MESSAGE 结束唤醒 %d", mHandlerMessageQueue.size());
            pthread_mutex_unlock(&mRenderMutex);
            continue;
        }

        pthread_mutex_lock(&mMessageQueMutex);
        VideoQueueMessage msg = mHandlerMessageQueue.front();
        mHandlerMessageQueue.pop();
        pthread_mutex_unlock(&mMessageQueMutex);
        //LOGD("VideoQueue MESSAGE TYPE %d", msg.msgType);
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
                releaseHandler();
                isQuited = false;
                break;
            default:
                break;
        }
        pthread_mutex_unlock(&mRenderMutex);
    }
    LOGD("VideoQueue processMessages 结束");
    pthread_exit(0);
}

void VideoQueue::createHandler() {
    EglShareContext::getInstance().lock();
    mContext = mEglCore.createGL(EglShareContext::getInstance().getShareContext());
    LOGD("videoQueue createHandler %ld", mContext);
    EglShareContext::getInstance().setShareContext(mContext);
    EglShareContext::getInstance().unlock();
}

void VideoQueue::releaseHandler() {
    LOGD("videoQueue releaseHandler");
    mEglCore.makeCurrent(mPbufferSurface, mContext);
    GlRenderUtil::deleteFrameBuffer(mFbo);
    mGlRender.onDestroy();
    mEglCore.destroySurface(mPbufferSurface);
    mEglCore.destroyGL(mContext);
    mContext = EGL_NO_CONTEXT;
    clearHandler();
}

void VideoQueue::createSurfaceHandler(int frameWidth, int frameHeight) {
    LOGD("VideoQueue createBufferSurface %dx%d", frameWidth, frameHeight);
    mPbufferSurface = mEglCore.createBufferSurface(frameWidth, frameHeight);
    mEglCore.makeCurrent(mPbufferSurface, mContext);
    mGlRender.onCreated();
    mGlRender.onChangeSize(frameWidth, frameHeight);
    mFbo = GlRenderUtil::createFrameBuffer();
}

void VideoQueue::renderHandler(void* frame) {
    VideoFrame * videoFrame = (VideoFrame*) frame;
    if (mPbufferSurface == EGL_NO_SURFACE) {
        createSurfaceHandler(videoFrame->frameWidth, videoFrame->frameHeight);
        LOGD("videoQueue renderHandler mPbufferSurface: %d", mPbufferSurface);
    }
    mEglCore.makeCurrent(mPbufferSurface, mContext);
    // 创建texture2D
    int outTexture = GlRenderUtil::createTexture(videoFrame->frameWidth, videoFrame->frameHeight);
    // 绑定到fbo
    LOGD("videoQueue 创建FBO纹理 %d mFbo %d onDraw %d x %d", outTexture, mFbo, videoFrame->frameWidth, videoFrame->frameHeight);
    GlRenderUtil::bindFrameTexture(mFbo, outTexture);
    // 绘制VideoFrame 到 fbo中
    mGlRender.onDraw(videoFrame);
    mEglCore.swapBuffers(mPbufferSurface);
    // 解绑 fbo
    GlRenderUtil::unBindFrameTexture();
    TextureFrame* textureFrame = new TextureFrame;
    textureFrame->screenHeight = videoFrame->frameHeight;
    textureFrame->screenWidth = videoFrame->frameWidth;
    textureFrame->duration = videoFrame->duration;
    textureFrame->timestamp = videoFrame->timestamp;
    textureFrame->textureId = outTexture;

    pthread_mutex_lock(&mTextureQueMutex);
    mTextureFrameQue.push(textureFrame);
    pthread_mutex_unlock(&mTextureQueMutex);
    delete videoFrame;
}

void VideoQueue::clearHandler() {
    LOGD("VideoQueue::clearHandler");
    pthread_mutex_lock(&mMessageQueMutex);
    while (!mHandlerMessageQueue.empty()) {
        VideoQueueMessage message = mHandlerMessageQueue.front();
        mHandlerMessageQueue.pop();
        if (message.msgType == VIDEOQUEUE_MESSAGE_PUSH) {
            delete(message.value);
        }
    }
    pthread_mutex_unlock(&mMessageQueMutex);

    pthread_mutex_lock(&mTextureQueMutex);
    while (!mTextureFrameQue.empty()) {
        TextureFrame* textureFrame = mTextureFrameQue.front();
        mTextureFrameQue.pop();
        delete(textureFrame);
    }
    pthread_mutex_unlock(&mTextureQueMutex);
    isClearing = false;
}

double VideoQueue::getAllDuration() {
    return mAllDuration;
}