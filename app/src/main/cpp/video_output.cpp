//
// Created by 橙俊森 on 2018/10/15.
//

#include "video_output.h"
extern "C" {
#include "android_log.h"
}

extern "C" {
#include <unistd.h>
}

VideoOutput::VideoOutput() {
    mNativeWindow = NULL;
    isInited = false;
}

VideoOutput::~VideoOutput() {

}

void VideoOutput::prepare(ANativeWindow *nativeWindow) {
    mNativeWindow = nativeWindow;
    isInited = false;
}

void VideoOutput::release() {
    ANativeWindow_release(mNativeWindow);
}

void VideoOutput::output(const VideoFrame &videoFrame) {
    if (videoFrame.width <= 0 || videoFrame.height <= 0) {
        return;
    }
    if (!isInited) {
        ANativeWindow_setBuffersGeometry(mNativeWindow, videoFrame.width, videoFrame.height, WINDOW_FORMAT_RGBA_8888);
        isInited = true;
        LOGE("初始化ANativeWindow");
    }
    ANativeWindow_lock(mNativeWindow, &mNativeOutBuffer, NULL);
    uint8_t *dst = (uint8_t *) mNativeOutBuffer.bits;
    int destStride = mNativeOutBuffer.stride * 4;
    uint8_t * src=  videoFrame.rgb;
    int srcStride = videoFrame.linesize;

    for (int i = 0; i <videoFrame.height; ++i) {
        // 将 rgb_frame 中每一行的数据复制给 nativewindow
        memcpy(dst + i * destStride,  src + i * srcStride, srcStride);
    }
    ANativeWindow_unlockAndPost(mNativeWindow);
    usleep(1000 * 16);
}



