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
        if (luma) {
            LOGE("delete luma");
            delete (luma);
        }
        if (chromaB) {
            LOGE("delete chromaB");
            delete (chromaB);
        }
        if (chromaR) {
            LOGE("delete chromaR");
            delete (chromaR);
        }
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
    AudioFrame() : data(NULL) {}
    ~AudioFrame() {
       if (data) delete(data);
    }
    int samplerate;
    int channels;
    double timestamp;
    char *data;
    int size;
    double duration;
};


class TextureFrame {
public:
    TextureFrame() {}
    ~TextureFrame() {}
    double timestamp;
    int screenHeight;
    int screenWidth;
    int textureId;
    double duration;
};
#endif //SPLAYER_AUDIOFRAME_H

