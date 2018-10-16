//
// Created by chengjunsen on 2018/10/12.
//
#ifndef SPLAYER_VIDEO_DECODER_H
#define SPLAYER_VIDEO_DECODER_H
#include <string>
#include <vector>
#include "media_frame.h"

extern "C" {
#include "libavcodec/avcodec.h" // 编码
#include "libavformat/avformat.h" // 封装格式
#include "libswscale/swscale.h" // 变换信息
#include "libswresample/swresample.h" //音频采样
#include "android_log.h"
};

/**
 * 负责解码音视频
 */
class MediaDecoder {
public:
    MediaDecoder();

    ~MediaDecoder();

    bool prepare(const char* path);

    bool readFrame(bool& isVideoFrame);

    VideoFrame* decodeVideoFrame();

    std::vector<AudioFrame*> decodeAudioFrame();

    void finish();

    int64_t getMediaDuration();

private:
    bool init(const char* path);

    bool getMediaInfo(const char* path);

    bool initVideoCodec();

    bool initAudioCodec();

    bool initVideoFrameAndSwsContext();

    bool initAudioFrameAndSwrContext();

    bool initTempPacket();

    void release();
private:
    AVFormatContext *mformatContext;
    AVCodecContext *mVideoCodecContext;
    AVCodecContext *mAudioCodecContext;
    AVCodec *mVideoCodec;
    AVCodec *mAudioCodec;
    int mVideoStreamIndex;
    int mAudioStreamIndex;

    SwsContext* mSwsContext;
    SwrContext* mSwrContext;
    int mSwrBufferSize;
    uint8_t *mAudioOutBuffer;
    uint8_t *mVideoOutBuffer;
    AVFrame *mVideoFrame;
    AVFrame *mRgbFrame;
    AVFrame *mAudioFrame;
    AVPacket *mTempPacket;
};

#endif //SPLAYER_VIDEO_DECODER_H
