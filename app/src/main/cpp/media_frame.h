//
// Created by chengjunsen on 2018/10/15.
//
#ifndef SPLAYER_AUDIOFRAME_H
#define SPLAYER_AUDIOFRAME_H

#include <string>
#include "android_log.h"

class VideoFrame {
public:
    VideoFrame():luma(NULL), chromaB(NULL), chromaR(NULL) {}
    ~VideoFrame() {
        if (luma) delete(luma);
        if (chromaB) delete(chromaB);
        if (chromaR) delete(chromaR);
    }
    uint8_t * luma;
    uint8_t * chromaB;
    uint8_t * chromaR;
    double pts;
    int height;
    int width;
};

class AudioFrame {
public:
    AudioFrame() : data(NULL) {}
    ~AudioFrame() {
       if (data) delete(data);
    }
    int samplerate;
    int channelCount;
    double pts;
    char *data;
    int size;
};

#endif //SPLAYER_AUDIOFRAME_H

