//
// Created by chengjunsen on 2018/10/12.
//
#include "media_synchronizer.h"

MediaSynchronizer::MediaSynchronizer() {
    mMediaDecoder = NULL;
    mVideoOutput = NULL;
}

MediaSynchronizer::~MediaSynchronizer() {

}

void MediaSynchronizer::prepare(const char *path) {
    mMediaDecoder = new MediaDecoder();
    mMediaDecoder->prepare(path);
}

void MediaSynchronizer::start() {
    startDecodeThread();
}

void MediaSynchronizer::finish() {
    mMediaDecoder->finish();
    delete mMediaDecoder;
    mMediaDecoder = NULL;
    mVideoOutput = NULL;
}

void MediaSynchronizer::startDecodeThread() {
    pthread_cond_init(&mDecoderCond, NULL);
    pthread_mutex_init(&mDecoderMutex, NULL);
    int ret = pthread_create(&mDecoderThread, NULL, runDecoderThread, this);
    if (ret != 0) {
        LOGE("创建 DecodeThread 失败");
    }
}

void *MediaSynchronizer::runDecoderThread(void *self) {
    MediaSynchronizer* synchronizer = (MediaSynchronizer*)self;
    MediaDecoder* videoDecoder = synchronizer->mMediaDecoder;
    bool isVideoFrame;
    while (videoDecoder->readFrame(isVideoFrame)) {
        if (isVideoFrame) {
            VideoFrame* frame = videoDecoder->decodeVideoFrame();
            if (synchronizer->mVideoOutput != NULL) {
                synchronizer->mVideoOutput->output(*frame);
            }
            free(frame->rgb);
            delete frame;
        } else {
            videoDecoder->decodeAudioFrame();
        }
    }
    videoDecoder->finish();
    return 0;
}

void MediaSynchronizer::setVideoOutput(IVideoOutput *videoOutput) {
    mVideoOutput = videoOutput;
}
