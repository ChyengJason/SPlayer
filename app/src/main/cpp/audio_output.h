//
// Created by chengjunsen on 2018/10/16.
//

#ifndef SPLAYER_AUDIO_OUTPUT_H
#define SPLAYER_AUDIO_OUTPUT_H


#include "opensl/audio_player.h"
#include "media_frame.h"

typedef AudioFrame* (*GetAudioFrameCallback)();

class AudioOutput: public AudioPlayer {
public:
    AudioOutput();
    ~AudioOutput();
    bool start(int channel, int samplerate, GetAudioFrameCallback);
    bool pause();
    bool resume();
    void stop();

protected:
    virtual int getPcmDataCallback(char**buffer, int maxSize)  ;

private:
    double curPresentTime;
    GetAudioFrameCallback mGetAudioCallback;
};


#endif //SPLAYER_AUDIO_OUTPUT_H
