//
// Created by chengjunsen on 2018/10/17.
//

#ifndef SPLAYER_GL_SHADER_SOURCE_H_
#define SPLAYER_GL_SHADER_SOURCE_H_
#define GET_STR(x) #x

namespace GlShaderSource {

    const static char * VERTEX_BASE_SOURCE = GET_STR(
            attribute vec4 position;
            attribute vec2 texcoord;
            varying vec2 v_texcoord;
            void main() {
                v_texcoord = texcoord;
                gl_Position = position;
            }
    );


    const static char * FRAGMENT_BASE_SOURCE = GET_STR(
            precision mediump float;
            varying vec2 v_texcoord;
            uniform sampler2D sample_texture;
            void main() {
                gl_FragColor = texture2D(sample_texture, v_texcoord);
            }
    );

    //https://blog.csdn.net/lidec/article/details/73732369
    const static char * VERTEX_YUV_SOURCE = GET_STR(
            attribute vec4 position;
            attribute vec2 texcoord;
            varying vec2 v_texcoord;
            void main() {
                gl_Position = position;
                v_texcoord = texcoord;
            }
    );

    const static char * FRAGMENT_YUV_SOURCE = GET_STR(
            precision mediump float;
            varying vec2 v_texcoord;
            uniform sampler2D texture_y;
            uniform sampler2D texture_u;
            uniform sampler2D texture_v;
            void main() {
                vec3 yuv;
                vec3 rgb;
                yuv.x = texture2D(texture_y, v_texcoord).r;
                yuv.y = texture2D(texture_u, v_texcoord).r - 0.5;
                yuv.z = texture2D(texture_v, v_texcoord).r - 0.5;
                rgb = mat3( 1,       1,         1,
                            0,       -0.39465,  2.03211,
                            1.13983, -0.58060,  0) * yuv;
                gl_FragColor = vec4(rgb, 1);
            }
    );

};
#endif //SPLAYER_GL_SHADER_SOURCE_H_
