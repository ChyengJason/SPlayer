//
// Created by 橙俊森 on 2018/10/15.
//

#ifndef SPLAYER_MEDIA_OUTPUT_H
#define SPLAYER_MEDIA_OUTPUT_H

#include <android/native_window_jni.h>
#include "media_frame.h"
#include "egl/egl_core.h"
#include "render/gl_base_render.h"
#include "render/gl_yuv_render.h"
#include <queue>

enum MsgType {
    MESSAGE_CREATE_CONTEXT,
    MESSAGE_CREATE_SURFACE,
    MESSAGE_UPDATE_SURFACE,
    MESSAGE_DESTROY_SURFACE,
    MESSAGE_QUIT,
    MESSAGE_RENDER,
    MESSAGE_CHANGE_SIZE
};

struct Message {
    Message(MsgType type) : msgType(type), value(0) {}
    Message(MsgType type, void* val) : msgType(type), value(val) {}
    MsgType msgType;
    void* value;
};

class VideoOutput{
public:
    VideoOutput();
    ~VideoOutput();
    void start();
    void finish();
    void onCreated(ANativeWindow *nativeWindow);
    void onChangeSize(int screenWidth, int screenHeigth);
    void onDestroy();
    void output(void *frame);
    bool postMessage(Message msg);
    bool isSurfaceValid();

private:
    void createEglContextHandler();
    void createRenderHandlerThread();
    void releaseRenderHanlder();
    void renderTextureHandler(void *frame);
    void changeSizeHanlder();
    void processMessages();
    void createSurfaceHandler();
    void updateSurfaceHandler();
    void destroySurfaceHandler();
    static void* renderHandlerThread(void* self);

private:
    ANativeWindow *mNativeWindow;
    EglCore mEglCore;
    GlBaseRender mGlRender;
    EGLSurface mSurface;
    int screenWidth;
    int screenHeight;
    pthread_t mRenderHandlerThread;
    pthread_mutex_t mRenderHandlerMutex;
    pthread_cond_t mRenderHandlerCond;
    std::queue<Message> mHandlerMessageQueue;
    bool isThreadInited;
};


#endif //SPLAYER_MEDIA_OUTPUT_H
