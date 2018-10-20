//
// Created by chengjunsen on 2018/10/20.
//

#include "audio_decoder.h"

AudioDecoder::AudioDecoder() {

}

AudioDecoder::~AudioDecoder() {

}

void AudioDecoder::start(const char *path) {
    av_register_all();
    mformatContext = avformat_alloc_context();
    int error;
    char buf[] = "";
    if ((error = avformat_open_input(&mformatContext, path, NULL, NULL)) < 0) {
        av_strerror(error, buf, 1024);
        LOGE("Couldn't open file %s: %d(%s)", path, error, buf);
        LOGE("打开视频失败");
        return;
    }
    if (avformat_find_stream_info(mformatContext, NULL) < 0) {
        LOGE(" 获取内容失败 ");
        return;
    }
    mAudioStreamIndex = -1;
    for (int i = 0; i < mformatContext->nb_streams; ++i) {
        if (mformatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            mAudioStreamIndex = i;
        }
    }
    LOGE("音频流: %d", mAudioStreamIndex);
    if (mAudioStreamIndex == -1) {
        LOGE("音频流获取失败");
        return;
    }
    mAudioCodecContext = mformatContext->streams[mAudioStreamIndex]->codec;
    mAudioCodec = avcodec_find_decoder(mAudioCodecContext->codec_id);
    if (avcodec_open2(mAudioCodecContext, mAudioCodec, NULL) < 0) {
        LOGE("打开解码器失败");
        return;
    }

    packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    frame = av_frame_alloc();
    // 重采样
    mSwrContext = swr_alloc();
    // 缓存区
    mAudioOutBufferSize = 44100 * 2;
    mAudioOutBuffer = (uint8_t *) av_malloc(mAudioOutBufferSize);
    // 输出声道
    uint16_t  out_channel_layout = AV_CH_LAYOUT_STEREO;
    // 采样位数
    enum AVSampleFormat out_formart = AV_SAMPLE_FMT_S16;
    // 输出的采样率必须与输入相同
    int out_sample_rate = mAudioCodecContext->sample_rate;
    swr_alloc_set_opts(mSwrContext, out_channel_layout, out_formart, out_sample_rate,
                       mAudioCodecContext->channel_layout, mAudioCodecContext->sample_fmt, mAudioCodecContext->sample_rate, 0,
                       NULL);
    swr_init(mSwrContext);
    // 获取通道数
    mOutChannels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    mOpenSl = new OpenSL;
    mOpenSl->create(mAudioCodecContext->sample_rate, mAudioCodecContext->channels);
    mOpenSl->set(this);
    mOpenSl->play();
}

void AudioDecoder::getPcmData(void **pcm, size_t *pcm_size) {
    int got_frame;
    int count = 0;
    while (av_read_frame(mformatContext, packet) >= 0 && count ++ < 50) {
        if (packet->stream_index == mAudioStreamIndex) {
            avcodec_decode_audio4(mAudioCodecContext, frame, &got_frame, packet);
            if (got_frame) {
                swr_convert(mSwrContext, &mAudioOutBuffer, 44100 * 2, (const uint8_t **) frame->data, frame->nb_samples);
                int size = av_samples_get_buffer_size(NULL, mOutChannels, frame->nb_samples, AV_SAMPLE_FMT_S16, 1);
                *pcm = mAudioOutBuffer;
                *pcm_size = size;
                break;
            }
        }
    }
}

int OpenSL::getPcmDataCallback(char **buffer, int maxSize) {
    size_t size;
    mDecoder->getPcmData((void **)buffer, &size);
    return size;
}

void OpenSL::set(AudioDecoder *pDecoder) {
    mDecoder = pDecoder;
}
