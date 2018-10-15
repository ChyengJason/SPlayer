//
// Created by chengjunsen on 2018/10/12.
//
#include "media_decoder.h"

MediaDecoder::MediaDecoder() {

}

MediaDecoder::~MediaDecoder() {

}

bool MediaDecoder::prepare(const char* path) {
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
    mTempPacket = NULL;
    mSwrBufferSize = 0;
    return init(path);
}

bool MediaDecoder::init(const char* path) {
    if (!getMediaInfo(path)) {
        release();
        return false;
    }
    if (!initVideoCodec()) {
        release();
        return false;
    }
    if (!initAudioCodec()) {
        release();
        return false;
    }
    initVideoFrameAndSwsContext();
    initAudioFrameAndSwrContext();
    initTempPacket();
    return true;
}

void MediaDecoder::finish() {
    release();
}

bool MediaDecoder::getMediaInfo(const char* path) {
    int error;
    char buffer[] = "";
    // 注册网络协议
    avformat_network_init();
    av_register_all();
    // 获取上下文
    mformatContext = avformat_alloc_context();
    // 打开视频
    error = avformat_open_input(&mformatContext, path, NULL, NULL);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGE("打开视频失败 %s : %d(%s)", path, error, buffer);
        return false;
    }
    // 获取视频信息
    error = avformat_find_stream_info(mformatContext, NULL);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGE("获取视频信息失败 %s : %d(%s)", path, error, buffer);
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
    int error;
    char buffer[] = "";
    mAudioFrame = av_frame_alloc();
    mSwrContext = swr_alloc();

    int64_t outChannelLayout = av_get_default_channel_layout(mAudioCodecContext->channels);
    AVSampleFormat outSampleFormat = AV_SAMPLE_FMT_S16;
    int outSampleRate = mAudioCodecContext->sample_rate;  // 输出的采样率必须与输入相同
    int outNumChannels = mAudioCodecContext->channels;
    mSwrBufferSize = av_samples_get_buffer_size(NULL, outNumChannels, outSampleRate, outSampleFormat, 1);
    mAudioOutBuffer = (uint8_t*) av_malloc(mSwrBufferSize);
    swr_alloc_set_opts(mSwrContext, outChannelLayout, outSampleFormat, outSampleRate,
                       mAudioCodecContext->channel_layout, mAudioCodecContext->sample_fmt, mAudioCodecContext->sample_rate,
                       0, NULL);
    error = swr_init(mSwrContext);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGE("initAudioFrameAndSwrContext 失败: %d(%s)", error, buffer);
        return false;
    }
    return true;
}

bool MediaDecoder::initTempPacket() {
    mTempPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(mTempPacket);
    return false;
}

bool MediaDecoder::readFrame(bool &isVideoFrame) {

    if (av_read_frame(mformatContext, mTempPacket) >= 0) {
        isVideoFrame = (mTempPacket->stream_index == mVideoStreamIndex);
        return true;
    } else {
        av_free_packet(mTempPacket);
        return false;
    }
}

