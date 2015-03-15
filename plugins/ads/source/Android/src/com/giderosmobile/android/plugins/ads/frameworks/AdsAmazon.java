package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import java.util.Hashtable;
import java.util.Timer;
import java.util.TimerTask;

import com.amazon.device.ads.Ad;
import com.amazon.device.ads.AdError;
import com.amazon.device.ads.AdLayout;
import com.amazon.device.ads.AdListener;
import com.amazon.device.ads.AdProperties;
import com.amazon.device.ads.AdRegistration;
import com.amazon.device.ads.AdSize;
import com.amazon.device.ads.AdTargetingOptions;
import com.amazon.device.ads.InterstitialAd;
import com.giderosmobile.android.plugins.ads.*;

import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

public class AdsAmazon implements AdsInterface{
	
	private static WeakReference<Activity> sActivity;
	private AdSize currentType = AdSize.SIZE_320x50;
	private static String currentName = "320x50";
	private static Timer timer;
	private static boolean timerStarted = false;
	private static AdsManager mngr;
	private Hashtable<String, AdSize> adTypes;
	
	static AdsAmazon me;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		
		//create ad types
		startTimer();
		mngr = new AdsManager();
		adTypes = new Hashtable<String, AdSize>();
		adTypes.put("300x50", AdSize.SIZE_300x50);
		adTypes.put("320x50", AdSize.SIZE_320x50);
		adTypes.put("300x250", AdSize.SIZE_300x250);
		adTypes.put("600x90", AdSize.SIZE_600x90);
		adTypes.put("728x90", AdSize.SIZE_728x90);
		adTypes.put("1024x50", AdSize.SIZE_1024x50);
	}
	
	//on destroy event
	public void onDestroy()
	{	
		mngr.destroy();
		stopTimer();
	}
	
	public void onStart(){}

	public void onStop(){}
	
	public void onPause(){
		stopTimer();
	}
		
	public void onResume(){
		startTimer();
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		return false;
	}
	
	public void setKey(final Object parameters){
		SparseArray<String> param = (SparseArray<String>)parameters;
		AdRegistration.setAppKey(param.get(0));
	}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
		SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);

		if(type.equals("interstitial")){
			final InterstitialAd interstitialAd = new InterstitialAd(sActivity.get());
			mngr.set(interstitialAd, type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					interstitialAd.showAd();
				}
				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
			interstitialAd.setListener(new AdsAmazonListener(mngr.getState(type)));
			interstitialAd.loadAd();
		}
		else if(adTypes.get(type) != null || type.equals("auto"))
		{
			AdSize newType;
			if(type.equals("auto"))
				newType = adTypes.get(getAutoSize());
			else
				newType = adTypes.get(type);
	
			//if there is an existing ad view
			//destroy it
			if(mngr.get(type) == null)
			{
				final AdLayout adView = new AdLayout(sActivity.get(), newType);
				mngr.set(adView, type, new AdsStateChangeListener(){
					@Override
					public void onShow() {
						Ads.adDisplayed(me, type);
						if(type.equals("auto"))
						{
							currentName = getAutoSize();
							currentType = adTypes.get(currentName);
						}
						else
						{
							currentName = type;
							currentType = adTypes.get(type);
						}
						Ads.addAd(AdsAmazon.me, adView, getWidth(), getHeight());
					}
					
					@Override
					public void onDestroy() {
						hideAd(type);
						adView.destroy();
					}	
					
					@Override
					public void onHide() {
						Ads.removeAd(AdsAmazon.me, adView);
						Ads.adDismissed(AdsAmazon.me, type);
					}	
				});
				mngr.setAutoKill(type, false);
				adView.setListener(new AdsAmazonListener(mngr.getState(type)));
				Ads.addAd(this, adView, getWidth(), getHeight());
				adView.loadAd();
				Ads.removeAd(this, adView);
			}
		}
		else
		{
			Ads.adError(this, "Unknown type: " + type);
		}
	}
	
	public void showAd(final Object parameters)
	{
		SparseArray<String> param = (SparseArray<String>)parameters;
		String type = param.get(0);
		if(mngr.get(type) == null)
		{
			loadAd(parameters);
		}
		mngr.show(type);
	}
	
	//remove ad
	public void hideAd(String type)
	{
		mngr.hide(type);
	}
	
	public int getWidth(){
		return (int) Ads.DipsToPixels(currentType.getWidth());
	}
	
	public int getHeight(){
		return (int) Ads.DipsToPixels(currentType.getHeight());
	}

	private String getAutoSize() {
		String[] typearray = {"1024x50", "728x90", "600x90", "320x50", "300x50"};
        int[][] maparray = { { 0, 1024, 50 }, 
        		{1, 728, 90 }, 
        		{2, 600, 90 },
        		{3, 320, 50 },
        		{4, 300, 50 }};

        for (int i = 0; i < maparray.length; i++) {
                if (Ads.DipsToPixels(maparray[i][1]) <= Ads.screenWidth
                        && Ads.DipsToPixels(maparray[i][2]) <= Ads.screenHeight) {
                    return typearray[maparray[i][0]];
                }
            }
        return "300x50";
    }
	
	public static synchronized void startTimer()
	{
		if(!timerStarted)
		{
			timerStarted = true;
			try{
				timer = new Timer();
				TimerTask refreshTask = new TimerTask () {
					@Override
					public void run () {
						try{
							sActivity.get().runOnUiThread(new Runnable() 
							{
								public void run() 
								{
									try{
										if(mngr.get(currentName) != null)
											((AdLayout) mngr.get(currentName)).loadAd(new AdTargetingOptions());
									}catch(Exception e){}
								}
							});
						}catch(Exception e){
							timerStarted = false;
							timer = null;
						}
					}
				};
				timer.schedule(refreshTask, 0, 1000*40);
			}catch(Exception e){
				timerStarted = false;
				timer = null;
			}
		}
	}
	
	public static synchronized void stopTimer()
	{
		if(timerStarted)
		{
			timerStarted = false;
			timer.cancel();
			timer = null;
		}
	}

	@Override
	public void enableTesting() {
		AdRegistration.enableTesting(true);
	}
	
}

class AdsAmazonListener implements AdListener{
	
	private AdsState state;
	
	AdsAmazonListener(AdsState type){
		state = type;
	}
	
	@Override
	public void onAdCollapsed(Ad arg0) {
		Ads.adActionEnd(AdsAmazon.me, state.getType());		
	}

	@Override
	public void onAdExpanded(Ad arg0) {
		Ads.adActionBegin(AdsAmazon.me, state.getType());		
	}

	@Override
	public void onAdFailedToLoad(Ad arg0, AdError arg1) {
		Ads.adFailed(AdsAmazon.me, state.getType(), arg1.getMessage());
		AdsAmazon.me.hideAd(state.getType());
		state.reset();
	}

	@Override
	public void onAdLoaded(Ad arg0, AdProperties arg1) {
		state.load();
		Ads.adReceived(AdsAmazon.me, state.getType());
	}

	@Override
	public void onAdDismissed(Ad arg0) {
		Ads.adDismissed(AdsAmazon.me, state.getType());
	}
}
