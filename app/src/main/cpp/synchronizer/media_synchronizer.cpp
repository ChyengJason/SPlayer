//
// Created by chengjunsen on 2018/10/12.
//
#include <unistd.h>
#include "media_synchronizer.h"

MediaSynchronizer::MediaSynchronizer() {
    pthread_cond_init(&mDecoderCond, NULL);
    pthread_mutex_init(&mDecoderMutex, NULL);
    mMediaDecoder = new MediaDecoder;
    mAudioQue = new AudioQueue;
    mTextureQue = new VideoQueue;
    mVideoOutput = new VideoOutput(this);
    mAudioOutput = new AudioOutput(this);
    curPresentTime = 0;
}

MediaSynchronizer::~MediaSynchronizer() {
    pthread_mutex_destroy(&mDecoderMutex);
    pthread_cond_destroy(&mDecoderCond);
    delete(mMediaDecoder);
    delete(mAudioQue);
    delete(mTextureQue);
    delete(mVideoOutput);
    delete(mAudioOutput);
}

void MediaSynchronizer::prepare(const char *path) {
    mMediaDecoder->prepare(path);
}

void MediaSynchronizer::start() {
    if (!mTextureQue->isRunning()) {
        mTextureQue->start(mMediaDecoder->getWidth(),  mMediaDecoder->getHeight());
    }
    if (!mAudioQue->isRunning()) {
        mAudioQue->start();
    }
    mAudioOutput->start(getChannelCount(), getSamplerate());
    curPresentTime = 0;
    startDecodeThread();
}

void MediaSynchronizer::finish() {
    mMediaDecoder->finish();
    mAudioQue->finish();
    mTextureQue->finish();
    mAudioOutput->finish();
    mVideoOutput->onDestroy();
    // 停止线程
    isRunning = false;
}

void MediaSynchronizer::startDecodeThread() {
    int ret = pthread_create(&mDecoderThread, NULL, runDecoderThread, this);
    if (ret != 0) {
        LOGE("创建 DecodeThread 失败");
    }
}

void *MediaSynchronizer::runDecoderThread(void *self) {
    MediaSynchronizer* synchronizer = (MediaSynchronizer*)self;
    synchronizer->runDecoding();
    return 0;
}

void MediaSynchronizer::runDecoding() {
    int count = 0;
    AVPacket* packet;
    isRunning = true;
    while (isRunning) {
        if (mTextureQue->size() > 15) {
            usleep(1000 * 10);
            mVideoOutput->signalRenderFrame();
            mAudioOutput->signalRenderFrame();
            continue;
        }

        if ((packet = mMediaDecoder->readFrame()) == NULL) {
            if (mAudioQue->isEmpty() && mTextureQue->isEmpty()) {
                isRunning = false;
            } else {
                mVideoOutput->signalRenderFrame();
                mAudioOutput->signalRenderFrame();
            }
            continue;
        }
        if (mMediaDecoder->isVideoPacket(packet)) {
            LOGD("解码视频帧 NO.%d", ++count);
            std::vector<VideoFrame*> frames = mMediaDecoder->decodeVideoFrame(packet);
            mTextureQue->push(frames);
            mVideoOutput->signalRenderFrame();
        } else if (mMediaDecoder->isAudioPacket(packet)){
            LOGD("解码音频帧 NO.%d", ++count);
            std::vector<AudioFrame*> frames = mMediaDecoder->decodeAudioFrame(packet);
            mAudioQue->push(frames);
            mAudioOutput->signalRenderFrame();
        }
    }
}

long MediaSynchronizer::getProgress() {
    return curPresentTime;
}

long MediaSynchronizer::getDuration() {
    return mMediaDecoder->getMediaDuration();
}

int MediaSynchronizer::getSamplerate() {
    return mMediaDecoder ? mMediaDecoder->getSamplerate() : 0;
}

int MediaSynchronizer::getChannelCount() {
    return mMediaDecoder ? mMediaDecoder->getChannelCount() : 0;
}

void MediaSynchronizer::onSurfaceCreated(ANativeWindow *window) {
    mVideoOutput->onCreated(window);
    mTextureQue->start(mMediaDecoder->getWidth(),  mMediaDecoder->getHeight());
    mAudioQue->start();
}

void MediaSynchronizer::onSurfaceSizeChanged(int width, int height) {
    mVideoOutput->onChangeSize(width, height);
}

void MediaSynchronizer::onSurfaceDestroy() {
    mVideoOutput->onDestroy();
    mMediaDecoder->finish();
    mAudioQue->finish();
    mTextureQue->finish();
    mAudioOutput->finish();
    isRunning = false;
}

TextureFrame *MediaSynchronizer::getTetureFrame() {
    // 在Video渲染线程获取，不会阻塞主线程
    LOGE("MediaSynchronizer getTetureFrame TexuteQueue Size: %d", mTextureQue->size());
    usleep(1000 * 35);
    return !mTextureQue->isEmpty() ? mTextureQue->pop() : NULL;
}

AudioFrame *MediaSynchronizer::getAudioFrame() {
    // 在Audio渲染线程获取，不会阻塞主线程
    LOGE("MediaSynchronizer getAudioFrame AudioQueue Size: %d", mAudioQue->size());
    usleep(1000 * 10);
    return !mAudioQue->isEmpty() ? mAudioQue->pop() : NULL;
}

void MediaSynchronizer::setWaterMark(int imgWidth, int imgHeight, void *buffer) {
    mTextureQue->setWaterMark(imgWidth, imgHeight, buffer);
}

