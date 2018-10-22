//
// Created by chengjunsen on 2018/10/22.
//
#ifndef SPLAYER_EGL_SHARE_CONTEXT_H
#define SPLAYER_EGL_SHARE_CONTEXT_H

#include <EGL/egl.h>

class EglShareContext{
public:
    static EGLContext getShareContext();
    static void setShareContext(EGLContext context);

private:
    static EGLContext mShareContext;
};

#endif

