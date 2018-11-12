package com.jscheng.splayer;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.NonNull;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;

import com.jscheng.splayer.player.VideoPlayer;
import com.jscheng.splayer.utils.MediaUtil;
import com.jscheng.splayer.utils.PermissionUtil;
import com.jscheng.splayer.utils.StorageUtil;
import com.jscheng.splayer.widget.ProgressView;
import com.jscheng.splayer.widget.VideoSurfaceView;

public class MainActivity extends BaseActivity implements SurfaceHolder.Callback, View.OnClickListener, ProgressView.ProgressSeekListener{
    private static final String TAG = "CJS";
    private static final int REQUEST_CODE = 1;
    private static final int REQUEST_PICK = 2;
    private static final int PROGRESS_MSG = 3;
    private VideoPlayer mVideoPlayer;
    private VideoSurfaceView mVideoView;
    private ProgressView mProgressView;
    private Handler mHandler;
    private Button mOpenBtn;
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
        mProgressView = findViewById(R.id.progress_view);
        mProgressView.setSeekListener(this);
        mOpenBtn = findViewById(R.id.open_media_btn);
        mOpenBtn.setOnClickListener(this);
        mHandler = new Handler(getMainLooper()) {
            @Override
            public void handleMessage(Message msg) {
                if (msg.what == PROGRESS_MSG) {
                    mProgressView.setProgress(mVideoPlayer.getProgress());
                    if (mVideoPlayer.getProgress() == mVideoPlayer.getDuration()) {
                        mOpenBtn.setVisibility(View.VISIBLE);
                    } else {
                        mHandler.sendEmptyMessageDelayed(PROGRESS_MSG, 1000L);
                    }
                }
            }
        };
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mVideoPlayer.onSurfaceCreated(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        mVideoPlayer.onSurfaceSizeChanged(width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mVideoPlayer.onSurfaceDestroy();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.open_media_btn:
                chooseAndPlayMedia();
                break;
            case R.id.video_view:
                break;
            default:
                break;
        }
    }

    private void chooseAndPlayMedia() {
        boolean storePermission = PermissionUtil.checkPermissionsAndRequest(this,
                PermissionUtil.STORAGE,
                REQUEST_CODE,
                "申请读取文件失败");
        if (!storePermission) {
            return;
        }
//        MediaUtil.pickVideoFile(this, REQUEST_PICK);
        mOpenBtn.setVisibility(View.GONE);
        mVideoPlayer.start(StorageUtil.getSDPath() + "/media.mp4");
        mProgressView.setDuration(mVideoPlayer.getDuration());
        mHandler.sendEmptyMessageDelayed(PROGRESS_MSG, 1000L);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_PICK && resultCode == RESULT_OK) {
            String videoPath = StorageUtil.getUrlAbsulotePath(this, data.getData());
            Log.e(TAG, "onActivityResult: " + videoPath + " " + mVideoView.getHeight() +"x" + mVideoView.getWidth() );
            mOpenBtn.setVisibility(View.GONE);
            mVideoPlayer.start(videoPath);
            mProgressView.setDuration(mVideoPlayer.getDuration());
            mHandler.sendEmptyMessageDelayed(PROGRESS_MSG, 1000L);
        }
    }

    @Override
    public void seek(int duration) {
        mVideoPlayer.seek(duration);
    }

    @Override
    public void pause() {
        mVideoPlayer.pause();
    }

    @Override
    public void resume() {
        mVideoPlayer.resume();
    }
}
