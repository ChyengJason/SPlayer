//
// Created by chengjunsen on 2018/10/12.
//

#include <unistd.h>
#include "media_player_controller.h"

static MediaPlayerController*instance = NULL;

MediaPlayerController::MediaPlayerController() {
//    mSynchronizer = new MediaSynchronizer;
    mVideoOutput = new VideoOutput;
    mMediaDecoder = new MediaDecoder;
    mVieoQue = new VideoQueue;
//    mAudioOutput = new AudioOutput;
    instance = this;
//    mStatus = UNINITED;
}

MediaPlayerController::~MediaPlayerController() {

}

void MediaPlayerController::start(const char *path) {
//    if (mStatus != UNINITED) {
//        LOGE("start -> MediaPlayerController is not uninited %d", mStatus);
//        return;
//    }
//    mSynchronizer->prepare(path);
//    mSynchronizer->start();
//    int channelCount = mSynchronizer->getChannelCount();
//    int samplerate = mSynchronizer->getSamplerate();
//    mAudioOutput->start(channelCount, samplerate, getAudioFrame);
//    mStatus = PLAY;
//    if (!mVideoOutput->isSurfaceValid()) {
//        pause();
//    }
//    int count = 0;
//    while (mStatus == PLAY) {
//        TextureFrame* textureFrame = getTextureFrame();
//        if ( textureFrame != NULL) {
//            LOGE("controller output textureFrame %d", textureFrame->textureId);
//            mVideoOutput->output(textureFrame);
//        }
//        if (count++ > 3) {
//            break;
//        }
//    }
    mMediaDecoder->prepare(path);
    mVideoOutput->start();
    mVieoQue->start(mMediaDecoder->getWidth(), mMediaDecoder->getHeight());
    int count = 0;
    TextureFrame* textureFrame = NULL;
    while(true) {
        AVPacket* packet = mMediaDecoder->readFrame();
        LOGE("readFrame finish");
        if (mMediaDecoder->isVideoPacket(packet)) {
            count++;
            std::vector<VideoFrame*> frames = mMediaDecoder->decodeVideoFrame(packet);
            mVieoQue->push(frames);
            if ((textureFrame = mVieoQue->pop()) != NULL) {
                mVideoOutput->output(textureFrame);
                usleep(1000 * 100);
                delete(textureFrame);
            }
        }
        if (count > 50) {
            break;
        }
    }
}

void MediaPlayerController::stop() {
//    if (mStatus == STOP || mStatus == UNINITED) {
//        LOGE("stop -> MediaPlayerController is stopped or uninited %d", mStatus);
//        return;
//    }
//    mSynchronizer->finish();
//    mVideoOutput->onDestroy();
//    mAudioOutput->stop();
//    mStatus = STOP;
//    release();
}

void MediaPlayerController::pause() {
//    if (mStatus != PLAY) {
//        LOGE("pause -> MediaPlayerController is not playing %d", mStatus);
//        return;
//    }
//    mStatus = PAUSE;
//    mAudioOutput->pause();
}

void MediaPlayerController::seek(double position) {
//    if (mStatus != PLAY && mStatus != PAUSE) {
//        LOGE("seek -> MediaPlayerController is not playing or paused %d", mStatus);
//        return;
//    }
}

void MediaPlayerController::suspend() {
//    if (mStatus != PLAY) {
//        LOGE("suspend -> MediaPlayerController is not playing %d", mStatus);
//        return;
//    }
//    mStatus = SUSPEND;
//    mAudioOutput->pause();
}

void MediaPlayerController::resume() {
//    if ( mStatus != PAUSE && mStatus != SUSPEND) {
//        LOGE("resume -> MediaPlayerController is not paused or suspend %d", mStatus);
//        return;
//    }
//    mStatus = PLAY;
//    mAudioOutput->resume();
}

long MediaPlayerController::getDuration() {
//    return mSynchronizer->getDuration();
    return 0;
}

long MediaPlayerController::getProgress() {
//    return mSynchronizer->getProgress();
    return 0;
}

void MediaPlayerController::onSurfaceCreated(ANativeWindow *window) {
//    if (mStatus == UNINITED) {
        mVideoOutput->onCreated(window);
//    } else {
//        mVideoOutput->onUpdated(window);
//    }
//    if (mStatus == SUSPEND) {
//        resume();
//    }
}

void MediaPlayerController::onSurfaceSizeChanged(int screenWidth, int screenHeight) {
    mVideoOutput->onChangeSize(screenWidth, screenHeight);
}

void MediaPlayerController::onSurfaceDestroy() {
    mVideoOutput->onDestroy();
//    LOGE("MediaPlayerController onSurfaceDestroy status %d", mStatus);
//    if (mStatus == PLAY) {
//        suspend();
//    }
}

void MediaPlayerController::release() {
//    if (mSynchronizer != NULL) {
//        delete mSynchronizer;
//        mSynchronizer = NULL;
//    }
    if (mVideoOutput != NULL) {
        delete mVideoOutput;
        mVideoOutput = NULL;
    }
//    if (mAudioOutput != NULL) {
//        delete mAudioOutput;
//        mAudioOutput = NULL;
//    }
    if (instance != NULL) {
        instance = NULL;
    }
}

AudioFrame *MediaPlayerController::getAudioFrame() {
//    return instance->mSynchronizer->getAudioFrame();
    return NULL;
}

TextureFrame *MediaPlayerController::getTextureFrame() {
//    return instance->mSynchronizer->getTextureFrame();
    return NULL;
}