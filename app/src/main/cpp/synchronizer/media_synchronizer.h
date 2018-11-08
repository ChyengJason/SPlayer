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

const float MAX_BUFFER_DURATION = 3.0;
const float MAX_FRAME_DIFF = 0.002;
const float MAX_JUDGE_DIFF = 1.0;

/**
 * 负责同步音视频
 */
class MediaSynchronizer : public virtual IVideoOutput, public virtual IAudioOutput {
public:
    MediaSynchronizer();
    virtual ~MediaSynchronizer();
    virtual TextureFrame* getTetureFrame();
    virtual AudioFrame* getAudioFrame();
    void prepare(const char* path);
    void start();
    void finish();
    void seek(float);
    void onSurfaceCreated(ANativeWindow* window);
    void onSurfaceSizeChanged(int width, int height);
    void onSurfaceDestroy();
    float getDuration();
    float getProgress();
private:
    void startDecodeThread();
    static void* runDecoderThread(void* self);
    void runDecoding();
    bool decodeFrame();
    void correctTime(TextureFrame *textureFrame);
    void correctTime(AudioFrame *audioFrame);

private:
    MediaDecoder* mMediaDecoder;
    pthread_t mDecoderThread;
    pthread_cond_t mDecoderCond;
    pthread_mutex_t mDecoderMutex;
    pthread_cond_t mTextureCond;
    pthread_cond_t mAudioCond;
    VideoQueue* mTextureQue;
    AudioQueue* mAudioQue;
    VideoOutput *mVideoOutput;
    AudioOutput *mAudioOutput;
    long mDuration;
    bool isSurfaceCreated;
    double mVideoClock;
    double mAudioClock;
    double mVideoDuration;
    double mAudioDuration;
    double mSeekSeconds;
    bool isVideoSeek;
    bool isAudioSeek;
};

#endif //SPLAYER_VIDEO_SYNCHRONIZER_H
