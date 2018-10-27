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

enum VideoOutputMessageType {
    MESSAGE_CREATE_CONTEXT,
    MESSAGE_QUIT,
    MESSAGE_RENDER,
    MESSAGE_CHANGE_SIZE
};

struct VideoOutputMessage {
    VideoOutputMessage(VideoOutputMessageType type) : msgType(type), value(0) {}
    VideoOutputMessage(VideoOutputMessageType type, void* val) : msgType(type), value(val) {}
    VideoOutputMessageType msgType;
    void* value;
};

class VideoOutput{
public:
    VideoOutput();
    ~VideoOutput();
    void onCreated(ANativeWindow *nativeWindow);
    void onChangeSize(int screenWidth, int screenHeigth);
    void onDestroy();
    void output(void *frame);
    bool postMessage(VideoOutputMessage msg);
    bool isSurfaceValid();

private:
    void createContextHandler();
    void createRenderHandlerThread();
    void releaseRenderHanlder();
    void renderTextureHandler(void *frame);
    void changeSizeHanlder();
    void processMessages();
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
    std::queue<VideoOutputMessage> mHandlerMessageQueue;
    bool isThreadInited;
};


#endif //SPLAYER_MEDIA_OUTPUT_H
