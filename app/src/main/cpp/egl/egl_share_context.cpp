//
// Created by chengjunsen on 2018/10/22.
//

#include <EGL/egl.h>
#include "egl_share_context.h"
EglShareContext::mShareContext = EGL_NO_CONTEXT;

EGLContext EglShareContext::getShareContext() {
    return mShareContext;
}

void EglShareContext::setShareContext(EGLContext context) {
    mShareContext = EGL_NO_CONTEXT;
}
