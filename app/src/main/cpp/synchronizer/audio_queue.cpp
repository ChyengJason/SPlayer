//
// Created by chengjunsen on 2018/10/22.
//

#include "audio_queue.h"

AudioQueue::AudioQueue()
        : isInited(false) {
    pthread_mutex_init(&mQueMutex, NULL);
}

AudioQueue::~AudioQueue() {
    pthread_mutex_destroy(&mQueMutex);
}

void AudioQueue::start() {
    isInited = true;
    mAllDuration = 0;
}

void AudioQueue::push(AudioFrame *frame) {
    pthread_mutex_lock(&mQueMutex);
    mAudioFrameQue.push(frame);
    mAllDuration += frame->duration;
    pthread_mutex_unlock(&mQueMutex);
}

void AudioQueue::push(std::vector<AudioFrame *> frames) {
    pthread_mutex_lock(&mQueMutex);
    if (!isInited || frames.empty()) {
        return;
    }
    for (int i = 0; i < frames.size(); ++i) {
        mAudioFrameQue.push(frames[i]);
        mAllDuration += frames[i]->duration;
    }
    pthread_mutex_unlock(&mQueMutex);
}

AudioFrame *AudioQueue::pop() {
    pthread_mutex_lock(&mQueMutex);
    AudioFrame* frame = NULL;
    if (!mAudioFrameQue.empty()) {
        frame = mAudioFrameQue.front();
        mAudioFrameQue.pop();
        mAllDuration -= frame->duration;
    }
    pthread_mutex_unlock(&mQueMutex);
    return frame;
}

void AudioQueue::clear() {
    pthread_mutex_lock(&mQueMutex);
    mAllDuration = 0;
    while(!mAudioFrameQue.empty()) {
        AudioFrame* audioFrame = mAudioFrameQue.front();
        mAudioFrameQue.pop();
        delete(audioFrame);
    }
    pthread_mutex_unlock(&mQueMutex);
}

int AudioQueue::size() {
    return mAudioFrameQue.size();
}

bool AudioQueue::isEmpty() {
    return mAudioFrameQue.empty();
}

void AudioQueue::finish() {
    clear();
    isInited = false;
}

bool AudioQueue::isRunning() {
    return isInited;
}

double AudioQueue::getAllDuration() {
    return mAllDuration;
}
