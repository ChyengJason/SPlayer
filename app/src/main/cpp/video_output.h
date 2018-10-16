//
// Created by 橙俊森 on 2018/10/15.
//

#ifndef SPLAYER_MEDIA_OUTPUT_H
#define SPLAYER_MEDIA_OUTPUT_H

#include <android/native_window_jni.h>
#include "media_frame.h"
#include "egl/egl_core.h"
#include <queue>

class IVideoOutput {
public:
    virtual void output(const VideoFrame& videoFrame) = 0;
};

enum Message {
    MESSAGE_CREATE_CONTEXT,
    MESSAGE_QUIT,
    MESSAGE_NONE,
    MESSAGE_RENDER
};

class VideoOutput : public virtual IVideoOutput {
public:
    VideoOutput();
    ~VideoOutput();
    void onSurfaceCreated(ANativeWindow *nativeWindow, int screenHeight, int screenWidth);
    void onSurfaceDestroy();
    virtual void output(const VideoFrame& videoFrame);
    void stop();
    bool postMessage(Message msg);

private:
    void createHandlerEglContext();
    void createRenderHandlerThread();
    void releaseRenderHanlder();
    void processMessages();
    Message dequeueHandlerMessage();
    static void* renderHandlerThread(void* self);

private:
    ANativeWindow *mNativeWindow;
    EglCore* mEglCore;
    EGLSurface  mSurface;
    pthread_t mRenderHandlerThread;
    pthread_mutex_t mRenderHandlerMutex;
    pthread_cond_t mRenderHandlerCond;
    std::queue<Message> mHandlerMessageQueue;
};


#endif //SPLAYER_MEDIA_OUTPUT_H
