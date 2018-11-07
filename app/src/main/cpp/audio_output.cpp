//
// Created by chengjunsen on 2018/10/16.
//

#include "audio_output.h"

AudioOutput::AudioOutput(IAudioOutput *callback) {
    mOutputInterface = callback;
}

AudioOutput::~AudioOutput() {
    mOutputInterface = NULL;
}

void AudioOutput::start(int channel, int samplerate) {
    AudioPlayer::create(samplerate, channel);
}


void AudioOutput::finish() {
    AudioPlayer::release();
}

bool AudioOutput::pause() {
    return AudioPlayer::pause();
}

bool AudioOutput::getAudioDataCallback(char **data, int *size) {
    AudioFrame* audioFrame = mOutputInterface->getAudioFrame();
    if (audioFrame == NULL || audioFrame->size <= 0) {
        *size = 4;
        *data = new char[*size];
        memset(*data, 0, *size);
    } else {
        *size = audioFrame->size;
        *data = audioFrame->data;
    }
    delete audioFrame;
    return true;
}

void AudioOutput::signalRenderFrame() {
    if (!AudioPlayer::isRunning()) {
        AudioPlayer::play();
    }
}
