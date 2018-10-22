//
// Created by chengjunsen on 2018/10/22.
//

#include "audio_queue.h"

AudioQueue::AudioQueue() {
    pthread_mutex_init(&mQueMutex, NULL);
}

AudioQueue::~AudioQueue() {
    pthread_mutex_destroy(&mQueMutex);
}

void AudioQueue::start() {

}

void AudioQueue::push(AudioFrame *frame) {
    pthread_mutex_lock(&mQueMutex);
    mAudioFrameQue.push(frame);
    pthread_mutex_unlock(&mQueMutex);
}

void AudioQueue::push(std::vector<AudioFrame *> frames) {
    pthread_mutex_lock(&mQueMutex);
    for (int i = 0; i < frames.size(); ++i) {
        mAudioFrameQue.push(frames[i]);
    }
    pthread_mutex_unlock(&mQueMutex);
}

AudioFrame *AudioQueue::pop() {
    AudioFrame* result = NULL;
    pthread_mutex_lock(&mQueMutex);
    if (!mAudioFrameQue.empty()) {
        result = mAudioFrameQue.front();
        mAudioFrameQue.pop();
    }
    return result;
}

void AudioQueue::clear() {
    pthread_mutex_lock(&mQueMutex);
    while(!mAudioFrameQue.empty()) {
        mAudioFrameQue.pop();
    }
    pthread_mutex_unlock(&mQueMutex);
}

int AudioQueue::size() {
    return mAudioFrameQue.size();
}

bool AudioQueue::isEmpty() {
    return mAudioFrameQue.empty();
}

void AudioQueue::release() {
    clear();
}
