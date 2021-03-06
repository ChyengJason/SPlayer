package com.jscheng.splayer.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import com.jscheng.splayer.R;

/**
 * Created By Chengjunsen on 2018/11/6
 */
public class ProgressView extends RelativeLayout implements SeekBar.OnSeekBarChangeListener, View.OnClickListener {
    private static final String TAG = "ProgressView";
    private int duration;
    private int progress;
    private TextView durationTextView;
    private TextView progressTextView;
    private SeekBar seekBar;
    private View contentView;
    private Button pauseBtn;
    private Button resumeBtn;
    private ProgressSeekListener seekListener;
    public ProgressView(Context context) {
        super(context);
        init(context);
    }

    public ProgressView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public ProgressView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context);
    }

    private void init(Context context) {
        seekListener = null;
        contentView = View.inflate(context, R.layout.progress_view, this);
        durationTextView = contentView.findViewById(R.id.duration_textview);
        progressTextView = contentView.findViewById(R.id.progress_textview);
        resumeBtn = contentView.findViewById(R.id.progress_resume);
        pauseBtn = contentView.findViewById(R.id.progress_pause);
        seekBar = contentView.findViewById(R.id.seek_bar);
        seekBar.setOnSeekBarChangeListener(this);
        pauseBtn.setOnClickListener(this);
        resumeBtn.setOnClickListener(this);
        resumeBtn.setVisibility(View.INVISIBLE);
    }

    public void setSeekListener(ProgressSeekListener listener) {
        this.seekListener = listener;
    }

    public void setDuration(float duration) {
        this.duration = (int)duration;
        Log.e(TAG, "setDuration: "+ duration);
        if (this.duration > 0) {
            int mins = this.duration / 60;
            int secs = this.duration % 60;
            durationTextView.setText(formatDate(mins, secs));
            seekBar.setMax(this.duration);
        } else {
            durationTextView.setText(this.duration);
        }
    }

    public void setProgress(float progress) {
        this.progress = (int)progress;
        if (this.progress > 0) {
            int mins = this.progress / 60;
            int secs = this.progress % 60;
            progressTextView.setText(formatDate(mins, secs));
            seekBar.setProgress(this.progress);
        } else {
            progressTextView.setText(this.progress + "");
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        seekBar.setThumb(getContext().getDrawable(R.drawable.progress_thumb_pressed));
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        seekBar.setThumb(getContext().getDrawable(R.drawable.progress_thumb_normal));
        if (seekListener != null) {
            int progress = seekBar.getProgress();
            Log.e(TAG, "onProgressChanged: " + progress );
            seekListener.seek(progress);
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.progress_pause:
                pauseBtn.setVisibility(View.INVISIBLE);
                resumeBtn.setVisibility(View.VISIBLE);
                if (seekListener != null) {
                    seekListener.pause();
                }
                break;
            case R.id.progress_resume:
                pauseBtn.setVisibility(View.VISIBLE);
                resumeBtn.setVisibility(View.INVISIBLE);
                if (seekListener != null) {
                    seekListener.resume();
                }
                break;
            default:
                break;
        }
    }

    public interface ProgressSeekListener{
        void seek(int duration);
        void pause();
        void resume();
    }

    private String formatDate(int mins, int seconds) {
        StringBuilder builder = new StringBuilder();
        builder.append(mins).append(":");
        if (seconds < 10) {
            builder.append("0");
        }
        builder.append(seconds);
        return builder.toString();
    }
}
