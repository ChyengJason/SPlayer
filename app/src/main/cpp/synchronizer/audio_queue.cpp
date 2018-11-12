//
// Created by chengjunsen on 2018/10/22.
//

#include "audio_queue.h"
#include "iostream"

AudioQueue::AudioQueue()
        : isFinish(true)
        , isClearing(false)
        , mMediaDecoder(NULL){
    pthread_mutex_init(&mFrameQueMutex, NULL);
    pthread_mutex_init(&mPacketMutex, NULL);
//    pthread_mutex_init(&mDecodeMutex, NULL);
//    pthread_cond_init(&mDecodeCond, NULL);
}

AudioQueue::~AudioQueue() {
    pthread_mutex_destroy(&mPacketMutex);
    pthread_mutex_destroy(&mFrameQueMutex);
//    pthread_mutex_destroy(&mDecodeMutex);
//    pthread_cond_destroy(&mDecodeCond);
    mMediaDecoder = NULL;
}

void AudioQueue::start(MediaDecoder* decoder) {
    if (!isRunning()) {
        mMediaDecoder = decoder;
        mAllDuration = 0;
        pthread_create(&mDecodeThread, NULL, runDecode, this);
    }
}

void AudioQueue::push(AVPacket* packet) {
    pthread_mutex_lock(&mPacketMutex);
    mPacketQue.push(packet);
    pthread_mutex_unlock(&mPacketMutex);
//    pthread_cond_signal(&mDecodeCond);
}

AudioFrame *AudioQueue::pop() {
    pthread_mutex_lock(&mFrameQueMutex);
    AudioFrame* frame = NULL;
    if (!mAudioFrameQue.empty()) {
        frame = mAudioFrameQue.front();
        mAudioFrameQue.pop();
        mAllDuration = std::max(mAllDuration - frame->duration, 0.0);
    }
    pthread_mutex_unlock(&mFrameQueMutex);
    return frame;
}

void AudioQueue::clear() {
    isClearing = true;
//    pthread_cond_signal(&mDecodeCond);
}

int AudioQueue::size() {
    return mAudioFrameQue.size();
}

bool AudioQueue::isEmpty() {
    return mAudioFrameQue.empty();
}

void AudioQueue::finish() {
    isFinish = true;
//    pthread_cond_signal(&mDecodeCond);
}

bool AudioQueue::isRunning() {
    return !isFinish;
}

double AudioQueue::getAllDuration() {
    return mAllDuration;
}

void* AudioQueue::runDecode(void *self) {
    AudioQueue* context = (AudioQueue*)self;
    context->runDecodeImpl();
    return 0;
}

void AudioQueue::runDecodeImpl() {
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

int AudioQueue::packetCacheSize() {
    return mPacketQue.size();
}

void AudioQueue::runClearing() {
    LOGD("AudioQueue 开始清空");
    pthread_mutex_lock(&mFrameQueMutex);
    pthread_mutex_lock(&mPacketMutex);
    isClearing = true;
    while(!mAudioFrameQue.empty()) {
        AudioFrame* audioFrame = mAudioFrameQue.front();
        mAudioFrameQue.pop();
        delete(audioFrame);
    }
    while(!mPacketQue.empty()) {
        AVPacket* packet = mPacketQue.front();
        mPacketQue.pop();
        mMediaDecoder->freePacket(packet);
    }
    isClearing = false;
    mAllDuration = 0;
    pthread_mutex_unlock(&mPacketMutex);
    pthread_mutex_unlock(&mFrameQueMutex);
    LOGD("AudioQueue 完成清空");
}

void AudioQueue::runDecoding() {
    std::vector<AudioFrame*> frames;
    pthread_mutex_lock(&mPacketMutex);
    if (!mPacketQue.empty()) {
        AVPacket* packet = mPacketQue.front();
        mPacketQue.pop();
        frames = mMediaDecoder->decodeAudioFrame(packet);
        mMediaDecoder->freePacket(packet);
    }
    pthread_mutex_unlock(&mPacketMutex);

    pthread_mutex_lock(&mFrameQueMutex);
    for (int i = 0; i < frames.size(); ++i) {
        mAudioFrameQue.push(frames[i]);
        mAllDuration += frames[i]->duration;
    }
    pthread_mutex_unlock(&mFrameQueMutex);
}

bool AudioQueue::clearing() {
    return isClearing;
}
