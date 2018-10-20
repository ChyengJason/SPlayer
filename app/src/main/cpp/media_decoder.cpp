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
    mYuvFrame = NULL;
    mSwsContext = NULL;
    mVideoOutBuffer = NULL;
    mSwrContext = NULL;
    mAudioOutBuffer = NULL;
    mAudioOutBufferSize = 0;
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
    initPacket();
    initVideoFrameAndSwsContext();
    initAudioFrameAndSwrContext();

    psl = new MOpenSL;
    psl->create(mAudioCodecContext->sample_rate, mAudioCodecContext->channels);
    psl->set(this);
    psl->play();
    return true;
}

void MediaDecoder::finish() {
    release();
}

bool MediaDecoder::getMediaInfo(const char* path) {
    int error;
    char buffer[] = "";
    // 注册网络协议
    //avformat_network_init();
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

    if (mVideoCodecContext->pix_fmt != AV_PIX_FMT_YUV420P) {
        mYuvFrame = av_frame_alloc();
        // 关联缓存区
        mVideoOutBuffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, mVideoCodecContext->width, mVideoCodecContext->height));
        avpicture_fill((AVPicture*)mYuvFrame, mVideoOutBuffer, AV_PIX_FMT_YUV420P, mVideoCodecContext->width, mVideoCodecContext->height);
        // 视频图像的转换, 比如格式转换
        mSwsContext = sws_getContext(mVideoCodecContext->width, mVideoCodecContext->height, mVideoCodecContext->pix_fmt,
                                     mVideoCodecContext->width, mVideoCodecContext->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
        LOGE("创建YUV420p转换器");
    } else {
        mYuvFrame = NULL;
        mSwsContext = NULL;
    }
    LOGE("视频宽度: %d, 高度: %d", mVideoCodecContext->width, mVideoCodecContext->height);
    return true;
}

