//
// Created by chengjunsen on 2018/10/12.
//

#include "media_player_controller.h"

static MediaPlayerController*instance = NULL;

MediaPlayerController::MediaPlayerController() {
    mSynchronizer = new MediaSynchronizer;
    mVideoOutput = new VideoOutput;
    mAudioOutput = new AudioOutput;
    instance = this;
}

MediaPlayerController::~MediaPlayerController() {

}

void MediaPlayerController::start(const char *path) {
    mSynchronizer->prepare(path);
    int channelCount = mSynchronizer->getChannelCount();
    int samplerate = mSynchronizer->getSamplerate();
    LOGE("channelCount: %d samplerate:%d", channelCount, samplerate);
    mSynchronizer->start();
    mAudioOutput->start(channelCount, samplerate, getAudioFrame);
}

void MediaPlayerController::stop() {
    mSynchronizer->finish();
    mVideoOutput->onDestroy();
    mAudioOutput->stop();
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
    mSynchronizer->finish();
    mVideoOutput->onDestroy();
    mAudioOutput->stop();
}

void MediaPlayerController::release() {
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
    if (instance != NULL) {
        instance = NULL;
    }
}

AudioFrame *MediaPlayerController::getAudioFrame() {
    LOGE("MediaPlayerController getAudioFrame");
    return instance->mSynchronizer->getAudioFrame();
}

VideoFrame *MediaPlayerController::getVideoFrame() {
    return instance->mSynchronizer->getVideoFrame();
}




