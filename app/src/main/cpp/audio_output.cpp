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

int AudioOutput::getPcmDataCallback(char **buffer, int maxSize) {
    AudioFrame* audioFrame = mGetAudioCallback();
    LOGE("audio output getPcmDataCallback actualSize: %d maxsize: %d", audioFrame->size, maxSize);
    int actualSize = 0;
    if (audioFrame != NULL && audioFrame->size > 0) {
        curPresentTime = audioFrame->pts;
        actualSize = audioFrame->size;
        *buffer = audioFrame->data;
        //memcpy(buffer, audioFrame->data, std::min(audioFrame->size, maxSize));
    }
    LOGE("audio output getPcmDataCallback2: %d", actualSize);
    //delete(audioFrame);
    return actualSize;
}
