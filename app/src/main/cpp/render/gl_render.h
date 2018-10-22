//
// Created by chengjunsen on 2018/10/17.
//

#ifndef SPLAYER_GL_RENDER_H
#define SPLAYER_GL_RENDER_H

#include "gl_render_util.h"

class GlRender {
public:
    GlRender();
    ~GlRender();
    void onCreated();
    void onChangeSize(int width, int height);
    void onDestroy();
    void onDraw(int textureId);

private:
    int loadVertexShader();
    int loadFragmentShader();
    void createVertexBufferObjects();

private:
    int screenWidth;
    int screenHeight;
    GLuint program;
    GLint vexPosition;
    GLint fragCoord;
    GLint fragTexture;
    GLuint verPosArrayBufferId;
    GLuint fragCoordArrayBufferId;
};


#endif //SPLAYER_GL_RENDER_H
