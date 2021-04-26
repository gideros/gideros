package com.giderosmobile.android.plugins.mapplugin;

import java.lang.ref.WeakReference;
//import java.lang.reflect.Method;
//import java.util.Hashtable;
//import java.util.Map;

import android.app.Activity;
import android.content.Intent;
//import android.content.pm.PackageManager.NameNotFoundException;
//import android.os.Bundle;
import androidx.fragment.app.FragmentActivity;
//import android.util.SparseArray;

public class MapPluginJava {
	
	private static WeakReference<Activity> sActivity;
	private static FragmentActivity parentFragmentActivity = null;
	private static long sData;
	private static MapOverlay mMap = null;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
		parentFragmentActivity = (FragmentActivity) activity;
		mMap = new MapOverlay(parentFragmentActivity);
	}
	
	public static void onDestroy()
	{	
		cleanup();
	}
	
	public static void onStart()
	{	
	}
	
	public static void onActivityResult(final int requestCode, final int resultCode, final Intent data)
	{	
	}
	
	public static void init(long data)
	{
		sData = data;
	}
	
	public static void cleanup(){
		sData = 0;
	}
	
	public static void initialize()
	{
	}
	
	public static void destroy(String iabtype)
	{
	}
	
	public static void setDimensions(final int width, final int height)
	{
		try
		{	
			// Non UI thread
			Runnable myRunnable = new Runnable(){
				
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.setDimensions(width, height);
					}
				}
				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}

	public static void setType(final int type)
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.setType(type);
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}

	public static void setZoom(final int zoom)
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.setZoom(zoom);
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static void hide()
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.hide();
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static void show()
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.show();
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}

	public static void clear()
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.clear();
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}

	public static void setPosition(final int x, final int y)
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.setPosition(x, y);
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	public static int addMarker(final double lat, final double lon, final String title)
	{
		int new_marker_index = mMap.getNextAddedMarkerIndex();
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.addMarker(lat, lon, title);
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
		return new_marker_index;
	}
	
	public static void addMarkerAtIndex(final double lat, final double lon, final String title, final int index)
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.addMarkerAtIndex(lat, lon, title, index);
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static void setMarkerHue(final int idx, final double hue)
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.setMarkerHue(idx, (float) hue);
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}

	public static void setMarkerAlpha(final int idx, final double alpha)
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.setMarkerAlpha(idx, (float) alpha);
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}

	public static void setMarkerCoordinates(final int idx, final double lat, final double lon)
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.setMarkerCoordinates(idx, lat, lon);
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}

	public static void setCenterCoordinates(final double lat, final double lon)
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.setCenterCoordinates(lat, lon);
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}

	public static void setMarkerTitle(final int idx, final String title)
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.setMarkerTitle(idx, title);
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static int mapClicked()
	{
		try
		{	
			Runnable myRunnable = new Runnable(){
				@Override
				public void run() {
					if (mMap != null)
					{
						mMap.setClickListener();
						mMap.update();
					}
				}				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
		

		
		if (mMap.mapClicked())
			return 1;
		return 0;
	}

	public static double getMapClickLatitude()
	{
		return (mMap.getMapClickLatitude());
	}

	public static double getMapClickLongitude()
	{
		return (mMap.getMapClickLongitude());
	}

	public static double getCenterLatitude()
	{
		return (mMap.getCenterLatitude());
	}

	public static double getCenterLongitude()
	{
		return (mMap.getCenterLongitude());
	}

	public static String getMarkerTitle(int idx)
	{
		return (mMap.getMarkerTitle(idx));
	}

	public static double getMarkerLatitude(int idx)
	{
		return (mMap.getMarkerLatitude(idx));
	}

	public static double getMarkerLongitude(int idx)
	{
		return (mMap.getMarkerLongitude(idx));
	}
	
	public static int getClickedMarkerIndex()
	{
		return (mMap.getClickedMarkerIndex());
	}
	
}
