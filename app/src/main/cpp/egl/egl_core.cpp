//
// Created by chengjunsen on 2018/10/16.
//

#include <android/native_window.h>
#include "egl_core.h"
#include "../android_log.h"

EglCore::EglCore()
        : mEglConfig()
        , mEglDisplay(EGL_NO_DISPLAY)
        , mShareEglContext(EGL_NO_CONTEXT){

}

EglCore::~EglCore() {

}

void EglCore::createGL() {
    createGL(mShareEglContext);
}

void EglCore::createGL(const EGLContext &shareContext) {
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
    setDisplay(EGL_DEFAULT_DISPLAY);
    // 设置属性
    setConfig(configAttribs);
    // 创建上下文
    createContext(shareContext);
}

void EglCore::destroyGL() {
    eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (mShareEglContext) {
        eglDestroyContext(mEglDisplay, mShareEglContext);
        mShareEglContext = EGL_NO_CONTEXT;
    }
    eglDestroyContext(mEglDisplay, mShareEglContext);
    mShareEglContext = EGL_NO_CONTEXT;
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
        destroyGL();
        return false;
    }
    return true;
}

bool EglCore::createContext(EGLContext context) {
    int contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
    };
    mShareEglContext = eglCreateContext(mEglDisplay, mEglConfig, context, contextAttribs);
    if (mShareEglContext == EGL_NO_CONTEXT) {
        return false;
    }
    return true;
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

void EglCore::destroySurface(EGLSurface surface) {
    eglDestroySurface(mEglDisplay, surface);
}


bool EglCore::makeCurrent(EGLSurface surface) {
    return makeCurrent(surface, surface);
}

bool EglCore::makeCurrent(EGLSurface draw, EGLSurface read) {
    eglMakeCurrent(mEglDisplay, draw, read, mShareEglContext);
    return false;
}