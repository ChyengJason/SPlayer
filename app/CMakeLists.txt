cmake_minimum_required(VERSION 3.4.1)

add_library(avcodec-56 SHARED IMPORTED)
set_target_properties(avcodec-56 PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/libs/armeabi/libavcodec-56.so)

add_library(avdevice-56 SHARED IMPORTED)
set_target_properties(avdevice-56 PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/libs/armeabi/libavdevice-56.so)

add_library(avfilter-5 SHARED IMPORTED)
set_target_properties(avfilter-5 PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/libs/armeabi/libavfilter-5.so)

add_library(avformat-56 SHARED IMPORTED)
set_target_properties(avformat-56 PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/libs/armeabi/libavformat-56.so)

add_library(avutil-54 SHARED IMPORTED)
set_target_properties(avutil-54 PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/libs/armeabi/libavutil-54.so)

add_library(postproc-53 SHARED IMPORTED)
set_target_properties(postproc-53 PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/libs/armeabi/libpostproc-53.so)

add_library(swresample-1 SHARED IMPORTED)
set_target_properties(swresample-1 PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/libs/armeabi/libswresample-1.so)

add_library(swscale-3 SHARED IMPORTED)
set_target_properties(swscale-3 PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/libs/armeabi/libswscale-3.so)

add_library(video-player-lib SHARED
            src/main/cpp/media_decoder.cpp
            src/main/cpp/media_player_controller.cpp
            src/main/cpp/synchronizer/media_synchronizer.cpp
            src/main/cpp/synchronizer/video_queue.cpp
            src/main/cpp/synchronizer/audio_queue.cpp
            src/main/cpp/video_output.cpp
            src/main/cpp/audio_output.cpp
            src/main/cpp/egl/egl_core.cpp
            src/main/cpp/egl/egl_share_context.cpp
            src/main/cpp/render/gl_render_util.cpp
            src/main/cpp/render/gl_base_render.cpp
            src/main/cpp/render/gl_yuv_render.cpp
            src/main/cpp/render/gl_watermark_render.cpp
            src/main/cpp/opensl/audio_player.cpp
            src/main/jni/media_player.cpp)

find_library(log-lib log )

include_directories(${CMAKE_SOURCE_DIR}/libs/include)

target_link_libraries( video-player-lib
                       avfilter-5
                       avcodec-56
                       avdevice-56
                       avformat-56
                       avutil-54
                       postproc-53
                       swresample-1
                       swscale-3
                       ${log-lib}
                       EGL
                       GLESv3
                       android
                       OpenSLES
                       jnigraphics)