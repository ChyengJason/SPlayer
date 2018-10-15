//
// Created by 橙俊森 on 2018/10/15.
//

#ifndef SPLAYER_MEDIA_OUTPUT_H
#define SPLAYER_MEDIA_OUTPUT_H

#include <android/native_window_jni.h>
#include "media_frame.h"

class IVideoOutput {
public:
    virtual void output(const VideoFrame& videoFrame) = 0;
};

class VideoOutput : public virtual IVideoOutput {
public:
    VideoOutput();
    ~VideoOutput();
    void prepare(ANativeWindow *nativeWindow);
    virtual void output(const VideoFrame& videoFrame);
    void release();
private:
    bool isInited;
    ANativeWindow *mNativeWindow;
    ANativeWindow_Buffer mNativeOutBuffer;

};


#endif //SPLAYER_MEDIA_OUTPUT_H
