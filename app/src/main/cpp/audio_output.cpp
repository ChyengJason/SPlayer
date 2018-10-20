//
// Created by chengjunsen on 2018/10/16.
//

#include "audio_output.h"

AudioOutput::AudioOutput() {
}

AudioOutput::~AudioOutput() {
}

bool AudioOutput::start(int channel, int samplerate, GetAudioFrameCallback callback) {
    curPresentTime = 0;
    mGetAudioCallback = callback;
    AudioPlayer::create(samplerate, channel);
    AudioPlayer::play();
    return true;
}

void AudioOutput::stop() {
    release();
}

bool AudioOutput::pause() {
    return AudioPlayer::pause();
}

bool AudioOutput::resume() {
    return AudioPlayer::play();
}

bool AudioOutput::getAudioFrameCallback(AudioFrame **copyFrame) {
    AudioFrame* audioFrame = mGetAudioCallback();
    if (audioFrame == NULL || audioFrame->size <= 0)
        return false;
    *copyFrame = audioFrame;
    return true;
}