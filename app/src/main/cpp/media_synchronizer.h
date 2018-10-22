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
#include "video_queue.h"
#include "audio_queue.h"

const float MAX_BUFFER_DURATION = 1.0;
const float MIN_BUFFER_DURATION = 0.5;
const float MAX_FRAME_DIFF_DIFF = 0.05;
/**
 * 负责同步音视频
 */
class MediaSynchronizer {
public:
    MediaSynchronizer();
    ~MediaSynchronizer();
    void prepare(const char* path);
    void changeSize(int width, int height);
    void start();
    void finish();
    long getProgress();
    long getDuration();
    int getSamplerate();
    int getChannelCount();
    TextureFrame* getTextureFrame();
    AudioFrame* getAudioFrame();

private:
    void startDecodeThread();
    static void* runDecoderThread(void* self);

private:
    long curPresentTime;
    MediaDecoder* mMediaDecoder;
    pthread_t mDecoderThread;
    pthread_cond_t mDecoderCond;
    pthread_mutex_t mDecoderMutex;
    VideoQueue* mTextureQue;
    AudioQueue* mAudioQue;
};

#endif //SPLAYER_VIDEO_SYNCHRONIZER_H
