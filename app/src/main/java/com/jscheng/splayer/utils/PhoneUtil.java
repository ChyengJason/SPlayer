package com.jscheng.splayer.utils;

import android.os.Build;

public class PhoneUtil {
    public static boolean isMIUI() {
        return isThirdSystem("xiaomi");
    }

    /**
     * 判断华为就填写huawei,魅族就填写meizu
     */
    public static boolean isThirdSystem(String name) {
        String manufacturer = Build.MANUFACTURER;
        if (name.equalsIgnoreCase(manufacturer)) {
            return true;
        }
        return false;
    }
}
