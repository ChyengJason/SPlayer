package com.jscheng.splayer.player;

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

    public native void prepare(String path);

    public native void printConfig();
}
