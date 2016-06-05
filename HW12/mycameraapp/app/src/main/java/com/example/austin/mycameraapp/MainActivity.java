package com.example.austin.mycameraapp;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import java.io.IOException;
import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;

public class MainActivity extends Activity implements TextureView.SurfaceTextureListener {
    private Camera mCamera;
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private Bitmap bmp = Bitmap.createBitmap(640,480,Bitmap.Config.ARGB_8888);
    private Canvas canvas = new Canvas(bmp);
    private Paint paint1 = new Paint();
    private TextView mTextView;
    SeekBar myControl;
    TextView myTextView;
    TextView myTextView2;
    int threshold = 0;

    static long prevtime = 0; // for FPS calculation

    private void setMyControlListener() {
        myControl.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {



            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                threshold = progress;
                myTextView.setText("The value is: "+progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }


    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); // keeps the screen from turning off

        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
        mSurfaceHolder = mSurfaceView.getHolder();

        mTextureView = (TextureView) findViewById(R.id.textureview);
        mTextureView.setSurfaceTextureListener(this);

        mTextView = (TextView) findViewById(R.id.cameraStatus);


        paint1.setColor(0xffff0000); // red
        paint1.setTextSize(24);

        myControl = (SeekBar) findViewById(R.id.seek1);

        myTextView = (TextView) findViewById(R.id.textView01);
        myTextView.setText("Enter whatever you Like!");
        myTextView2 = (TextView) findViewById(R.id.green);

        setMyControlListener();
    }

    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mCamera = Camera.open();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480);
        // parameters.setColorEffect(Camera.Parameters.EFFECT_MONO); // black and white
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); // no autofocusing
        parameters.setAutoWhiteBalanceLock(true);
        mCamera.setParameters(parameters);
        mCamera.setDisplayOrientation(90); // rotate to portrait mode

        try {
            mCamera.setPreviewTexture(surface);
            mCamera.startPreview();
        } catch (IOException ioe) {
            // Something bad happened
        }
    }

    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        // Ignored, Camera does all the work for us
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }

    // the important function
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        // Invoked every time there's a new Camera preview frame
        mTextureView.getBitmap(bmp);

        final Canvas c = mSurfaceHolder.lockCanvas();
        if (c != null) {

            int[] pixels = new int[bmp.getWidth()];
            // int[] pixelstop = new int[bmp.getWidth()];
            // int[] pixelsmid = new int[bmp.getWidth()];
            // int[] pixelsbot = new int[bmp.getWidth()];
            int startY = 240; // which row in the bitmap to analyse to read
            // int startYtop = 100;
            // int startYmid = 240;
            // int startYbot = 380;

            // only look at one row in the image
            bmp.getPixels(pixels, 0, bmp.getWidth(), 0, startY, bmp.getWidth(), 1); // (array name, offset inside array, stride (size of row), start x, start y, num pixels to read per row, num rows to read)
            //  bmp.getPixels(pixelstop, 0, bmp.getWidth(), 0, startYtop, bmp.getWidth(), 1);
            //  bmp.getPixels(pixelsmid, 0, bmp.getWidth(), 0, startYmid, bmp.getWidth(), 1);
            //  bmp.getPixels(pixelsbot, 0, bmp.getWidth(), 0, startYbot, bmp.getWidth(), 1);

            // pixels[] is the RGBA data (in black an white).
            // instead of doing center of mass on it, decide if each pixel is dark enough to consider black or white
            // then do a center of mass on the thresholded array
            int[] thresholdedPixels = new int[bmp.getWidth()];
            // int[] thresholdedPixelstop = new int[bmp.getWidth()];
            // int[] thresholdedPixelsmid = new int[bmp.getWidth()];
            // int[] thresholdedPixelsbot = new int[bmp.getWidth()];
            int wbTotal = 0; // total mass
            // int wbTotaltop = 0;
            // int wbTotalmid = 0;
            // int wbTotalbot = 0;
            int wbCOM = 0; // total (mass time position)
            // int wbCOMtop = 0;
            // int wbCOMmid = 0;
            // int wbCOMbot = 0;
            for (int i = 0; i < bmp.getWidth(); i++) {
                // sum the red, green and blue, subtract from 255 to get the darkness of the pixel.
                // if it is greater than some value (600 here), consider it black
                // play with the 600 value if you are having issues reliably seeing the line
                if (255-(green(pixels[i])-red(pixels[i])) > threshold) {
                    thresholdedPixels[i] = 255*3;
                }
                else {
                    thresholdedPixels[i] = 0;
                }
                wbTotal = wbTotal + thresholdedPixels[i];
                wbCOM = wbCOM + thresholdedPixels[i]*i;

                /*if ((255-green(pixelstop[i])*3) > threshold) {
                    thresholdedPixelstop[i] = 255*3;
                }
                else {
                    thresholdedPixelstop[i] = 0;
                }
                wbTotaltop = wbTotaltop + thresholdedPixelstop[i];
                wbCOMtop = wbCOMtop + thresholdedPixelstop[i]*i;

                if ((255-green(pixelsmid[i])*3) > threshold) {
                    thresholdedPixelsmid[i] = 255*3;
                }
                else {
                    thresholdedPixelsmid[i] = 0;
                }
                wbTotalmid = wbTotalmid + thresholdedPixelsmid[i];
                wbCOMmid = wbCOMmid + thresholdedPixelsmid[i]*i;

                if ((255-green(pixelsbot[i])*3) > threshold) {
                    thresholdedPixelsbot[i] = 255*3;
                }
                else {
                    thresholdedPixelsbot[i] = 0;
                }
                wbTotalbot = wbTotalbot + thresholdedPixelsbot[i];
                wbCOMbot = wbCOMbot + thresholdedPixelsbot[i]*i;*/
            }
            int COM;
            // int COMtop;
            // int COMmid;
            // int COMbot;
            //watch out for divide by 0
            if (wbTotal<=0) {
                COM = bmp.getWidth()/2;
            }
            else {
                COM = wbCOM/wbTotal;
            }

//            if (wbTotaltop<=0) {
//                COMtop = bmp.getWidth()/2;
//            }
//            else {
//                COMtop = wbCOMtop/wbTotaltop;
//            }
//
//            if (wbTotalmid<=0) {
//                COMmid = bmp.getWidth()/2;
//            }
//            else {
//                COMmid = wbCOMmid/wbTotalmid;
//            }
//
//            if (wbTotalbot<=0) {
//                COMbot = bmp.getWidth()/2;
//            }
//            else {
//                COMbot = wbCOMbot/wbTotalbot;
//            }

            // draw a circle where you think the COM is
            canvas.drawCircle(COM, startY, 5, paint1);
            // canvas.drawCircle(COMtop, startYtop, 5, paint1);
            // canvas.drawCircle(COMmid, startYmid, 5, paint1);
            // canvas.drawCircle(COMbot, startYbot, 5, paint1);

            // also write the value as text
            canvas.drawText("COM = " + COM, 10, 200, paint1);
            // canvas.drawText("COMtop = " + COMtop, 10, 100, paint1);
            // canvas.drawText("COMmid = " + COMmid, 10, 200, paint1);
            // canvas.drawText("COMbot = " + COMbot, 10, 300, paint1);
            c.drawBitmap(bmp, 0, 0, null);
            mSurfaceHolder.unlockCanvasAndPost(c);

            // calculate the FPS to see how fast the code is running
            long nowtime = System.currentTimeMillis();
            long diff = nowtime - prevtime;
            //mTextView.setText("FPS " + 1000/diff);

            int red = red(pixels[320]);
            int green = green(pixels[320]);
            mTextView.setText("red = " + red);
            myTextView2.setText("green = " + green);
            prevtime = nowtime;
        }
    }
}