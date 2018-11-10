//
// Created by chengjunsen on 2018/10/22.
//

#ifndef SPLAYER_AUDIO_QUEUE_H
#define SPLAYER_AUDIO_QUEUE_H

#include <queue>
#include <libavcodec/avcodec.h>
#include "../media_frame.h"
#include "../media_decoder.h"

class AudioQueue {
public:
    AudioQueue();
    ~AudioQueue();
    void start(MediaDecoder* mediaDecoder);
    void finish();
    void push(AVPacket* packet);
    AudioFrame* pop();
    bool isEmpty();
    void clear();
    int size();
    bool isRunning();
    double getAllDuration();
    int packetCacheSize();
    bool clearing();

private:
    static void* runDecode(void* self);
    void runDecodeImpl();
    void runClearing();
    void runDecoding();

private:
    bool isFinish;
    pthread_mutex_t mFrameQueMutex;
    pthread_mutex_t mPacketMutex;
    pthread_cond_t mDecodeCond;
    pthread_t mDecodeThread;
    pthread_mutex_t mDecodeMutex;
    std::queue<AVPacket*> mPacketQue;
    std::queue<AudioFrame*> mAudioFrameQue;
    double mAllDuration;
    MediaDecoder* mMediaDecoder;
    bool isClearing;
};


#endif //SPLAYER_AUDIO_QUEUE_H
