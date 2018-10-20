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

    AVPacket* readFrame();

    VideoFrame* decodeVideoFrame(AVPacket*);

    std::vector<AudioFrame*> decodeAudioFrame(AVPacket*);

    void finish();

    int64_t getMediaDuration();

    int getSamplerate();

    int getChannelCount();

    bool isVideoPacket(AVPacket* const packet);

    bool isAudioPacket(AVPacket* const packet);

private:
    bool init(const char* path);

    bool getMediaInfo(const char* path);

    bool initVideoCodec();

    bool initAudioCodec();

    bool initVideoFrameAndSwsContext();

    bool initAudioFrameAndSwrContext();

    void release();

    VideoFrame *createVideoFrame(double pts, AVFrame *videoFrame);

    AudioFrame *createAudioFrame(double pts, int samplerate, int channelCount, int size, uint8_t* data);

    void copyFrameData(uint8_t *dst, uint8_t *src, int width, int height, int linesize);

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
    AVFrame *mAudioFrame;
    AVFrame *mYuvFrame;
};

#endif //SPLAYER_VIDEO_DECODER_H
