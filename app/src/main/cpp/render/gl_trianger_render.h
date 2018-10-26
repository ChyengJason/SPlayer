//
// Created by 橙俊森 on 2018/10/25.
//

#ifndef SPLAYER_GL_TRIANGER_RENDER_H
#define SPLAYER_GL_TRIANGER_RENDER_H
#include "gl_render_util.h"

class GlTriangerRender {
public:
  GlTriangerRender();
  ~GlTriangerRender();
  void onCreated();
  void onChangeSize(int width, int height);
  void onDestroy();
  void onDraw();

private:
    int loadVertexShader();
    int loadFragmentShader();

private:
    int program;
    GLuint vexPositionHandle;
};

#endif //SPLAYER_GL_TRIANGER_RENDER_H