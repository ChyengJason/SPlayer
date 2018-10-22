//
// Created by chengjunsen on 2018/10/22.
//

#ifndef SPLAYER_AUDIO_QUEUE_H
#define SPLAYER_AUDIO_QUEUE_H

#include <queue>
#include "media_frame.h"

class AudioQueue {
public:
    AudioQueue();
    ~AudioQueue();
    void push(AudioFrame* frame);
    void push(std::vector<AudioFrame*> frames);
    AudioFrame* pop();
    bool isEmpty();
    void clear();
    int size();

public:
    pthread_mutex_t mQueMutex;
    std::queue<AudioFrame*> mAudioFrameQue;

    void release();

    void start();
};


#endif //SPLAYER_AUDIO_QUEUE_H
