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
#include "util/sync_queue.h"
#include <queue>

enum VideoOutputMessage{
    MESSAGE_CREATE_CONTEXT,
    MESSAGE_QUIT,
    MESSAGE_RENDER,
    MESSAGE_CHANGE_SIZE
};

class IVideoOutput {
public:
    virtual ~IVideoOutput() {}
    virtual TextureFrame* getTetureFrame() = 0;
};

class VideoOutput{
public:
    VideoOutput(IVideoOutput* callback);
    ~VideoOutput();
    void onCreated(ANativeWindow *nativeWindow);
    void onChangeSize(int screenWidth, int screenHeigth);
    void onDestroy();
    void output(void *frame);
    void postMessage(VideoOutputMessage msg);
    bool isSurfaceValid();
    void signalRenderFrame();

private:
    void createContextHandler();
    void createRenderHandlerThread();
    void releaseRenderHanlder();
    void renderTextureHandler();
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
    pthread_mutex_t mRenderMutex;
    pthread_cond_t mRenderCond;
    SyncQueue<VideoOutputMessage> mMessageQueue;
    bool isThreadInited;
    IVideoOutput* mOutputInterface;
};


#endif //SPLAYER_MEDIA_OUTPUT_H
