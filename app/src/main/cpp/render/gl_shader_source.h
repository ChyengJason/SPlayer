//
// Created by chengjunsen on 2018/10/17.
//

#ifndef SPLAYER_GL_SHADER_SOURCE_H
#define SPLAYER_GL_SHADER_SOURCE_H

namespace GlShaderSource {

    const char * VERTEX_BASE_SOURCE = ""
            "attribute vec4 position;   "
            "attribute vec2 texcoord;   "
            "varying vec2 v_texcoord;   "
            "void main() {"
            "  gl_Position = position;  "
            "  v_texcoord = texcoord;   "
            "} ";

    const char * FRAGMENT_BASE_SOURCE = ""
            "precision mediump float;   "
            "varing vec2 v_texcoord;    "
            "uniform sampler2D sample_texture"
            "void main() {"
            "  gl_FragColor = texture2D(sample_texture, v_texcoord); "
            "} ";

};
#endif //SPLAYER_GL_SHADER_SOURCE_H
