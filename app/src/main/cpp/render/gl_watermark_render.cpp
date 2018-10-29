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
        0, 1, // 左上角
        0, 0, //  左下角
        1, 1, // 右上角
        1, 0  // 右上角
};

const int CoordsPerVertexCount = 2;

const int CoordsPerTextureCount = 2;

const int VertexCount = 4;

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
    vexPosition = glGetAttribLocation(program, "position");
    fragCoord = glGetAttribLocation(program, "texcoord");
    fragTexture = glGetUniformLocation(program, "sample_texture");
}

void GlWaterMarkRender::onChangeSize(int width, int height) {
    mFrameWidth = width;
    mFrameHeight = height;
}

void GlWaterMarkRender::onDestroy() {
    GlRenderUtil::useProgram(program);
    if (isCreatedMarkTexture && mWatermarkTextureId >= 0) {
        GlRenderUtil::deleteTexture(mWatermarkTextureId);
        isCreatedMarkTexture = false;
        mWatermarkTextureId = -1;
    }
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
    LOGE("GlWaterMarkRender::onDraw");
    GlRenderUtil::useProgram(program);
//    glClear(GL_DEPTH_BUFFER_BIT |GL_COLOR_BUFFER_BIT);
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); //指定混合模式
    glViewport(0, 0, mFrameWidth, mFrameHeight);
    glEnableVertexAttribArray(verPosArrayBufferId);
    glEnableVertexAttribArray(fragCoordArrayBufferId);

    glBindBuffer(GL_ARRAY_BUFFER, verPosArrayBufferId);
    glVertexAttribPointer(vexPosition, CoordsPerVertexCount, GL_FLOAT, false, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, fragCoordArrayBufferId);
    glVertexAttribPointer(fragCoord, CoordsPerTextureCount, GL_FLOAT, false, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mWatermarkTextureId);
    glUniform1i(fragTexture, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, VertexCount);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(vexPosition);
    glDisableVertexAttribArray(fragCoord);
    glDisable(GL_BLEND);
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
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    LOGE("GlWaterMarkRender::createWaterMarkTexture %dx%d", mImgWidth, mImgHeight);
    isCreatedMarkTexture = true;
    mWatermarkTextureId = GlRenderUtil::createBitmapTexture(mImgWidth, mImgHeight, mBuffer);
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
