//
// Created by chengjunsen on 2018/10/17.
//

#include "gl_render_util.h"
#include "../util/android_log.h"

GLuint GlRenderUtil::createProgram(int vertexShader, int fragmentShader) {
    int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    checkError("AttachVertexShader");
    glAttachShader(program, fragmentShader);
    checkError("AttachVertexShader");
    LOGE("GlRenderUtil::glLinkProgram1 %d %d, %d", program, vertexShader, fragmentShader);
    glLinkProgram(program);
    LOGE("GlRenderUtil::glLinkProgram2 %d %d, %d", program, vertexShader, fragmentShader);
    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        int len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        char* log = new char[len];
        glGetProgramInfoLog(program, len, NULL, log);
        LOGE("createProgam: link error");
        LOGE("createProgam: %s", log);
        glDeleteProgram(program);
        delete[] log;
        return -1;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    LOGE("GlRenderUtil::createProgram %d %d, %d", program, vertexShader, fragmentShader);
    return program;
}

void GlRenderUtil::useProgram(int program) {
    glUseProgram(program);
}

void GlRenderUtil::deleteProgram(int program) {
    glDeleteProgram(program);
}

GLuint GlRenderUtil::loadShader(GLenum shaderType, const char* shaderSource) {
    int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        int len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        char* log = new char[len];
        glGetShaderInfoLog(shader, len, NULL, log);
        LOGE("createProgam: link error");
        LOGE("createProgam error: %s", log);
        glDeleteShader(shader);
        delete[] log;
        return -1;
    }
    return shader;
}

int GlRenderUtil::createTexture() {
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (GlRenderUtil::checkError("glTexParameter")) {
        return -1;
    }
    return textureId;
}

int GlRenderUtil::createTexture(int width, int height) {
    if (width <= 0 || height <= 0 ) {
        LOGE("cretaeTexture width or height <= 0");
        return -1;
    }
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // 设置环绕方向 S，截取纹理坐标到 [1/2n,1-1/2n]。将导致永远不会与 border 融合
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // 设置环绕方向 T，截取纹理坐标到 [1/2n,1-1/2n]。将导致永远不会与 border 融合
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // 设置缩小过滤为使用纹理中坐标最接近的一个像素的颜色作为需要绘制的像素颜色
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // 设置放大过滤为使用纹理中坐标最接近的若干个颜色，通过加权平均算法得到需要绘制的像素颜色
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    checkError("createTexture");
    return texture;
}

int GlRenderUtil::createTexture(int width, int height, void* buffer) {
    if (width <= 0 || height <= 0 ) {
        LOGE("cretaeTexture width or height <= 0");
        return -1;
    }
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    if (GlRenderUtil::checkError("glTexParameter")) {
        return -1;
    }
    return textureId;
}

int GlRenderUtil::createExternalTexture() {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    if (GlRenderUtil::checkError("createExternalTexture")) {
        return -1;
    }
    return texture;
}

bool GlRenderUtil::checkError(const char *tip) {
    if (glGetError() != GL_NO_ERROR) {
        LOGE("GlRenderUtil checkError %s %d", tip, glGetError());
        return true;
    }
    return false;
}

GLuint GlRenderUtil::createPixelsBuffer() {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    checkError("createPixelsBuffer");
    return buffer;
}

GLuint GlRenderUtil::createFrameBuffer() {
    GLuint buffer;
    glGenFramebuffers(1, &buffer);
    checkError("createFrameBuffer");
    return buffer;
}

GLuint GlRenderUtil::createRenderBuffer() {
    GLuint buffer;
    glGenRenderbuffers(1, &buffer);
    checkError("createRenderBuffer");
    return buffer;
}

void GlRenderUtil::bindFrameTexture(int frameBufferId, int textureId) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
    checkError("bindFrameTexture");
}

void GlRenderUtil::bindFrameRender(int frameBufferId, int renderId, int width, int height) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, renderId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderId);
}

void GlRenderUtil::deleteFrameBuffer(GLuint fbo) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
}

void GlRenderUtil::deleteTexture(GLuint texture) {
    glDeleteTextures(1, &texture);
}

void GlRenderUtil::unBindFrameTexture() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
