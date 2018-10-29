//
// Created by chengjunsen on 2018/10/12.
//

#ifndef SPLAYER_VIDEO_SYNCHRONIZER_H
#define SPLAYER_VIDEO_SYNCHRONIZER_H

#include "../media_decoder.h"
#include "../media_frame.h"
#include <queue>
#include "pthread.h"
#include "../video_output.h"
#include "video_queue.h"
#include "audio_queue.h"
#include "../audio_output.h"

const float MAX_BUFFER_DURATION = 1.0;
const float MIN_BUFFER_DURATION = 0.5;
const float MAX_FRAME_DIFF_DIFF = 0.05;
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
    long getProgress();
    long getDuration();
    int getSamplerate();
    int getChannelCount();
    void onSurfaceCreated(ANativeWindow* window);
    void onSurfaceSizeChanged(int width, int height);
    void onSurfaceDestroy();
    void setWaterMark(int imgWidth, int imgHeight, void *buffer);

private:
    void startDecodeThread();
    static void* runDecoderThread(void* self);
    void runDecoding();

private:
    long curPresentTime;
    bool isRunning;
    MediaDecoder* mMediaDecoder;
    pthread_t mDecoderThread;
    pthread_cond_t mDecoderCond;
    pthread_mutex_t mDecoderMutex;
    VideoQueue* mTextureQue;
    AudioQueue* mAudioQue;
    VideoOutput *mVideoOutput;
    AudioOutput *mAudioOutput;
};

#endif //SPLAYER_VIDEO_SYNCHRONIZER_H
