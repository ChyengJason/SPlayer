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
    void createGL();
    void createGL(const EGLContext& shareContext);
    void destroyGL();
    void destroySurface(EGLSurface surface);
    EGLSurface  createWindowSurface(ANativeWindow* nativeWindow);
    bool makeCurrent(EGLSurface draw, EGLSurface read);
    bool makeCurrent(EGLSurface surface);
    bool swapBuffers(EGLSurface surface);
    EGLContext getShareContext();
private:
    bool setDisplay(EGLNativeDisplayType type);
    bool setConfig(int configs[]);
    bool createContext(EGLContext context);

    EGLConfig mEglConfig;
    EGLDisplay mEglDisplay;
    EGLContext mShareEglContext;
};


#endif //SPLAYER_EGL_CORE_H
