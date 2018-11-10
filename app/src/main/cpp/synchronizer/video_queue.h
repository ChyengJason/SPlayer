//
// Created by chengjunsen on 2018/10/22.
//

#ifndef SPLAYER_VIDEO_QUEUE_H
#define SPLAYER_VIDEO_QUEUE_H

#include <queue>
#include "../media_decoder.h"

class VideoQueue {
public:
    VideoQueue();
    ~VideoQueue();
    void start(MediaDecoder* mediaDecoder);
    void finish();
    void push(AVPacket* packet);
    int packetCacheSize();
    VideoFrame* pop();
    bool isEmpty();
    void clear();
    int size();
    bool isRunning();
    double getAllDuration();
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
    std::queue<VideoFrame*> mVideoFrameQue;
    double mAllDuration;
    MediaDecoder* mMediaDecoder;
    bool isClearing;
};


#endif //SPLAYER_VIDEO_QUEUE_H
