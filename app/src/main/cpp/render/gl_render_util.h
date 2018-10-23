//
// Created by chengjunsen on 2018/10/17.
//

#ifndef SPLAYER_GL_RENDER_UTIL_H
#define SPLAYER_GL_RENDER_UTIL_H

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>
#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>

namespace GlRenderUtil {

    GLuint createProgram(int vertexShader, int fragmentShader);

    void useProgram(int program);

    void deleteProgram(int program);

    GLuint loadShader(GLenum shaderType, const char* shaderSource);

    GLuint createTexture(int width, int height);

    void deleteTexture(GLuint texture);

    GLuint createExternalTexture();

    void checkError(const char* tip);

    GLuint createPixelsBuffer();

    GLuint createFrameBuffer();

    GLuint createRenderBuffer();

    void deleteFrameBuffer(GLuint fbo);

    void bindFrameTexture(int frameBufferId, int textureId);

    void unBindFrameTexture();

    void bindFrameRender(int frameBufferId, int renderId, int width, int height);
};


#endif //SPLAYER_GL_RENDER_UTIL_H
