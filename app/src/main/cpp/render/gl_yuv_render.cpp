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
}

void GlYuvRender::onChangeSize(int width, int height) {
    LOGE("GlYuvRender::onChangeSize %d x %d", width, height);
    this->frameWidth = width;
    this->frameHeight = height;
}

void GlYuvRender::onDestroy() {
    freeTextures();
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
    LOGE("GlYuvRender::onDraw");
    GlRenderUtil::useProgram(program);
    glViewport(0, 0, frameWidth, frameHeight);
    glClearColor(1.0f, 1.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnableVertexAttribArray(vexPositionHandle);
    glEnableVertexAttribArray(fragCoordHandle);

    glVertexAttribPointer(vexPositionHandle, CoordsPerVertexCount, GL_FLOAT, false, 0, VertexCoordData);
    // 用 GPU 中的缓冲数据，不再 RAM 中取数据，所以后 2 个参数为 0
    glVertexAttribPointer(fragCoordHandle, CoordsPerTextureCount, GL_FLOAT, false, 0, TextureCoordData);

    //绑定纹理
//    bindTexture(GL_TEXTURE0, textureY, videoFrame->frameWidth, videoFrame->frameHeight, videoFrame->luma);
//    glUniform1i(textureYHandle, 0); //对应纹理第2层
//    bindTexture(GL_TEXTURE1, textureU, videoFrame->frameWidth / 2, videoFrame->frameHeight / 2, videoFrame->chromaB);
//    glUniform1i(textureUHandle, 1); //对应纹理第2层
//    bindTexture(GL_TEXTURE2, textureV, videoFrame->frameWidth / 2, videoFrame->frameHeight / 2, videoFrame->chromaR);
//    glUniform1i(textureVHandle, 2); //对应纹理第3层

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
    LOGE("GlYuvRender::onDraw finish");
}

void GlYuvRender::bindTexture(int glTexture, int textureHandle, int width, int height, void *buffer) {
    LOGE("bindTexture %d x %d", width, height);
    glActiveTexture(glTexture);
    if (width % 16 != 0) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
}

void GlYuvRender::freeTextures() {
    GlRenderUtil::deleteTexture(textureY);
    GlRenderUtil::deleteTexture(textureU);
    GlRenderUtil::deleteTexture(textureV);
}
