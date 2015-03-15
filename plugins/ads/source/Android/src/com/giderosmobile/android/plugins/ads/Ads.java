package com.giderosmobile.android.plugins.ads;

import java.lang.ref.WeakReference;
import java.util.Hashtable;

import android.app.Activity;
import android.content.Context;
import android.net.ConnectivityManager;
import android.util.DisplayMetrics;
import android.util.SparseArray;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.widget.FrameLayout;

public class Ads {
	
	private static WeakReference<Activity> sActivity;
	private static FrameLayout layout;
	private static FrameLayout.LayoutParams params;
	private static Object lock = new Object();
	private static long sData = 0;
	private static Hashtable<String, AdsInterface> ads;
	private static Hashtable<String, AdsGLayout> pos;
	public static int screenWidth;
	public static int screenHeight;
	public static double screenDensity;
	
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
		
		ads = new Hashtable<String, AdsInterface>();
		pos = new Hashtable<String, AdsGLayout>();
	
		params = new FrameLayout.LayoutParams(
				FrameLayout.LayoutParams.WRAP_CONTENT,
				FrameLayout.LayoutParams.WRAP_CONTENT);
		params.gravity = Gravity.LEFT | Gravity.TOP;
		
		DisplayMetrics metrics = new DisplayMetrics();
		sActivity.get().getWindowManager().getDefaultDisplay().getMetrics(metrics);

		screenHeight = metrics.heightPixels;
		screenWidth = metrics.widthPixels;
		screenDensity = metrics.density;
		
