//
// Created by chengjunsen on 2018/10/12.
//

#include "media_player_controller.h"
#include <vector>

MediaPlayerController::MediaPlayerController() {
    mVideoOutput = new VideoOutput;
    mMediaDecoder = new MediaDecoder;
}

MediaPlayerController::~MediaPlayerController() {

}

void MediaPlayerController::start(const char *path) {
    mMediaDecoder->prepare(path);
    mVideoOutput->start();
    AVPacket* packet;
    int count = 0;
    while ((packet = mMediaDecoder->readFrame()) != NULL) {
        if (mMediaDecoder->isVideoPacket(packet) ) {
            std::vector<VideoFrame*> vec= mMediaDecoder->decodeVideoFrame(packet);
            if (!vec.empty()) {
                mVideoOutput->output(vec[0]);
                break;
            }
            count++;
        }
    }
}

void MediaPlayerController::stop() {
}

void MediaPlayerController::pause() {
}

void MediaPlayerController::seek(double position) {
}

void MediaPlayerController::suspend() {
}

void MediaPlayerController::resume() {
}

long MediaPlayerController::getDuration() {
    return 0;
}

long MediaPlayerController::getProgress() {
    return 0;
}

void MediaPlayerController::onSurfaceCreated(ANativeWindow *window) {
    mVideoOutput->onCreated(window);
}

void MediaPlayerController::onSurfaceSizeChanged(int screenWidth, int screenHeight) {
    mVideoOutput->onChangeSize(screenWidth, screenHeight);
}

void MediaPlayerController::onSurfaceDestroy() {
    mVideoOutput->onDestroy();
}

void MediaPlayerController::release() {
}