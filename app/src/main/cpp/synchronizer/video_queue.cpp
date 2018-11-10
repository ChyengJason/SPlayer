//
// Created by chengjunsen on 2018/10/22.
//

#include "video_queue.h"
#include "iostream"

VideoQueue::VideoQueue()
        : isFinish(true)
        , mMediaDecoder(NULL){
    pthread_mutex_init(&mFrameQueMutex, NULL);
    pthread_mutex_init(&mPacketMutex, NULL);
}

VideoQueue::~VideoQueue() {
    pthread_mutex_destroy(&mPacketMutex);
    pthread_mutex_destroy(&mFrameQueMutex);
    mMediaDecoder = NULL;
}

void VideoQueue::start(MediaDecoder* decoder) {
    if (!isRunning()) {
        mMediaDecoder = decoder;
        mAllDuration = 0;
        pthread_create(&mDecodeThread, NULL, runDecode, this);
    }
}

void VideoQueue::push(AVPacket* packet) {
    pthread_mutex_lock(&mPacketMutex);
    mPacketQue.push(packet);
    pthread_mutex_unlock(&mPacketMutex);
}

VideoFrame *VideoQueue::pop() {
    pthread_mutex_lock(&mFrameQueMutex);
    VideoFrame* frame = NULL;
    if (!mVideoFrameQue.empty()) {
        frame = mVideoFrameQue.front();
        mVideoFrameQue.pop();
        mAllDuration = std::max(mAllDuration - frame->duration, 0.0);
    }
    pthread_mutex_unlock(&mFrameQueMutex);
    return frame;
}

void VideoQueue::clear() {
    isClearing = true;
}

int VideoQueue::size() {
    return mVideoFrameQue.size();
}

bool VideoQueue::isEmpty() {
    return mVideoFrameQue.empty();
}

void VideoQueue::finish() {
    isFinish = true;
}

bool VideoQueue::isRunning() {
    return !isFinish;
}

double VideoQueue::getAllDuration() {
    return mAllDuration;
}

void* VideoQueue::runDecode(void *self) {
    VideoQueue* context = (VideoQueue*)self;
    context->runDecodeImpl();
    return 0;
}

void VideoQueue::runDecodeImpl() {
    isFinish = false;
    while (!isFinish) {
        if (isClearing) {
            runClearing();
        } else if (mMediaDecoder != NULL) {
            runDecoding();
        }
    }
    runClearing();
}

int VideoQueue::packetCacheSize() {
    return mPacketQue.size();
}

void VideoQueue::runClearing() {
    LOGD("VideoQueue 开始清空");
    pthread_mutex_lock(&mPacketMutex);
    pthread_mutex_lock(&mFrameQueMutex);
    while(!mPacketQue.empty()) {
        AVPacket* packet = mPacketQue.front();
        mPacketQue.pop();
        mMediaDecoder->freePacket(packet);
    }
    while(!mVideoFrameQue.empty()) {
        VideoFrame* audioFrame = mVideoFrameQue.front();
        mVideoFrameQue.pop();
        delete(audioFrame);
    }
    isClearing = false;
    mAllDuration = 0;
    pthread_mutex_unlock(&mFrameQueMutex);
    pthread_mutex_unlock(&mPacketMutex);
    LOGD("VideoQueue 完成清空");
}

void VideoQueue::runDecoding() {
    std::vector<VideoFrame*> frames;
    pthread_mutex_lock(&mPacketMutex);
    if (!mPacketQue.empty()) {
        AVPacket* packet = mPacketQue.front();
        mPacketQue.pop();
        frames = mMediaDecoder->decodeVideoFrame(packet);
        mMediaDecoder->freePacket(packet);
    }
    pthread_mutex_unlock(&mPacketMutex);

    pthread_mutex_lock(&mFrameQueMutex);
    for (int i = 0; i < frames.size(); ++i) {
        mVideoFrameQue.push(frames[i]);
        mAllDuration += frames[i]->duration;
    }
    pthread_mutex_unlock(&mFrameQueMutex);
}

bool VideoQueue::clearing() {
    return isClearing;
}
