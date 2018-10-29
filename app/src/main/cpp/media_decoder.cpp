//
// Created by chengjunsen on 2018/10/12.
//
#include "media_decoder.h"

MediaDecoder::MediaDecoder() {
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
    mVideoStreamIndex = -1;
    mAudioStreamIndex = -1;
    mOutChannels = 0;
    mOutSampleRate = 0;
    mOutChannelLayout = 0;
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
    mVideoStreamIndex = -1;
    mAudioStreamIndex = -1;
    mOutChannels = 0;
    mOutSampleRate = 0;
    mOutChannelLayout = 0;

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

    return true;
}

void MediaDecoder::finish() {
    release();
}

bool MediaDecoder::getMediaInfo(const char* path) {
    LOGD("path %s", path);
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
    int degress = 0;

    AVStream *videoStream = mformatContext->streams[mAudioStreamIndex];
    mAudioCodecContext = videoStream->codec;
    // 寻找解码器
    mAudioCodec = avcodec_find_decoder(mAudioCodecContext->codec_id);
    // 打开解码器
    error = avcodec_open2(mAudioCodecContext, mAudioCodec, NULL);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGE("打开音频解码器失败: %d(%s)", error, buffer);
        return false;
    }
    AVDictionary *videoStreamMetadata = videoStream->metadata;
    AVDictionaryEntry* entry = NULL;
    while ((entry = av_dict_get(videoStreamMetadata, "", entry, AV_DICT_IGNORE_SUFFIX))){
        printf("entry: key is %s value is %s\n", entry->key, entry->value);
        if (0 == strcmp(entry->key, "rotate")) {
            degress = atoi(entry->value);
        }
    }
    LOGD("video 角度：%d", degress);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGE("打开视频解码器失败: %d(%s)", error, buffer);
        return false;
    }
    return true;
}

bool MediaDecoder::initVideoFrameAndSwsContext() {
    mVideoFrame = av_frame_alloc();
    mYuvFrame = av_frame_alloc();
    if (mVideoCodecContext->pix_fmt != AV_PIX_FMT_YUV420P && mVideoCodecContext->pix_fmt != AV_PIX_FMT_YUVJ420P) {
        // 关联缓存区
        mVideoOutBuffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, mVideoCodecContext->width, mVideoCodecContext->height));
        avpicture_fill((AVPicture*)mYuvFrame, mVideoOutBuffer, AV_PIX_FMT_YUV420P, mVideoCodecContext->width, mVideoCodecContext->height);
        // 视频图像的转换, 比如格式转换
        mSwsContext = sws_getContext(mVideoCodecContext->width, mVideoCodecContext->height, mVideoCodecContext->pix_fmt,
                                     mVideoCodecContext->width, mVideoCodecContext->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
        LOGD("创建YUV420p转换器");
    } else {
        mVideoOutBuffer = NULL;
        mSwsContext = NULL;
        LOGD("不需要创建YUV420p转换器");
    }
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

    LOGD("samperate: %d", mOutSampleRate);
    LOGD("channel: %d", mOutChannels);
    LOGD("bufferSize: %d" , mAudioOutBufferSize);

    swr_alloc_set_opts(mSwrContext, mOutChannelLayout, mOutFormat, mOutSampleRate,
                       mAudioCodecContext->channel_layout, mAudioCodecContext->sample_fmt, mAudioCodecContext->sample_rate,
                       0, NULL);
    error = swr_init(mSwrContext);
    if (error < 0) {
        av_strerror(error, buffer, 1024);
        LOGD("initAudioFrameAndSwrContext 失败: %d(%s)", error, buffer);
        return false;
    }
    return true;
}

void MediaDecoder::initPacket() {
    packet = new AVPacket;
    av_init_packet(packet);
}

AVPacket* MediaDecoder::readFrame() {
    if (mformatContext == NULL) {
        return NULL;
    }
    if (av_read_frame(mformatContext, packet) >= 0) {
        return packet;
    }
    return NULL;
}

