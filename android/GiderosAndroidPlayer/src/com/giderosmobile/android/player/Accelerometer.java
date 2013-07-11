package com.giderosmobile.android.player;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;

/**
 * 
 * This class is used for controlling the Accelerometer
 *
 */
public class Accelerometer implements SensorEventListener {
	
	private Context mContext;
	private SensorManager mSensorManager;
	private Sensor mAccelerometer;
	private int mNaturalOrientation;
	private boolean isEnabled_;

	public Accelerometer(){
		Activity activity = WeakActivityHolder.get();

		//Get an instance of the SensorManager
	    mSensorManager = (SensorManager) activity.getSystemService(Context.SENSOR_SERVICE);
	    mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
	    
	    Display display = ((WindowManager)activity.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
	    mNaturalOrientation = display.getRotation();
	    
	    isEnabled_ = false;
	}

	boolean isAvailable()
	{
		return mAccelerometer != null;
	}

	public void enable() {
		if (!isAvailable())
			return;
		if (isEnabled_)
			return;
		mSensorManager.registerListener(this, mAccelerometer, SensorManager.SENSOR_DELAY_GAME);
		isEnabled_ = true;
	}

	public void disable () {
		if (!isAvailable())
			return;
		if (!isEnabled_)
			return;
		mSensorManager.unregisterListener(this);
		isEnabled_ = false;
	}

	@Override
	public void onSensorChanged(SensorEvent event) {
		Activity activity = WeakActivityHolder.get();
		
		if (event.sensor.getType() != Sensor.TYPE_ACCELEROMETER){
            return;
		}

		float x = event.values[0];
		float y = event.values[1];
		float z = event.values[2];
		
		/*
		 * Because the axes are not swapped when the device's screen orientation changes. 
		 * So we should swap it here.
		 * In tablets such as Motorola Xoom, the default orientation is landscape, so should
		 * consider this.
		 */
		int orientation = activity.getResources().getConfiguration().orientation;
		if ((orientation == Configuration.ORIENTATION_LANDSCAPE) && (mNaturalOrientation != Surface.ROTATION_0)){
			float tmp = x;
			x = -y;
			y = tmp;
		}
		else if ((orientation == Configuration.ORIENTATION_PORTRAIT) && (mNaturalOrientation != Surface.ROTATION_0))
		{
			 float tmp = x;
	         x = y;
	         y = -tmp;
		}
		
        onSensorChanged(-x / 9.80665f, -y / 9.80665f, -z / 9.80665f);
	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
	}

	private static native void onSensorChanged(float x, float y, float z);
}