VideoFrame* MediaDecoder::decodeVideoFrame() {
    int frameCount;
    double pts;
    int error;
    char buffer[] = "";
    if (mTempPacket == NULL || mTempPacket->stream_index != mVideoStreamIndex) {
        return NULL;
    }
    // 解码
    error = avcodec_decode_video2(mVideoCodecContext, mVideoFrame, &frameCount, mTempPacket);
    // 转换成RGBA格式，后续要改为转成YUV格式->渲染成Texture->gles->播放
    // RGBA格式，解码图像数据只保存在data[0] lineSize[0] 中。但如果是YUV就会有data[0] data[1] data[2]
    int height = sws_scale(mSwsContext, (const uint8_t *const *)mVideoFrame->data, mVideoFrame->linesize, 0, mVideoFrame->height,
              mRgbFrame->data, mRgbFrame->linesize);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGE("decodeVideoFrame失败: %d(%s)", error, buffer);
        return NULL;
    }

    if ((pts = av_frame_get_best_effort_timestamp(mVideoFrame)) == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(mformatContext->streams[mVideoStreamIndex]->time_base);
    // av_frame_get_best_effort_timestamp 可能失败，播放需要做纠正

    VideoFrame *videoFrame = new VideoFrame();
    videoFrame->pts = pts;
    videoFrame->height = mVideoCodecContext->height;
    videoFrame->width = std::min(mVideoCodecContext->width, mRgbFrame->linesize[0]);
    videoFrame->size =  videoFrame->width * videoFrame->height;
    videoFrame->linesize = mRgbFrame->linesize[0];
    videoFrame->rgb = new uint8_t[videoFrame->height * videoFrame->width];
    copyFrameData(videoFrame->rgb, mRgbFrame->data[0], videoFrame->width, videoFrame->height, videoFrame->linesize);

    LOGE("视频解码 height: %d, size: %ld, pts: %f",videoFrame->width, videoFrame->size, videoFrame->pts);

    LOGE("视频解码 pts: %f", pts);
    av_free_packet(mTempPacket);
    return videoFrame;
}

std::vector<AudioFrame*> MediaDecoder::decodeAudioFrame() {
    int error;
    char buffer[] = "";
    std::vector<AudioFrame*> audioFrames;

    if (mTempPacket == NULL || mTempPacket->stream_index != mAudioStreamIndex) {
        return audioFrames;
    }
    // 对于视频帧，一个AVPacket是一帧视频帧；对于音频帧，一个AVPacket有可能包含多个音频帧
    int packetSize = mTempPacket->size;
    while (packetSize > 0) {
        int gotframe = 0;
        int len = avcodec_decode_audio4(mAudioCodecContext, mAudioFrame, &gotframe, mTempPacket);
        if (len < 0) {
            break;
        }
        if (gotframe) {
            error = swr_convert(mSwrContext, &mAudioOutBuffer, mAudioFrame->nb_samples * 2, (const uint8_t **) mAudioFrame->data, mAudioFrame->nb_samples);
            if (error < 0) {
                av_strerror(error, buffer, 1024);
                LOGE("decodeAudioFrame失败: %d(%s)", error, buffer);
                return audioFrames;
            }
            // 一个 packet 中可以包含多个帧，packet 中的 PTS 比真正的播放的 PTS 可能会早很多,播放的时候需要重新纠正
            double pts = av_q2d(mformatContext->streams[mAudioStreamIndex]->time_base) * mTempPacket->pts;
            //AudioFrame* audioFrame = new AudioFrame;
            //audioFrame->pts = pts;
            //audioFrame->channelCount = mAudioCodecContext->channels;
            //audioFrame->samplerate = mAudioCodecContext->sample_rate;
            //audioFrame->size = mSwrBufferSize;
            //audioFrame->data = (char*)malloc(mSwrBufferSize);
            //strcpy(audioFrame->data, (char*)mAudioFrame->data[0]);

            //LOGE("音频解码 pts: %f, channelCount: %d, samplerate: %d, size: %d",audioFrame->pts, audioFrame->channelCount, audioFrame->samplerate, audioFrame->size);
            //audioFrames.push_back(audioFrame);
            //delete audioFrame;
            LOGE("音频解码 pts: %f", pts);
        }
        if (0 == len) {
            break;
        }
        packetSize -= len;
    }
    av_free_packet(mTempPacket);
    return audioFrames;
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
    if (mTempPacket) {
        av_free_packet(mTempPacket);
        av_free(mTempPacket);
        mTempPacket = NULL;
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

void MediaDecoder::copyFrameData(uint8_t *dst, uint8_t *src, int width, int height, int linesize) {
    for (int i = 0; i < height; i++) {
        memcpy(dst, src, width);
        dst += width;
        src += linesize;
    }
}
