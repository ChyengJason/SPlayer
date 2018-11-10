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
    mVideoQue = new VideoQueue;
    mVideoOutput = new VideoOutput(this);
    mAudioOutput = new AudioOutput(this);
    mDuration = 0;
    mAudioClock = 0;
    mVideoClock = 0;
    mAudioDuration = 0;
    mVideoDuration = 0;
    seekSeconds = 0;
    isSeeking = false;
    isSurfaceCreated = false;
}

MediaSynchronizer::~MediaSynchronizer() {
    pthread_mutex_destroy(&mDecoderMutex);
    pthread_cond_destroy(&mDecoderCond);
    pthread_cond_destroy(&mTextureCond);
    pthread_cond_destroy(&mAudioCond);
    delete(mMediaDecoder);
    delete(mAudioQue);
    delete(mVideoQue);
    delete(mVideoOutput);
    delete(mAudioOutput);
}

void MediaSynchronizer::prepare(const char *path) {
    mMediaDecoder->prepare(path);
    mDuration = mMediaDecoder->getMediaDuration();
    isStarted = false;
}

void MediaSynchronizer::start() {
    mAudioQue->start(mMediaDecoder);
    mVideoQue->start(mMediaDecoder);
    mAudioOutput->start(mMediaDecoder->getChannelCount(), mMediaDecoder->getSamplerate());
    mAudioClock = 0;
    mVideoClock = 0;
    mAudioDuration = 0;
    mVideoDuration = 0;
    seekSeconds = 0;
    isStarted = true;
    isPaused = false;
    isSeeking = false;
    startDecodeThread();
    pthread_cond_signal(&mDecoderCond);
    pthread_cond_signal(&mTextureCond);
    pthread_cond_signal(&mAudioCond);
}

void MediaSynchronizer::finish() {
    isStarted = false;
    isPaused = false;
    isSeeking = false;
    seekSeconds = 0;
    mMediaDecoder->finish();
    mAudioQue->finish();
    mVideoQue->finish();
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
    bool isDecodeFinish = false;
    while (isStarted) {
        LOGD("rundecoding audioCacheSize %d, videoCacheSize %d", mAudioQue->packetCacheSize(), mVideoQue->packetCacheSize());
        LOGD("rundecoding audioSize %d, videoSize %d", mAudioQue->size(), mVideoQue->size());

        if (!isSurfaceCreated) { // surface未创建
            LOGE("rundecoding isSurfaceCreated 进入等待");
            pthread_mutex_lock(&mDecoderMutex);
            pthread_cond_wait(&mDecoderCond, &mDecoderMutex);
            pthread_mutex_unlock(&mDecoderMutex);
            LOGE("rundecoding isSurfaceCreated 唤醒");
        } else if (isSeeking) { // 指定进度
            pthread_mutex_lock(&mDecoderMutex);
            pthread_mutex_lock(&mAudioMutex);
            pthread_mutex_lock(&mTextureMutex);
            mAudioClock = 0;
            mVideoClock = 0;
            mMediaDecoder->seek(seekSeconds);
            seekSeconds = 0;
            isDecodeFinish = false;
            mAudioQue->clear();
            mVideoQue->clear();
            pthread_mutex_unlock(&mTextureMutex);
            pthread_mutex_unlock(&mAudioMutex);
            pthread_mutex_unlock(&mDecoderMutex);
            isSeeking = false;
            LOGE("MediaSynchronizer seek 完成");
        } else if ((mAudioQue->packetCacheSize() >= MAX_CACHE_PACKET_SIZE || mAudioQue->getAllDuration() > MAX_BUFFER_DURATION) &&
                (mVideoQue->packetCacheSize() >= MAX_CACHE_PACKET_SIZE || mVideoQue->getAllDuration() > MAX_BUFFER_DURATION)) {
            if (mAudioQue->getAllDuration() > 0 && mVideoQue->getAllDuration() > 0) {
                pthread_cond_signal(&mAudioCond);
                pthread_cond_signal(&mTextureCond);
            }
            usleep(20 * 1000);
        } else if (!isDecodeFinish){
            isDecodeFinish = !decodeFrame();
            if (mAudioQue->getAllDuration() > 0 && mVideoQue->getAllDuration() > 0) {
                pthread_cond_signal(&mAudioCond);
                pthread_cond_signal(&mTextureCond);
            }
        }
    }
}

bool MediaSynchronizer::decodeFrame() {
    AVPacket* packet;
    bool result;
    if ((packet = mMediaDecoder->readFrame()) == NULL) {
        result = false;
    } else if (mMediaDecoder->isVideoPacket(packet)) {
        LOGE("rundecoding decodeFrame mTextureQue");
        mVideoQue->push(packet);
        result = true;
    } else if (mMediaDecoder->isAudioPacket(packet)){
        LOGE("rundecoding decodeFrame mAudioQue");
        mAudioQue->push(packet);
        result = true;
    }
    return result;
}