std::vector<VideoFrame*> MediaDecoder::decodeVideoFrame(AVPacket* packet) {
    LOGD("MediaDecoder decodeVideoFrame");
    std::vector<VideoFrame*> vec;
    if (packet == NULL || packet->stream_index != mVideoStreamIndex) {
        LOGE("不是視頻幀或者是空");
        return vec;
    }

    AVStream *videoStream = mformatContext->streams[mVideoStreamIndex];
    int gotframe = 0;
    double timestamp;
    double duration;
    int pktSize = packet->size;
    LOGD("MediaDecoder pktSize %d", pktSize);
    while (pktSize > 0) {
        //LOGD("开始解码视频帧");
        // 解码图像数据：RGBA格式保存在data[0] ，YUV格式有data[0] data[1] data[2]
        int len = avcodec_decode_video2(mVideoCodecContext, mVideoFrame, &gotframe, packet);
        //LOGD("解码视频帧：len %d, frameCount %d pktSize %d", len, gotframe, pktSize);
        if (len < 0) {
            LOGE("decode video error, skip packet");
            break;
        }
        if (gotframe) {
            AVFrame *newFrame = scaleVideoFrame();
            int pts = av_frame_get_best_effort_timestamp(newFrame);
            if (pts == AV_NOPTS_VALUE) {
                pts = 0;
            }
            timestamp = pts * r2d(videoStream->time_base);
            duration = av_frame_get_pkt_duration(newFrame) * r2d(videoStream->time_base);
            if (duration <= 0 && packet->pts > 0) {
                duration = 1.0 / packet->pts;
            }
            VideoFrame *videoFrame = createVideoFrame(timestamp, duration, newFrame);
            LOGD("解码视频：time : %lf, duration : %lf packt->pts ：%ld ", timestamp, duration, packet->pts);
            vec.push_back(videoFrame);
        }
        if (0 == len) {
            break;
        }
        pktSize -= len;
        av_free_packet(packet);
    }
    //LOGD("解码完成 %d", vec.size());
    return vec;
}

