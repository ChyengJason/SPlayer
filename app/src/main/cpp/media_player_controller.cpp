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
    mStatus = UNINITED;
}

MediaPlayerController::~MediaPlayerController() {

}

void MediaPlayerController::start(const char *path) {
    if (mStatus != UNINITED) {
        LOGE("start -> MediaPlayerController is not uninited %d", mStatus);
        return;
    }
    mSynchronizer->prepare(path);
    mSynchronizer->start();
    int channelCount = mSynchronizer->getChannelCount();
    int samplerate = mSynchronizer->getSamplerate();
    mAudioOutput->start(channelCount, samplerate, getAudioFrame);
    mStatus = PLAY;
    if (!mVideoOutput->isSurfaceValid()) {
        pause();
    }
}

void MediaPlayerController::stop() {
    if (mStatus == STOP || mStatus == UNINITED) {
        LOGE("stop -> MediaPlayerController is stopped or uninited %d", mStatus);
        return;
    }
    mSynchronizer->finish();
    mVideoOutput->onDestroy();
    mAudioOutput->stop();
    mStatus = STOP;
    release();
}

void MediaPlayerController::pause() {
    if (mStatus != PLAY) {
        LOGE("pause -> MediaPlayerController is not playing %d", mStatus);
        return;
    }
    mStatus = PAUSE;
    mAudioOutput->pause();
}

void MediaPlayerController::seek(double position) {
    if (mStatus != PLAY && mStatus != PAUSE) {
        LOGE("seek -> MediaPlayerController is not playing or paused %d", mStatus);
        return;
    }
}

void MediaPlayerController::suspend() {
    if (mStatus != PLAY) {
        LOGE("suspend -> MediaPlayerController is not playing %d", mStatus);
        return;
    }
    mStatus = SUSPEND;
    mAudioOutput->pause();
}

void MediaPlayerController::resume() {
    if ( mStatus != PAUSE && mStatus != SUSPEND) {
        LOGE("resume -> MediaPlayerController is not paused or suspend %d", mStatus);
        return;
    }
    mStatus = PLAY;
    mAudioOutput->resume();
}

long MediaPlayerController::getDuration() {
    return mSynchronizer->getDuration();
}

long MediaPlayerController::getProgress() {
    return mSynchronizer->getProgress();
}

void MediaPlayerController::onSurfaceCreated(ANativeWindow *window) {
    if (mStatus == UNINITED) {
        mVideoOutput->onCreated(window);
    } else {
        mVideoOutput->onUpdated(window);
    }
    if (mStatus == SUSPEND) {
        resume();
    }
}

void MediaPlayerController::onSurfaceSizeChanged(int width, int height) {
    mVideoOutput->onChangeSize(width, height);
    mSynchronizer->changeSize(width, height);
}

void MediaPlayerController::onSurfaceDestroy() {
    mVideoOutput->onDestroy();
    LOGE("mstatus %d", mStatus);
    if (mStatus == PLAY) {
        suspend();
    }
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
    return instance->mSynchronizer->getAudioFrame();
}

TextureFrame *MediaPlayerController::getTextureFrame() {
    return instance->mSynchronizer->getTextureFrame();
}