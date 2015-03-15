package com.giderosmobile.android.plugins.notification;

import android.content.Context;

import com.google.android.gcm.*;

public class GCMReceiver extends GCMBroadcastReceiver { 
	  @Override
	  protected String getGCMIntentServiceClassName(Context context) { 
		  return "com.giderosmobile.android.plugins.notification.GCMIntentService"; 
	  } 
}