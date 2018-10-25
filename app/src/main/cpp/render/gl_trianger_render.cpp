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

const float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

GlTriangerRender::GlTriangerRender() {

}

GlTriangerRender::~GlTriangerRender() {

}

void GlTriangerRender::onCreated() {
    LOGE("GlYuvRender onCreated");
    program = GlRenderUtil::createProgram(loadVertexShader(), loadFragmentShader());
    vexPositionHandle = glGetAttribLocation(program, "vPosition");
    colorHandle = glGetUniformLocation(program, "vColor");
    LOGE("program: %d", program);
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
    glViewport(0,0,width,height);
}

void GlTriangerRender::onDestroy() {

}

void GlTriangerRender::onDraw() {
    //将程序加入到OpenGLES2.0环境
    glUseProgram(program);
    //启用三角形顶点的句柄
    glEnableVertexAttribArray(vexPositionHandle);
    //准备三角形的坐标数据
    glVertexAttribPointer(vexPositionHandle, 2, GL_FLOAT, false, 0, triangleCoords);
    //获取片元着色器的vColor成员的句柄
    //设置绘制三角形的颜色
    glUniform4fv(colorHandle, 4, color);
    //绘制三角形
    glDrawArrays(GL_TRIANGLES, 0, 4);
    //禁止顶点数组的句柄
    glDisableVertexAttribArray(vexPositionHandle);
}
