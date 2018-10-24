//
// Created by chengjunsen on 2018/10/22.
//

#ifndef SPLAYER_VIDEO_QUEUE_H
#define SPLAYER_VIDEO_QUEUE_H

#include <queue>
#include "media_frame.h"
#include "egl/egl_core.h"
#include "render/gl_base_render.h"
#include "render/gl_yuv_render.h"

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

private:
    void createRenderThread();
    static void* runRender(void *self);
    void signalRender();
    void createRender();
    void processRender();
    void createFboRender();
    void destroyRender();
    TextureFrame *textureRender(const VideoFrame *pFrame);

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
    GlYuvRender mGlRender;
    EGLSurface mPbufferSurface;
    int frameWidth;
    int frameHeight;
    int mFbo;
};


#endif //SPLAYER_VIDEO_QUEUE_H
