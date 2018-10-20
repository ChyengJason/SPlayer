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

enum MsgType {
    MESSAGE_CREATE_CONTEXT,
    MESSAGE_CREATE_SURFACE,
    MESSAGE_UPDATE_SURFACE,
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

class VideoOutput{
public:
    VideoOutput();
    ~VideoOutput();
    void start();
    void onCreated(ANativeWindow *nativeWindow);
    void onUpdated(ANativeWindow *nativeWindow);
    void onChangeSize(int screenWidth, int screenHeigth);
    void onDestroy();
    void output(VideoFrame& videoFrame);
    bool postMessage(Message msg);
    EGLContext getShareContext();
    bool isSurfaceValid();

private:
    void createEglContextHandler();
    void createRenderHandlerThread();
    void releaseRenderHanlder();
    void renderTextureHandler(int textureId);
    void changeSizeHanlder();
    void processMessages();
    void createSurfaceHandler();
    void updateSurfaceHandler();
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
    bool isInited;
};


#endif //SPLAYER_MEDIA_OUTPUT_H
