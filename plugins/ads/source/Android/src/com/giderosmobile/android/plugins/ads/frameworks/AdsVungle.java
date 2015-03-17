package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;
import com.vungle.sdk.VunglePub;


public class AdsVungle implements AdsInterface, VunglePub.EventListener{
	
	private WeakReference<Activity> sActivity;
	private AdsManager mngr;
	private boolean hasVideo;
	private boolean hasV4VC;
	private static AdsVungle me;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		hasVideo = false;
		hasV4VC = false;
		me = this;
		sActivity = activity;
		mngr = new AdsManager();
	}
	
	//on destroy event
	public void onDestroy(){}
	
	public void onStart(){}

	public void onStop(){}
	
	public void onPause()
	{
		 VunglePub.onPause();
	}

	public void onResume()
	{
		 VunglePub.onResume();
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event) {
			return false;
	}
	
	public void setKey(final Object parameters){
		SparseArray<String> param = (SparseArray<String>)parameters;
		VunglePub.init(sActivity.get(), param.get(0));
		VunglePub.setEventListener(this);
	}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
		final SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);
		if(type.equals("video"))
		{
			if(VunglePub.isVideoAvailable())
			{
				mngr.set(VunglePub.class, type, new AdsStateChangeListener(){

					@Override
					public void onShow() {
						Ads.adDisplayed(me, type);
						hasVideo = true;
						VunglePub.displayAdvert();
					}

					@Override
					public void onDestroy() {}	
					@Override
					public void onHide() {}	
				});
				mngr.load(type);
			}
			else
			{
				Ads.adFailed(this, type, "No video available");
			}
		}
		else if(type.equals("v4vc"))
		{
			mngr.set(VunglePub.class, type, new AdsStateChangeListener(){

				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					hasV4VC = true;
					String username = "Player";
					if(param.size() >= 2)
						username = param.get(1);
					final boolean wasAdPlayed = VunglePub.displayIncentivizedAdvert(username, true);
					if(!wasAdPlayed)
						Ads.adFailed(AdsVungle.me, type, "No V4VC");
				}

				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
			mngr.load(type);
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
		return 0;
	}
	
	public int getHeight(){
		return 0;
	}

	@Override
	public void enableTesting() {
		
	}

	@Override
	public void onVungleAdEnd() {
		if(hasVideo)
			Ads.adDismissed(this, "video");
		hasVideo = false;
		if(hasV4VC)
			Ads.adDismissed(this, "v4vc");
		hasV4VC = false;
	}

	@Override
	public void onVungleAdStart() {
		if(hasVideo)
			Ads.adReceived(this, "video");
		if(hasV4VC)
			Ads.adReceived(this, "v4vc");
	}

	@Override
	public void onVungleView(double watchedSeconds, double totalAdSeconds) {
        final double watchedPercent = watchedSeconds / totalAdSeconds;
        if (watchedPercent >= 0.8) {
            if(hasVideo)
            	Ads.adActionEnd(this, "video");
    		hasVideo = false;
    		if(hasV4VC)
    			Ads.adActionEnd(this, "v4vc");
    		hasV4VC = false;
        }
    }

}
