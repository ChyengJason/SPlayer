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
    mVideoCodec = NULL;
    mVideoFrame = NULL;
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
    initPacket();
    initVideoFrameAndSwsContext();

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
    mVideoStreamIndex = -1;

    for (int i = 0; i < mformatContext->nb_streams; ++i) {
        enum AVMediaType type = mformatContext->streams[i]->codec->codec_type;
        if (type == AVMEDIA_TYPE_VIDEO) {
            mVideoStreamIndex = i;
        }
    }
    if ( mVideoStreamIndex == -1) {
        LOGE("获取视频流id: %d ", mVideoStreamIndex);
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

bool MediaDecoder::initVideoFrameAndSwsContext() {
    mVideoFrame = av_frame_alloc();
    if (AV_PIX_FMT_YUV420P == mVideoCodecContext->pix_fmt || AV_PIX_FMT_YUVJ420P == mVideoCodecContext->pix_fmt) {
        LOGE("确定是AV_PIX_FMT_YUV420P");
    }
    LOGE("视频宽度: %d, 高度: %d", mVideoCodecContext->width, mVideoCodecContext->height);
    return true;
}

void MediaDecoder::initPacket() {
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
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
    std::vector<VideoFrame*> vec;
    int gotframe = 0;
    double timestamp = 0;
    double duration = 0;
    int pktSize = packet->size;
    while (pktSize > 0) {
        LOGE("开始解码解码视频帧");
        int len = avcodec_decode_video2(mVideoCodecContext, mVideoFrame, &gotframe, packet);
        LOGE("解码视频帧：len %d, frameCount %d pktSize %d", len, gotframe, pktSize);
        if (len < 0) {
            LOGE("decode video error, skip packet");
            break;
        }
        if (gotframe) {
            VideoFrame *videoFrame = createVideoFrame(timestamp, duration, mVideoFrame);
            LOGD("解码视频：time : %lf, duration : %lf packt->pts ：%ld ", timestamp, duration, packet->pts);
            vec.push_back(videoFrame);
        }
        if (0 == len) {
            break;
        }
        pktSize -= len;
    }
    LOGE("解码完成");
    return vec;
}

int64_t MediaDecoder::getMediaDuration() {
    if (mformatContext) {
        return mformatContext->duration;
    }
    return 0;
}

VideoFrame *MediaDecoder::createVideoFrame(double timestamp , double duration, AVFrame *videoFrame) {
    LOGE("createVideoFrame");
    VideoFrame* yuvFrame = new VideoFrame;
    yuvFrame->frameWidth = mVideoCodecContext->width;
    yuvFrame->frameHeight = mVideoCodecContext->height;
    yuvFrame->timestamp = 0;
    yuvFrame->duration = 0;

    int width = mVideoCodecContext->width;
    int height = mVideoCodecContext->height;

    int lumaLength = std::min(videoFrame->linesize[0], width) * height;
    uint8_t * luma = new uint8_t[lumaLength];
    copyFrameData(luma, videoFrame->data[0], width, height, videoFrame->linesize[0]);
    yuvFrame->luma = luma;

    int chromaBLength = std::min(videoFrame->linesize[1], width / 2) * height / 2;
    uint8_t * chromaB = new uint8_t[chromaBLength];
    copyFrameData(chromaB, videoFrame->data[1], width/2, height/2, videoFrame->linesize[1]);
    yuvFrame->chromaB = chromaB;

    int chromaRLength = std::min(videoFrame->linesize[2], width / 2) * height / 2;
    uint8_t * chromaR = new uint8_t[chromaRLength];
    copyFrameData(chromaR, videoFrame->data[2], width/2, height/2, videoFrame->linesize[2]);
    yuvFrame->chromaR = chromaR;

    LOGE("size %d x %d, linesize %d  %d  %d", width, height, videoFrame->linesize[0], videoFrame->linesize[1], videoFrame->linesize[2]);
    LOGE("lumaLen %d chromaB %d chromaR %d", lumaLength, chromaBLength, chromaRLength);
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
    if (packet) {
        av_free_packet(packet);
        av_free(packet);
        packet = NULL;
    }
    if (mVideoFrame) {
        av_frame_free(&mVideoFrame);
        mVideoFrame = NULL;
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

bool MediaDecoder::isVideoPacket(AVPacket * const packet) {
    return packet->stream_index == mVideoStreamIndex;
}

double MediaDecoder::r2d(AVRational r) {
    return r.num == 0 || r.den == 0 ? 0.:(double)r.num/(double)r.den;
}

int MediaDecoder::getWidth() {
    return mVideoCodecContext->width;
}

int MediaDecoder::getHeight() {
    return mVideoCodecContext->height;
}
