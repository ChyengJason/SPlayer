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

void VideoQueue::start(MediaDecoder* decoder) {
    if (!isThreadInited) {
        LOGD("VideoQueue start 当前主线程：%lu", (unsigned long)pthread_self());
        mAllDuration = 0;
        mMediaDecoder = decoder;
        createRenderThread();
        postMessage(VIDEOQUEUE_MESSAGE_CREATE);
        isThreadInited = true;
    }
}

void VideoQueue::finish()  {
    LOGD("VideoQueue::finish");
    mMediaDecoder = NULL;
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
}

void VideoQueue::push(AVPacket* packet) {
    if (!isThreadInited) {
        return;
    }
    postMessage(VideoQueueMessage(VIDEOQUEUE_MESSAGE_PUSH, packet));
}

bool VideoQueue::isEmpty() {
    return mTextureFrameQue.empty();
}

TextureFrame *VideoQueue::pop() {
    pthread_mutex_lock(&mTextureQueMutex);
    TextureFrame* frame = NULL;
    if (!mTextureFrameQue.empty()) {
        frame = mTextureFrameQue.front();
        mTextureFrameQue.pop();
        mAllDuration -= frame->duration;
    }
    pthread_mutex_unlock(&mTextureQueMutex);
    return frame;
}

void VideoQueue::clear() {
    LOGD("VideoQueue 开始清空");
    mAllDuration = 0;
    pthread_mutex_lock(&mMessageQueMutex);
    pthread_mutex_lock(&mTextureQueMutex);
    while (!mHandlerMessageQueue.empty()) {
        VideoQueueMessage message = mHandlerMessageQueue.front();
        mHandlerMessageQueue.pop();
        if (message.msgType == VIDEOQUEUE_MESSAGE_PUSH) {
            delete(message.value);
        }
    }

    while (!mTextureFrameQue.empty()) {
        TextureFrame* textureFrame = mTextureFrameQue.front();
        mTextureFrameQue.pop();
        delete(textureFrame);
    }
    pthread_mutex_unlock(&mTextureQueMutex);
    pthread_mutex_unlock(&mMessageQueMutex);
    LOGD("VideoQueue 完成清空");
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

void VideoQueue::renderHandler(void* packet) {
    AVPacket * mPacket = (AVPacket*) packet;
    std::vector<VideoFrame*> videoFrames;
    if (mMediaDecoder != NULL) {
        videoFrames = mMediaDecoder->decodeVideoFrame(mPacket);
        mMediaDecoder->freePacket(mPacket);
    }

    if (videoFrames.empty()) {
        return;
    }

    if (mPbufferSurface == EGL_NO_SURFACE) {
        createSurfaceHandler(videoFrames[0]->frameWidth, videoFrames[0]->frameHeight);
        LOGD("videoQueue renderHandler mPbufferSurface: %d", mPbufferSurface);
    }

    for (int i = 0; i < videoFrames.size(); ++i) {
        mEglCore.makeCurrent(mPbufferSurface, mContext);
        // 创建texture2D
        int outTexture = GlRenderUtil::createTexture(videoFrames[i]->frameWidth, videoFrames[i]->frameHeight);
        // 绑定到fbo
        LOGD("videoQueue 创建FBO纹理 %d mFbo %d onDraw %d x %d", outTexture, mFbo, videoFrames[i]->frameWidth, videoFrames[i]->frameHeight);
        GlRenderUtil::bindFrameTexture(mFbo, outTexture);
        // 绘制VideoFrame 到 fbo中
        mGlRender.onDraw(videoFrames[i]);
        mEglCore.swapBuffers(mPbufferSurface);
        // 解绑 fbo
        GlRenderUtil::unBindFrameTexture();
        TextureFrame* textureFrame = new TextureFrame;
        textureFrame->screenHeight = videoFrames[i]->frameHeight;
        textureFrame->screenWidth = videoFrames[i]->frameWidth;
        textureFrame->duration = videoFrames[i]->duration;
        textureFrame->timestamp = videoFrames[i]->timestamp;
        textureFrame->textureId = outTexture;

        pthread_mutex_lock(&mTextureQueMutex);
        mAllDuration += videoFrames[i]->duration;
        mTextureFrameQue.push(textureFrame);
        pthread_mutex_unlock(&mTextureQueMutex);
        delete videoFrames[i];
    }
}

void VideoQueue::clearHandler() {

}

double VideoQueue::getAllDuration() {
    return mAllDuration;
}
