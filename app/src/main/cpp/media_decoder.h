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

/**
 * 负责解码音视频
 */
class MediaDecoder {
public:
    MediaDecoder(const char *path);

    ~MediaDecoder();

    void start();

private:
    void createDecoderThread();

    void sleep(int sec);

    bool initMediaInfo();

    bool initVideoCodec();

    bool initAudioCodec();

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
    std::vector<int> mMediaVec;
    std::vector<int> mAudioVec;
};

#endif //SPLAYER_VIDEO_DECODER_H
