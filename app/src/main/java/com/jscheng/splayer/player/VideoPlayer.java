package com.jscheng.splayer.player;

import android.view.Surface;

/**
 * Created By Chengjunsen on 2018/10/12
 */
public class VideoPlayer {
    static {
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avutil-54");
        System.loadLibrary("postproc-53");
        System.loadLibrary("swresample-1");
        System.loadLibrary("swscale-3");
        System.loadLibrary("video-player-lib");
    }

    public native void onSurfaceCreated(Surface surface);

    public native void onSurfaceSizeChanged(int screenWidth, int screenHeight);

    public native void onSurfaceDestroy();

    public native void printConfig();

    public native void start(String path);

    public native void stop();

    public native void seek(double position);

    public native void pause();

    public native void resume();

    public native long getDuration();

    public native long getProgress();


}
