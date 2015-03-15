package com.giderosmobile.android.plugins.ads;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.view.KeyEvent;

public interface AdsInterface {
	
	public void onCreate(WeakReference<Activity> activity);
	
	public void onDestroy();
	
	public void onStart();
	
	public void onStop();
	
	public void onPause();
		
	public void onResume();
	
	public boolean onKeyUp(int keyCode, KeyEvent event);
	
	public void setKey(final Object parameters);
	
	public void enableTesting();
	
	public void loadAd(final Object parameters);
	
	public void showAd(final Object parameters);
	
	public void hideAd(String type);
	
	public int getWidth();
	
	public int getHeight();
}
