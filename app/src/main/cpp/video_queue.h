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
#include "sync_queue.h"

enum VideoQueueMessageType {
    VIDEOQUEUE_MESSAGE_CREATE,
    VIDEOQUEUE_MESSAGE_QUIT,
    VIDEOQUEUE_MESSAGE_PUSH,
    VIDEOQUEUE_MESSAGE_CLEAR
};

struct VideoQueueMessage {
    VideoQueueMessage(VideoQueueMessageType type) : msgType(type), value(0) {}
    VideoQueueMessage(VideoQueueMessageType type, void* val) : msgType(type), value(val) {}
    VideoQueueMessageType msgType;
    void* value;
};

class VideoQueue {
public:
    VideoQueue();
    ~VideoQueue();
    void push(std::vector<VideoFrame*> frames);
    TextureFrame* pop();
    bool isEmpty();
    void clear();
    int size();
    void start(int width, int height);
    void finish();

private:
    void postMessage(VideoQueueMessage msg);
    void postMessage(std::vector<VideoQueueMessage> msgs);
    void createRenderThread();
    static void* runRender(void *self);
    void processMessages();
    void createHandler();
    void releaseHandler();
    void renderHandler(void *Frame);
    void clearHandler();

private:
    pthread_mutex_t mRenderMutex;
    pthread_cond_t mRenderCond;
    pthread_t mRenderThread;

    SyncQueue<VideoQueueMessage> mHandlerMessageQueue;
    SyncQueue<TextureFrame*> mTextureFrameQue;
    void pushTextureFrame(TextureFrame* textureFrame);

    bool isThreadInited;
    EglCore mEglCore;
    GlYuvRender mGlRender;
    EGLContext mContext;
    EGLSurface mPbufferSurface;
    int frameWidth;
    int frameHeight;
    int mFbo;
};


#endif //SPLAYER_VIDEO_QUEUE_H
