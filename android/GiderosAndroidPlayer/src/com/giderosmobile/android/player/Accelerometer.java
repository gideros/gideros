package com.giderosmobile.android.player;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.view.Surface;

public class Accelerometer implements SensorEventListener {
	private SensorManager mSensorManager;
	private Sensor mAccelerometer;
	private boolean isEnabled_;
	private int mNativeOrientation;

	public Accelerometer() {
		Activity activity = WeakActivityHolder.get();

		mSensorManager = (SensorManager)activity.getSystemService(Context.SENSOR_SERVICE);
		mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

		int rotation = 0;
		switch (activity.getWindowManager().getDefaultDisplay().getRotation()) {
		case Surface.ROTATION_0:
			rotation = 0;
			break;
		case Surface.ROTATION_90:
			rotation = 90;
			break;
		case Surface.ROTATION_180:
			rotation = 180;
			break;
		case Surface.ROTATION_270:
			rotation = 270;
			break;
		}

		int orientation = activity.getResources().getConfiguration().orientation;
		if (((rotation == 0 || rotation == 180) && (orientation == Configuration.ORIENTATION_LANDSCAPE)) || ((rotation == 90 || rotation == 270) && (orientation == Configuration.ORIENTATION_PORTRAIT))) {
			mNativeOrientation = Configuration.ORIENTATION_LANDSCAPE;
		} else {
			mNativeOrientation = Configuration.ORIENTATION_PORTRAIT;
		}

		isEnabled_ = false;
	}

	boolean isAvailable() {
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

	public void disable() {
		if (!isAvailable())
			return;
		if (!isEnabled_)
			return;
		mSensorManager.unregisterListener(this);
		isEnabled_ = false;
	}

	@Override
	public void onSensorChanged(SensorEvent event) {
		if (event.sensor.getType() != Sensor.TYPE_ACCELEROMETER) {
			return;
		}

		float x, y, z;
		if (mNativeOrientation == Configuration.ORIENTATION_PORTRAIT) {
			x = event.values[0];
			y = event.values[1];
			z = event.values[2];
		} else {
			x = event.values[1];
			y = -event.values[0];
			z = event.values[2];
		}

		onSensorChanged(-x / 9.80665f, -y / 9.80665f, -z / 9.80665f);
	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
	}

	private static native void onSensorChanged(float x, float y, float z);
}
