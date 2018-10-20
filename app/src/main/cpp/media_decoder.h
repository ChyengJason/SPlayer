//
// Created by chengjunsen on 2018/10/12.
//
#ifndef SPLAYER_VIDEO_DECODER_H
#define SPLAYER_VIDEO_DECODER_H
#include <string>
#include <vector>
#include "media_frame.h"
#include "opensl/audio_player.h"

extern "C" {
#include "libavcodec/avcodec.h" // 编码
#include "libavformat/avformat.h" // 封装格式
#include "libswscale/swscale.h" // 变换信息
#include "libswresample/swresample.h" //音频采样
#include "android_log.h"
};

class MOpenSL;

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

    void getPcmData(void **pcm, size_t *pcm_size);

private:
    bool init(const char* path);

    bool getMediaInfo(const char* path);

    bool initVideoCodec();

    bool initAudioCodec();

    bool initVideoFrameAndSwsContext();

    bool initAudioFrameAndSwrContext();

    void initPacket();

    void release();

    VideoFrame *createVideoFrame(double pts, AVFrame *videoFrame);

    AudioFrame *createAudioFrame(double pts, int size, uint8_t* data);

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

    int64_t mOutChannelLayout;
    AVSampleFormat mOutFormat;
    int mOutSampleRate;
    int mOutChannels;

    int mAudioOutBufferSize;
    uint8_t *mAudioOutBuffer;
    uint8_t *mVideoOutBuffer;
    AVFrame *mVideoFrame;
    AVFrame *mYuvFrame;
    AVFrame *mAudioFrame;
    AVPacket*packet;
    MOpenSL *psl;
};


class MOpenSL : public AudioPlayer {
public:
    virtual int getPcmDataCallback(char**buffer, int maxSize) {
        size_t size;
        mDecoder->getPcmData((void **)buffer, &size);
        return size;
    }

    void set(MediaDecoder *pDecoder) {
        mDecoder = pDecoder;
    }

private:
    MediaDecoder *mDecoder;
};


#endif //SPLAYER_VIDEO_DECODER_H
