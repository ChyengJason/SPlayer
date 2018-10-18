//
// Created by chengjunsen on 2018/10/17.
//

#ifndef SPLAYER_GL_SHADER_SOURCE_H
#define SPLAYER_GL_SHADER_SOURCE_H

namespace GlShaderSource {

    const char * VERTEX_BASE_SOURCE = ""
            "attribute vec4 position;   \n"
            "attribute vec2 texcoord;   \n"
            "varying vec2 v_texcoord;   \n"
            "void main() {          \n"
            "  v_texcoord = texcoord;   \n"
            "  gl_Position = position;  \n"
            "} ";

    const char * FRAGMENT_BASE_SOURCE = ""
            "precision mediump float;   \n"
            "varying vec2 v_texcoord;    \n"
            "uniform sampler2D sample_texture; \n"
            "void main() {          \n"
            "  gl_FragColor = texture2D(sample_texture, v_texcoord); \n"
            "} ";

};
#endif //SPLAYER_GL_SHADER_SOURCE_H
