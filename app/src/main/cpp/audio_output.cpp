//
// Created by chengjunsen on 2018/10/16.
//

#include "audio_output.h"

AudioOutput::AudioOutput(IAudioOutput *callback) {
    mOutputInterface = callback;
    mData = NULL;
    mDataSize = 0;
}

AudioOutput::~AudioOutput() {
    mOutputInterface = NULL;
    if(mDataSize != 0) {
        delete(mData);
        mData = NULL;
        mDataSize = 0;
    }
}

void AudioOutput::start(int channel, int samplerate) {
    curPresentTime = 0;
    AudioPlayer::create(samplerate, channel);
}


void AudioOutput::finish() {
    AudioPlayer::release();
    if(mDataSize != 0) {
        delete(mData);
        mData = NULL;
        mDataSize = 0;
    }
}

bool AudioOutput::pause() {
    return AudioPlayer::pause();
}

bool AudioOutput::getAudioDataCallback(char **data, int *size) {
    AudioFrame* audioFrame = mOutputInterface->getAudioFrame();
    if (audioFrame == NULL || audioFrame->size <= 0)
        return false;
    copyAudioFrame(audioFrame);
    *data = mData;
    *size = mDataSize;
    return true;
}

void AudioOutput::signalRenderFrame() {
    if (!AudioPlayer::isRunning()) {
        AudioPlayer::play();
    }
}

void AudioOutput::copyAudioFrame(AudioFrame *pFrame) {
//    if (mDataSize != 0) {
//        delete(mData);
//        mDataSize = 0;
//    }
//    mDataSize = pFrame->size;
//    mData = new char[mDataSize];
//    memcmp(mData, pFrame->data, mDataSize);
//    delete pFrame;
    mDataSize = pFrame->size;
    mData = pFrame->data;
}
