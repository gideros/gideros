package com.giderosmobile.android.player;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.GeomagneticField;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Build;
import android.content.pm.PackageManager;
import android.Manifest;

public class Geolocation
{
	private LocationManager locationManager_;
	private Location currentBestLocation = null;
	private LocationListener locationListener_ = null;

	private SensorManager sensorManager;
	private Sensor magneticSensor;
	private Sensor mAccelerometer;
	private SensorEventListener magneticListener_ = null;
	boolean gps_enabled = false;
	boolean network_enabled = false;
	private double locThreshold = 0;

    private float[] mMatrixR;
    private float[] mMatrixI;
    private float[] mMatrixValues;

	Geolocation()
	{
		Activity activity = WeakActivityHolder.get();

		locationManager_ = (LocationManager)activity.getSystemService(Context.LOCATION_SERVICE);

		sensorManager = (SensorManager)activity.getSystemService(Context.SENSOR_SERVICE);
        magneticSensor = sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
		mAccelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		
	      mMatrixR = new float[9];
	      mMatrixI = new float[9];
	      mMatrixValues = new float[3];
	}

	public boolean isAvailable()
	{
		Activity activity = WeakActivityHolder.get();
		boolean request=false;
		gps_enabled = locationManager_.isProviderEnabled(LocationManager.GPS_PROVIDER);
		network_enabled = locationManager_.isProviderEnabled(LocationManager.NETWORK_PROVIDER);

		if (android.os.Build.VERSION.SDK_INT >= 23) {
		if (gps_enabled&&activity.checkSelfPermission(
                Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
				gps_enabled=false;
				request=true;
		}
		if (network_enabled&&activity.checkSelfPermission(
                Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
				network_enabled=false;
				request=true;
		}
		if (request&&(!gps_enabled))
			activity.requestPermissions(
					new String[]{Manifest.permission.ACCESS_COARSE_LOCATION,Manifest.permission.ACCESS_FINE_LOCATION},
						0);
			
		}
		return gps_enabled || network_enabled;
	}

	public boolean isHeadingAvailable()
	{
		return magneticSensor != null;
	}

	public void setAccuracy(double accuracy)
	{
	}
	
	public double getAccuracy()
	{
		return 0;
	}
	
	public void setThreshold(double threshold)
	{
			locThreshold=threshold;
			if (locationListener_ != null) {
				stopUpdatingLocation();
				startUpdatingLocation();
			}
	}
	public double getThreshold()
	{
		return locThreshold;		
	}
	public void start()
	{
		startUpdatingLocation();
		startUpdatingHeading();
	}
	public void stop()
	{
		stopUpdatingLocation();
		stopUpdatingHeading();
	}
	
	public void startUpdatingLocation()
	{
		WeakActivityHolder.get().runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				if (locationListener_ != null)
					return;

				locationListener_ = new LocationListener() {
				    public void onLocationChanged(Location location)
				    {
				        if (isBetterLocation(location, currentBestLocation)) 
				        {
				            currentBestLocation = location;
				            sendLocationUpdate(currentBestLocation);
				        }
				    }
				    public void onStatusChanged(String arg0, int arg1, Bundle arg2) {}
				    public void onProviderDisabled(String arg0) {}
				    public void onProviderEnabled(String arg0) {}
				};
				if (gps_enabled)
					locationManager_.requestLocationUpdates(LocationManager.GPS_PROVIDER, 0, (float) locThreshold, locationListener_);
				if (network_enabled)
					locationManager_.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 0, (float) locThreshold, locationListener_);

				if (gps_enabled) {
					Location location = locationManager_.getLastKnownLocation(LocationManager.GPS_PROVIDER);
					if (location != null)
						locationListener_.onLocationChanged(location);
				}
			}
		});		
	}

	public void stopUpdatingLocation()
	{
		WeakActivityHolder.get().runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				if (locationListener_ == null)
					return;

				locationManager_.removeUpdates(locationListener_);
				locationListener_ = null;
			}
		});		
	}

	public void startUpdatingHeading()
	{
		WeakActivityHolder.get().runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				if (magneticSensor == null)
					return;
		
				if (magneticListener_ != null)
					return;
				
				magneticListener_ = new SensorEventListener() {
					float[] mAcc=new float[3];
					boolean hasAcc;
					
					@Override
					public void onAccuracyChanged(Sensor sensor, int accuracy) { }
		
					@Override
					public void onSensorChanged(SensorEvent event)
					{
				        float x = event.values[0];
				        float y = event.values[1];
				        float z = event.values[2];
				        
				        if (event.sensor.getType()==Sensor.TYPE_ACCELEROMETER) {
				        	hasAcc=true;
				        	mAcc[0]=x;
				        	mAcc[1]=y;
				        	mAcc[2]=z;
				        }
				        else {
				        	double magneticHeading = (Math.atan2(x, -y) + Math.PI) * (180 / Math.PI);
				        	
				            double trueHeading=magneticHeading;
				            if (hasAcc&&SensorManager.getRotationMatrix(mMatrixR, mMatrixI,
				                    mAcc,event.values)) {
				                SensorManager.getOrientation(mMatrixR, mMatrixValues);
				                //Smoothing ?
				                trueHeading = mMatrixValues[0] * (180 / Math.PI);
				                magneticHeading = trueHeading;
				                if (currentBestLocation!=null) {
				                	GeomagneticField geomagneticField = new GeomagneticField(
				                            (float)currentBestLocation.getLatitude(),
				                            (float)currentBestLocation.getLongitude(),
				                            (float)currentBestLocation.getAltitude(),
				                            System.currentTimeMillis());
				                	trueHeading+=geomagneticField.getDeclination();
				                }
				            }
				        
				        	onHeadingChanged(magneticHeading, trueHeading, x, y, z);
				        }
					}			
				};
							
				sensorManager.registerListener(magneticListener_, magneticSensor, SensorManager.SENSOR_DELAY_NORMAL);
				if (mAccelerometer!=null)
					sensorManager.registerListener(magneticListener_, mAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
			}
		});		
	}

	public void stopUpdatingHeading()
	{
		WeakActivityHolder.get().runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				if (magneticSensor == null)
					return;
		
				if (magneticListener_ == null)
					return;
		
				sensorManager.unregisterListener(magneticListener_, magneticSensor);
				if (mAccelerometer!=null)
					sensorManager.unregisterListener(magneticListener_, mAccelerometer);
				magneticListener_=null; 
			}
		});		
	}

	private void sendLocationUpdate(Location location)
	{
		double latitude = location.getLatitude();
		double longitude = location.getLongitude();
		double altitude = location.getAltitude();
		double speed = location.hasSpeed()?location.getSpeed():-1;
		double course = location.hasBearing()?location.getBearing():-1;
		double accuracy = location.hasAccuracy()?location.getAccuracy():-1;
		int type=0;
		String provider = location.getProvider();
		if (LocationManager.GPS_PROVIDER.equals(provider)) type='P';
		if (LocationManager.NETWORK_PROVIDER.equals(provider)) type='N';
					
		onLocationChanged(latitude, longitude, altitude, speed, course,accuracy,type);
	}
	
    private static final int TWO_MINUTES = 1000 * 60 * 2;

    /** Determines whether one Location reading is better than the current Location fix
      * @param location  The new Location that you want to evaluate
      * @param currentBestLocation  The current Location fix, to which you want to compare the new one
      */
    protected boolean isBetterLocation(Location location, Location currentBestLocation) {
        if (currentBestLocation == null) {
            // A new location is always better than no location
            return true;
        }

        // Check whether the new location fix is newer or older
        long timeDelta = location.getTime() - currentBestLocation.getTime();
        boolean isSignificantlyNewer = timeDelta > TWO_MINUTES;
        boolean isSignificantlyOlder = timeDelta < -TWO_MINUTES;
        boolean isNewer = timeDelta > 0;

        // If it's been more than two minutes since the current location, use the new location
        // because the user has likely moved
        if (isSignificantlyNewer) {
            return true;
        // If the new location is more than two minutes older, it must be worse
        } else if (isSignificantlyOlder) {
            return false;
        }

        // Check whether the new location fix is more or less accurate
        int accuracyDelta = (int) (location.getAccuracy() - currentBestLocation.getAccuracy());
        boolean isLessAccurate = accuracyDelta > 0;
        boolean isMoreAccurate = accuracyDelta < 0;
        boolean isSignificantlyLessAccurate = accuracyDelta > 200;

        // Check if the old and new location are from the same provider
        boolean isFromSameProvider = isSameProvider(location.getProvider(),
                currentBestLocation.getProvider());

        // Determine location quality using a combination of timeliness and accuracy
        if (isMoreAccurate) {
            return true;
        } else if (isNewer && !isLessAccurate) {
            return true;
        } else if (isNewer && !isSignificantlyLessAccurate && isFromSameProvider) {
            return true;
        }
        return false;
    }

    /** Checks whether two providers are the same */
    private boolean isSameProvider(String provider1, String provider2) {
        if (provider1 == null) {
          return provider2 == null;
        }
        return provider1.equals(provider2);
    }

	private static native void onLocationChanged(double latitude, double longitude, double altitude, double speed, double course, double accuracy, int type);
	private static native void onHeadingChanged(double magneticHeading, double trueHeading, float magX, float magY, float magZ);
}
