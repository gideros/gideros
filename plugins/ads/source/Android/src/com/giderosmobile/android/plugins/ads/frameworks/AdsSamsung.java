package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import java.util.Hashtable;

import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;
import com.sec.android.ad.*;
import com.sec.android.ad.info.*;
import com.sec.android.ad.vast.AdHubVideoPlayer;
import com.sec.android.ad.vast.AdVideoListener;

public class AdsSamsung implements AdsInterface, AdInterstitialListener, AdVideoListener{
	
	private WeakReference<Activity> sActivity;
	private String adsID = "";
	private AdSize currentType = AdSize.BANNER_320x50;
	private String currentName = "small_banner";
	private AdsManager mngr;
	
	
	static AdsSamsung me;
	
	//all AdMob banner types
	private static Hashtable<String, AdSize> adTypes;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		
		//create ad types
		mngr = new AdsManager();
		adTypes = new Hashtable<String, AdSize>();
		adTypes.put("small_banner", AdSize.BANNER_320x50);
		adTypes.put("medium_banner", AdSize.BANNER_640x100);
		adTypes.put("rectangle_banner", AdSize.TABLET_300x250);
		adTypes.put("large_banner", AdSize.TABLET_728x90);
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
		SparseArray<String> param = (SparseArray<String>)parameters;
		adsID = param.get(0);
	}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
		if(!adsID.equals(""))
		{
			SparseArray<String> param = (SparseArray<String>)parameters;
			final String type = param.get(0);
			String placeId = null;
			if(param.size() >= 2)
			{
				placeId = adsID;
				adsID = param.get(1);
			}

			if(type.equals("interstitial"))
			{
				final AdHubInterstitial adHubInterstitial = new AdHubInterstitial(sActivity.get(), adsID);
				mngr.set(adHubInterstitial, type, new AdsStateChangeListener(){
					@Override
					public void onShow() {
						Ads.adDisplayed(me, type);
						adHubInterstitial.startAd();
					}
					@Override
					public void onDestroy() {}	
					@Override
					public void onHide() {}	
				});
				adHubInterstitial.setListener(this);
			}
			else if(type.equals("video"))
			{
				final AdHubVideoPlayer video = new AdHubVideoPlayer(sActivity.get(), adsID);
				mngr.set(video, type, new AdsStateChangeListener(){
					@Override
					public void onShow() {
						Ads.adDisplayed(me, type);
						video.startAdAppLaunchRoll();
					}
					@Override
					public void onDestroy() {}	
					@Override
					public void onHide() {}	
				});
				video.setListener(this);
				
			}
			else
			{
				if(adTypes.get(type) != null || type.equals("auto"))
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
						final AdHubView adView = new AdHubView(sActivity.get());
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
								Ads.addAd(AdsSamsung.me, adView);
							}
							@Override
							public void onDestroy() {
								hideAd(type);
							}	
							@Override
							public void onHide() {
								Ads.removeAd(AdsSamsung.me, adView);
								Ads.adDismissed(AdsSamsung.me, type);
							}	
						});
						mngr.setAutoKill(type, false);
						adView.init(sActivity.get(), adsID, newType);
						adView.setListener(new AdsSamsungListener(mngr.getState(type)));
						adView.startAd();
					}
				}
				else
				{
					Ads.adError(this, "Unknown type: " + type);
				}
			}
			if(placeId != null)
			{
				adsID = placeId;
			}
		}
		else
		{
			Ads.adError(this, "Provide Inventory ID");
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

	
	@Override
	public void enableTesting() {
		
	}
	
	private String getAutoSize() {
		String[] arr = {"large_banner", "medium_banner", "small_banner"};
        int[][] maparray = { { 728, 90 }, {
            640, 100 }, {
            320, 50 } };

        for (int i = 0; i < maparray.length; i++) {
                if (maparray[i][0] * Ads.screenDensity <= Ads.screenWidth
                        && maparray[i][1] * Ads.screenDensity <= Ads.screenHeight) {
                    return arr[i];
                }
            }
        return "small_banner";
    }

	@Override
	public void onAdInterstitialClosed(AdHubInterstitial arg0) {
		Ads.adDismissed(this, "interstitial");
		
	}

	@Override
	public void onAdInterstitialFailed(AdHubInterstitial arg0, Exception error) {
		mngr.reset("interstitial");
		Ads.adFailed(this, "interstitial", error.getLocalizedMessage());
	}

	@Override
	public void onAdInterstitialReceived(AdHubInterstitial arg0) {
		mngr.load("interstitial");
		Ads.adReceived(this, "interstitial");
		
	}

	@Override
	public void onAdVideoFailed(Exception error) {
		mngr.reset("video");
		Ads.adFailed(this, "video", error.getLocalizedMessage());
	}

	@Override
	public void onAdVideoReceived() {
		mngr.load("video");
		Ads.adReceived(this, "video");
	}

	@Override
	public void onAdVideoRefusedByUser() {
		mngr.reset("video");
		Ads.adFailed(this, "video", "Canceled by user");
	}

	@Override
	public void onContentVideoFailed(Exception error) {
		mngr.reset("video");
		Ads.adFailed(this, "video", error.getLocalizedMessage());
	}

	@Override
	public void onContentVideoReceived() {
		//Ads.adReceived(this);
	}

	@Override
	public void onVideoClosed() {
		Ads.adDismissed(this, "video");
	}
	
}

class AdsSamsungListener implements AdNotificationListener{
	
	private AdsState state;
	
	AdsSamsungListener(AdsState type){
		state = type;
	}
	
	@Override
	public void onAdFailed(AdHubView arg0, Exception error) {
		state.reset();
		Ads.adFailed(AdsSamsung.me, state.getType(), error.getLocalizedMessage());
	}

	@Override
	public void onAdReceived(AdHubView arg0) {
		state.load();
		Ads.adReceived(AdsSamsung.me, state.getType());
		
	}
}
