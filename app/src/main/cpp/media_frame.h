//
// Created by chengjunsen on 2018/10/15.
//
#ifndef SPLAYER_AUDIOFRAME_H
#define SPLAYER_AUDIOFRAME_H

#include <string>
#include "util/android_log.h"

class VideoFrame {
public:
    VideoFrame():luma(NULL), chromaB(NULL), chromaR(NULL) {}
    ~VideoFrame() {
        if (luma) delete (luma);
        if (chromaB) delete (chromaB);
        if (chromaR) delete (chromaR);
    }

public:
    uint8_t * luma;
    uint8_t * chromaB;
    uint8_t * chromaR;
    double timestamp;
    int frameHeight;
    int frameWidth;
    double duration;
};

class AudioFrame {
public:
    AudioFrame() : isSkip(false), data(NULL) {}
    ~AudioFrame() {
    }
    int samplerate;
    int channels;
    double timestamp;
    char *data;
    int size;
    double duration;
    bool isSkip;
};


class TextureFrame {
public:
    TextureFrame(): isSkip(false) {}
    ~TextureFrame() {}
    double timestamp;
    int screenHeight;
    int screenWidth;
    int textureId;
    double duration;
    bool isSkip;
};
#endif //SPLAYER_AUDIOFRAME_H

