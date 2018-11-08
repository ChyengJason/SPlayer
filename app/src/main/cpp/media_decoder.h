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
#include "util/android_log.h"
};
/**
 * 负责解码音视频
 */
class MediaDecoder {
public:
    MediaDecoder();
    ~MediaDecoder();
    bool prepare(const char* path);
    void finish();
    AVPacket* readFrame();
    std::vector<VideoFrame*> decodeVideoFrame(AVPacket*);
    std::vector<AudioFrame*> decodeAudioFrame(AVPacket*);
    float getMediaDuration();
    int getSamplerate();
    int getChannelCount();
    bool isVideoPacket(AVPacket* const packet);
    bool isAudioPacket(AVPacket* const packet);
    int getHeight();
    int getWidth();
    void freePacket(AVPacket* packet);
    void seek(float d);

private:
    bool init(const char* path);
    bool initVideoCodec();
    bool initAudioCodec();
    bool initVideoFrameAndSwsContext();
    bool initAudioFrameAndSwrContext();
    bool getMediaInfo(const char* path);
    void release();
    AVFrame* scaleVideoFrame();
    VideoFrame *createVideoFrame(double timestamp , double duration, AVFrame *videoFrame);
    AudioFrame *createAudioFrame(double timestamp , double duration, int size, uint8_t* data);
    void copyFrameData(uint8_t *dst, uint8_t *src, int width, int height, int linesize);
    double r2d(AVRational r);

private:
    AVFormatContext *mformatContext;
    AVCodecContext *mVideoCodecContext;
    AVCodecContext *mAudioCodecContext;
    AVCodec *mVideoCodec;
    AVCodec *mAudioCodec;

    // 音频相关
    int mAudioStreamIndex;
    SwrContext* mSwrContext;
    uint8_t *mAudioOutBuffer;
    int mAudioOutBufferSize;
    AVFrame *mAudioFrame;
    int64_t mOutChannelLayout;
    AVSampleFormat mOutFormat;
    int mOutSampleRate;
    int mOutChannels;

    // 视频相关
    int mVideoStreamIndex;
    SwsContext* mSwsContext;
    uint8_t *mVideoOutBuffer;
    AVFrame *mVideoFrame;
    AVFrame *mYuvFrame;
};

#endif //SPLAYER_VIDEO_DECODER_H
