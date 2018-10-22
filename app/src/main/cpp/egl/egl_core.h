//
// Created by chengjunsen on 2018/10/16.
//

#ifndef SPLAYER_EGL_CORE_H
#define SPLAYER_EGL_CORE_H

#include <EGL/egl.h>
#include <EGL/eglext.h>

class EglCore {
public:
    EglCore();
    ~EglCore();
    EGLContext createGL(EGLContext context);
    void destroyGL(EGLContext context);
    void destroySurface(EGLSurface surface);
    EGLSurface createWindowSurface(ANativeWindow* nativeWindow);
    EGLSurface createBufferSurface(int width, int height);
    bool makeCurrent(EGLSurface draw, EGLSurface read, EGLContext context);
    bool makeCurrent(EGLSurface surface, EGLContext context);
    bool swapBuffers(EGLSurface surface);
private:
    EGLContext createContext(EGLContext context);
    bool setDisplay(EGLNativeDisplayType type);
    bool setConfig(int configs[]);
    EGLConfig mEglConfig;
    EGLDisplay mEglDisplay;
};


#endif //SPLAYER_EGL_CORE_H
