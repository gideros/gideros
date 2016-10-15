package com.giderosmobile.android;

import java.lang.reflect.Method;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.Gravity;
import android.graphics.Color;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.text.InputType;

import com.giderosmobile.android.player.*;
import com.giderosmobile.androidtemplate.R;

public class AndroidTemplateActivity extends Activity implements OnTouchListener
{
	static
	{
		System.loadLibrary("gvfs");
		System.loadLibrary("lua");
		System.loadLibrary("gideros");

		System.loadLibrary("luasocket");
		System.loadLibrary("lfs");
		System.loadLibrary("lsqlite3");
		System.loadLibrary("json");
		System.loadLibrary("bitop");
		//Line below is a marker for plugin insertion scripts. Do not remove or change
		//GIDEROS-STATIC-INIT//
	}

	static private String[] externalClasses = {
		//Line below is a marker for plugin insertion scripts. Do not remove or change
		//GIDEROS-EXTERNAL-CLASS//
			null
	};
	
	private GLSurfaceView mGLView;

	private boolean mHasFocus = false;
	private boolean mPlaying = false;
    
    private static FrameLayout splashLayout;
	private static ImageView splash;
	private static FrameLayout layout;
	private static int hasSplash = -1;
		
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
				
        mGLView = new GiderosGLSurfaceView(this);
		setContentView(mGLView);
		mGLView.setOnTouchListener(this);
        
        boolean showSplash = true;
        
		if(showSplash && getResources().getIdentifier("splash", "drawable", getPackageName()) != 0){
			layout = (FrameLayout)getWindow().getDecorView();
			hasSplash = 11;
			//create a layout for animation
			splashLayout = new FrameLayout(this);
			//parameters for layout
			FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
		                FrameLayout.LayoutParams.MATCH_PARENT ,
		                FrameLayout.LayoutParams.MATCH_PARENT,
		                Gravity.CENTER);
			splashLayout.setLayoutParams(params);
			//set background color
			splashLayout.setBackgroundColor(Color.parseColor("#ffffff"));
		 
			//create image view for animation
			splash = new ImageView(this);
			//image view parameters
			FrameLayout.LayoutParams params2 = new FrameLayout.LayoutParams(
		                 FrameLayout.LayoutParams.WRAP_CONTENT,
		                 FrameLayout.LayoutParams.WRAP_CONTENT,
		                 Gravity.CENTER);
			splash.setLayoutParams(params2);
		 
			//scale your image
			splash.setScaleType(ImageView.ScaleType.CENTER );
		 
			//load image source     
			splash.setBackgroundResource(R.drawable.splash);
		
