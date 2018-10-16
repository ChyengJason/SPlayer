//
// Created by chengjunsen on 2018/10/16.
//

#ifndef SPLAYER_AUDIO_OUTPUT_H
#define SPLAYER_AUDIO_OUTPUT_H


class AudioOutput {
public:
    AudioOutput();
    ~AudioOutput();
    bool start(int channel, int samplerate, int foramt);
    void stop();
};


#endif //SPLAYER_AUDIO_OUTPUT_H
