package com.jscheng.splayer;

import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import com.jscheng.splayer.player.VideoPlayer;
import com.jscheng.splayer.utils.PermissionUtil;
import com.jscheng.splayer.utils.StorageUtil;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "CJS";
    private static final int REQUEST_CODE = 1;
    private TextView mTextview;
    private VideoPlayer mVideoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mTextview = findViewById(R.id.sample_text);
        mVideoPlayer = new VideoPlayer();
        boolean storePermission = PermissionUtil.checkPermissionsAndRequest(this, PermissionUtil.STORAGE, REQUEST_CODE, "申请读取文件失败");
        if (storePermission) {
            playMedia();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CODE && PermissionUtil.isPermissionsAllGranted(this, PermissionUtil.STORAGE).isEmpty()) {
            playMedia();
        }
    }

    private void playMedia() {
        mVideoPlayer.prepare(StorageUtil.getSDPath() + "/" + "media.mp4");
    }
}
