//
// Created by chengjunsen on 2018/10/12.
//

#include "media_player_controller.h"

MediaPlayerController::MediaPlayerController() {
    mSynchronizer = new MediaSynchronizer;
    mVideoOutput = new VideoOutput;
    mAudioOutput = new AudioOutput;
}

MediaPlayerController::~MediaPlayerController() {
    if (mSynchronizer != NULL) {
        delete mSynchronizer;
        mSynchronizer = NULL;
    }
    if (mVideoOutput != NULL) {
        delete mVideoOutput;
        mVideoOutput = NULL;
    }
    if (mAudioOutput != NULL) {
        delete mAudioOutput;
        mAudioOutput = NULL;
    }
}

void MediaPlayerController::start(const char *path) {
    mSynchronizer->setVideoOutput(mVideoOutput);
    mSynchronizer->start(path);
}

void MediaPlayerController::stop() {

}

void MediaPlayerController::pause() {

}

void MediaPlayerController::seek(double position) {

}

void MediaPlayerController::resume() {

}

long MediaPlayerController::getDuration() {
    return mSynchronizer->getDuration();
}

long MediaPlayerController::getProgress() {
    return mSynchronizer->getProgress();
}

void MediaPlayerController::onSurfaceCreated(ANativeWindow *window) {
    mVideoOutput->onCreated(window);
}

void MediaPlayerController::onSurfaceSizeChanged(int width, int height) {
    mVideoOutput->onChangeSize(width, height);
}

void MediaPlayerController::onSurfaceDestroy() {
    mVideoOutput->onDestroy();
    mSynchronizer->finish();
}


