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
    mAllDuration = 0;
}

void AudioQueue::push(AudioFrame *frame) {
    mAudioFrameQue.push(frame);
    mAllDuration += frame->duration;
}

void AudioQueue::push(std::vector<AudioFrame *> frames) {
    if (!isInited || frames.empty()) {
        return;
    }
    for (int i = 0; i < frames.size(); ++i) {
        mAudioFrameQue.push(frames[i]);
        mAllDuration += frames[i]->duration;
    }
}

AudioFrame *AudioQueue::pop() {
    AudioFrame* frame = mAudioFrameQue.pop();
    mAllDuration -= frame->duration;
    return frame;
}

void AudioQueue::clear() {
    mAllDuration = 0;
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

double AudioQueue::getAllDuration() {
    return mAllDuration;
}