		layout = (FrameLayout)sActivity.get().getWindow().getDecorView();
	}
	
	//on destroy event
	public static void onDestroy()
	{	
		for (AdsInterface value : ads.values()) {
			value.onDestroy();
		}
		ads.clear();
	}
	
	//on destroy event
	public static void onStart()
	{	
		for (AdsInterface value : ads.values()) {
			value.onStart();
		}
	}
	
	//on destroy event
	public static void onStop()
	{	
		for (AdsInterface value : ads.values()) {
			value.onStop();
		}
	}
	
	public static void onPause()
	{	
		for (AdsInterface value : ads.values()) {
			value.onPause();
		}
	}
		
	public static void onResume()
	{	
		for (AdsInterface value : ads.values()) {
			value.onResume();
		}
	}
	
	public static boolean onKeyUp(final int keyCode, final KeyEvent event) {
		boolean res = false;
		for (AdsInterface value : ads.values()) {
			if(value.onKeyUp(keyCode, event))
			{
				res = true;
				break;
			}
		}
		return res;
	}
	
	public static void addAd(Object caller, final View view){
		String adName = modifyName(getCallerName(caller));
		if(view != null && view.getParent() == null)
		{
			layout.addView(view);
		}
		if(!pos.containsKey(adName))
		{
			pos.put(adName, new AdsGLayout());
		}
		AdsGLayout l = pos.get(adName);
		l.adView = view;
		applyParams(adName);
	}
	
	public static void addAd(Object caller, final View view, int width, int height){
		String adName = modifyName(getCallerName(caller));
		if(view != null && view.getParent() == null)
		{
			layout.addView(view);
		}
		if(!pos.containsKey(adName))
		{
			pos.put(adName, new AdsGLayout());
		}
		AdsGLayout l = pos.get(adName);
		l.width = width;
		l.height = height;
		l.adView = view;
		applyParams(adName);
	}
	
	public static void applyParams(final String adp){
		final AdsGLayout l = pos.get(adp);
		if(l != null && l.adView != null)
		{
			if(screenWidth - l.x < getWidth(adp) || screenHeight - l.y < getHeight(adp))
			{
				String need = getWidth(adp) + "x" + getHeight(adp);
				String has = (screenWidth - l.x) + "x" + (screenHeight - l.y);
				if(sData != 0)
					Ads.onAdError(adp.toLowerCase(), "Ad does not have enough space. Need " + need + ", has " + has, sData);
			}
			try
			{
				sActivity.get().runOnUiThread(new Runnable() {
					@Override
					public void run() {
						params.setMargins(l.x, l.y, 0, 0);
						params.width = l.width;
						params.height = l.height;
						l.adView.setLayoutParams(params);
					}
				});
			}
			catch(Exception ex)	{}
		}
	}
	
	public static void removeAd(Object caller, final View view){
		String adName = modifyName(getCallerName(caller));
		if(pos.containsKey(adName))
		{
			pos.remove(adName);
		}
		if(view.getParent() != null)
		{
			layout.removeView(view);
		}
	}
	
	public static void init(long data){
		sData = data;
	}
	
	public static void cleanup()
	{
		sData = 0;
		try
		{
			Runnable myRunnable = new Runnable() {
				@Override
				public void run() {
					for (AdsInterface value : ads.values()) {
						value.onDestroy();
					}
					ads.clear();
				}
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static void initialize(String adprovider){
		final String adp = modifyName(adprovider);
		if(ads.get(adp) == null)
		{
			String className = "com.giderosmobile.android.plugins.ads.frameworks.Ads"+adp;
			Class classz = null;
			try {
				classz = Class.forName(className);
			} catch (ClassNotFoundException e) {
				e.printStackTrace();
			}
			try {
				ads.put(adp, (AdsInterface)classz.newInstance());
			} catch (InstantiationException e) {
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
			}

			try
			{
				Runnable myRunnable = new Runnable() {
					@Override
					public void run() {
						ads.get(adp).onCreate(sActivity);
					}
					
				};
				sActivity.get().runOnUiThread(myRunnable) ;
			}
			catch(Exception ex)	{}
		}
	}
	
	public static void destroy(String adprovider){
		final String adp = modifyName(adprovider);
		if(ads.get(adp) != null)
		{
			try
			{
				Runnable myRunnable = new Runnable() {
					@Override
					public void run() {
						if(ads.containsKey(adp))
						{
							ads.get(adp).onDestroy();
							ads.remove(adp);
						}
					}
				};
				sActivity.get().runOnUiThread(myRunnable) ;
			}
			catch(Exception ex)	{}
		}
	}
	
	public static void enableTesting(String adprovider){
		final String adp = modifyName(adprovider);
		try
		{
			Runnable myRunnable = new Runnable() {
				@Override
				public void run() {
					if(ads.containsKey(adp))
					{
						ads.get(adp).enableTesting();
					}
				}
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static void setKey(String adprovider, final Object parameters){
		final String adp = modifyName(adprovider);
		try
		{
			Runnable myRunnable = new Runnable() {
				@Override
				public void run() {
					if(ads.containsKey(adp))
					{
						ads.get(adp).setKey(parameters);
					}
				}
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	//load an Ad
		public static void loadAd(final String adprovider, final Object parameters){
			if(!hasConnection())
	        {
	            SparseArray<String> param = (SparseArray<String>)parameters;
	            adFailed(adprovider, "No Internet Connection", param.get(0));
	            return;
	        }
			final String adp = modifyName(adprovider);
			try
			{	
				// Non UI thread
				Runnable myRunnable = new Runnable(){
					
					@Override
					public void run() {
						if(ads.containsKey(adp))
						{
							ads.get(adp).loadAd(parameters);
						}
					}
					
				};
				sActivity.get().runOnUiThread(myRunnable) ;
			}
			catch(Exception ex)	{}
		}
		
	public static void showAd(final String adprovider, final Object parameters){
		if(!hasConnection())
        {
            SparseArray<String> param = (SparseArray<String>)parameters;
            adFailed(adprovider, "No Internet Connection", param.get(0));
            return;
        }
		final String adp = modifyName(adprovider);
		try
		{	
			// Non UI thread
			Runnable myRunnable = new Runnable(){
				
				@Override
				public void run() {
					if(ads.containsKey(adp))
					{
						ads.get(adp).showAd(parameters);
					}
					synchronized ( this )
					{
						this.notify();
					}
				}
				
			};
			synchronized( myRunnable ) {
				sActivity.get().runOnUiThread(myRunnable) ;
				
				try {
					myRunnable.wait(100) ;
				} catch (InterruptedException e) {
				}
			}
		}
		catch(Exception ex)	{}
	}
	
	//remove ad
	public static void hideAd(String adprovider, final String type){
		final String adp = modifyName(adprovider);
		try
		{
			Runnable myRunnable = new Runnable() {
				@Override
				public void run() {
					if(ads.containsKey(adp))
					{
						ads.get(adp).hideAd(type);
					}
				}
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex){}
	}
	
	public static void setPosition(String adprovider, int x, int y){
		params.gravity = 0;
		params.gravity = Gravity.LEFT | Gravity.TOP;
		
		final String adp = modifyName(adprovider);
		if(!pos.containsKey(adp))
		{
			pos.put(adp, new AdsGLayout());
		}
		AdsGLayout l = pos.get(adp);
		l.x = x;
		l.y = y;
		Runnable myRunnable = new Runnable() {
			@Override
			public void run() {
				applyParams(adp);
			}
		};
		sActivity.get().runOnUiThread(myRunnable) ;
	}
	
	//set alignment of ad (both horizontal and vertical)
	public static void setAlignment(String adprovider, final String hor, final String ver){
			
		/*int x = 0;
		int y = 0;
			
		if(hor.equals("center"))
			x = (int) Math.floor((screenWidth - Ads.getWidth(adprovider))/2);
		else if(hor.equals("right"))
			x = screenWidth - Ads.getWidth(adprovider);
			
		if(ver.equals("center"))
			y = (int) Math.floor((screenHeight - Ads.getHeight(adprovider))/2);
		else if(ver.equals("bottom"))
			y = screenHeight - Ads.getHeight(adprovider);
		setPosition(adprovider, x, y);*/
		
		params.gravity = 0;
		
		if(hor.equals("center"))
			params.gravity |= Gravity.CENTER_HORIZONTAL;
		else if(hor.equals("right"))
			params.gravity |= Gravity.RIGHT;
		else
			params.gravity |= Gravity.LEFT;
			
		if(ver.equals("center"))
			params.gravity |= Gravity.CENTER_VERTICAL;
		else if(ver.equals("bottom"))
			params.gravity |= Gravity.BOTTOM;
		else
			params.gravity |= Gravity.TOP;
		
		final String adp = modifyName(adprovider);
		if(!pos.containsKey(adp))
		{
			pos.put(adp, new AdsGLayout());
		}
		AdsGLayout l = pos.get(adp);
		l.x = 0;
		l.y = 0;
		Runnable myRunnable = new Runnable() {
			@Override
			public void run() {
				applyParams(adp);
			}
		};
		sActivity.get().runOnUiThread(myRunnable) ;
	}
	
	public static void setX(String adprovider, int x){
		String adp = modifyName(adprovider);
		if(!pos.containsKey(adp))
		{
			pos.put(adp, new AdsGLayout());
		}
		AdsGLayout l = pos.get(adp);
		setPosition(adprovider, x, l.y);
	}
	
	public static void setY(String adprovider, int y){
		String adp = modifyName(adprovider);
		if(!pos.containsKey(adp))
		{
			pos.put(adp, new AdsGLayout());
		}
		AdsGLayout l = pos.get(adp);
		setPosition(adprovider, l.x, y);
	}
	
	public static int getX(String adprovider){
		String adp = modifyName(adprovider);
		if(pos.containsKey(adp))
		{
			AdsGLayout l = pos.get(adp);
			return l.x;
		}
		return 0;
	}
	
	public static int getY(String adprovider){
		String adp = modifyName(adprovider);
		if(pos.containsKey(adp))
		{
			AdsGLayout l = pos.get(adp);
			return l.y;
		}
		return 0;
	}
	
	public static int getWidth(String adprovider){
		String adp = modifyName(adprovider);
		int width = 0;
		if(ads.containsKey(adp))
		{
			width = ads.get(adp).getWidth();
		}
		return width;
	}
	
	public static int getHeight(String adprovider){
		String adp = modifyName(adprovider);
		int height = 0;
		if(ads.containsKey(adp))
		{
			height = ads.get(adp).getHeight();
		}
		return height;
	}

	public static void adReceived(Object caller, String adType){
		if (sData != 0)
			onAdReceived(getCallerName(caller), adType, sData);
	}
	
	public static void adFailed(Object caller, String adType, String error){
		if (sData != 0)
			onAdFailed(getCallerName(caller), adType, error, sData);
	}
	
	public static void adActionBegin(Object caller, String adType){
		if (sData != 0)
			onAdActionBegin(getCallerName(caller), adType, sData);
	}
	
	public static void adActionEnd(Object caller, String adType){
		if (sData != 0)
			onAdActionEnd(getCallerName(caller), adType, sData);
	}
	
	public static void adDismissed(Object caller, String adType){
		if (sData != 0)
			onAdDismissed(getCallerName(caller), adType, sData);
	}
	
	public static void adDisplayed(Object caller, String adType){
		if (sData != 0)
			onAdDisplayed(getCallerName(caller), adType, sData);
	}
	
	public static void adError(Object caller, String error){
		if (sData != 0)
			onAdError(getCallerName(caller), error, sData);
	}
	
	
	private static native void onAdReceived(String ad, String adType, long data);
	private static native void onAdFailed(String ad, String adType, String error, long data);
	private static native void onAdActionBegin(String ad, String adType, long data);
	private static native void onAdActionEnd(String ad, String adType, long data);
	private static native void onAdDismissed(String ad, String adType, long data);
	private static native void onAdDisplayed(String ad, String adType, long data);
	private static native void onAdError(String ad, String error, long data);
	
	private static String modifyName(String adprovider){
		return adprovider.substring(0,1).toUpperCase() + adprovider.substring(1).toLowerCase();
	}
	
	private static String getCallerName(Object cls){
		 String name = cls.getClass().getName();
         name = name.replace("com.giderosmobile.android.plugins.ads.frameworks.Ads", "");
         if(name.contains("$"))
        	 name = name.substring(0, name.indexOf("$"));
         return name.toLowerCase();
	}
	
	private static boolean hasConnection() {
        ConnectivityManager conMgr = (ConnectivityManager)sActivity.get().getSystemService(Context.CONNECTIVITY_SERVICE);
        // ARE WE CONNECTED TO THE NET
        if (conMgr != null && conMgr.getActiveNetworkInfo() != null
                && conMgr.getActiveNetworkInfo().isAvailable()
                && conMgr.getActiveNetworkInfo().isConnected()) {

            return true;
        }
        return false;
	}
	
	public static int DipsToPixels(int dip){
		return (int)TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dip, sActivity.get().getResources().getDisplayMetrics());
	}
	
}

class AdsGLayout{
	public int x = 0;
	public int y = 0;
	public int width = FrameLayout.LayoutParams.WRAP_CONTENT;
	public int height = FrameLayout.LayoutParams.WRAP_CONTENT;
	public View adView = null;
}
