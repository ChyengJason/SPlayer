package com.jscheng.splayer.utils;

import android.app.Activity;
import android.content.Intent;
import android.os.Build;
import android.provider.MediaStore;

public class MediaUtil {
    public static void pickVideoFile(Activity activity, int requestCode) {
        if(PhoneUtil.isMIUI()){
            //是否是小米设备,是的话用到弹窗选取入口的方法去选取视频
            Intent intent = new Intent(Intent.ACTION_PICK,android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
            intent.setDataAndType(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,"video/*");
            activity.startActivityForResult(Intent.createChooser(intent, "选择要导入的视频"), requestCode);
        } else {
            //直接跳到系统相册去选取视频
            Intent intent = new Intent();
            if (Build.VERSION.SDK_INT < 19) {
                intent.setAction(Intent.ACTION_GET_CONTENT);
                intent.setType("video/*");
            } else {
                intent.setAction(Intent.ACTION_OPEN_DOCUMENT);
                intent.addCategory(Intent.CATEGORY_OPENABLE);
                intent.setType("video/*");
            }
            activity.startActivityForResult(Intent.createChooser(intent, "选择要导入的视频"), requestCode);
        }
    }
}
