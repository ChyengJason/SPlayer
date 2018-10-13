//
// Created by chengjunsen on 2018/10/12.
//
#include "media_decoder.h"

MediaDecoder::MediaDecoder() {

}

MediaDecoder::~MediaDecoder() {

}

void MediaDecoder::start(const char* path) {
    mformatContext = NULL;
    mVideoCodecContext = NULL;
    mAudioCodecContext = NULL;
    mVideoCodec = NULL;
    mAudioCodec = NULL;
    mVideoFrame = NULL;
    mRgbFrame = NULL;
    mAudioFrame = NULL;
    mSwsContext = NULL;
    mVideoOutBuffer = NULL;
    mSwrContext = NULL;
    mAudioOutBuffer = NULL;
    mPath =new char[strlen(path)];
    strcpy(mPath, path);
    LOGE("视频地址是： %s", mPath);
    void* status;
    LOGD("处理线程 %lu",(unsigned long)pthread_self());
    createDecoderThread();
    //pthread_join(mDecoderThread, &status);
    LOGD("解码线程 %lu status： %ld",(unsigned long)mDecoderThread,(long)status);
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
    LOGD("解码线程 %lu ",(unsigned long)pthread_self());
    MediaDecoder* videoDecoder = (MediaDecoder*)self;
    if (!videoDecoder->getMediaInfo()) {
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
    videoDecoder->initVideoFrameAndSwsContext();
    videoDecoder->initAudioFrameAndSwrContext();
    videoDecoder->readFrames();
    videoDecoder->release();
    return 0;
}

bool MediaDecoder::getMediaInfo() {
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

bool MediaDecoder::initVideoFrameAndSwsContext() {
    mVideoFrame = av_frame_alloc();
    mRgbFrame = av_frame_alloc();
    // 关联缓存区
    mVideoOutBuffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGBA, mVideoCodecContext->width, mVideoCodecContext->height));
    avpicture_fill((AVPicture*)mRgbFrame, mVideoOutBuffer, AV_PIX_FMT_RGBA, mVideoCodecContext->width, mVideoCodecContext->height);
    // 视频图像的转换, 比如格式转换
    mSwsContext = sws_getContext(mVideoCodecContext->width, mVideoCodecContext->height, mVideoCodecContext->pix_fmt,
                                 mVideoCodecContext->width, mVideoCodecContext->height, AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
    LOGE("视频宽度: %d, 高度: %d", mVideoCodecContext->width, mVideoCodecContext->height);
    return true;
}

bool MediaDecoder::initAudioFrameAndSwrContext() {
    mAudioFrame = av_frame_alloc();
    mSwrContext = swr_alloc();

    int64_t outChannelLayout = av_get_default_channel_layout(mAudioCodecContext->channels);
    AVSampleFormat outSampleFormat = AV_SAMPLE_FMT_S16;
    int outSampleRate = mVideoCodecContext->sample_rate;  // 输出的采样率必须与输入相同
    int outNumChannels = mAudioCodecContext->channels;

    mAudioOutBuffer = (uint8_t*) av_malloc(av_samples_get_buffer_size(NULL, outNumChannels, outSampleRate, outSampleFormat, 1));
    swr_alloc_set_opts(mSwrContext, outChannelLayout, outSampleFormat, outSampleRate,
                       mVideoCodecContext->channel_layout, mVideoCodecContext->sample_fmt, mVideoCodecContext->sample_rate,
                       0, NULL);
    swr_init(mSwrContext);
    return true;
}

void MediaDecoder::readFrames() {
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    while(av_read_frame(mformatContext, packet) >= 0) {
        if (packet->stream_index == mVideoStreamIndex) {
            decodeVideoFrame(packet);
        } else if (packet->stream_index == mAudioStreamIndex) {
            decodeAudioFrame(packet);
        }
        av_free_packet(packet);
    }
    av_free(packet);
}

void MediaDecoder::decodeVideoFrame(AVPacket *packet) {
    int frameCount;
    // 解码
    int error = avcodec_decode_video2(mVideoCodecContext, mVideoFrame, &frameCount, packet);
    // 转换成RGBA格式
    sws_scale(mSwsContext, (const uint8_t *const *)mVideoFrame->data, mVideoFrame->linesize, 0, mVideoFrame->height,
              mRgbFrame->data, mRgbFrame->linesize);
    double pts;
    if ((pts = av_frame_get_best_effort_timestamp(mVideoFrame)) == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(mformatContext->streams[mVideoStreamIndex]->time_base);
    // av_frame_get_best_effort_timestamp可能失败，播放需要做纠正
    // TODO 转化成VideoFrame存入队列
    LOGE("视频解码 pts: %f", pts);

}

void MediaDecoder::decodeAudioFrame(AVPacket *packet) {
    // 对于视频帧，一个AVPacket是一帧视频帧；对于音频帧，一个AVPacket有可能包含多个音频帧
    int outChannelCount = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    int packetSize = packet->size;
    while (packetSize > 0) {
        int gotframe = 0;
        int len = avcodec_decode_audio4(mAudioCodecContext, mAudioFrame, &gotframe, packet);
        if (len < 0) {
            break;
        }
        if (gotframe) {
            int error = swr_convert(mSwrContext, &mAudioOutBuffer, mAudioFrame->nb_samples * 2, (const uint8_t **) mAudioFrame->data, mAudioFrame->nb_samples);
            int numChannels = mAudioCodecContext->channels;
            // 一个 packet 中可以包含多个帧，packet 中的 PTS 比真正的播放的 PTS 可能会早很多,播放的时候需要重新纠正
            double pts = av_q2d(mformatContext->streams[mAudioStreamIndex]->time_base) * packet->pts;
            // TODO 转化成AudioFrame存入队列
            LOGE("音频解码 pts: %f", pts);
        }
        if (0 == len) {
            break;
        }
        packetSize -= len;
    }
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
    if (mRgbFrame) {
        av_frame_free(&mRgbFrame);
        mRgbFrame = NULL;
    }
    if (mVideoFrame) {
        av_frame_free(&mVideoFrame);
        mVideoFrame = NULL;
    }
    if (mAudioFrame) {
        av_frame_free(&mAudioFrame);
        mAudioFrame = NULL;
    }
    if (mSwsContext) {
        sws_freeContext(mSwsContext);
        mSwsContext = NULL;
    }
    if (mVideoOutBuffer) {
        av_free(mVideoOutBuffer);
        mVideoOutBuffer = NULL;
    }
    if (mSwrContext) {
        swr_free(&mSwrContext);
        mSwrContext = NULL;
    }
    if (mAudioOutBuffer) {
        av_free(mAudioOutBuffer);
        mAudioOutBuffer = NULL;
    }
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