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
    void finish();
    AVPacket* readFrame();
    std::vector<VideoFrame*> decodeVideoFrame(AVPacket*);
    int64_t getMediaDuration();
    bool isVideoPacket(AVPacket* const packet);
    int getHeight();
    int getWidth();

private:
    bool init(const char* path);
    bool initVideoCodec();
    bool initVideoFrameAndSwsContext();
    void initPacket();
    bool getMediaInfo(const char* path);
    void release();
    VideoFrame *createVideoFrame(double timestamp , double duration, AVFrame *videoFrame);
    void copyFrameData(uint8_t *dst, uint8_t *src, int width, int height, int linesize);
    double r2d(AVRational r);

private:
    AVFormatContext *mformatContext;
    AVCodecContext *mVideoCodecContext;
    AVCodec *mVideoCodec;
    AVPacket*packet;

    // 视频相关
    int mVideoStreamIndex;
    AVFrame *mVideoFrame;
};

#endif //SPLAYER_VIDEO_DECODER_H
