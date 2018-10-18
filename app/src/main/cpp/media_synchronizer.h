//
// Created by chengjunsen on 2018/10/12.
//

#ifndef SPLAYER_VIDEO_SYNCHRONIZER_H
#define SPLAYER_VIDEO_SYNCHRONIZER_H

#include "media_decoder.h"
#include "media_frame.h"
#include <queue>
#include "pthread.h"
#include "video_output.h"

/**
 * 负责同步音视频
 */
class MediaSynchronizer {
public:
    MediaSynchronizer();
    ~MediaSynchronizer();
    void start(const char* path);
    void finish();
    long getProgress();
    long getDuration();
    void setVideoOutput(IVideoOutput* mVideoOutput);

private:
    void startDecodeThread();
    static void* runDecoderThread(void* self);
    void clearVideoFrameQue();
    void clearAudioFrameQue();
    void pushVideoFrameQue(VideoFrame*);
    void pushAudioFrameQue(std::vector<AudioFrame*>);
    VideoFrame* popVideoFrameQue();
    AudioFrame* popAudioFrameQue();

private:
    long playTime;
    MediaDecoder* mMediaDecoder;
    pthread_t mDecoderThread;
    pthread_cond_t mDecoderCond;
    pthread_mutex_t mDecoderMutex;
    pthread_mutex_t mVideoQueMutex;
    pthread_mutex_t mAudioQueMutex;

    IVideoOutput* mVideoOutput;
    std::queue<VideoFrame*> mVideoFrameQue;
    std::queue<AudioFrame*> mAudioFrameQue;
};


#endif //SPLAYER_VIDEO_SYNCHRONIZER_H
