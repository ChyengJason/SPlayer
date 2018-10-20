//
// Created by chengjunsen on 2018/10/12.
//
#include <unistd.h>
#include "media_synchronizer.h"

MediaSynchronizer::MediaSynchronizer() {
    mMediaDecoder = new MediaDecoder;
    curPresentTime = 0;
}

MediaSynchronizer::~MediaSynchronizer() {
    pthread_mutex_destroy(&mVideoQueMutex);
    pthread_mutex_destroy(&mAudioQueMutex);
    pthread_mutex_destroy(&mDecoderMutex);
    pthread_cond_destroy(&mDecoderCond);
}

void MediaSynchronizer::prepare(const char *path) {
    mMediaDecoder->prepare(path);
}

void MediaSynchronizer::start() {
    curPresentTime = 0;
    startDecodeThread();
}

void MediaSynchronizer::finish() {
    mMediaDecoder->finish();
    clearAudioFrameQue();
    clearVideoFrameQue();
    // 停止线程
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
    int count = 0;
    AVPacket* packet;
    while ((packet = videoDecoder->readFrame()) != NULL) {
//        if (videoDecoder->isVideoPacket(packet)) {
//            LOGD("解码视频帧 %d", ++count);
//            VideoFrame* frame = videoDecoder->decodeVideoFrame(packet);
//            synchronizer->pushVideoFrameQue(frame);
//        } else
    if (videoDecoder->isAudioPacket(packet)){
            LOGD("解码音频帧 %d", ++count);
            std::vector<AudioFrame*> frames = videoDecoder->decodeAudioFrame(packet);
            synchronizer->pushAudioFrameQue(frames);
        }
    }
    videoDecoder->finish();
    return 0;
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

VideoFrame *MediaSynchronizer::getVideoFrame() {
    LOGE("MediaSynchronizer getVideoFrame");
    pthread_mutex_lock(&mVideoQueMutex);
    VideoFrame* frame = NULL;
    if (!mVideoFrameQue.empty()) {
        frame = mVideoFrameQue.front();
        mVideoFrameQue.pop();
    }
    pthread_mutex_unlock(&mVideoQueMutex);
    return frame;
}

AudioFrame *MediaSynchronizer::getAudioFrame() {
    AudioFrame* frame = NULL;
    pthread_mutex_lock(&mAudioQueMutex);
    if (!mAudioFrameQue.empty()) {
        frame = mAudioFrameQue.front();
        mAudioFrameQue.pop();
    }
    LOGE("MediaSynchronizer getAudioFrame queue size: %d", mAudioFrameQue.size());
    pthread_mutex_unlock(&mAudioQueMutex);
    return frame;
}

long MediaSynchronizer::getProgress() {
    return mMediaDecoder->getMediaDuration();
}

long MediaSynchronizer::getDuration() {
    return curPresentTime;
}

int MediaSynchronizer::getSamplerate() {
    return mMediaDecoder ? mMediaDecoder->getSamplerate() : 0;
}

int MediaSynchronizer::getChannelCount() {
    return mMediaDecoder ? mMediaDecoder->getChannelCount() : 0;
}