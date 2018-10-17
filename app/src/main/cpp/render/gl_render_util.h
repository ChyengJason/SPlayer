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

    int createProgram(int vertexShader, int fragmentShader);

    void useProgram(int program);

    void deleteProgram(int program);

    int loadShader(GLenum shaderType, const char* shaderSource);

    int createTexture(int width, int height);

    int createExternalTexture();

    void checkError(const char* tip);

    int createPixelsBuffer();

    int createFrameBuffer();

    int createRenderBuffer();

    void bindFrameTexture(int frameBufferId, int textureId);

    void bindFrameRender(int frameBufferId, int renderId, int width, int height);
};


#endif //SPLAYER_GL_RENDER_UTIL_H
