package com.jscheng.splayer;

import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

import com.jscheng.splayer.player.VideoPlayer;
import com.jscheng.splayer.utils.PermissionUtil;
import com.jscheng.splayer.utils.StorageUtil;
import com.jscheng.splayer.widget.VideoSurfaceView;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback{
    private static final String TAG = "CJS";
    private static final int REQUEST_CODE = 1;
    private VideoPlayer mVideoPlayer;
    private VideoSurfaceView mVideoView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams. FLAG_FULLSCREEN, WindowManager.LayoutParams. FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);
        mVideoView = findViewById(R.id.video_view);
        mVideoPlayer = new VideoPlayer();
        mVideoView.getHolder().addCallback(this);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CODE && PermissionUtil.isPermissionsAllGranted(this, PermissionUtil.STORAGE).isEmpty()) {
            playMedia();
        }
    }

    private void playMedia() {
        mVideoPlayer.start(StorageUtil.getSDPath() + "/" + "media.mp4");
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mVideoPlayer.onSurfaceCreated(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        mVideoPlayer.onSurfaceSizeChanged(width, height);
        boolean storePermission = PermissionUtil.checkPermissionsAndRequest(this,
                PermissionUtil.STORAGE,
                REQUEST_CODE,
                "申请读取文件失败");
        if (storePermission) {
            playMedia();
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mVideoPlayer.onSurfaceDestroy();
    }
}
