//
// Created by chengjunsen on 2018/10/23.
//

#ifndef SPLAYER_GL_YUV_RENDER_H
#define SPLAYER_GL_YUV_RENDER_H
#include "gl_render_util.h"

class GlYuvRender {
public:
    GlYuvRender();
    ~GlYuvRender();
    void onCreated();
    void onChangeSize(int width, int height);
    void onDestroy();
    void onDraw(void *luma, void *chromaB, void *chromaR);

private:
    int loadVertexShader();
    int loadFragmentShader();
    void createVertexBufferObjects();
    GLuint createTextures();
    void bindTexture(int glTexture, int textureHandle, int width, int height, void*buffer);
    void freeTextures();

private:
    int frameWidth;
    int frameHeight;
    GLuint program;
    GLint vexPositionHandle;
    GLint fragCoordHandle;
    GLint textureYHandle;
    GLint textureUHandle;
    GLint textureVHandle;
    GLuint textureY;
    GLuint textureU;
    GLuint textureV;
    GLuint verPosArrayBufferId;
    GLuint fragCoordArrayBufferId;
};
#endif //SPLAYER_GL_YUV_RENDER_H