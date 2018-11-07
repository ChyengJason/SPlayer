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
    mDuration = 0;
    mAudioClock = 0;
    mVideoClock = 0;
    mAudioInterval = 0;
    mVideoInterval = 0;
    isSurfaceCreated = false;
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
    mDuration = mMediaDecoder->getMediaDuration();
    mStatus = STOP;
}

void MediaSynchronizer::start() {
    mAudioOutput->start(mMediaDecoder->getChannelCount(), mMediaDecoder->getSamplerate());
//    mTextureQue->start();
    mAudioClock = 0;
    mVideoClock = 0;
    mAudioInterval = 0;
    mVideoInterval = 0;
    mSeekSeconds = 0;
    mStatus = PLAY;
    startDecodeThread();
}

void MediaSynchronizer::finish() {
    mMediaDecoder->finish();
    mAudioQue->finish();
    mTextureQue->finish();
    mAudioOutput->finish();
    mVideoOutput->onDestroy();
    // 停止线程
    mStatus = STOP;
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
    LOGD("MediaSynchronizer::runDecoding");
    while (mStatus != STOP) {
        if (!isSurfaceCreated) {
            pthread_mutex_lock(&mDecoderMutex);
            pthread_cond_wait(&mDecoderCond, &mDecoderMutex);
            pthread_mutex_unlock(&mDecoderMutex);
        } else if (mStatus == PLAY) {
            double audioDuration = mAudioQue->getAllDuration();
            double videoDuration = mTextureQue->getAllDuration();
            LOGE("audioDuration: %lf, videoDuration: %lf", audioDuration, videoDuration);
            if (audioDuration < MIN_BUFFER_DURATION || videoDuration < MIN_BUFFER_DURATION) {
                bool success = decodeFrame();
                if (!success && mAudioQue->isEmpty() && mTextureQue->isEmpty()) {
                    mStatus = STOP;
                    break;
                }
                mAudioOutput->signalRenderFrame();
                mVideoOutput->signalRenderFrame();
            } else if (audioDuration > MAX_BUFFER_DURATION || videoDuration > MIN_BUFFER_DURATION) {
                mAudioOutput->signalRenderFrame();
                mVideoOutput->signalRenderFrame();
                pthread_mutex_lock(&mDecoderMutex);
                pthread_cond_wait(&mDecoderCond, &mDecoderMutex);
                pthread_mutex_unlock(&mDecoderMutex);
            }
        } else if (mStatus == SEEK) {
            mMediaDecoder->seek(mSeekSeconds);
            mTextureQue->clear();
            mAudioQue->clear();
            mStatus = PLAY;
            usleep(20 * 1000);
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
        std::vector<VideoFrame*> frames = mMediaDecoder->decodeVideoFrame(packet);
        LOGD("解码视频帧 %d", frames.size());
        mTextureQue->push(frames);
        mVideoOutput->signalRenderFrame();
    } else if (mMediaDecoder->isAudioPacket(packet)){
        std::vector<AudioFrame*> frames = mMediaDecoder->decodeAudioFrame(packet);
        LOGD("解码音频帧 %d", frames.size());
        mAudioQue->push(frames);
        mAudioOutput->signalRenderFrame();
    }
    return true;
}

void MediaSynchronizer::onSurfaceCreated(ANativeWindow *mwindow) {
    mVideoOutput->onCreated(mwindow);
    //usleep(1000 * 1000);
    mAudioQue->start();
    mTextureQue->start();
    isSurfaceCreated = true;
    pthread_cond_signal(&mDecoderCond);
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
}

TextureFrame *MediaSynchronizer::getTetureFrame() {
    // 在Video渲染线程获取，不会阻塞主线程
    TextureFrame* textureFrame = mTextureQue->pop() ;
    if (textureFrame != NULL) {
        double currentPts = textureFrame->timestamp;
        if (currentPts <= 0) {
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
    AudioFrame* audioFrame = mAudioQue->pop() ;
    if (audioFrame != NULL) {
        // 修正pts
        double currentPts = audioFrame->timestamp;
        if (currentPts <= 0) {
            currentPts = mAudioClock + mAudioInterval;
        }
        double diff = currentPts - mAudioClock;
        if (diff >= MAX_FRAME_JUDGE || diff <= -MAX_FRAME_JUDGE) {
            audioFrame->isSkip = true;
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

void MediaSynchronizer::seek(float seconds) {
    mStatus = SEEK;
    mSeekSeconds = seconds;
    pthread_cond_signal(&mDecoderCond);
}

float MediaSynchronizer::getDuration() {
    return mDuration;
}

float MediaSynchronizer::getProgress() {
    return mVideoClock;
}
