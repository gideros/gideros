package com.giderosmobile.android.plugins.controller;

import java.lang.ref.WeakReference;

import android.app.Activity;

public interface GControllerInterface {

	public void onCreate(WeakReference<Activity> activity);
	
	public void onDestroy();
	
	public void onPause();
	
	public void onResume();
	
	public void vibrate(String id, long ms);
	
	public String getControllerName(String id);
}