void MediaSynchronizer::onSurfaceCreated(ANativeWindow *mwindow) {
    mVideoOutput->onCreated(mwindow);
    isSurfaceCreated = true;
    pthread_cond_signal(&mDecoderCond);
    pthread_cond_signal(&mTextureCond);
    pthread_cond_signal(&mAudioCond);
}

void MediaSynchronizer::onSurfaceSizeChanged(int width, int height) {
    mVideoOutput->onChangeSize(width, height);
    pthread_cond_signal(&mDecoderCond);
    pthread_cond_signal(&mTextureCond);
    pthread_cond_signal(&mAudioCond);
}

void MediaSynchronizer::onSurfaceDestroy() {
    mVideoOutput->onDestroy();
    mMediaDecoder->finish();
    mAudioQue->finish();
    mVideoQue->finish();
    mAudioOutput->finish();
    isSurfaceCreated = false;
    isStarted = false;
    pthread_cond_signal(&mDecoderCond);
    pthread_cond_signal(&mTextureCond);
    pthread_cond_signal(&mAudioCond);
}

VideoFrame *MediaSynchronizer::getVideoFrame() {
    // 在Video渲染线程获取，不会阻塞主线程
    VideoFrame * videoFrame = NULL;
    while (videoFrame == NULL) {
        pthread_mutex_lock(&mTextureMutex);
        // mAudioClock <= 0 保证音频先播放，视频才有基准时间
        if (isSeeking || isPaused || mAudioClock <= 0 || (videoFrame = mVideoQue->pop())== NULL) {
            LOGE("getVideoFrame 进入等待");
            pthread_cond_signal(&mDecoderCond);
            pthread_cond_wait(&mTextureCond, &mTextureMutex);
        }
        pthread_mutex_unlock(&mTextureMutex);
        if (!mVideoOutput->isRunning()) {
            break;
        }
    }
    correctTime(videoFrame);
    pthread_cond_signal(&mDecoderCond);
    return videoFrame;
}

AudioFrame *MediaSynchronizer::getAudioFrame() {
    // 在Audio渲染线程获取，不会阻塞主线程
    AudioFrame* audioFrame = NULL;
    while (audioFrame == NULL) {
        pthread_mutex_lock(&mAudioMutex);
        if (isSeeking || isPaused || (audioFrame = mAudioQue->pop()) == NULL) {
            LOGE("getAudioFrame 进入等待");
            pthread_cond_signal(&mDecoderCond);
            pthread_cond_wait(&mAudioCond, &mAudioMutex);
        }
        pthread_mutex_unlock(&mAudioMutex);
        if (!mAudioOutput->isRunning()) {
            break;
        }
    }
    correctTime(audioFrame);
    pthread_cond_signal(&mDecoderCond);
	pthread_cond_signal(&mTextureCond);
    return audioFrame;
}

void MediaSynchronizer::seek(float seconds) {
    if (isStarted && isSurfaceCreated) {
        isSeeking = true;
        seekSeconds = seconds;
        pthread_cond_signal(&mDecoderCond);
        pthread_cond_signal(&mTextureCond);
        pthread_cond_signal(&mAudioCond);
    }
}

float MediaSynchronizer::getDuration() {
    return mDuration;
}

float MediaSynchronizer::getProgress() {
    return mAudioClock;
}

void MediaSynchronizer::correctTime(VideoFrame *videoFrame) {
    if (videoFrame == NULL) {
        return;
    }
    double pts = videoFrame->timestamp;
    if (pts <= mVideoClock) {
        pts = mVideoClock + mVideoDuration;
    }
    double delay;
    double diff = pts - mAudioClock;
    if (diff < -MAX_JUDGE_DIFF || diff > MAX_JUDGE_DIFF) {
        delay = 0;
        videoFrame->isSkip = true;
        LOGE("跳帧 videoclock:%lf, audioclock:%lf", mVideoClock, mAudioClock);
    } else if (diff >= MAX_FRAME_DIFF) {
        delay = diff;
    } else {
        delay = 0;
    }

    if (delay > 0) {
        LOGE("video sleep %ld", delay);
        usleep(delay * 1000000);
    }
    mVideoDuration = videoFrame->duration;
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

void MediaSynchronizer::pause() {
    if (isStarted && isSurfaceCreated && !isPaused) {
        isPaused = true;
        pthread_cond_signal(&mDecoderCond);
        pthread_cond_signal(&mTextureCond);
        pthread_cond_signal(&mAudioCond);
    }
}

void MediaSynchronizer::resume() {
    if (isStarted && isSurfaceCreated && isPaused) {
        isPaused = false;
        pthread_cond_signal(&mDecoderCond);
        pthread_cond_signal(&mTextureCond);
        pthread_cond_signal(&mAudioCond);
    }
}
