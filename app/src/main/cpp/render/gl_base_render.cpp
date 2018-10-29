//
// Created by chengjunsen on 2018/10/17.
//

#include "gl_base_render.h"
#include "gl_shader_source.h"
#include "../util/android_log.h"

const float VertexCoordData[] = {
        -1, -1,// 左下角
        1, -1, // 右下角
        -1, 1, // 左上角
        1, 1,  // 右上角
};

const float TextureCoordData[] = {
        0, 1, // 左上角
        0, 0, //  左下角
        1, 1, // 右上角
        1, 0  // 右上角
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

GlBaseRender ::GlBaseRender () {

}

GlBaseRender ::~GlBaseRender () {

}

void GlBaseRender ::onCreated() {
    program = GlRenderUtil::createProgram(loadVertexShader(), loadFragmentShader());
    vexPosition = glGetAttribLocation(program, "position");
    fragCoord = glGetAttribLocation(program, "texcoord");
    fragTexture = glGetUniformLocation(program, "sample_texture");
    createVertexBufferObjects();
}

void GlBaseRender ::onChangeSize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

void GlBaseRender ::onDestroy() {
    glDeleteBuffers(1, &verPosArrayBufferId);
    glDeleteBuffers(1, &fragCoordArrayBufferId);
    GlRenderUtil::deleteProgram(program);
}

int GlBaseRender ::loadVertexShader() {
    int shader = GlRenderUtil::loadShader(GL_VERTEX_SHADER, GlShaderSource::VERTEX_BASE_SOURCE);
    return shader;
}

int GlBaseRender::loadFragmentShader() {
    int shader = GlRenderUtil::loadShader(GL_FRAGMENT_SHADER, GlShaderSource::FRAGMENT_BASE_SOURCE);
    return shader;
}

void GlBaseRender ::onDraw(int textureId) {
    GlRenderUtil::useProgram(program);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnableVertexAttribArray(vexPosition);
    glEnableVertexAttribArray(fragCoord);

    glBindBuffer(GL_ARRAY_BUFFER, verPosArrayBufferId);
    glVertexAttribPointer(vexPosition, CoordsPerVertexCount, GL_FLOAT, false, 0, 0);
    // 用 GPU 中的缓冲数据，不再 RAM 中取数据，所以后 2 个参数为 0
    glBindBuffer(GL_ARRAY_BUFFER, fragCoordArrayBufferId);
    glVertexAttribPointer(fragCoord, CoordsPerTextureCount, GL_FLOAT, false, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(fragTexture, 4);
    // 绘制 GLES30.GL_TRIANGLE_STRIP: 复用坐标
    glDrawArrays(GL_TRIANGLE_STRIP, 0, VertexCount);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(vexPosition);
    glDisableVertexAttribArray(fragCoord);
}

void GlBaseRender ::createVertexBufferObjects() {
    GLuint * vbo = new GLuint[2];
    glGenBuffers(2, vbo);

    verPosArrayBufferId = vbo[0];
    // ARRAY_BUFFER 将使用 Float*Array 而 ELEMENT_ARRAY_BUFFER 必须使用 Uint*Array
    glBindBuffer(GL_ARRAY_BUFFER, verPosArrayBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexCoordData), VertexCoordData, GL_STATIC_DRAW);

    fragCoordArrayBufferId = vbo[1];
    glBindBuffer(GL_ARRAY_BUFFER, fragCoordArrayBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FrameCoordData), FrameCoordData, GL_STATIC_DRAW);
}

