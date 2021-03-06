//
// Created by chengjunsen on 2018/10/12.
//

#ifndef SPLAYER_VIDEO_SYNCHRONIZER_H
#define SPLAYER_VIDEO_SYNCHRONIZER_H

#include "pthread.h"
#include "video_queue.h"
#include "audio_queue.h"
#include "../media_decoder.h"
#include "../media_frame.h"
#include "../video_output.h"
#include "../audio_output.h"

const float MAX_BUFFER_DURATION = 0.3;
const float MAX_FRAME_DIFF = 0.002;
const float MAX_JUDGE_DIFF = 1.0;
const float MIN_BUFFER_DURATION = 0.1;
const float MAX_CACHE_PACKET_SIZE = 30;

/**
 * 负责同步音视频
 */
class MediaSynchronizer : public virtual IVideoOutput, public virtual IAudioOutput {
public:
    MediaSynchronizer();
    virtual ~MediaSynchronizer();
    virtual VideoFrame* getVideoFrame();
    virtual AudioFrame* getAudioFrame();
    void start(const char* path);
    void finish();
    void seek(float);
    void pause();
    void resume();
    void onSurfaceCreated(ANativeWindow* window);
    void onSurfaceSizeChanged(int width, int height);
    void onSurfaceDestroy();
    float getDuration();
    float getProgress();

private:
    void startDecodeThread();
    static void* runDecoderThread(void* self);
    void runDecoding();
    void runSeeking();
    bool decodeFrame();
    void correctTime(VideoFrame *videoFrame);
    void correctTime(AudioFrame *audioFrame);

private:
    MediaDecoder* mMediaDecoder;
    pthread_t mDecoderThread;
    pthread_cond_t mDecoderCond;
    pthread_mutex_t mDecoderMutex;
    pthread_cond_t mTextureCond;
    pthread_mutex_t mTextureMutex;
    pthread_cond_t mAudioCond;
    pthread_mutex_t mAudioMutex;
    VideoQueue* mVideoQue;
    AudioQueue* mAudioQue;
    VideoOutput *mVideoOutput;
    AudioOutput *mAudioOutput;
    bool isStarted;
    bool isPaused;
    bool isSeeking;
    long mDuration;
    bool isSurfaceCreated;
    bool isDecodeFinish;
    double mVideoClock;
    double mAudioClock;
    double mVideoDuration;
    double mAudioDuration;
    long seekSeconds;
};

#endif //SPLAYER_VIDEO_SYNCHRONIZER_H
