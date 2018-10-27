//
// Created by chengjunsen on 2018/10/12.
//

#ifndef SPLAYER_VIDEOPLAYERCONTROLLER_H
#define SPLAYER_VIDEOPLAYERCONTROLLER_H


#include <android/native_window.h>
#include "media_synchronizer.h"
#include "audio_output.h"

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
    static TextureFrame* getTextureFrame();
    static AudioFrame* getAudioFrame();
    static void* run(void* self);
    void startImp();
private:
//    MediaSynchronizer *mSynchronizer;
    VideoOutput *mVideoOutput;
    MediaDecoder *mMediaDecoder;
    VideoQueue *mVieoQue;
//    AudioOutput *mAudioOutput;
    pthread_t mThread;
    char* mPath;
};


#endif //SPLAYER_VIDEOPLAYERCONTROLLER_H
