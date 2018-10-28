//
// Created by chengjunsen on 2018/10/22.
//

#include "audio_queue.h"

AudioQueue::AudioQueue()
        : isInited(false) {
}

AudioQueue::~AudioQueue() {
}

void AudioQueue::start() {
    isInited = true;
}

void AudioQueue::push(AudioFrame *frame) {
    mAudioFrameQue.push(frame);
}

void AudioQueue::push(std::vector<AudioFrame *> frames) {
    mAudioFrameQue.push(frames);
}

AudioFrame *AudioQueue::pop() {
    return mAudioFrameQue.pop();
}

void AudioQueue::clear() {
    while(!mAudioFrameQue.isEmpty()) {
        AudioFrame* audioFrame = mAudioFrameQue.pop();
        delete(audioFrame);
    }
}

int AudioQueue::size() {
    return mAudioFrameQue.size();
}

bool AudioQueue::isEmpty() {
    return mAudioFrameQue.isEmpty();
}

void AudioQueue::finish() {
    clear();
    isInited = false;
}

bool AudioQueue::isRunning() {
    return isInited;
}
