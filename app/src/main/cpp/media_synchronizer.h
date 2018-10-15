//
// Created by chengjunsen on 2018/10/12.
//

#ifndef SPLAYER_VIDEO_SYNCHRONIZER_H
#define SPLAYER_VIDEO_SYNCHRONIZER_H

#include "media_decoder.h"
#include "media_frame.h"
#include <queue>
/**
 * 负责同步音视频
 */
class MediaSynchronizer {
public:
    MediaSynchronizer();
    ~MediaSynchronizer();
    void start(const char* path);
    void finish();

private:
    void startDecodeThread();
    static void* runDecoderThread(void* self);

private:
    char* mPath;
    MediaDecoder* mMediaDecoder;
    pthread_t mDecoderThread;
    pthread_cond_t mDecoderCond;
    pthread_mutex_t mDecoderMutex;
    std::queue<VideoFrame> mVideoFrameQue;
    std::queue<AudioFrame> mAudioFrameQue;
};


#endif //SPLAYER_VIDEO_SYNCHRONIZER_H
