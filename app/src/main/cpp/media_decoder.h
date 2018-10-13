//
// Created by chengjunsen on 2018/10/12.
//
#ifndef SPLAYER_VIDEO_DECODER_H
#define SPLAYER_VIDEO_DECODER_H
#include <string>
#include <vector>

extern "C" {
#include "libavcodec/avcodec.h" // 编码
#include "libavformat/avformat.h" // 封装格式
#include "libswscale/swscale.h" // 变换信息
#include "libswresample/swresample.h" //音频采样
#include "android_log.h"
};

class AudioFrame;
class VideoFrame;
/**
 * 负责解码音视频
 */
class MediaDecoder {
public:
    MediaDecoder();

    ~MediaDecoder();

    void start(const char *path);

private:
    void createDecoderThread();

    void sleep(int sec);

    bool getMediaInfo();

    bool initVideoCodec();

    bool initAudioCodec();

    bool initVideoFrameAndSwsContext();

    bool initAudioFrameAndSwrContext();

    void readFrames();

    void decodeVideoFrame(AVPacket *pPacket);

    void decodeAudioFrame(AVPacket *pPacket);

    void release();

    static void *run(void *self);

    void copyFrameData(uint8_t * dst, uint8_t * src, int width, int height, int linesize);
private:
    char *mPath;
    pthread_t mDecoderThread;
    pthread_cond_t mDecoderCond;
    pthread_mutex_t mDecoderMutex;
    AVFormatContext *mformatContext;
    AVCodecContext *mVideoCodecContext;
    AVCodecContext *mAudioCodecContext;
    AVCodec *mVideoCodec;
    AVCodec *mAudioCodec;
    int mVideoStreamIndex;
    int mAudioStreamIndex;

    SwsContext* mSwsContext;
    SwrContext* mSwrContext;
    uint8_t *mAudioOutBuffer;
    uint8_t *mVideoOutBuffer;
    AVFrame *mVideoFrame;
    AVFrame *mRgbFrame;
    AVFrame *mAudioFrame;
    std::vector<VideoFrame*> mVideoVec;
    std::vector<AudioFrame*> mAudioVec;
};

class VideoFrame {
public:
    double pts;
    int height;
    int width;
    uint8_t * rgb;
    long size;
    long linesize;
};

class AudioFrame {
public:
    int samplerate;
    int channelCount;
    int pts;
    char *samples;
    int size;
};

class YuvVideoFrame : public VideoFrame {
public:
    uint8_t * luma;
    uint8_t * chromaB;
    uint8_t * chromaR;
};

#endif //SPLAYER_VIDEO_DECODER_H
