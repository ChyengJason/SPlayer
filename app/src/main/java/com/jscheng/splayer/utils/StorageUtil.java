package com.jscheng.splayer.utils;

import android.os.Environment;

import java.io.File;

/**
 * Created By Chengjunsen on 2018/9/5
 */
public class StorageUtil {
    public static String getDirName() {
        return "SPlayer";
    }

    public static String getSDPath() {
        // 判断是否挂载
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            return Environment.getExternalStorageDirectory().getAbsolutePath();
        }
        return Environment.getRootDirectory().getAbsolutePath();
    }

    public static String getImagePath() {
        String path = getSDPath() + "/" + getDirName() + "/image/";
        dirNotExistAndCreate(path);
        return path;
    }

    public static String getVedioPath() {
        String path = getSDPath() + "/" + getDirName() + "/video/";
        dirNotExistAndCreate(path);
        return path;
    }

    public static boolean dirNotExistAndCreate(String path) {
        File mDir = new File(path);
        if (!mDir.exists()) {
            return mDir.mkdirs();
        }
        return true;
    }

    public static boolean isFileExist(String path) {
        File mFile = new File(path);
        return mFile.exists();
    }

    public static boolean deleteFile(String path) {
        File mFile = new File(path);
        if (mFile.exists()) {
            return mFile.delete();
        }
        return true;
    }
}