//
// Created by chengjunsen on 2018/10/23.
//

#ifndef SPLAYER_GL_YUV_RENDER_H
#define SPLAYER_GL_YUV_RENDER_H
#include "gl_render_util.h"
#include "../media_frame.h"

class GlYuvRender {
public:
    GlYuvRender();
    ~GlYuvRender();
    void onCreated();
    void onChangeSize(int width, int height);
    void onDestroy();
    void onDraw(const VideoFrame* videoFrame);

private:
    int loadVertexShader();
    int loadFragmentShader();
    void createVertexBufferObjects();
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
    int textureY;
    int textureU;
    int textureV;
    GLuint verPosArrayBufferId;
    GLuint fragCoordArrayBufferId;
};
#endif //SPLAYER_GL_YUV_RENDER_H
