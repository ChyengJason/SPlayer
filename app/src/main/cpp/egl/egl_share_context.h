//
// Created by chengjunsen on 2018/10/22.
//
#ifndef SPLAYER_EGL_SHARE_CONTEXT_H
#define SPLAYER_EGL_SHARE_CONTEXT_H

#include <EGL/egl.h>
#include <pthread.h>

class EglShareContext{
public:
    static EglShareContext& getInstance();
    ~EglShareContext();
    EGLContext getShareContext();
    void setShareContext(EGLContext context);
    void clearShareContext();
    void lock();
    void unlock();
private:
    static EglShareContext instance;
    EglShareContext();
    EGLContext mShareContext;
    pthread_mutex_t mMutex;
};

#endif

