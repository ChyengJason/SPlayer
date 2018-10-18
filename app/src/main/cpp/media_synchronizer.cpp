//
// Created by chengjunsen on 2018/10/12.
//
#include "media_synchronizer.h"

MediaSynchronizer::MediaSynchronizer() {
    mMediaDecoder = new MediaDecoder;
    mVideoOutput = NULL;
    playTime = 0;
}

MediaSynchronizer::~MediaSynchronizer() {
    pthread_mutex_destroy(&mVideoQueMutex);
    pthread_mutex_destroy(&mAudioQueMutex);
    pthread_mutex_destroy(&mDecoderMutex);
    pthread_cond_destroy(&mDecoderCond);
}

void MediaSynchronizer::start(const char *path) {
    playTime = 0;
    mMediaDecoder->prepare(path);
    startDecodeThread();
}

void MediaSynchronizer::finish() {
    mMediaDecoder->finish();
    mVideoOutput = NULL;
}

void MediaSynchronizer::startDecodeThread() {
    LOGE("创建解码线程");
    pthread_cond_init(&mDecoderCond, NULL);
    pthread_mutex_init(&mDecoderMutex, NULL);
    pthread_mutex_init(&mVideoQueMutex, NULL);
    pthread_mutex_init(&mAudioQueMutex, NULL);
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
            LOGE("解码视频帧");
            VideoFrame* frame = videoDecoder->decodeVideoFrame();
            if (synchronizer->mVideoOutput != NULL) {
                synchronizer->mVideoOutput->output(*frame);
            }
            delete frame;
        } else {
            LOGE("解码音频帧");
            videoDecoder->decodeAudioFrame();
        }
    }
    videoDecoder->finish();
    return 0;
}

void MediaSynchronizer::setVideoOutput(IVideoOutput *videoOutput) {
    mVideoOutput = videoOutput;
}

void MediaSynchronizer::clearVideoFrameQue() {
    pthread_mutex_lock(&mVideoQueMutex);
    while(!mVideoFrameQue.empty()) {
        mVideoFrameQue.pop();
    }
    pthread_mutex_unlock(&mVideoQueMutex);
}

void MediaSynchronizer::clearAudioFrameQue() {
    pthread_mutex_lock(&mAudioQueMutex);
    while(!mAudioFrameQue.empty()) {
        mAudioFrameQue.pop();
    }
    pthread_mutex_unlock(&mAudioQueMutex);
}

void MediaSynchronizer::pushAudioFrameQue(std::vector<AudioFrame*> vec) {
    pthread_mutex_lock(&mAudioQueMutex);
    std::vector<AudioFrame*>::iterator begin = vec.begin();
    for (; begin != vec.end() ; ++begin) {
        mAudioFrameQue.push(*begin);
    }
    pthread_mutex_unlock(&mAudioQueMutex);
}

void MediaSynchronizer::pushVideoFrameQue(VideoFrame *frame) {
    pthread_mutex_lock(&mVideoQueMutex);
    mVideoFrameQue.push(frame);
    pthread_mutex_unlock(&mVideoQueMutex);
}

VideoFrame *MediaSynchronizer::popVideoFrameQue() {
    pthread_mutex_lock(&mVideoQueMutex);
    VideoFrame* frame = NULL;
    if (!mVideoFrameQue.empty()) {
        frame = mVideoFrameQue.front();
        mVideoFrameQue.pop();
    }
    pthread_mutex_unlock(&mVideoQueMutex);
    return frame;
}

AudioFrame *MediaSynchronizer::popAudioFrameQue() {
    pthread_mutex_lock(&mAudioQueMutex);
    AudioFrame* frame = NULL;
    if (!mAudioFrameQue.empty()) {
        frame = mAudioFrameQue.front();
        mAudioFrameQue.pop();
    }
    pthread_mutex_unlock(&mAudioQueMutex);
    return frame;
}

long MediaSynchronizer::getProgress() {
    return mMediaDecoder->getMediaDuration();
}

long MediaSynchronizer::getDuration() {
    return playTime;
}