bool MediaDecoder::initAudioFrameAndSwrContext() {
    int error;
    char buffer[] = "";
    mSwrContext = swr_alloc();
    mAudioFrame = av_frame_alloc();
     // 输出的采样率必须与输入相同
    mOutChannelLayout = AV_CH_LAYOUT_STEREO;

    mOutFormat = AV_SAMPLE_FMT_S16;
    mOutSampleRate = mAudioCodecContext->sample_rate;
    mOutChannels = av_get_channel_layout_nb_channels(mOutChannelLayout);
    mAudioOutBufferSize = av_samples_get_buffer_size(NULL, mOutChannels, mOutSampleRate, mOutFormat, 1);
    mAudioOutBuffer = (uint8_t*) av_malloc(mAudioOutBufferSize);

    LOGE("outChannelLayout: %ld", mOutChannelLayout);
    LOGE("samperate: %d", mOutSampleRate);
    LOGE("channel: %d", mOutChannels);
    LOGE("bufferSize: %d" , mAudioOutBufferSize);

    swr_alloc_set_opts(mSwrContext, mOutChannelLayout, mOutFormat, mOutSampleRate,
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

void MediaDecoder::initPacket() {
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
}

AVPacket* MediaDecoder::readFrame() {
    if (av_read_frame(mformatContext, packet) >= 0) {
        return packet;
    }
    return NULL;
}

VideoFrame* MediaDecoder::decodeVideoFrame(AVPacket* packet) {
    VideoFrame *videoFrame = NULL;
    char buffer[] = "";
    int frameCount;
    double pts;
    int error;
    if (packet == NULL || packet->stream_index != mVideoStreamIndex) {
        return videoFrame;
    }
    // 解码图像数据：RGBA格式保存在data[0] ，YUV格式有data[0] data[1] data[2]
    error = avcodec_decode_video2(mVideoCodecContext, mVideoFrame, &frameCount, packet);
    // av_frame_get_best_effort_timestamp 可能失败，播放需要做纠正
    if ((pts = av_frame_get_best_effort_timestamp(mVideoFrame)) == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(mformatContext->streams[mVideoStreamIndex]->time_base);
    // 若非YUV420p格式
    if (mSwsContext && mYuvFrame) {
        sws_scale(mSwsContext, (const uint8_t *const *)mVideoFrame->data, mVideoFrame->linesize, 0, mVideoFrame->height,
                  mYuvFrame->data, mYuvFrame->linesize);
        if (error < 0) {
            av_strerror(error, buffer, 1024);
            LOGE("decodeVideoFrame失败: %d(%s)", error, buffer);
            return videoFrame;
        }
        videoFrame = createVideoFrame(pts, mYuvFrame);
    } else {
        videoFrame = createVideoFrame(pts, mVideoFrame);
    }
    return videoFrame;
}

std::vector<AudioFrame*> MediaDecoder::decodeAudioFrame(AVPacket* packet) {
    int error = 0;
    char buffer[] = "";
    AVFrame* frame = av_frame_alloc();

    std::vector<AudioFrame*> audioFrames;

    if (packet == NULL || packet->stream_index != mAudioStreamIndex) {
        return audioFrames;
    }
    // 对于视频帧，一个AVPacket是一帧视频帧；对于音频帧，一个AVPacket有可能包含多个音频帧
    int packetSize = packet->size;
    while (packetSize > 0) {
        int gotframe = 0;
        int len = avcodec_decode_audio4(mAudioCodecContext, frame, &gotframe, packet);
        if (len < 0) {
            break;
        }
        if (gotframe) {
            int swrlen = swr_convert(mSwrContext, &mAudioOutBuffer, mAudioOutBufferSize, (const uint8_t **) frame->data, frame->nb_samples);
            if (swrlen  < 0) {
                av_strerror(error, buffer, 1024);
                LOGE("decodeAudioFrame失败: %d(%s)", error, buffer);
                return audioFrames;
            }
            if (swrlen == mAudioOutBufferSize) {
                LOGE("audio buffer is probably too small");
            }
            int size = swrlen * mAudioCodecContext->channels  * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);//每声道采样数 x 声道数 x 每个采样字节数
            // 一个 packet 中可以包含多个帧，packet 中的 PTS 比真正的播放的 PTS 可能会早很多,播放的时候需要重新纠正
            double pts = av_q2d(mformatContext->streams[mAudioStreamIndex]->time_base) * packet->pts;
            LOGD("创建音频帧size: %d",size);
            AudioFrame* audioFrame = createAudioFrame(pts, size, mAudioOutBuffer);
            if(audioFrame) audioFrames.push_back(audioFrame);
        }
        if (0 == len) {
            break;
        }
        packetSize -= len;
    }
    return audioFrames;
}

int64_t MediaDecoder::getMediaDuration() {
    if (mformatContext) {
        return mformatContext->duration;
    }
    return 0;
}

AudioFrame *MediaDecoder::createAudioFrame(double pts, int size, uint8_t* data) {
    if (size <= 0 || data == NULL) {
        return NULL;
    }
    AudioFrame* audioFrame = new AudioFrame;
    audioFrame->size = size;
    audioFrame->pts = pts;
    audioFrame->samplerate = mOutSampleRate;
    audioFrame->channels = mOutChannels;
    audioFrame->data = new char[size];
    memcpy(audioFrame->data, (char*)data, size);
    return audioFrame;
}

VideoFrame *MediaDecoder::createVideoFrame(double pts, AVFrame *videoFrame) {
    VideoFrame* yuvFrame = new VideoFrame;
    yuvFrame->width = mVideoCodecContext->width;
    yuvFrame->height = mVideoCodecContext->height;
    yuvFrame->pts = pts;

    int width = std::min(videoFrame->linesize[0], mVideoCodecContext->width);
    int height = mVideoCodecContext->height;
    int lumaLength = width * height;
    uint8_t * luma = new uint8_t[lumaLength];
    copyFrameData(luma, videoFrame->data[0], width, height, videoFrame->linesize[0]);
    yuvFrame->luma = luma;

    width = std::min(videoFrame->linesize[1], mVideoCodecContext->width / 2);
    height = mVideoCodecContext->height / 2;
    int chromaBLength = width * height;
    uint8_t * chromaB = new uint8_t[chromaBLength];
    copyFrameData(chromaB, videoFrame->data[1], width, height, videoFrame->linesize[1]);
    yuvFrame->chromaB = chromaB;

    width = std::min(videoFrame->linesize[2], mVideoCodecContext->width / 2);
    height = mVideoCodecContext->height / 2;
    int chromaRLength = width * height;
    uint8_t * chromaR = new uint8_t[chromaRLength];
    copyFrameData(chromaR, videoFrame->data[2], width, height, videoFrame->linesize[2]);
    yuvFrame->chromaR = chromaR;
    return yuvFrame;
}

void MediaDecoder::copyFrameData(uint8_t * dst, uint8_t * src, int width, int height, int linesize) {
    for (int i = 0; i <height; ++i) {
        memcpy(dst, src, width);
        dst += width;
        src += linesize;
    }
}

void MediaDecoder::release() {
    if (packet) {
        av_free_packet(packet);
        packet = NULL;
    }
    if (mYuvFrame) {
        av_frame_free(&mYuvFrame);
        mYuvFrame = NULL;
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

int MediaDecoder::getSamplerate() {
    return mOutSampleRate;
}

int MediaDecoder::getChannelCount() {
    return mOutChannels;
}

bool MediaDecoder::isVideoPacket(AVPacket * const packet) {
    return packet->stream_index == mVideoStreamIndex;
}

bool MediaDecoder::isAudioPacket(AVPacket * const packet) {
    return packet->stream_index == mAudioStreamIndex;
}

void MediaDecoder::getPcmData(void **pcm, size_t *pcm_size) {
    while (packet = readFrame()) {
        if (packet->stream_index == mAudioStreamIndex) {
            std::vector<AudioFrame*> frames = decodeAudioFrame(packet);
            if (!frames.empty()) {
                *pcm = frames[0]->data;
                *pcm_size = frames[0]->size;
            }
            break;
        }
    }
}
