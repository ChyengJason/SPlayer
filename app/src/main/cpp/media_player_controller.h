//
// Created by chengjunsen on 2018/10/12.
//

#ifndef SPLAYER_VIDEOPLAYERCONTROLLER_H
#define SPLAYER_VIDEOPLAYERCONTROLLER_H

#include <android/native_window.h>
#include "synchronizer/media_synchronizer.h"

class MediaPlayerController {
public:
    MediaPlayerController();
    ~MediaPlayerController();
    void start(const char *path);
    void stop();
    void pause();
    void seek(float position);
    void resume();
    void release();
    void suspend();
    float getDuration();
    float getProgress();
    void onSurfaceCreated(ANativeWindow* window);
    void onSurfaceSizeChanged(int width, int height);
    void onSurfaceDestroy();

private:
    MediaSynchronizer *mSynchronizer;
};

#endif //SPLAYER_VIDEOPLAYERCONTROLLER_H
