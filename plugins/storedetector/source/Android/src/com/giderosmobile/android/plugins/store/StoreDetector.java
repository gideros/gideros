package com.giderosmobile.android.plugins.store;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.os.Build;

class StoreDetector
{
	private static WeakReference<Activity> sActivity;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
	}
	
	public static String getStore(){
		String installer = null;
    	try {
    	    installer = sActivity.get().getPackageManager().getInstallerPackageName(sActivity.get().getPackageName());
    	} catch (Exception e) {}
    	
    	if(installer != null)
    	{
    		if(installer.equals("com.android.vending") || installer.equals("com.google.android.feedback")){
    			return "googleplay";
    		}
    		else if(installer.equals("com.amazon.venezia")){
    			return "amazon";
    		}
    	}
    	
    	if(Build.MANUFACTURER.equals("Amazon")){
			return "amazon";
		}
		else if("cardhu".equals(Build.DEVICE) || Build.DEVICE.startsWith("ouya")){
			return "ouya";
		}
    	return "android";
	}
	
	
	public static boolean isConsole(){
		boolean ouya = ("cardhu".equals(Build.DEVICE)) || (Build.DEVICE.startsWith("ouya"));
		boolean fireTV = (Build.MODEL.startsWith("AFT"));
		return (ouya || fireTV);
	}
}