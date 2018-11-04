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
    mAudioClock = 0;
    mVideoClock = 0;
    mAudioInterval = 0;
    mVideoInterval = 0;
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
    mAudioOutput->start(mMediaDecoder->getChannelCount(), mMediaDecoder->getSamplerate());
    mAudioClock = 0;
    mVideoClock = 0;
    mAudioInterval = 0;
    mVideoInterval = 0;
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
    isRunning = true;
    while (isRunning) {
        double audioDuration = mAudioQue->getAllDuration();
        double videoDuration = mTextureQue->getAllDuration();
        LOGE("audioDuration: %lf, videoDuration: %lf", audioDuration, videoDuration);
        if (audioDuration < MIN_BUFFER_DURATION || videoDuration < MIN_BUFFER_DURATION) {
            bool success = decodeFrame();
            if (!success && mAudioQue->isEmpty() && mTextureQue->isEmpty()) {
                isRunning = false;
                break;
            } else {
                mAudioOutput->signalRenderFrame();
                mVideoOutput->signalRenderFrame();
                pthread_mutex_lock(&mDecoderMutex);
                pthread_cond_wait(&mDecoderCond, &mDecoderMutex);
                pthread_mutex_unlock(&mDecoderMutex);
            }
        } else if (audioDuration > MAX_BUFFER_DURATION || videoDuration > MIN_BUFFER_DURATION) {
            mAudioOutput->signalRenderFrame();
            mVideoOutput->signalRenderFrame();
            pthread_mutex_lock(&mDecoderMutex);
            pthread_cond_wait(&mDecoderCond, &mDecoderMutex);
            pthread_mutex_unlock(&mDecoderMutex);
        }
    }
    finish();
}

bool MediaSynchronizer::decodeFrame() {
    AVPacket* packet;
    if ((packet = mMediaDecoder->readFrame()) == NULL) {
        return false;
    }
    if (mMediaDecoder->isVideoPacket(packet)) {
        LOGD("解码视频帧");
        std::vector<VideoFrame*> frames = mMediaDecoder->decodeVideoFrame(packet);
        mTextureQue->push(frames);
        mVideoOutput->signalRenderFrame();
    } else if (mMediaDecoder->isAudioPacket(packet)){
        LOGD("解码音频帧");
        std::vector<AudioFrame*> frames = mMediaDecoder->decodeAudioFrame(packet);
        mAudioQue->push(frames);
        mAudioOutput->signalRenderFrame();
    }
    return true;
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
//    LOGE("MediaSynchronizer getTetureFrame TexuteQueue Size: %d", mTextureQue->size());
    TextureFrame* textureFrame = !mTextureQue->isEmpty() ? mTextureQue->pop() : NULL;
    if (textureFrame != NULL) {
        double currentPts = textureFrame->timestamp;
        if (currentPts - mVideoClock <= 0 ) {
            currentPts = mVideoClock + mVideoInterval;
        }

        // 计算跟当前参考的时钟比较
        double diff = currentPts - mAudioClock;
        double delay = 0;
        if (diff >= MAX_FRAME_JUDGE || diff <= -MAX_FRAME_JUDGE) {
            delay = 0;
            textureFrame->isSkip = true;
        } else if (diff < -MAX_FRAME_DIFF) {
            delay = 0;
        } else if (diff > MAX_FRAME_DIFF) {
            delay = diff;
        }
        if (delay >= MAX_FRAME_DIFF) {
            usleep(delay * 1000000);
            LOGE("视频帧delay : %lf", delay);
        } else

        LOGE("videopts: %lf, audioclock: %lf, videoclock %lf, interval: %lf, delay: %lf",
             currentPts, mAudioClock, mVideoClock, mVideoInterval, delay);

        // 设置当前时间
        mVideoInterval = currentPts - mVideoClock;
        if (mVideoInterval <= 0) {
            mVideoInterval = textureFrame->duration;
        }
        mVideoClock = currentPts;
        pthread_cond_signal(&mDecoderCond);
    }
    return textureFrame;
}

AudioFrame *MediaSynchronizer::getAudioFrame() {
    // 在Audio渲染线程获取，不会阻塞主线程
    AudioFrame* audioFrame = !mAudioQue->isEmpty() ? mAudioQue->pop() : NULL;
    if (audioFrame != NULL) {
        // 修正pts
        double currentPts = audioFrame->timestamp;
        if (currentPts - mAudioClock <= 0) {
            currentPts = mAudioClock + mAudioInterval;
        }
        LOGE("audiopts: %lf, audio clock: %lf, interval: %lf", currentPts, mAudioClock, mAudioInterval);
        // 设置时间
        mAudioInterval = currentPts - mAudioClock;
        if (mAudioInterval <= 0) {
            mAudioInterval = audioFrame->duration;
        }
        mAudioClock = currentPts;
    }
    pthread_cond_signal(&mDecoderCond);
    return audioFrame;
}
