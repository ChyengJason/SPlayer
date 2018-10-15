//
// Created by chengjunsen on 2018/10/15.
//
#ifndef SPLAYER_AUDIOFRAME_H
#define SPLAYER_AUDIOFRAME_H

#include <string>

class VideoFrame {
public:
    double pts;
    int height;
    int width;
    uint8_t * rgb;
    long size;
    long linesize;
};

class AudioFrame {
public:
    int samplerate;
    int channelCount;
    double pts;
    char *data;
    int size;
};


#endif //SPLAYER_AUDIOFRAME_H

//uint8_t * luma;
//uint8_t * chromaB;
//uint8_t * chromaR;