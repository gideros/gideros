package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;
import com.heyzap.sdk.ads.HeyzapAds;
import com.heyzap.sdk.ads.IncentivizedAd;
import com.heyzap.sdk.ads.HeyzapAds.OnIncentiveResultListener;
import com.heyzap.sdk.ads.HeyzapAds.OnStatusListener;
import com.heyzap.sdk.ads.InterstitialAd;
import com.heyzap.sdk.ads.VideoAd;

public class AdsHeyzap implements AdsInterface, OnStatusListener, OnIncentiveResultListener{
	
	private static WeakReference<Activity> sActivity;
	private AdsManager mngr;
	private boolean interstitial;
	private boolean video;
	private boolean v4vc;
	private AdsHeyzap me;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		interstitial = false;
		video = false;
		v4vc = false;
		sActivity = activity;
		mngr = new AdsManager();
		if(android.os.Build.MANUFACTURER.equals("Amazon"))
		{
			HeyzapAds.start(sActivity.get(), HeyzapAds.DISABLE_AUTOMATIC_FETCH|HeyzapAds.AMAZON);
		}
		else
		{
			HeyzapAds.start(sActivity.get(), HeyzapAds.DISABLE_AUTOMATIC_FETCH);
		}
		HeyzapAds.setOnStatusListener(this);
		HeyzapAds.setOnIncentiveResultListener(this);
	}
	
	//on destroy event
	public void onDestroy()
	{	
		mngr.destroy();
	}
	
	public void onStart(){}

	public void onStop(){}
	
	public void onPause(){}
		
	public void onResume(){}
	
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		return false;
	}
	
	public void setKey(final Object parameters){

	}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
		SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);
		final String tag = param.get(1);
		if(type != null && type.equals("video"))
		{
			mngr.set(VideoAd.class, type, new AdsStateChangeListener(){

				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					if(tag != null)
						VideoAd.display(sActivity.get(), tag);
					else
						VideoAd.display(sActivity.get());
				}

				@Override
				public void onDestroy() {
					hideAd(type);
				}	
				@Override
				public void onHide() {
					VideoAd.dismiss();
				}	
			});
			if(tag != null)
				VideoAd.fetch(tag);
			else
				VideoAd.fetch();
		}
		else if(type != null && type.equals("v4vc")){
			mngr.set(IncentivizedAd.class, type, new AdsStateChangeListener(){

				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					IncentivizedAd.display(sActivity.get());
				}

				@Override
				public void onDestroy() {
					hideAd(type);
				}	
				@Override
				public void onHide() {
					IncentivizedAd.dismiss();
				}	
			});
			IncentivizedAd.fetch();
		}
		else
		{
			mngr.set(InterstitialAd.class, type, new AdsStateChangeListener(){

				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					if(tag != null)
						InterstitialAd.display(sActivity.get(), tag);
					else
						InterstitialAd.display(sActivity.get());
				}

				@Override
				public void onDestroy() {
					hideAd(type);
				}	
				@Override
				public void onHide() {
					InterstitialAd.dismiss();
				}	
			});
			if(tag != null)
				InterstitialAd.fetch(tag);
			else
				InterstitialAd.fetch();
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
	public void onAvailable(String tag) {
		if(InterstitialAd.isAvailable(tag))
		{
			mngr.load("interstitial");
			Ads.adReceived(this, "interstitial");
			interstitial = true;
		}
		if(VideoAd.isAvailable(tag))
		{
			mngr.load("video");
			Ads.adReceived(this, "video");
			video = true;
		}
		if(IncentivizedAd.isAvailable())
		{
			mngr.load("v4vc");
			Ads.adReceived(this, "v4vc");
			v4vc = true;
		}
	}

	@Override
	public void onClick(String arg0) {
		if(interstitial)
			Ads.adActionBegin(this, "interstitial");
		if(video)
			Ads.adActionBegin(this, "video");
		if(v4vc)
			Ads.adActionBegin(this, "v4vc");
	}

	@Override
	public void onFailedToFetch(String arg0) {
		if(mngr.get("interstitial") != null)
			Ads.adFailed(this, "interstitial", "Failed to receive");
		mngr.reset("interstitial");
		if(mngr.get("video") != null)
			Ads.adFailed(this, "video", "Failed to receive");
		mngr.reset("video");
		if(mngr.get("v4vc") != null)
			Ads.adFailed(this, "v4vc", "Failed to receive");
		mngr.reset("v4vc");
	}

	@Override
	public void onFailedToShow(String arg0) {
		if(mngr.get("interstitial") != null)
			Ads.adFailed(this, "interstitial", "Failed to show");
		mngr.reset("interstitial");
		if(mngr.get("video") != null)
			Ads.adFailed(this, "video", "Failed to show");
		mngr.reset("video");
		if(mngr.get("v4vc") != null)
			Ads.adFailed(this, "v4vc", "Failed to show");
		mngr.reset("v4vc");
	}

	@Override
	public void onHide(String arg0) {
		if(interstitial)
			Ads.adDismissed(this, "interstitial");
		interstitial = false;
		if(video)
			Ads.adDismissed(this, "video");
		video = false;
		if(v4vc)
			Ads.adDismissed(this, "v4vc");
		v4vc = false;
	}

	@Override
	public void onShow(String arg0) {
	}

	@Override
	public void enableTesting() {
		//no testing options
		
	}

	@Override
	public void onComplete() {
		Ads.adActionEnd(this, "v4vc");
	}

	@Override
	public void onIncomplete() {}

	@Override
	public void onAudioFinished() {}

	@Override
	public void onAudioStarted() {}
	
	
	
}