std::vector<AudioFrame*> MediaDecoder::decodeAudioFrame(AVPacket* packet) {
    std::vector<AudioFrame*> audioFrames;
    if (packet == NULL || packet->stream_index != mAudioStreamIndex) {
        return audioFrames;
    }

    int error = 0;
    char buffer[] = "";
    AVStream* audioStream = mformatContext->streams[mAudioStreamIndex];
    // 对于视频帧，一个AVPacket是一帧视频帧；对于音频帧，一个AVPacket有可能包含多个音频帧
    int packetSize = packet->size;
    while (packetSize > 0) {
        int gotframe = 0;
        int len = avcodec_decode_audio4(mAudioCodecContext, mAudioFrame, &gotframe, packet);
        if (len < 0) {
            break;
        }
        if (gotframe) {
            int swrlen = swr_convert(mSwrContext, &mAudioOutBuffer, mAudioOutBufferSize, (const uint8_t **) mAudioFrame->data, mAudioFrame->nb_samples);
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
            double timestamp = r2d(audioStream->time_base) * packet->pts;
            double duration = av_frame_get_pkt_duration(mAudioFrame) * r2d(audioStream->time_base);
            AudioFrame* audioFrame = createAudioFrame(timestamp, duration, size, mAudioOutBuffer);
            LOGD("解码音频：time : %lf, duration : %lf packt->pts ：%ld ", timestamp, duration, packet->pts);
            if(audioFrame) audioFrames.push_back(audioFrame);
            av_free_packet(packet);
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

AudioFrame *MediaDecoder::createAudioFrame(double timestamp , double duration, int size, uint8_t* data) {
    if (size <= 0 || data == NULL) {
        return NULL;
    }
    AudioFrame* audioFrame = new AudioFrame;
    audioFrame->size = size;
    audioFrame->timestamp  = timestamp ;
    audioFrame->duration = duration;
    audioFrame->samplerate = mOutSampleRate;
    audioFrame->channels = mOutChannels;
    audioFrame->data = new char[size];
    memcpy(audioFrame->data, (char*)data, size);
    return audioFrame;
}

VideoFrame *MediaDecoder::createVideoFrame(double timestamp , double duration, AVFrame *videoFrame) {
    VideoFrame* yuvFrame = new VideoFrame;
    yuvFrame->frameWidth = mVideoCodecContext->width;
    yuvFrame->frameHeight = mVideoCodecContext->height;
    yuvFrame->timestamp = 0;
    yuvFrame->duration = 0;

    int width = mVideoCodecContext->width;
    int height = mVideoCodecContext->height;

    int lumaLength = height * std::min(videoFrame->linesize[0], width);
    yuvFrame->luma = new uint8_t[lumaLength];
    copyFrameData(yuvFrame->luma, videoFrame->data[0], width, height, videoFrame->linesize[0]);

    int chromaBLength = height / 2 * std::min(videoFrame->linesize[1], width / 2);
    yuvFrame->chromaB = new uint8_t[chromaBLength];
    copyFrameData(yuvFrame->chromaB, videoFrame->data[1], width/2, height/2, videoFrame->linesize[1]);

    int chromaRLength = height / 2 * std::min(videoFrame->linesize[2], width /2);
    yuvFrame->chromaR = new uint8_t[chromaRLength];
    copyFrameData(yuvFrame->chromaR, videoFrame->data[2], width/2, height/2, videoFrame->linesize[2]);

    LOGD("size %d x %d, linesize %d  %d  %d", width, height, videoFrame->linesize[0], videoFrame->linesize[1], videoFrame->linesize[2]);
    LOGD("lumaLen %d chromaB %d chromaR %d", lumaLength, chromaBLength, chromaRLength);
    return yuvFrame;
}

void MediaDecoder::copyFrameData(uint8_t * dst, uint8_t * src, int width, int height, int linesize) {
    width = std::min(linesize, width);
    for (int i = 0; i < height; ++i) {
        memcpy(dst, src, width);
        dst += width;
        src += linesize;
    }
}

void MediaDecoder::release() {
    LOGD("MediaDecoder::release");
    if (mAudioStreamIndex != -1) {
        delete mformatContext->streams[mAudioStreamIndex];
        mAudioStreamIndex = -1;
    }
    if (mVideoStreamIndex != -1) {
        delete mformatContext->streams[mVideoStreamIndex];
        mVideoStreamIndex = -1;
    }
    if (packet) {
        av_free_packet(packet);
        delete packet;
        packet = NULL;
    }
    if (mYuvFrame) {
        av_free(mYuvFrame);
        mYuvFrame = NULL;
    }
    if (mVideoFrame) {
        av_free(mVideoFrame);
        mVideoFrame = NULL;
    }
    if (mAudioFrame) {
        av_frame_free(&mAudioFrame);
        av_free(&mAudioFrame);
        mAudioFrame = NULL;
    }
    if (mSwsContext) {
        sws_freeContext(mSwsContext);
        mSwsContext = NULL;
    }
    if (mVideoOutBuffer) {
        delete mAudioOutBuffer;
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
        avformat_close_input(&mformatContext);
        avformat_free_context(mformatContext);
        mformatContext = NULL;
    }
    LOGD("MediaDecoder::release finish");
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

double MediaDecoder::r2d(AVRational r) {
    return r.num == 0 || r.den == 0 ? 0.:(double)r.num/(double)r.den;
}

int MediaDecoder::getWidth() {
    return mVideoCodecContext != NULL ? mVideoCodecContext->width : 0;
}

int MediaDecoder::getHeight() {
    return mVideoCodecContext != NULL ? mVideoCodecContext->height : 0;
}

AVFrame *MediaDecoder::scaleVideoFrame() {
    if (mVideoOutBuffer == NULL || mSwsContext == NULL) {
        return mVideoFrame;
    } else {
        sws_scale(mSwsContext, (const uint8_t *const *) mVideoFrame->data, mVideoFrame->linesize, 0,
                  mVideoFrame->height, mYuvFrame->data, mYuvFrame->linesize);
        return mYuvFrame;
    }
}
