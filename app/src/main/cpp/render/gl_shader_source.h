//
// Created by chengjunsen on 2018/10/17.
//

#ifndef SPLAYER_GL_SHADER_SOURCE_H_
#define SPLAYER_GL_SHADER_SOURCE_H_

namespace GlShaderSource {

    const static char * VERTEX_BASE_SOURCE = ""
            "attribute vec4 position;   \n"
            "attribute vec2 texcoord;   \n"
            "varying vec2 v_texcoord;   \n"
            "void main() {          \n"
            "  v_texcoord = texcoord;   \n"
            "  gl_Position = position;  \n"
            "} ";

    const static char * FRAGMENT_BASE_SOURCE = ""
            "precision mediump float;   \n"
            "varying vec2 v_texcoord;    \n"
            "uniform sampler2D sample_texture; \n"
            "void main() {          \n"
            "  gl_FragColor = texture2D(sample_texture, v_texcoord); \n"
            "} ";

    //https://blog.csdn.net/lidec/article/details/73732369
    const static char * VERTEX_YUV_SOURC = VERTEX_BASE_SOURCE;

    const static char * FRAGMENT_YUV_SOURCE = ""
            "precision mediump float;       \n"
            "varying vec2 v_texcoord;       \n"
            "uniform sampler2D texture_y;   \n"
            "uniform sampler2D texture_u;   \n"
            "uniform sampler2D texture_v;   \n"
            "void main() {                  \n"
            "   vec3 yuv;                   \n"
            "   vec3 rgb;                   \n"
            "   yuv.x = texture2D(texture_y, v_texcoord).r;       \n"
            "   yuv.y = texture2D(texture_u, v_texcoord).r - 0.5; \n"
            "   yuv.z = texture2D(texture_v, v_texcoord).r - 0.5; \n"
            "   rgb = mat3 ( 1,       1,        1,           \n"
            "               0,       -0.39465, 2.03211,      \n"
            "               1.13983, -0.58060, 0 ) * yuv;    \n"
            "   gl_FragColor = vec4(rgb, 1); \n"
            "}  ";

};
#endif //SPLAYER_GL_SHADER_SOURCE_H_
