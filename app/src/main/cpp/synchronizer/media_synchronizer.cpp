//
// Created by chengjunsen on 2018/10/12.
//
#include <unistd.h>
#include "media_synchronizer.h"
MediaSynchronizer::MediaSynchronizer() {
    pthread_cond_init(&mDecoderCond, NULL);
    pthread_cond_init(&mTextureCond, NULL);
    pthread_cond_init(&mAudioCond, NULL);
    pthread_mutex_init(&mDecoderMutex, NULL);
    pthread_mutex_init(&mTextureMutex, NULL);
    pthread_mutex_init(&mAudioMutex, NULL);
    mMediaDecoder = new MediaDecoder;
    mAudioQue = new AudioQueue;
    mTextureQue = new VideoQueue;
    mVideoOutput = new VideoOutput(this);
    mAudioOutput = new AudioOutput(this);
    mDuration = 0;
    mAudioClock = 0;
    mVideoClock = 0;
    mAudioDuration = 0;
    mVideoDuration = 0;
    isSurfaceCreated = false;
}

MediaSynchronizer::~MediaSynchronizer() {
    pthread_mutex_destroy(&mDecoderMutex);
    pthread_cond_destroy(&mDecoderCond);
    pthread_cond_destroy(&mTextureCond);
    pthread_cond_destroy(&mAudioCond);
    delete(mMediaDecoder);
    delete(mAudioQue);
    delete(mTextureQue);
    delete(mVideoOutput);
    delete(mAudioOutput);
}

void MediaSynchronizer::prepare(const char *path) {
    mMediaDecoder->prepare(path);
    mDuration = mMediaDecoder->getMediaDuration();
}

void MediaSynchronizer::start() {
    mAudioOutput->start(mMediaDecoder->getChannelCount(), mMediaDecoder->getSamplerate());
    mAudioClock = 0;
    mVideoClock = 0;
    mAudioDuration = 0;
    mVideoDuration = 0;
    startDecodeThread();
    pthread_cond_signal(&mDecoderCond);
    pthread_cond_signal(&mTextureCond);
    pthread_cond_signal(&mAudioCond);
}

void MediaSynchronizer::finish() {
    mMediaDecoder->finish();
    mAudioQue->finish();
    mTextureQue->finish();
    mAudioOutput->finish();
    mVideoOutput->onDestroy();
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
    while (true) {
        double audioDuration = mAudioQue->getAllDuration();
        double videoDuration = mTextureQue->getAllDuration();
        LOGD("runDecoding audioDuration %lf, videoDuration %lf", audioDuration, videoDuration);
        if (!isSurfaceCreated || (audioDuration > MAX_BUFFER_DURATION && videoDuration > MAX_BUFFER_DURATION)) {
            pthread_mutex_lock(&mDecoderMutex);
            pthread_cond_wait(&mDecoderCond, &mDecoderMutex);
            pthread_mutex_unlock(&mDecoderMutex);
        } else if (!decodeFrame()){
            break;
        }
        pthread_cond_signal(&mTextureCond);
        pthread_cond_signal(&mAudioCond);
    }
}

bool MediaSynchronizer::decodeFrame() {
    pthread_mutex_lock(&mDecoderMutex);
    AVPacket* packet;
    bool result;
    if ((packet = mMediaDecoder->readFrame()) == NULL) {
        result = false;
    } else if (mMediaDecoder->isVideoPacket(packet)) {
        LOGE("decodeFrame mTextureQue");
        mTextureQue->push(packet);
        result = true;
    } else if (mMediaDecoder->isAudioPacket(packet)){
        LOGE("decodeFrame mAudioQue");
        mAudioQue->push(packet);
        result = true;
    }
    pthread_mutex_unlock(&mDecoderMutex);
    return result;
}

