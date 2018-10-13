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
    std::vector<VideoFrame> mVideoVec;
    std::vector<AudioFrame> mAudioVec;
};

class VideoFrame {
public:
    int pts;
    int frameCount;
    int height;
    int width;
};

class AudioFrame {
public:
    int position;
};

#endif //SPLAYER_VIDEO_DECODER_H
