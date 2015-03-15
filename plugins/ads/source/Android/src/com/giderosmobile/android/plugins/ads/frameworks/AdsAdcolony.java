package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import android.app.Activity;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;

import com.jirbo.adcolony.*;

public class AdsAdcolony implements AdsInterface, AdColonyAdAvailabilityListener, AdColonyV4VCListener{
	
	private WeakReference<Activity> sActivity;
	private AdsManager mngr;
	static AdsAdcolony me;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		sActivity = activity;
		me = this;
		mngr = new AdsManager();
	}
	
	//on destroy event
	public void onDestroy(){}
	
	public void onStart(){}

	public void onStop(){}
	
	public void onPause()
	{
		AdColony.pause();
	}

	public void onResume()
	{
		AdColony.resume(sActivity.get());
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event) {
			return false;
	}
	
	public void setKey(final Object parameters){
		SparseArray<String> param = (SparseArray<String>)parameters;
		String versionName = "version:";
		try {
			versionName.concat(sActivity.get().getPackageManager().getPackageInfo(sActivity.get().getPackageName(), 0).versionName);
		} catch (NameNotFoundException e) {
			versionName.concat("1.0");
		}
		if(android.os.Build.MANUFACTURER.equals("Amazon"))
			versionName.concat(",store:amazon");
		else
			versionName.concat(",store:google");
		String zone = param.get(1);
		if(zone != null)
			AdColony.configure(sActivity.get(), versionName, param.get(0), zone);
		else
			AdColony.configure(sActivity.get(), versionName, param.get(0));
		AdColony.addV4VCListener( this );
		AdColony.addAdAvailabilityListener(this);
	}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
		SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);
		if(type.equals("video") || type.equals("auto"))
		{
			final AdColonyVideoAd videoAd;
			if(param.get(1) != null)
				videoAd = new AdColonyVideoAd(param.get(1));
			else
				videoAd = new AdColonyVideoAd();
			if(videoAd.getAvailableViews() > 0)
			{
				mngr.set(videoAd, type, new AdsStateChangeListener(){

					@Override
					public void onShow() {
						Ads.adDisplayed(me, type);
						videoAd.show();
					}

					@Override
					public void onDestroy() {}	
					@Override
					public void onHide() {}	
				});
				videoAd.withListener(new AdsAdcolonyListener(mngr.getState(type)));
				mngr.load(type);
				Ads.adReceived(me, type);
			}
			else
			{
				Ads.adFailed(this, type, "Still downloading");
			}
		}
		else if(type.equals("v4vc"))
		{
			String popup = param.get(2);
			final AdColonyV4VCAd rewardAd;
			if(param.get(1) != null)
				rewardAd = new AdColonyV4VCAd(param.get(1));
			else
				rewardAd = new AdColonyV4VCAd();
			if(popup != null && popup.equals("true"))
				rewardAd.withConfirmationDialog().withResultsDialog();
			else
				rewardAd.withResultsDialog();
			
			if(rewardAd.getAvailableViews() > 0)
			{
				mngr.set(rewardAd, type, new AdsStateChangeListener(){

					@Override
					public void onShow() {
						Ads.adDisplayed(me, type);
						rewardAd.show();
					}

					@Override
					public void onDestroy() {}	
					@Override
					public void onHide() {}	
				});
				rewardAd.withListener(new AdsAdcolonyListener(mngr.getState(type)));
				mngr.load(type);
				Ads.adReceived(me, type);
			}
			else
			{
				Ads.adFailed(this, "v4vc", "No V4VC");
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
		return 0;
	}
	
	public int getHeight(){
		return 0;
	}
	
	@Override
	public void enableTesting() {
		// is enabled inside Adcolony account
		
	}

	@Override
	public void onAdColonyV4VCReward(AdColonyV4VCReward reward) {
		if(reward.success())
	    {
			Ads.adActionEnd(this, "v4vc");
	    }
	}

	@Override
	public void onAdColonyAdAvailabilityChange(boolean available, String zone){}
}

class AdsAdcolonyListener implements AdColonyAdListener{

	private AdsState state;
	
	AdsAdcolonyListener(AdsState type){
		state = type;
	}
	
	@Override
	public void onAdColonyAdAttemptFinished(AdColonyAd ad) {
		if(ad.canceled())
			Ads.adDismissed(AdsAdcolony.me, state.getType());
		else if(ad.noFill())
			Ads.adFailed(AdsAdcolony.me, state.getType(), "no fill");
		else if(ad.notShown())
			Ads.adFailed(AdsAdcolony.me, state.getType(), "can not be shown");
		
		state.reset();			
	}

	@Override
	public void onAdColonyAdStarted(AdColonyAd arg0) {}
	
}
