//
// Created by chengjunsen on 2018/10/12.
//

#include <unistd.h>
#include "media_player_controller.h"

static MediaPlayerController*instance = NULL;

MediaPlayerController::MediaPlayerController() {
    mSynchronizer = new MediaSynchronizer;
    instance = this;
}

MediaPlayerController::~MediaPlayerController() {

}

void MediaPlayerController::start(const char *path) {
    mSynchronizer->start(path);
}

void MediaPlayerController::stop() {
}

void MediaPlayerController::pause() {
    mSynchronizer->pause();
}

void MediaPlayerController::seek(float position) {
    mSynchronizer->seek(position);
}

void MediaPlayerController::suspend() {
}

void MediaPlayerController::resume() {
    mSynchronizer->resume();
}

float MediaPlayerController::getDuration() {
    return mSynchronizer->getDuration();
}

float MediaPlayerController::getProgress() {
    return mSynchronizer->getProgress();
}

void MediaPlayerController::onSurfaceCreated(ANativeWindow *window) {
    mSynchronizer->onSurfaceCreated(window);
}

void MediaPlayerController::onSurfaceSizeChanged(int screenWidth, int screenHeight) {
    mSynchronizer->onSurfaceSizeChanged(screenWidth, screenHeight);
}

void MediaPlayerController::onSurfaceDestroy() {
    mSynchronizer->onSurfaceDestroy();
}

void MediaPlayerController::release() {
    if (mSynchronizer != NULL) {
        delete mSynchronizer;
        mSynchronizer = NULL;
    }
    if (instance != NULL) {
        instance = NULL;
    }
}
