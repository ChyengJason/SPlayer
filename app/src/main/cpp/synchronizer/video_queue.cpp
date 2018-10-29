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
    pthread_mutex_init(&mRenderMutex, NULL);
    pthread_cond_init(&mRenderCond, NULL);
}

VideoQueue::~VideoQueue() {
    pthread_detach(mRenderThread);
    pthread_cond_destroy(&mRenderCond);
    pthread_mutex_destroy(&mRenderMutex);
}

void VideoQueue::start(int width, int height) {
    if (width <= 0 || height <= 0) {
        LOGE("VideoQueue start error: %d x %d", width, height);
        return;
    }
    LOGD("VideoQueue start : %d x %d", width, height);
    this->frameWidth = width;
    this->frameHeight = height;
    createRenderThread();
    postMessage(VIDEOQUEUE_MESSAGE_CREATE);
    isThreadInited = true;
}

void VideoQueue::finish()  {
    LOGD("VideoQueue::finish");
    isThreadInited = false;
    postMessage(VIDEOQUEUE_MESSAGE_QUIT);
    //pthread_detach(mRenderThread);
}

bool VideoQueue::isRunning() {
    return isThreadInited;
}

void VideoQueue::postMessage(VideoQueueMessage msg) {
    mHandlerMessageQueue.push(msg);
    pthread_cond_signal(&mRenderCond);
    //LOGD("VideoQueue pthread_cond_signal %d", mHandlerMessageQueue.size());
}

void VideoQueue::postMessage(std::vector<VideoQueueMessage> msgs) {
    mHandlerMessageQueue.push(msgs);
    pthread_cond_signal(&mRenderCond);
    //LOGD("VideoQueue pthread_cond_signal %d", mHandlerMessageQueue.size());
}

void VideoQueue::push(std::vector<VideoFrame*> frames) {
    //LOGD("VideoQueue pushFrames %d", frames.size());
    if (!isThreadInited || frames.empty()) {
        return;
    }
    std::vector<VideoQueueMessage> msgs;
    for (int i = 0; i < frames.size(); ++i) {
        msgs.push_back(VideoQueueMessage(VIDEOQUEUE_MESSAGE_PUSH, frames[i]));
    }
    postMessage(msgs);
}

bool VideoQueue::isEmpty() {
    return mTextureFrameQue.isEmpty();
}

TextureFrame *VideoQueue::pop() {
    return mTextureFrameQue.pop();
}

void VideoQueue::clear() {
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
            LOGE("VideoQueue processMessages 失败");
            return ;
        }
        if (mHandlerMessageQueue.isEmpty()) {
            //LOGD("VideoQueue MESSAGE 等待");
            pthread_cond_wait(&mRenderCond, &mRenderMutex);
            //LOGD("VideoQueue MESSAGE 结束唤醒 %d", mHandlerMessageQueue.size());
            pthread_mutex_unlock(&mRenderMutex);
            continue;
        }
        VideoQueueMessage msg = mHandlerMessageQueue.pop();
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
    LOGD("videoQueue createHandler");
    if (EglShareContext::getShareContext() == EGL_NO_CONTEXT) {
        LOGE("VideoQueue EglShareContext::getShareContext() == EGL_NO_CONTEXT error");
        return;
    }
    mContext = mEglCore.createGL(EglShareContext::getShareContext());
    LOGD("VideoQueue createBufferSurface %dx%d", frameWidth, frameHeight);
    mPbufferSurface = mEglCore.createBufferSurface(frameWidth, frameHeight);
    mEglCore.makeCurrent(mPbufferSurface, mContext);
    mGlRender.onCreated();
//    mWaterMarkRender.onCreated();
    mGlRender.onChangeSize(frameWidth, frameHeight);
//    mWaterMarkRender.onChangeSize(frameWidth, frameHeight);
    mFbo = GlRenderUtil::createFrameBuffer();
}

void VideoQueue::releaseHandler() {
    LOGD("videoQueue releaseHandler");
    mEglCore.makeCurrent(mPbufferSurface, mContext);
    GlRenderUtil::deleteFrameBuffer(mFbo);
    mGlRender.onDestroy();
//    mWaterMarkRender.onDestroy();
    mEglCore.destroySurface(mPbufferSurface);
    mEglCore.destroyGL(mContext);
    mContext = EGL_NO_CONTEXT;
    clearHandler();
}

void VideoQueue::renderHandler(void* frame) {
    //LOGD("videoQueue renderHandler");
    VideoFrame * videoFrame = (VideoFrame*) frame;
    mEglCore.makeCurrent(mPbufferSurface, mContext);
    // 创建texture2D
    int outTexture = GlRenderUtil::createTexture(frameWidth, frameHeight);
    // 绑定到fbo
    LOGD("videoQueue 创建FBO纹理 %d mFbo %d onDraw %d x %d", outTexture, mFbo, videoFrame->frameWidth, videoFrame->frameHeight);
    GlRenderUtil::bindFrameTexture(mFbo, outTexture);
    // 绘制VideoFrame 到 fbo中
    mGlRender.onDraw(videoFrame);
//    mWaterMarkRender.onDraw();
    mEglCore.swapBuffers(mPbufferSurface);
    // 解绑 fbo
    GlRenderUtil::unBindFrameTexture();
    TextureFrame* textureFrame = new TextureFrame;
    textureFrame->screenHeight = videoFrame->frameHeight;
    textureFrame->screenWidth = videoFrame->frameWidth;
    textureFrame->duration = videoFrame->duration;
    textureFrame->timestamp = videoFrame->timestamp;
    textureFrame->textureId = outTexture;
    mTextureFrameQue.push(textureFrame);
    delete videoFrame;
}

void VideoQueue::clearHandler() {
    LOGD("VideoQueue::clearHandler");
    while (!mHandlerMessageQueue.isEmpty()) {
        VideoQueueMessage message = mHandlerMessageQueue.pop();
        if (message.msgType == VIDEOQUEUE_MESSAGE_PUSH) {
            delete(message.value);
        }
    }

    while(!mTextureFrameQue.isEmpty()) {
        TextureFrame* textrueFrame = mTextureFrameQue.pop();
        delete(textrueFrame);
    }
}

void VideoQueue::setWaterMark(int imgWidth, int imgHeight, void *buffer) {
//    mWaterMarkRender.setWaterMark(imgWidth, imgHeight, buffer);
}
