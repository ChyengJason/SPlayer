//
// Created by chengjunsen on 2018/10/29.
//

#include "gl_watermark_render.h"
#include "../util/android_log.h"
#include "gl_shader_source.h"

const float VertexCoordData[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f,
};

const float TextureCoordData[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
};

GlWaterMarkRender::GlWaterMarkRender() {
    useWaterMark = false;
    isCreatedMarkTexture = false;
    mWatermarkTextureId = -1;
}

GlWaterMarkRender::~GlWaterMarkRender() {
    if (useWaterMark) {
        delete mBuffer;
        mBuffer = NULL;
    }
}

void GlWaterMarkRender::onCreated() {
    program = GlRenderUtil::createProgram(loadVertexShader(), loadFragmentShader());
    GlRenderUtil::useProgram(program);
    createVertexBufferObjects();
}

void GlWaterMarkRender::onChangeSize(int width, int height) {
    mFrameWidth = width;
    mFrameHeight = height;
}

void GlWaterMarkRender::onDestroy() {
    GlRenderUtil::deleteTexture(mWatermarkTextureId);
    isCreatedMarkTexture = false;
    mWatermarkTextureId = -1;
    glDeleteBuffers(1, &verPosArrayBufferId);
    glDeleteBuffers(1, &fragCoordArrayBufferId);
    GlRenderUtil::deleteProgram(program);
}

void GlWaterMarkRender::onDraw() {
    if (!useWaterMark) {
        return;
    }
    if (!isCreatedMarkTexture) {
        createWaterMarkTexture();
    }
    GlRenderUtil::useProgram(program);
    glViewport(0, 0, mFrameWidth, mFrameHeight);
    glEnableVertexAttribArray(verPosArrayBufferId);
    glEnableVertexAttribArray(fragCoordArrayBufferId);
}

void GlWaterMarkRender::createVertexBufferObjects() {
    LOGD("GlWaterMarkRender::createVertexBufferObjects");
    GLuint * vbo = new GLuint[2];
    glGenBuffers(2, vbo);

    verPosArrayBufferId = vbo[0];
    // ARRAY_BUFFER 将使用 Float*Array 而 ELEMENT_ARRAY_BUFFER 必须使用 Uint*Array
    glBindBuffer(GL_ARRAY_BUFFER, verPosArrayBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexCoordData), VertexCoordData, GL_STATIC_DRAW);

    fragCoordArrayBufferId = vbo[1];
    glBindBuffer(GL_ARRAY_BUFFER, fragCoordArrayBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TextureCoordData), TextureCoordData, GL_STATIC_DRAW);
}

void GlWaterMarkRender::createWaterMarkTexture() {
    if (!useWaterMark || isCreatedMarkTexture || mBuffer == NULL)
        return;
    if (mWatermarkTextureId < 0) {
        GlRenderUtil::deleteTexture(mWatermarkTextureId);
        mWatermarkTextureId = -1;
    }
    LOGE("GlWaterMarkRender::createWaterMarkTexture");
    isCreatedMarkTexture = true;
    mWatermarkTextureId = GlRenderUtil::createTexture(mImgWidth, mImgHeight, mBuffer);
    LOGE("GlWaterMarkRender::createWaterMarkTexture finish");
}

int GlWaterMarkRender::loadVertexShader() {
    int shader = GlRenderUtil::loadShader(GL_VERTEX_SHADER, GlShaderSource::VERTEX_BASE_SOURCE);
    return shader;
}

int GlWaterMarkRender::loadFragmentShader() {
    int shader = GlRenderUtil::loadShader(GL_FRAGMENT_SHADER, GlShaderSource::FRAGMENT_BASE_SOURCE);
    return shader;
}

void GlWaterMarkRender::setWaterMark(int imgWidth, int imgHeight, void *buffer) {
    mImgWidth = imgWidth;
    mImgHeight = imgHeight;
    mBuffer = buffer;
    useWaterMark = true;
    isCreatedMarkTexture = false;
}
