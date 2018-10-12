//
// Created by chengjunsen on 2018/10/12.
//
#include "media_decoder.h"

MediaDecoder::MediaDecoder(const char* path)
    : mformatContext(NULL)
    , mVideoCodecContext(NULL)
    , mAudioCodecContext(NULL)
    , mVideoCodec(NULL)
    , mAudioCodec(NULL) {
    mPath =new char[strlen(path)];
    strcpy(mPath, path);
    LOGE("视频地址是： %s", mPath);
}

MediaDecoder::~MediaDecoder() {

}

void MediaDecoder::start() {
    void* status;
    createDecoderThread();
    pthread_join(mDecoderThread, &status);
    LOGE("thread %lu exit status is %ld",(unsigned long)mDecoderThread,(long)status);
}

void MediaDecoder::createDecoderThread() {
    /**
     * int pthread_create(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg);
     *  thread 线程 id
     *  attr 创建属性
     *  start_routine 即 run in thread 回调，函数指针
     *  arg 即 run in thread 回调中的参数
     */
    pthread_cond_init(&mDecoderCond, NULL);
    pthread_mutex_init(&mDecoderMutex, NULL);
    int ret = pthread_create(&mDecoderThread, NULL, run, this);
    if (ret != 0) {
        LOGE("创建pthread失败");
    }
}

void* MediaDecoder::run(void* self) {
    MediaDecoder* videoDecoder = (MediaDecoder*)self;
    if (!videoDecoder->initMediaInfo()) {
        videoDecoder->release();
        return 0;
    }
    if (!videoDecoder->initVideoCodec()) {
        videoDecoder->release();
        return 0;
    }
    if (!videoDecoder->initAudioCodec()) {
        videoDecoder->release();
        return 0;
    }
    return 0;
}

bool MediaDecoder::initMediaInfo() {
    int error;
    char buffer[] = "";
    // 注册网络协议
    avformat_network_init();
    av_register_all();
    // 获取上下文
    mformatContext = avformat_alloc_context();
    // 打开视频
    error = avformat_open_input(&mformatContext, mPath, NULL, NULL);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGE("打开视频失败 %s : %d(%s)", mPath, error, buffer);
        return false;
    }
    // 获取视频信息
    error = avformat_find_stream_info(mformatContext, NULL);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGE("获取视频信息失败 %s : %d(%s)", mPath, error, buffer);
        return false;
    }
    // 获取视频流id和音频流id
    mAudioStreamIndex = -1;
    mVideoStreamIndex = -1;

    for (int i = 0; i < mformatContext->nb_streams; ++i) {
        enum AVMediaType type = mformatContext->streams[i]->codec->codec_type;
        if (type == AVMEDIA_TYPE_VIDEO) {
            mVideoStreamIndex = i;
        } else if (type == AVMEDIA_TYPE_AUDIO) {
            mAudioStreamIndex = i;
        }
    }
    if (mAudioStreamIndex == -1 || mVideoStreamIndex == -1) {
        LOGE("获取视频流id: %d 或音频流id失败: %d", mVideoStreamIndex, mAudioStreamIndex);
        return false;
    }
    return true;
}

bool MediaDecoder::initVideoCodec() {
    int error;
    char buffer[] = "";
    mVideoCodecContext = mformatContext->streams[mVideoStreamIndex]->codec;
    // 寻找解码器
    mVideoCodec = avcodec_find_decoder(mVideoCodecContext->codec_id);
    // 打开解码器
    error = avcodec_open2(mVideoCodecContext, mVideoCodec, NULL);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGE("打开视频解码器失败: %d(%s)", error, buffer);
        return false;
    }
    return true;
}

bool MediaDecoder::initAudioCodec() {
    int error;
    char buffer[] = "";
    mAudioCodecContext = mformatContext->streams[mAudioStreamIndex]->codec;
    // 寻找解码器
    mAudioCodec = avcodec_find_decoder(mAudioCodecContext->codec_id);
    // 打开解码器
    error = avcodec_open2(mAudioCodecContext, mAudioCodec, NULL);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGE("打开音频解码器失败: %d(%s)", error, buffer);
        return false;
    }
    return true;
}

void MediaDecoder::sleep(int sec) {
    struct timeval now;
    struct timespec outtime;
    pthread_mutex_lock(&mDecoderMutex);
    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + sec;
    outtime.tv_nsec = now.tv_usec * 1000;
    int ret = pthread_cond_timedwait(&mDecoderCond, &mDecoderMutex, &outtime);
    pthread_mutex_unlock(&mDecoderMutex);
    LOGE("sleep return : %d ", ret);
}

void MediaDecoder::release() {
    if (mAudioCodecContext) {
        avcodec_close(mAudioCodecContext);
        mAudioCodecContext = NULL;
    }
    if (mVideoCodecContext) {
        avcodec_close(mVideoCodecContext);
        mVideoCodecContext = NULL;
    }
    if (mformatContext) {
        avformat_free_context(mformatContext);
        mformatContext = NULL;
    }
}
