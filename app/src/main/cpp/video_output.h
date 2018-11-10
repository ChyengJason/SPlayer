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

class IVideoOutput {
public:
    virtual ~IVideoOutput() {}
    virtual VideoFrame* getVideoFrame() = 0;
};

class VideoOutput{
public:
    VideoOutput(IVideoOutput* callback);
    ~VideoOutput();
    void onCreated(ANativeWindow *nativeWindow);
    void onChangeSize(int screenWidth, int screenHeigth);
    void onDestroy();
    bool isRunning();

private:
    void createRunThread();
    static void* runHandler(void *self);
    void runHandlerImpl();
    void createContextHandler();
    void releaseContextHandler();
    void renderVideoFrameHandler(VideoFrame* frame);
    void changeSizeHandler();

private:
    ANativeWindow *mNativeWindow;
    EglCore mEglCore;
    GlYuvRender mGlRender;
    EGLContext  mContext;
    EGLSurface mSurface;
    int screenWidth;
    int screenHeight;
    pthread_t mRenderHandlerThread;
    bool isCreated;
    bool isChangeSized;
    bool isDestroy;
    IVideoOutput* mOutputInterface;
};


#endif //SPLAYER_MEDIA_OUTPUT_H
