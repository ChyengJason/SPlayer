//
// Created by chengjunsen on 2018/10/22.
//

#include <EGL/egl.h>
#include "egl_share_context.h"
#include "../util/android_log.h"

EglShareContext EglShareContext::instance;

EglShareContext::EglShareContext() {
    mShareContext = EGL_NO_CONTEXT;
    pthread_mutex_init(&mMutex, NULL);
}

EglShareContext::~EglShareContext() {
    pthread_mutex_destroy(&mMutex);
}

EGLContext EglShareContext::getShareContext() {
    return mShareContext;
}

void EglShareContext::setShareContext(EGLContext context) {
    if (mShareContext != EGL_NO_CONTEXT)
        return;
    mShareContext = context;
}

void EglShareContext::clearShareContext() {
    mShareContext = EGL_NO_CONTEXT;
}

EglShareContext &EglShareContext::getInstance() {
    return instance;
}

void EglShareContext::lock() {
    pthread_mutex_lock(&mMutex);
}

void EglShareContext::unlock() {
    pthread_mutex_unlock(&mMutex);
}

