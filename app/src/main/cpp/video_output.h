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
    virtual void output(VideoFrame& videoFrame) = 0;
};

enum MsgType {
    MESSAGE_CREATE_CONTEXT,
    MESSAGE_QUIT,
    MESSAGE_RENDER,
    MESSAGE_CHANGE_SIZE
};

struct Message {
    Message(MsgType type) : msgType(type), value(0) {}
    Message(MsgType type, int val) : msgType(type), value(val) {}
    MsgType msgType;
    int value;
};

class VideoOutput : public virtual IVideoOutput {
public:
    VideoOutput();
    ~VideoOutput();
    void onCreated(ANativeWindow *nativeWindow);
    void onChangeSize(int screenWidth, int screenHeigth);
    void onDestroy();
    virtual void output(VideoFrame& videoFrame);
    bool postMessage(Message msg);
    EGLContext getShareContext();

private:
    void createEglContextHandler();
    void createRenderHandlerThread();
    void releaseRenderHanlder();
    void renderTextureHandler(int textureId);
    void changeSizeHanlder();
    void processMessages();
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
};


#endif //SPLAYER_MEDIA_OUTPUT_H