//
// Created by chengjunsen on 2018/10/12.
//

#ifndef SPLAYER_VIDEOPLAYERCONTROLLER_H
#define SPLAYER_VIDEOPLAYERCONTROLLER_H


#include <android/native_window.h>
#include "media_synchronizer.h"
#include "audio_output.h"

enum MediaStatus {
    UNINITED,
    STOP,
    PLAY,
    PAUSE,
    SUSPEND
};

class MediaPlayerController {
public:
    MediaPlayerController();
    ~MediaPlayerController();
    void start(const char *path);
    void stop();
    void pause();
    void seek(double position);
    void resume();
    void release();
    void suspend();
    long getDuration();
    long getProgress();
    void onSurfaceCreated(ANativeWindow* window);
    void onSurfaceSizeChanged(int width, int height);
    void onSurfaceDestroy();

private:
    static VideoFrame* getVideoFrame();
    static AudioFrame* getAudioFrame();

private:
    MediaSynchronizer *mSynchronizer;
    VideoOutput *mVideoOutput;
    AudioOutput *mAudioOutput;
    MediaStatus mStatus;
};


#endif //SPLAYER_VIDEOPLAYERCONTROLLER_H
