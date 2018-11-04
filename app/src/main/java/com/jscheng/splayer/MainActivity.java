package com.jscheng.splayer;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.annotation.NonNull;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;

import com.jscheng.splayer.player.VideoPlayer;
import com.jscheng.splayer.utils.PermissionUtil;
import com.jscheng.splayer.utils.StorageUtil;
import com.jscheng.splayer.widget.VideoSurfaceView;

public class MainActivity extends BaseActivity implements SurfaceHolder.Callback, View.OnClickListener{
    private static final String TAG = "CJS";
    private static final int REQUEST_CODE = 1;
    private VideoPlayer mVideoPlayer;
    private VideoSurfaceView mVideoView;
    private Button mBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams. FLAG_FULLSCREEN, WindowManager.LayoutParams. FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);
        mBtn = findViewById(R.id.btn);
        mVideoView = findViewById(R.id.video_view);
        mVideoPlayer = new VideoPlayer();
        mVideoView.getHolder().addCallback(this);
        mBtn.setOnClickListener(this);
//        Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.mipmap.ic_launcher_round);
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
        mBtn.setText(mVideoPlayer.getDuration() + "");
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

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.btn) {
            mVideoPlayer.seek(mVideoPlayer.getProgress() + 10 );
        }
    }
}