			//add image view to layout
			splashLayout.addView(splash);
			//add image layout to main layout
			layout.addView(splashLayout);
		}
		
		WeakActivityHolder.set(this);

		GiderosApplication.onCreate(externalClasses,mGLView);
	}

	int[] id = new int[256];
	int[] x = new int[256];
	int[] y = new int[256];
    float[] pressure = new float[256];

	@Override
	public void onStart()
	{
		super.onStart();
		GiderosApplication.getInstance().onStart();
	}

	@Override
	public void onRestart()
	{
		super.onRestart();
		GiderosApplication.getInstance().onRestart();
	}

	@Override
	public void onStop()
	{
		GiderosApplication.getInstance().onStop();
		super.onStop();
	}

	@Override
	public void onDestroy()
	{
		GiderosApplication.onDestroy();
		super.onDestroy();
	}

	@Override
	protected void onPause()
	{
		if (mPlaying == true)
		{
			GiderosApplication.getInstance().onPause();
			mGLView.onPause();
			mPlaying = false;
		}
		
		super.onPause();
	}

	@Override
	protected void onResume()
	{
		super.onResume();

		if (mHasFocus == true && mPlaying == false)
		{
			mGLView.onResume();
			GiderosApplication.getInstance().onResume();
			mPlaying = true;
		}
	}
	
	@Override
	public void onLowMemory()
	{
		super.onLowMemory();

		GiderosApplication app = GiderosApplication.getInstance();
		if (app != null)
			app.onLowMemory();
	}
	 
	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		super.onActivityResult(requestCode, resultCode, data);
		GiderosApplication.getInstance().onActivityResult(requestCode, resultCode, data);
	}
	
	@TargetApi(Build.VERSION_CODES.KITKAT)
	@Override
	public void onWindowFocusChanged(boolean hasFocus)
	{
		super.onWindowFocusChanged(hasFocus);
		
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
	        if (hasFocus) {
	        	getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
	        			| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
	        			| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
	        			| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
	        			| View.SYSTEM_UI_FLAG_FULLSCREEN
	        			| View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
	        }
	    }
		
		mHasFocus = hasFocus;

		if (mHasFocus == true && mPlaying == false)
		{
			mGLView.onResume();
			GiderosApplication.getInstance().onResume();
			mPlaying = true;
		}
	}
	
	public boolean onTouch(View v, MotionEvent event)
	{
		GiderosApplication app = GiderosApplication.getInstance();
		if (app == null)
			return false;

		int size = event.getPointerCount();
		for (int i = 0; i < size; i++)
		{
			id[i] = event.getPointerId(i);
			x[i] = (int) event.getX(i);
			y[i] = (int) event.getY(i);
            pressure[i] = (float) event.getPressure(i);
		}

		int actionMasked = event.getActionMasked();
		boolean isPointer = (actionMasked == MotionEvent.ACTION_POINTER_DOWN || actionMasked == MotionEvent.ACTION_POINTER_UP);
		int actionIndex = isPointer ? event.getActionIndex() : 0;
				
		if (actionMasked == MotionEvent.ACTION_DOWN || actionMasked == MotionEvent.ACTION_POINTER_DOWN)
		{
			app.onTouchesBegin(size, id, x, y, pressure, actionIndex);
		} else if (actionMasked == MotionEvent.ACTION_MOVE)
		{
			app.onTouchesMove(size, id, x, y, pressure);
		} else if (actionMasked == MotionEvent.ACTION_UP || actionMasked == MotionEvent.ACTION_POINTER_UP)
		{
			app.onTouchesEnd(size, id, x, y, pressure, actionIndex);
		} else if (actionMasked == MotionEvent.ACTION_CANCEL)
		{
			app.onTouchesCancel(size, id, x, y, pressure);
		}

		return true;
	}

	@Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        //GIDEROS-ACTIVTIY-ONKEYDOWN//
		GiderosApplication app = GiderosApplication.getInstance();
		if (app != null && app.onKeyDown(keyCode, event) == true)
			return true;
		
		return super.onKeyDown(keyCode, event);
    }

	
	@Override
    public boolean onKeyUp(int keyCode, KeyEvent event)
    {
        //GIDEROS-ACTIVTIY-ONKEYUP//
		GiderosApplication app = GiderosApplication.getInstance();
		if (app != null && app.onKeyUp(keyCode, event) == true)
			return true;
		
		return super.onKeyUp(keyCode, event);
    }
	
	@Override
    public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) {
		GiderosApplication app = GiderosApplication.getInstance();
		if (app != null && app.onKeyMultiple(keyCode, repeatCount, event) == true)
			return true;
		
		return super.onKeyMultiple(keyCode, repeatCount, event);
    }	

	public void onRequestPermissionsResult(int requestCode,
			String permissions[], int[] grantResults) {
	}

    
    //GIDEROS-ACTIVTIY-METHODS//
    
    static public void dismisSplash(){
        if(hasSplash == -1){
            return;
        }
    	else if(hasSplash == 0){
    		hasSplash = -1;
    		new Handler(Looper.getMainLooper()).post(new Runnable() {
    		    @Override
    		    public void run() {
    		    	splashLayout.setVisibility(View.GONE);
    		    	splash.setBackgroundResource(0);
    		    	//remove animation view from main layout
    		    	layout.removeView(splashLayout);
    		    	splashLayout = null;
    		    	splash = null;
    		    	layout = null;
    		    }
    		});
    	}
    	else if(hasSplash > 0){
    		hasSplash--;
    	}
    }
}

class GiderosGLSurfaceView extends GLSurfaceView
{
	public GiderosGLSurfaceView(Context context)
	{
		super(context);
		setEGLContextClientVersion(2);
		setEGLConfigChooser(8,8,8,0,16,8);
		mRenderer = new GiderosRenderer();
		setRenderer(mRenderer);
		if (android.os.Build.VERSION.SDK_INT >= 11)
		{
			try
			{
				for (Method method : getClass().getMethods())
				{
					if (method.getName().equals("setPreserveEGLContextOnPause"))
					{
						method.invoke(this, true);
						break;
					}
				}
			}
			catch (Exception e)
			{
			}
		}
		setFocusable(true);
		setFocusableInTouchMode(true);
	}
	
	@Override
	public InputConnection onCreateInputConnection(EditorInfo outAttrs)
	{
	    outAttrs.actionLabel = "";
	    outAttrs.hintText = "";
	    outAttrs.initialCapsMode = 0;
	    outAttrs.initialSelEnd = outAttrs.initialSelStart = -1;
	    outAttrs.label = "";
	    outAttrs.imeOptions = EditorInfo.IME_ACTION_DONE | EditorInfo.IME_FLAG_NO_EXTRACT_UI;        
	    outAttrs.inputType = InputType.TYPE_NULL;        

	    return  new BaseInputConnection(this, false);       
	}     

	@Override
	public boolean onCheckIsTextEditor ()
	{
	    return true;
	}

	GiderosRenderer mRenderer;
}

class GiderosRenderer implements GLSurfaceView.Renderer
{
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		GiderosApplication.getInstance().onSurfaceCreated();
	}

	public void onSurfaceChanged(GL10 gl, int w, int h)
	{
		GiderosApplication.getInstance().onSurfaceChanged(w, h);
	}

	public void onDrawFrame(GL10 gl)
	{
		GiderosApplication app = GiderosApplication.getInstance();
		if (app != null)
		{
			//GIDEROS-ACTIVITY-PREDRAW//
			app.onDrawFrame();
			//GIDEROS-ACTIVITY-POSTDRAW//
			AndroidTemplateActivity.dismisSplash();
		}
	}
}
