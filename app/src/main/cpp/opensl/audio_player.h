//
// Created by chengjunsen on 2018/10/19.
//

#ifndef SPLAYER_AUDIO_PLAYER_H
#define SPLAYER_AUDIO_PLAYER_H

#include <string>

extern "C" {
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};

class AudioPlayer {
public:
    AudioPlayer();
    virtual ~AudioPlayer();
    void create(size_t samplerate, size_t channelCount);
    void release(); // 释放
    bool pause();
    bool play();
    bool setVolume(int level);
    bool isRunning();

protected:
    virtual bool getAudioDataCallback(char** data, int* size) = 0;

private:
    bool createEngine(); // 创建引擎
    bool createMixVolume(); // 创建混音器
    bool createPlayer(size_t samplerate, size_t channelCount); // 创建播放器
    static void PlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueueInterface, void *context);

private:
    SLObjectItf mEngineObject; // 引擎接口对象
    SLEngineItf mEngineEngine; // 具体引擎对象实例
    SLObjectItf mOutputMixObject; // 混音器接口对象
    SLEnvironmentalReverbItf  mOutputMixEnvirRevarb; // 具体混音器对象实例
    SLObjectItf mAudioPlayerObjet; // 播放器接口对象
    SLPlayItf mPlayer; // 播放器接口
    SLVolumeItf mVolume; // 音量
    SLAndroidSimpleBufferQueueItf  mBufferQueueInterface;// 缓冲区队列接口
    bool isPlaying;
};

#endif //SPLAYER_AUDIO_PLAYER_H
