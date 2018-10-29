package com.jscheng.splayer;

import android.app.Activity;
import android.app.Application;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.DisplayMetrics;

/**
 * Created By Chengjunsen on 2018/10/29
 */
public class BaseActivity extends AppCompatActivity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setCustomDensity(this, getApplication());
    }

    // 屏幕适配方式
    private static void setCustomDensity(@NonNull Activity activity, @NonNull Application application) {
        final DisplayMetrics appDisplayMetrics = application.getResources().getDisplayMetrics();
        final DisplayMetrics activityDisplayMetrics = activity.getResources().getDisplayMetrics();

        final float originalDenstity = appDisplayMetrics.density;
        final int originalDenstityDpi = appDisplayMetrics.densityDpi;
        final float originalScaleDensty = appDisplayMetrics.scaledDensity;

        final float targetDensity = appDisplayMetrics.widthPixels / 360;
        final int targetDenstityDpi = (int)(targetDensity * 160);
        final float targetScaleDensity = originalScaleDensty * (targetDensity / originalDenstity);

        appDisplayMetrics.density = targetDensity;
        appDisplayMetrics.densityDpi = targetDenstityDpi;
        appDisplayMetrics.scaledDensity = targetScaleDensity;

        activityDisplayMetrics.density = targetDensity;
        activityDisplayMetrics.densityDpi = targetDenstityDpi;
        activityDisplayMetrics.scaledDensity = targetScaleDensity;
    }
}
