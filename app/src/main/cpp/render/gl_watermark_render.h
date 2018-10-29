//
// Created by chengjunsen on 2018/10/29.
//

#ifndef SPLAYER_GL_WATERMARK_RENDER_H
#define SPLAYER_GL_WATERMARK_RENDER_H

#include "gl_render_util.h"

class GlWaterMarkRender {
public:
    GlWaterMarkRender();
    ~GlWaterMarkRender();
    void onCreated();
    void onChangeSize(int width, int height);
    void onDestroy();
    void onDraw();
    void setWaterMark(int imgWidth, int imgHeight, void* buffer);

private:
    int loadVertexShader();
    int loadFragmentShader();
    void createVertexBufferObjects();
    void createWaterMarkTexture();

private:
    int program;
    int mFrameWidth, mFrameHeight;
    int mImgWidth, mImgHeight;
    void* mBuffer;
    bool useWaterMark;
    bool isCreatedMarkTexture;
    int mWatermarkTextureId;
    GLint vexPosition;
    GLint fragCoord;
    GLint fragTexture;
    GLuint verPosArrayBufferId;
    GLuint fragCoordArrayBufferId;
};


#endif //SPLAYER_GL_WATERMARK_RENDER_H
