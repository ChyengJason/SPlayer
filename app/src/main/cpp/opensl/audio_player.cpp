//
// Created by chengjunsen on 2018/10/19.
//

#include <unistd.h>
#include "audio_player.h"
#include "audio_player_util.h"
#include "../util/android_log.h"

AudioPlayer::AudioPlayer() {
    isPlaying = false;
}

AudioPlayer::~AudioPlayer() {

}

void AudioPlayer::create(size_t samplerate, size_t channelCount) {
    LOGD("AudioPlayer createEngine：%d", createEngine());
    LOGD("AudioPlayer createMixVolume：%d", createMixVolume());
    LOGD("AudioPlayer createPlayer: %d", createPlayer(samplerate, channelCount));
    isPlaying = false;
}

bool AudioPlayer::createEngine() {
    SLresult result;
    result = slCreateEngine(&mEngineObject, 0, NULL, 0, NULL, NULL);// 创建引擎
    result = (*mEngineObject)->Realize(mEngineObject, SL_BOOLEAN_FALSE);// 实现 mEngineObject 接口对象
    result = (*mEngineObject)->GetInterface(mEngineObject, SL_IID_ENGINE, &mEngineEngine);// 通过 mEngineObject 的 GetInterface 方法初始化 mEngineEngine
    return result == SL_RESULT_SUCCESS;
}

bool AudioPlayer::createMixVolume() {
    SLresult result;
    result = (*mEngineEngine)->CreateOutputMix(mEngineEngine, &mOutputMixObject, 0, 0, 0); // 用引擎对象创建混音器对象
    result = (*mOutputMixObject)->Realize(mOutputMixObject, SL_BOOLEAN_FALSE); // 实现混音器接口对象
    result = (*mOutputMixObject)->GetInterface(mOutputMixObject, SL_IID_ENVIRONMENTALREVERB, &mOutputMixEnvirRevarb); // 初始化 mOutputMixEnvirRevarb
    SLEnvironmentalReverbSettings mOutputMixEnvirReverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
    // 设置
    if (result == SL_RESULT_SUCCESS) {
        result = (*mOutputMixEnvirRevarb)->SetEnvironmentalReverbProperties(mOutputMixEnvirRevarb, &mOutputMixEnvirReverbSettings);
    }
    return result == SL_RESULT_SUCCESS;
}

bool AudioPlayer::createPlayer(size_t samplerate, size_t channelCount) {
    // 缓冲区队列
    SLDataLocator_AndroidSimpleBufferQueue bufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    int channelMask = AudioPlayerUtil::getChannel(channelCount);
    int samplerateMask = AudioPlayerUtil::getSampleRate(samplerate);
    // pcm 参数配置
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM, // 播放 pcm 格式的数据
            channelCount, // 声道数量
            samplerateMask, // 采样率
            SL_PCMSAMPLEFORMAT_FIXED_16,// 采样位数 16 位
            SL_PCMSAMPLEFORMAT_FIXED_16,// 包含位数 跟采样位数一致就行
            channelMask, // 立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN // 结束标志
    };

    SLDataSource slDataSource = {&bufferQueue, &pcm};
    SLDataLocator_OutputMix slDataLocatorOutputMix = {SL_DATALOCATOR_OUTPUTMIX, mOutputMixObject};
    SLDataSink slDataSink = {&slDataLocatorOutputMix, NULL};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    // 创建播放器
    SLresult  result;
    result = (*mEngineEngine)->CreateAudioPlayer(mEngineEngine, &mAudioPlayerObjet, &slDataSource, &slDataSink, 3, ids, req);
    // 初始化播放器
    result = (*mAudioPlayerObjet)->Realize(mAudioPlayerObjet, SL_BOOLEAN_FALSE);

    // 获取音量接口
    result = (*mAudioPlayerObjet)->GetInterface(mAudioPlayerObjet, SL_IID_VOLUME, &mVolume);
    // 注册缓冲区
    result = (*mAudioPlayerObjet)->GetInterface(mAudioPlayerObjet, SL_IID_BUFFERQUEUE, &mBufferQueueInterface);
    // 设置回调接口
    result = (*mBufferQueueInterface)->RegisterCallback(mBufferQueueInterface, PlayerCallback, this);
    // 获取 Player 接口
    result = (*mAudioPlayerObjet)->GetInterface(mAudioPlayerObjet, SL_IID_PLAY, &mPlayer);

    return result == SL_RESULT_SUCCESS;
}

void AudioPlayer::release() {
    isPlaying = false;
    if(mAudioPlayerObjet != NULL){
        (*mAudioPlayerObjet)->Destroy(mAudioPlayerObjet);
        mAudioPlayerObjet = NULL;
        mBufferQueueInterface = NULL;
        mPlayer = NULL;
    }
    if(mOutputMixObject != NULL){
        (*mOutputMixObject)->Destroy(mOutputMixObject);
        mOutputMixObject = NULL;
        mOutputMixEnvirRevarb = NULL;
    }
    if(mEngineObject != NULL){
        (*mEngineObject)->Destroy(mEngineObject);
        mEngineObject = NULL;
        mEngineEngine = NULL;
    }
}

void AudioPlayer::PlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueueInterface, void *context) {
    AudioPlayer* player = (AudioPlayer*) context;
    char* data = NULL;
    int size = 0;
    bool isExist = player->getAudioDataCallback(&data, &size);
    if (!isExist || data == NULL || size <= 0) {
        player->isPlaying = false;
        return;
    }
    SLresult result = (*bufferQueueInterface)->Enqueue(bufferQueueInterface, data, size);
//    LOGE("Enqueue %s size: %d , data.size: %d", AudioPlayerUtil::ResultToString(result), sizeof(data), strlen(data));
    if (result != SL_RESULT_SUCCESS) {
        player->isPlaying = false;
    }
}

bool AudioPlayer::pause() {
    SLresult result = (*mPlayer)->SetPlayState(mPlayer, SL_PLAYSTATE_PAUSED);
    if (result == SL_RESULT_SUCCESS) {
        isPlaying = false;
        return true;
    }
    return false;
}

bool AudioPlayer::play() {
    SLresult result = (*mPlayer)->SetPlayState(mPlayer, SL_PLAYSTATE_PLAYING);
    PlayerCallback(mBufferQueueInterface, this);
    if (result == SL_RESULT_SUCCESS) {
        isPlaying = true;
        return true;
    }
    return false;
}

/**
 * 设置音量 [0-100]
 * @param level
 * @return
 */
bool AudioPlayer::setVolume(int level) {
    SLresult result = (*mVolume)->SetVolumeLevel(mVolume, (SLmillibel) ((1.0f - level/ 100.0f) * -5000));
    return result;
}

bool AudioPlayer::isRunning() {
    return isPlaying;
}
