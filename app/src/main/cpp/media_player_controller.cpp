//
// Created by chengjunsen on 2018/10/12.
//

#include <unistd.h>
#include "media_player_controller.h"

static MediaPlayerController*instance = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void pthread_sleep( unsigned int ms )
{
    struct timespec deadline;
    struct timeval now;

    gettimeofday(&now,NULL);
    time_t seconds = ms/1000;
    long nanoseconds = (ms - seconds * 1000) * 1000000;

    deadline.tv_sec = now.tv_sec + seconds;
    deadline.tv_nsec = now.tv_usec * 1000 + nanoseconds;

    if( deadline.tv_nsec >= 1000000000L )
    {
        deadline.tv_nsec -= 1000000000L;
        deadline.tv_sec++;
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_timedwait(&cond,&mutex,&deadline);
    pthread_mutex_unlock(&mutex);
}

MediaPlayerController::MediaPlayerController() {
//    mSynchronizer = new MediaSynchronizer;
    mVideoOutput = new VideoOutput;
    mMediaDecoder = new MediaDecoder;
//    mVieoQue = new VideoQueue;
//    mAudioOutput = new AudioOutput;
    instance = this;
}

MediaPlayerController::~MediaPlayerController() {

}

void MediaPlayerController::start(const char *path) {
    LOGE("start %s", path);
    int count = 0;
    mMediaDecoder->prepare(path);
//    mVieoQue->start(mMediaDecoder->getWidth(), mMediaDecoder->getHeight());
//    while(count <= 2 ) {
//        AVPacket* packet = mMediaDecoder->readFrame();
//        if (mMediaDecoder->isVideoPacket(packet)) {
//            std::vector<VideoFrame*> frames = mMediaDecoder->decodeVideoFrame(packet);
//            if (!frames.empty()) {
//                count++;
//                mVieoQue->push(frames);
//                usleep(1000 * 1000);
//                if (!mVieoQue->isEmpty()) {
//                    TextureFrame* textureFrame = mVieoQue->pop();
//                    mVideoOutput->output(textureFrame);
//                } else {
//                    LOGE("mVieoQue isEmpty");
//                }
//            }
//        }
//    }

    while(true) {
        AVPacket* packet = mMediaDecoder->readFrame();
        if (!mMediaDecoder->isVideoPacket(packet)) {
            continue;
        }
        std::vector<VideoFrame*> frames = mMediaDecoder->decodeVideoFrame(packet);
        if (frames.empty()) {
            continue;
        }
        mVideoOutput->output(frames[0]);
        pthread_sleep(1000);
    }
}

void MediaPlayerController::stop() {
}

void MediaPlayerController::pause() {
}

void MediaPlayerController::seek(double position) {
}

void MediaPlayerController::suspend() {
}

void MediaPlayerController::resume() {
}

long MediaPlayerController::getDuration() {
//    return mSynchronizer->getDuration();
    return 0;
}

long MediaPlayerController::getProgress() {
//    return mSynchronizer->getProgress();
    return 0;
}

void MediaPlayerController::onSurfaceCreated(ANativeWindow *window) {
    mVideoOutput->onCreated(window);
}

void MediaPlayerController::onSurfaceSizeChanged(int screenWidth, int screenHeight) {
    mVideoOutput->onChangeSize(screenWidth, screenHeight);
}

void MediaPlayerController::onSurfaceDestroy() {
    mVideoOutput->onDestroy();
//    mMediaDecoder->finish();
//    mVieoQue->finish();
}

void MediaPlayerController::release() {
//    if (mSynchronizer != NULL) {
//        delete mSynchronizer;
//        mSynchronizer = NULL;
//    }
    if (mVideoOutput != NULL) {
        delete mVideoOutput;
        mVideoOutput = NULL;
    }
//    if (mAudioOutput != NULL) {
//        delete mAudioOutput;
//        mAudioOutput = NULL;
//    }
    if (instance != NULL) {
        instance = NULL;
    }
}

AudioFrame *MediaPlayerController::getAudioFrame() {
//    return instance->mSynchronizer->getAudioFrame();
    return NULL;
}

TextureFrame *MediaPlayerController::getTextureFrame() {
//    return instance->mSynchronizer->getTextureFrame();
    return NULL;
}