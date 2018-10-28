//
// Created by chengjunsen on 2018/10/16.
//

#include <android/native_window.h>
#include <GLES3/gl3.h>
#include "egl_core.h"
#include "../util/android_log.h"

EglCore::EglCore() : mEglConfig(), mEglDisplay(EGL_NO_DISPLAY) {

}

EglCore::~EglCore() {

}

EGLContext EglCore::createGL(EGLContext context) {
    int configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,      // 渲染类型
            EGL_RED_SIZE, 8,  // 指定 RGB 中的 R 大小（bits）
            EGL_GREEN_SIZE, 8, // 指定 G 大小
            EGL_BLUE_SIZE, 8,  // 指定 B 大小
            EGL_ALPHA_SIZE, 8, // 指定 Alpha 大小
            EGL_DEPTH_SIZE, 8, // 指定深度 (Z Buffer) 大小
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // 指定渲染 api 类别,
            EGL_NONE
    };
    // 设置显示设备
    if(!setDisplay(EGL_DEFAULT_DISPLAY)) {
        LOGE("core setDisplay failed");
        return EGL_NO_CONTEXT;
    }
    // 设置属性
    if(!setConfig(configAttribs)) {
        LOGE("core setconfig failed");
        return EGL_NO_CONTEXT;
    }
    // 创建上下文
    return createContext(context);
}

void EglCore::destroyGL(EGLContext context) {
    eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context != EGL_NO_CONTEXT) {
        eglDestroyContext(mEglDisplay, context);
        context = EGL_NO_CONTEXT;
    }
    eglDestroyContext(mEglDisplay, context);
    context = EGL_NO_CONTEXT;
    mEglDisplay = EGL_NO_DISPLAY;
    eglTerminate(mEglDisplay);
}

bool EglCore::setDisplay(EGLNativeDisplayType type) {
    mEglDisplay = eglGetDisplay(type);
    if (!eglInitialize(mEglDisplay, NULL, NULL)) {
        return false;
    }
    return true;
}

bool EglCore::setConfig(int *configAttribs) {
    int numConfigs;
    if (!eglChooseConfig(mEglDisplay, configAttribs, &mEglConfig, 1, &numConfigs)) {
        destroyGL(EGL_NO_CONTEXT);
        return false;
    }
    return true;
}

EGLContext EglCore::createContext(EGLContext context) {
    int contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
    };
    EGLContext shareContext = eglCreateContext(mEglDisplay, mEglConfig, context, contextAttribs);
    return shareContext;
}

EGLSurface EglCore::createWindowSurface(ANativeWindow *nativeWindow) {
    EGLSurface surface = NULL;
    EGLint format;
    if (nativeWindow == NULL) {
        LOGE("createWindowSurface nativeWindow is NULL");
        return surface;
    }
    if (!eglGetConfigAttrib(mEglDisplay, mEglConfig, EGL_NATIVE_VISUAL_ID, &format)) {
        LOGE("eglGetConfigAttrib() returned error %d", eglGetError());
        return surface;
    }
    ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format);
    if (!(surface = eglCreateWindowSurface(mEglDisplay, mEglConfig, nativeWindow, 0))) {
        LOGE("eglCreateWindowSurface() returned error %d", eglGetError());
    }
    return surface;
}

EGLSurface EglCore::createBufferSurface(int width, int height) {
    EGLSurface surface;
    EGLint PbufferAttributes[] = { EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE, EGL_NONE};
    if (!(surface = eglCreatePbufferSurface(mEglDisplay, mEglConfig, PbufferAttributes))) {
        LOGE("createBufferSurface error %d", eglGetError());
    }
    LOGE("EglCore createBufferSurface checkerror ：%d", glGetError());
    return surface;
}

void EglCore::destroySurface(EGLSurface surface) {
    if (surface != EGL_NO_CONTEXT) {
        eglDestroySurface(mEglDisplay, surface);
    }
}


bool EglCore::makeCurrent(EGLSurface surface, EGLContext context) {
    return makeCurrent(surface, surface, context);
}

bool EglCore::makeCurrent(EGLSurface draw, EGLSurface read, EGLContext context) {
    eglMakeCurrent(mEglDisplay, draw, read, context);
    //LOGE("EglCore makeCurrent checkerror ：%d", glGetError());
    return true;
}

bool EglCore::swapBuffers(EGLSurface surface) {
    return eglSwapBuffers(mEglDisplay, surface);
}
