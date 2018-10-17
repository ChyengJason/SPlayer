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
    void prepare(int width, int height);
    void changeSize(int widht, int height);
    void destroy();
    void draw(int textureId);

private:
    int loadVertexShader();
    int loadFragmentShader();
    void createVertexBufferObjects();

private:
    int screenWidth;
    int screenHeight;
    int program;
    int vexPosition;
    GLuint fragCoord;
    GLuint fragTexture;
    GLuint verPosArrayBufferId;
    GLuint fragCoordArrayBufferId;
};


#endif //SPLAYER_GL_RENDER_H
