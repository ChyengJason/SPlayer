//
// Created by chengjunsen on 2018/10/16.
//

#ifndef SPLAYER_AUDIO_OUTPUT_H
#define SPLAYER_AUDIO_OUTPUT_H


#include "opensl/audio_player.h"
#include "media_frame.h"

class IAudioOutput {
public:
    virtual ~IAudioOutput() {}
    virtual AudioFrame* getAudioFrame() = 0;
};

class AudioOutput: public AudioPlayer {
public:
    AudioOutput(IAudioOutput* callback);
    virtual ~AudioOutput();
    void start(int channel, int samplerate);
    bool pause();
    bool play();
    void finish();
    void signalRenderFrame();

protected:
    virtual bool getAudioDataCallback(char** data, int* size);

private:
    void copyAudioFrame(AudioFrame *pFrame);

private:
    double curPresentTime;
    IAudioOutput* mOutputInterface;
    char* mData;
    int mDataSize;
};


#endif //SPLAYER_AUDIO_OUTPUT_H
