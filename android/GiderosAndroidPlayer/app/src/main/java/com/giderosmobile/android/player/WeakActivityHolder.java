package com.giderosmobile.android.player;

import java.lang.ref.WeakReference;

import android.app.Activity;

public class WeakActivityHolder
{
	private static WeakReference<Activity> weakActivity_;	
	
	static public void set(Activity activity)
	{
		weakActivity_ =  new WeakReference<Activity>(activity);
	}
	
	static public Activity get()
	{
		return weakActivity_.get();
	}
}
