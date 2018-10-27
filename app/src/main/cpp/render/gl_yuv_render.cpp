//
// Created by chengjunsen on 2018/10/23.
//

#include "gl_yuv_render.h"
#include "gl_shader_source.h"
#include "../android_log.h"

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

const float FrameCoordData[] = {
        0, 0,
        1, 0,
        0, 1,
        1, 1
};

const int CoordsPerVertexCount = 2;

const int CoordsPerTextureCount = 2;

const int VertexCount = 4;

GlYuvRender::GlYuvRender() {

}

GlYuvRender::~GlYuvRender() {

}

void GlYuvRender::onCreated() {
    LOGE("GlYuvRender onCreated");
    program = GlRenderUtil::createProgram(loadVertexShader(), loadFragmentShader());
    GlRenderUtil::useProgram(program);
    vexPositionHandle = glGetAttribLocation(program, "position");
    fragCoordHandle = glGetAttribLocation(program, "texcoord");
    textureYHandle = glGetUniformLocation(program, "texture_y");
    textureUHandle = glGetUniformLocation(program, "texture_u");
    textureVHandle = glGetUniformLocation(program, "texture_v");
    createVertexBufferObjects();
    textureY = GlRenderUtil::createTexture();
    textureU = GlRenderUtil::createTexture();
    textureV = GlRenderUtil::createTexture();
    LOGE("program: %d", program);
    LOGE("textureY: %d", textureY);
    LOGE("textureU: %d", textureU);
    LOGE("textureV: %d", textureV);
    LOGE("textureYHandle: %d", textureYHandle);
    LOGE("textureUHandle: %d", textureUHandle);
    LOGE("textureVHandle: %d", textureVHandle);
    LOGE("verPosArrayBufferId: %d", verPosArrayBufferId);
}

void GlYuvRender::onChangeSize(int width, int height) {
    LOGE("GlYuvRender::onChangeSize");
    this->frameWidth = width;
    this->frameHeight = height;
}

void GlYuvRender::onDestroy() {
    freeTextures();
    glDeleteBuffers(1, &verPosArrayBufferId);
    glDeleteBuffers(1, &fragCoordArrayBufferId);
    GlRenderUtil::deleteProgram(program);
}

int GlYuvRender::loadVertexShader() {
    int shader = GlRenderUtil::loadShader(GL_VERTEX_SHADER, GlShaderSource::VERTEX_YUV_SOURCE);
    return shader;
}

int GlYuvRender::loadFragmentShader() {
    int shader = GlRenderUtil::loadShader(GL_FRAGMENT_SHADER, GlShaderSource::FRAGMENT_YUV_SOURCE);
    return shader;
}

void GlYuvRender::onDraw(const VideoFrame* videoFrame) {
    GlRenderUtil::useProgram(program);
    glViewport(0, 0, frameWidth, frameHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnableVertexAttribArray(vexPositionHandle);
    glEnableVertexAttribArray(fragCoordHandle);

    glBindBuffer(GL_ARRAY_BUFFER, verPosArrayBufferId);
    glVertexAttribPointer(vexPositionHandle, CoordsPerVertexCount, GL_FLOAT, false, 0, 0);
    // 用 GPU 中的缓冲数据，不再 RAM 中取数据，所以后 2 个参数为 0
    glBindBuffer(GL_ARRAY_BUFFER, fragCoordArrayBufferId);
    glVertexAttribPointer(fragCoordHandle, CoordsPerTextureCount, GL_FLOAT, false, 0, 0);

    //绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->frameWidth, videoFrame->frameHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, videoFrame->luma);
    glUniform1i(textureYHandle, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->frameWidth/2, videoFrame->frameHeight/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, videoFrame->chromaB);
    glUniform1i(textureUHandle, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->frameWidth/2, videoFrame->frameHeight/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, videoFrame->chromaR);
    glUniform1i(textureVHandle, 2);

    // 绘制 GLES30.GL_TRIANGLE_STRIP: 复用坐标
    glDrawArrays(GL_TRIANGLE_STRIP, 0, VertexCount);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(vexPositionHandle);
    glDisableVertexAttribArray(fragCoordHandle);
}

void GlYuvRender::createVertexBufferObjects() {
    LOGE("GlYuvRender::createVertexBufferObjects");
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

void GlYuvRender::freeTextures() {
    GlRenderUtil::deleteTexture(textureY);
    GlRenderUtil::deleteTexture(textureU);
    GlRenderUtil::deleteTexture(textureV);
}