void MediaSynchronizer::onSurfaceCreated(ANativeWindow *mwindow) {
    mVideoOutput->onCreated(mwindow);
    mAudioQue->start(mMediaDecoder);
    mTextureQue->start(mMediaDecoder);
    isSurfaceCreated = true;
    pthread_cond_signal(&mDecoderCond);
    pthread_cond_signal(&mTextureCond);
    pthread_cond_signal(&mAudioCond);
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
    isSurfaceCreated = false;
    pthread_cond_signal(&mDecoderCond);
    pthread_cond_signal(&mTextureCond);
    pthread_cond_signal(&mAudioCond);
}

TextureFrame *MediaSynchronizer::getTetureFrame() {
    // 在Video渲染线程获取，不会阻塞主线程
    TextureFrame * textureFrame = NULL;
    while (textureFrame == NULL) {
        pthread_mutex_lock(&mTextureMutex);
        if (mAudioClock <= 0) { // 音频先播放
            LOGE("音频先播放 getTetureFrame 进入等待 %lf", mAudioClock);
            pthread_cond_wait(&mTextureCond, &mTextureMutex);
            LOGE("getTetureFrame唤醒1");
        } else if ((textureFrame = mTextureQue->pop())== NULL) {
            LOGE("getTetureFrame 进入等待");
            pthread_cond_wait(&mTextureCond, &mTextureMutex);
            LOGE("getTetureFrame唤醒2");
        }
        pthread_mutex_unlock(&mTextureMutex);
        if (!mVideoOutput->isRunning()) {
            break;
        }
    }
    correctTime(textureFrame);
    pthread_cond_signal(&mDecoderCond);
    mVideoOutput->signalRenderFrame();
    return textureFrame;
}

AudioFrame *MediaSynchronizer::getAudioFrame() {
    // 在Audio渲染线程获取，不会阻塞主线程
    AudioFrame* audioFrame = NULL;
    while (audioFrame == NULL) {
        pthread_mutex_lock(&mAudioMutex);
        audioFrame = mAudioQue->pop();
        if (audioFrame == NULL) {
            LOGE("getAudioFrame 进入等待");
            pthread_cond_wait(&mAudioCond, &mAudioMutex);
        }
        pthread_mutex_unlock(&mAudioMutex);
        if (!mAudioOutput->isRunning()) {
            break;
        }
    correctTime(audioFrame);
    pthread_cond_signal(&mDecoderCond);
	pthread_cond_signal(&mTextureCond);
    return audioFrame;
}

void MediaSynchronizer::seek(float seconds) {

    pthread_mutex_lock(&mAudioMutex);
    mAudioQue->clear();
    pthread_mutex_unlock(&mAudioMutex);

    pthread_mutex_lock(&mTextureMutex);
    mTextureQue->clear();
    pthread_mutex_unlock(&mTextureMutex);

    pthread_mutex_lock(&mDecoderMutex);
    mMediaDecoder->seek(seconds);
    pthread_mutex_unlock(&mDecoderMutex);

    pthread_cond_signal(&mDecoderCond);
}

float MediaSynchronizer::getDuration() {
    return mDuration;
}

float MediaSynchronizer::getProgress() {
    return mAudioClock;
}

void MediaSynchronizer::correctTime(TextureFrame *textureFrame) {
    if (textureFrame == NULL) {
        return;
    }
    double pts = textureFrame->timestamp;
    if (pts == mVideoClock) {
        pts = mVideoClock + mVideoDuration;
    }

    double delay;
    double diff = pts - mAudioClock;
    if (diff < -MAX_JUDGE_DIFF || diff > MAX_JUDGE_DIFF) {
        delay = 0;
        textureFrame->isSkip = true;
    } else if (diff >= MAX_FRAME_DIFF) {
        delay = diff;
    } else {
        delay = 0;
    }

    if (delay > 0) {
        LOGE("video sleep %ld", delay);
        usleep(delay * 1000000);
    }
    mVideoDuration = textureFrame->duration;
    mVideoClock = pts;
}

void MediaSynchronizer::correctTime(AudioFrame *audioFrame) {
    if (audioFrame == NULL) {
        return;
    }
    double pts = audioFrame->timestamp;
    if (pts == mAudioClock) {
        pts = mAudioClock + mAudioDuration;
    }
    mAudioDuration = audioFrame->duration;
    mAudioClock = pts;
}
