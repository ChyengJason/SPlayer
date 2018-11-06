package com.jscheng.splayer.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import com.jscheng.splayer.R;

/**
 * Created By Chengjunsen on 2018/11/6
 */
public class ProgressView extends RelativeLayout implements SeekBar.OnSeekBarChangeListener {
    private int duration;
    private int progress;
    private TextView durationTextView;
    private TextView progressTextView;
    private SeekBar seekBar;
    private View contentView;

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
        contentView = View.inflate(context, R.layout.progress_view, this);
        durationTextView = contentView.findViewById(R.id.duration_textview);
        progressTextView = contentView.findViewById(R.id.progress_textview);
        seekBar = contentView.findViewById(R.id.seek_bar);
        seekBar.setOnSeekBarChangeListener(this);
        seekBar.setMax(100);
        seekBar.setProgress(50);
    }

    public void setDuration(int duration) {
        this.duration = duration;
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean b) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        seekBar.setThumb(getContext().getDrawable(R.drawable.progress_thumb_pressed));
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        seekBar.setThumb(getContext().getDrawable(R.drawable.progress_thumb_normal));
    }
}
