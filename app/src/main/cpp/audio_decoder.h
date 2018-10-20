//
// Created by chengjunsen on 2018/10/20.
//

#ifndef SPLAYER_AUDIO_DECODER_H
#define SPLAYER_AUDIO_DECODER_H

#include "opensl/audio_player.h"

extern "C" {
#include "libavcodec/avcodec.h" // 编码
#include "libavformat/avformat.h" // 封装格式
#include "libswscale/swscale.h" // 变换信息
#include "libswresample/swresample.h" //音频采样
#include "android_log.h"
};

class OpenSL;

class AudioDecoder {
public:
    AudioDecoder();
    ~AudioDecoder();
    void start(const char* path);
    void getPcmData(void **pcm, size_t *pcm_size);

private:
    AVFormatContext *mformatContext;
    AVCodecContext *mAudioCodecContext;
    AVCodec *mAudioCodec;
    int mAudioStreamIndex;

    SwrContext* mSwrContext;

    int64_t mOutChannelLayout;
    AVSampleFormat mOutFormat;
    int mOutSampleRate;
    int mOutChannels;
    AVPacket* packet;
    AVFrame* frame;

    int mAudioOutBufferSize;
    uint8_t *mAudioOutBuffer;
    OpenSL* mOpenSl;
};

class OpenSL : public AudioPlayer {
public:
    virtual int getPcmDataCallback(char**buffer, int maxSize);

    void set(AudioDecoder *pDecoder);

private:
    AudioDecoder *mDecoder;
};

#endif //SPLAYER_AUDIO_DECODER_H
