//
// Created by chengjunsen on 2018/10/12.
//
#include <unistd.h>
#include "media_synchronizer.h"

MediaSynchronizer::MediaSynchronizer() {
    mMediaDecoder = new MediaDecoder;
    mAudioQue = new AudioQueue;
    mTextureQue = new VideoQueue;
    curPresentTime = 0;
}

MediaSynchronizer::~MediaSynchronizer() {
    pthread_mutex_destroy(&mDecoderMutex);
    pthread_cond_destroy(&mDecoderCond);
    delete(mMediaDecoder);
    delete(mAudioQue);
    delete(mTextureQue);
}

void MediaSynchronizer::prepare(const char *path) {
    mMediaDecoder->prepare(path);
    int width = mMediaDecoder->getWidth();
    int height = mMediaDecoder->getHeight();
//    mTextureQue->start(width, height);
    mAudioQue->start();
}

void MediaSynchronizer::start() {
    curPresentTime = 0;
    startDecodeThread();
}

void MediaSynchronizer::finish() {
    mMediaDecoder->finish();
    mAudioQue->release();
//    mTextureQue->release();
    // 停止线程
}

void MediaSynchronizer::startDecodeThread() {
    LOGE("创建解码线程");
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
    int count = 0;
    AVPacket* packet;
    while ((packet = videoDecoder->readFrame()) != NULL) {
        if (videoDecoder->isVideoPacket(packet)) {
            LOGD("解码视频帧 %d", ++count);
//            VideoFrame* frame = videoDecoder->decodeVideoFrame(packet);
//            synchronizer->mTextureQue->push(frame);
        } else if (videoDecoder->isAudioPacket(packet)){
            LOGD("解码音频帧 NO.%d", ++count);
            std::vector<AudioFrame*> frames = videoDecoder->decodeAudioFrame(packet);
            synchronizer->mAudioQue->push(frames);
        }
    }
    videoDecoder->finish();
    return 0;
}

TextureFrame *MediaSynchronizer::getTextureFrame() {
//    return mTextureQue->pop();
    return NULL;
}

AudioFrame *MediaSynchronizer::getAudioFrame() {
    return mAudioQue->pop();
//    return NULL;
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
