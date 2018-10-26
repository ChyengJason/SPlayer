//
// Created by 橙俊森 on 2018/10/25.
//
#include "gl_trianger_render.h"
#include "../android_log.h"
#include "gl_shader_source.h"

const float triangleCoords[] = {
        0.5f,  0.5f, // top
        -0.5f, -0.5f, // bottom left
        0.5f, -0.5f// bottom right
};

GlTriangerRender::GlTriangerRender() {

}

GlTriangerRender::~GlTriangerRender() {

}

void GlTriangerRender::onCreated() {
    LOGE("GlTriangerRender onCreated");
    glClearColor(0.0f, 0.0f, 0.0f, 0.3f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    program = GlRenderUtil::createProgram(loadVertexShader(), loadFragmentShader());
}

int GlTriangerRender::loadVertexShader() {
    int shader = GlRenderUtil::loadShader(GL_VERTEX_SHADER, GlShaderSource::TRIANGER_VERTEX_SOURCE);
    return shader;
}

int GlTriangerRender::loadFragmentShader() {
    int shader = GlRenderUtil::loadShader(GL_FRAGMENT_SHADER, GlShaderSource::TRIANGER_FRAGMENT_SOURCE);
    return shader;
}

void GlTriangerRender::onChangeSize(int width, int height) {
    glViewport(0, 0, width, height);
}

void GlTriangerRender::onDestroy() {

}

void GlTriangerRender::onDraw() {
    LOGE("GlTriangerRender onDraw");
    GlRenderUtil::useProgram(program);
    LOGE("program: %d", program);
    vexPositionHandle = glGetAttribLocation(program, "vPosition");
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //启用三角形顶点的句柄
    glEnableVertexAttribArray(vexPositionHandle);
    //准备三角形的坐标数据
    glVertexAttribPointer(vexPositionHandle, 2, GL_FLOAT, false, 0, triangleCoords);
    //获取片元着色器的vColor成员的句柄
    //绘制三角形
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
    //禁止顶点数组的句柄
    glDisableVertexAttribArray(vexPositionHandle);
}
