package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.chartboost.sdk.CBLocation;
import com.chartboost.sdk.Chartboost;
import com.chartboost.sdk.ChartboostDelegate;
import com.chartboost.sdk.Model.CBError.CBImpressionError;
import com.giderosmobile.android.plugins.ads.*;

public class AdsChartboost extends ChartboostDelegate implements AdsInterface{
	
	private WeakReference<Activity> sActivity;
	private String adsID;
	private AdsManager mngr;
	static AdsChartboost me;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		mngr = new AdsManager();
	}
	
	//on destroy event
	public void onDestroy()
	{	
		Chartboost.onDestroy(sActivity.get());
		mngr.destroy();
	}
	
	public void onStart(){}

	public void onStop()
	{
		Chartboost.onStop(sActivity.get());
	}
	
	public void onPause(){
		Chartboost.onPause(sActivity.get());
	}
		
	public void onResume(){
		Chartboost.onResume(sActivity.get());
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		if (Chartboost.onBackPressed())
			// If a Chartboost view exists, close it and return
			return true;
		else
			// If no Chartboost view exists, continue on as normal
			return false;
	}
	
	public void setKey(final Object parameters){
		SparseArray<String> param = (SparseArray<String>)parameters;
		adsID = param.get(0);
		Chartboost.startWithAppId(sActivity.get(), adsID, param.get(1));
		Chartboost.setImpressionsUseActivities(true);
		Chartboost.setDelegate(this);
		Chartboost.onCreate(sActivity.get());
		Chartboost.onStart(sActivity.get());
		Chartboost.setShouldDisplayLoadingViewForMoreApps(true);
	}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
		SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);
		String second = param.get(1);
		if(second == null)
			second = CBLocation.LOCATION_DEFAULT;
		final String tag = second;
		if(type.equals("interstitial")){
			mngr.set(Chartboost.class, type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					Chartboost.showInterstitial(tag);
				}

				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
			if(Chartboost.hasInterstitial(tag)){
				mngr.load(type);
				Ads.adReceived(me, type);
			}
			else
				Chartboost.cacheInterstitial(tag);
		}
		else if(type.equals("moreapps")){
			mngr.set(Chartboost.class, type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					Chartboost.showMoreApps(tag);
				}

				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
			if(Chartboost.hasMoreApps(tag)){
				mngr.load(type);
				Ads.adReceived(me, type);
			}
			else
				Chartboost.cacheMoreApps(tag);
		}
		else if(type.equals("v4vc")){
			mngr.set(Chartboost.class, type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					Chartboost.showRewardedVideo(tag);
				}

				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
			if(Chartboost.hasRewardedVideo(tag)){
				mngr.load(type);
				Ads.adReceived(me, type);
			}
			else
				Chartboost.cacheRewardedVideo(tag);
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
	public void didDismissInterstitial(String arg0) {
		Ads.adDismissed(this, "interstitial");
	}

	@Override
	public void didDismissMoreApps(String tag) {
		Ads.adDismissed(this, "moreapps");
	}
	
	 @Override
     public void didDismissRewardedVideo(String location) {
		 Ads.adDismissed(this, "v4vc");
     }
	
	@Override
	public void didCloseMoreApps(String tag) {
		Ads.adActionEnd(this, "moreapps");
	}

	
	@Override
	public void didCloseInterstitial(String arg0) {
		Ads.adActionEnd(this, "interstitial");
	}
	
	@Override
    public void didCompleteRewardedVideo(String location, int reward) {
       Ads.adActionEnd(this, "v4vc");
    }
	
	@Override
	public void didCacheInterstitial(String arg0) {
		if(mngr.get("interstitial") != null && !mngr.isLoaded("interstitial")){
			mngr.load("interstitial");
			Ads.adReceived(this, "interstitial");
		}
	}

	@Override
	public void didCacheMoreApps(String tag) {
		if(mngr.get("moreapps") != null && !mngr.isLoaded("moreapps")){
			mngr.load("moreapps");
			Ads.adReceived(this, "moreapps");
		}
	}
	
	@Override
    public void didCacheRewardedVideo(String location) {
		if(mngr.get("v4vc") != null && !mngr.isLoaded("v4vc")){
			mngr.load("v4vc");
			Ads.adReceived(this, "v4vc");
		}
    }

	@Override
	public void didClickInterstitial(String arg0) {
		Ads.adActionBegin(this, "interstitial");
	}

	@Override
	public void didClickMoreApps(String tag) {
		Ads.adActionBegin(this, "moreapps");
	}
	
	@Override
	public boolean shouldRequestInterstitial(String arg0) {
		return true;
	}

	@Override
	public boolean shouldDisplayInterstitial(String arg0) {
		return true;
	}

	@Override
	public boolean shouldDisplayMoreApps(String tag) {
		return true;
	}
	
	@Override
	public boolean shouldRequestMoreApps(String tag) {
		return true;
	}
	
	@Override
    public boolean shouldDisplayRewardedVideo(String location) {
        return true;
    }

	@Override
	public void enableTesting() {
		//should be enabled inside chartboost account
		//String android_id = Secure.getString(sActivity.get().getBaseContext().getContentResolver(),Secure.ANDROID_ID);
		//Log.e("Chartboost", android_id);
	}

	@Override
	public void didFailToLoadInterstitial(String arg0, CBImpressionError error) {
		Ads.adFailed(this, "interstitial", error.name());
		mngr.reset("interstitial");
	}

	@Override
	public void didFailToLoadMoreApps(String tag, CBImpressionError error) {
		Ads.adFailed(this, "moreapps", error.name());
		mngr.reset("moreapps");
	}
	
	@Override
    public void didFailToLoadRewardedVideo(String location, CBImpressionError error) {
		Ads.adFailed(this, "v4vc", error.name());
		mngr.reset("v4vc");
    }
}
