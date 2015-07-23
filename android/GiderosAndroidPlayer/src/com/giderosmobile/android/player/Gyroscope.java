package com.giderosmobile.android.player;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

/**
 * 
 * This class is used for controlling the Gyroscope
 *
 */
public class Gyroscope implements SensorEventListener {
	
	private Context mContext;
	private SensorManager mSensorManager;
	private Sensor mGyroscope;
	private boolean isEnabled_;

	public Gyroscope(){
		Activity activity = WeakActivityHolder.get();

		//Get an instance of the SensorManager
	    mSensorManager = (SensorManager) activity.getSystemService(Context.SENSOR_SERVICE);
	    mGyroscope = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
	    
	    isEnabled_ = false;
	}
	
	boolean isAvailable()
	{
		return mGyroscope != null;
	}
	
	public void enable() {
		if (!isAvailable())
			return;
		if (isEnabled_)
			return;
		mSensorManager.registerListener(this, mGyroscope, SensorManager.SENSOR_DELAY_GAME);
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
		if (event.sensor.getType() != Sensor.TYPE_GYROSCOPE){
            return;
		}

		float x = event.values[0];
		float y = event.values[1];
		float z = event.values[2];

        onSensorChanged(x, y, z);
	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
	}

	private static native void onSensorChanged(float x, float y, float z);
}
