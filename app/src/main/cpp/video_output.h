//
// Created by 橙俊森 on 2018/10/15.
//

#ifndef SPLAYER_MEDIA_OUTPUT_H
#define SPLAYER_MEDIA_OUTPUT_H

#include <android/native_window_jni.h>
#include "media_frame.h"
#include "egl/egl_core.h"
#include "render/gl_render.h"
#include <queue>

class IVideoOutput {
public:
    virtual void output(const VideoFrame& videoFrame) = 0;
};

enum Message {
    MESSAGE_CREATE_CONTEXT,
    MESSAGE_QUIT,
    MESSAGE_NONE,
    MESSAGE_RENDER,
    MESSAGE_CHANGE_SIZE
};

class VideoOutput : public virtual IVideoOutput {
public:
    VideoOutput();
    ~VideoOutput();
    void onCreated(ANativeWindow *nativeWindow, int screenWidth, int screenHeight);
    void onChangeSize(int screenWidth, int screenHeigth);
    void onDestroy();
    virtual void output(const VideoFrame& videoFrame);
    bool postMessage(Message msg);

private:
    void createEglContextHandler();
    void createRenderHandlerThread();
    void releaseRenderHanlder();
    void renderTextureHandler();
    void processMessages();
    Message dequeueMessageHandler();
    static void* renderHandlerThread(void* self);

private:
    ANativeWindow *mNativeWindow;
    EglCore mEglCore;
    GlRender mGlRender;
    EGLSurface  mSurface;
    int screenWidth;
    int screenHeight;
    pthread_t mRenderHandlerThread;
    pthread_mutex_t mRenderHandlerMutex;
    pthread_cond_t mRenderHandlerCond;
    std::queue<Message> mHandlerMessageQueue;

    void changeSizeHanlder();
};


#endif //SPLAYER_MEDIA_OUTPUT_H
