//
// Created by chengjunsen on 2018/10/22.
//

#ifndef SPLAYER_VIDEO_QUEUE_H
#define SPLAYER_VIDEO_QUEUE_H

#include <queue>
#include "media_frame.h"
#include "egl/egl_core.h"
#include "render/gl_render.h"

class VideoQueue {
public:
    VideoQueue();
    ~VideoQueue();
    void release();
    void push(VideoFrame * frame);
    TextureFrame* pop();
    bool isEmpty();
    void clear();
    int size();

    void start(int width, int height);
    void changeSize(int width, int height);

private:
    void createRenderThread();
    static void* runRender(void *self);
    void signalRender();
    void createRender();
    void processRender();
    TextureFrame *textureRender(VideoFrame *pFrame);

private:
    pthread_mutex_t mVideoFrameMutex;
    pthread_mutex_t mTextureFrameMutex;
    pthread_cond_t mRenderCond;
    pthread_t mRenderThread;
    std::queue<VideoFrame*> mVideoFrameQue;
    std::queue<TextureFrame*> mTextureFrameQue;
    bool isCreateRendered;
    bool isRunning;

    EglCore mEglCore;
    GlRender mGlRender;
    EGLSurface mPbufferSurface;
    int width;
    int height;
};


#endif //SPLAYER_VIDEO_QUEUE_H
