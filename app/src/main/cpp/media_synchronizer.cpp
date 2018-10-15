//
// Created by chengjunsen on 2018/10/12.
//

#include "media_synchronizer.h"

MediaSynchronizer::MediaSynchronizer() {
    mMediaDecoder = NULL;
    mPath = NULL;
}

MediaSynchronizer::~MediaSynchronizer() {

}

void MediaSynchronizer::start(const char *path) {
    mPath = new char[strlen(path)];
    strcpy(mPath, path);
    startDecodeThread();
}

void MediaSynchronizer::finish() {

}

void MediaSynchronizer::startDecodeThread() {
    mMediaDecoder = new MediaDecoder();
    pthread_cond_init(&mDecoderCond, NULL);
    pthread_mutex_init(&mDecoderMutex, NULL);
    int ret = pthread_create(&mDecoderThread, NULL, runDecoderThread, this);
    if (ret != 0) {
        LOGE("创建 DecodeThread 失败");
    }
}

void *MediaSynchronizer::runDecoderThread(void *self) {
    MediaSynchronizer* synchronizer = (MediaSynchronizer*)self;
    MediaDecoder* dediaDecoder = synchronizer->mMediaDecoder;
    dediaDecoder->start(synchronizer->mPath);
    bool isVideoFrame;
    while (dediaDecoder->readFrame(isVideoFrame)) {
        if (isVideoFrame) {
            dediaDecoder->decodeVideoFrame();
        } else {
            dediaDecoder->decodeAudioFrame();
        }
    }
    dediaDecoder->end();
    return 0;
}
