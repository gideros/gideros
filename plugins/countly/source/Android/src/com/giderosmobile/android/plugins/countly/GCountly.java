package com.giderosmobile.android.plugins.countly;

import java.lang.ref.WeakReference;
import java.util.Map;

import ly.count.android.api.Countly;

import android.app.Activity;
import android.util.Log;

public class GCountly
{
	private static WeakReference<Activity> sActivity;
	private static volatile boolean sIsActive = false;
	
	public static void onCreate(Activity activity)
	{
		sActivity = new WeakReference<Activity>(activity);
	}

	public static void onStart()
	{
		if (sIsActive)
			Countly.sharedInstance().onStart();
	}

	public static void onStop()
	{
		if (sIsActive)
			Countly.sharedInstance().onStop();
	}

	public static void startSession(String apiKey)
	{
		if (!sIsActive)
		{
			Countly.sharedInstance().init(sActivity.get(), "https://cloud.count.ly", apiKey);
			Countly.sharedInstance().onStart();
			sIsActive = true;
		}
	}

	public static void logEvent(String eventId, int count, double sum, Map<String, String> parameters)
	{
		if(parameters != null && sum != 0)
			Countly.sharedInstance().recordEvent(eventId, parameters, count, sum);
		else if(parameters != null)
			Countly.sharedInstance().recordEvent(eventId, parameters, count);
		else if(sum != 0)
			Countly.sharedInstance().recordEvent(eventId, count, sum);
		else
			Countly.sharedInstance().recordEvent(eventId, count);
	}
}
