package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import java.util.Hashtable;
import java.util.Map;

import com.giderosmobile.android.plugins.ads.*;

import com.inmobi.commons.InMobi;
import com.inmobi.commons.InMobi.LOG_LEVEL;
import com.inmobi.monetization.IMBanner;
import com.inmobi.monetization.IMBannerListener;
import com.inmobi.monetization.IMErrorCode;
import com.inmobi.monetization.IMInterstitial;
import com.inmobi.monetization.IMInterstitialListener;

import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

public class AdsInmobi implements AdsInterface{
	
	private static WeakReference<Activity> sActivity;
	private static String adsID;
	private static Integer currentType = IMBanner.INMOBI_AD_UNIT_320X50;
	private static String currentName = "320x50";
	private AdsManager mngr;
	
	static AdsInmobi me;
	
	//all AdMob banner types
	private static Hashtable<String, Integer> adTypes;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		
		mngr = new AdsManager();
		//create ad types
		adTypes = new Hashtable<String, Integer>();
		adTypes.put("300x250", IMBanner.INMOBI_AD_UNIT_300X250);
		adTypes.put("728x90", IMBanner.INMOBI_AD_UNIT_728X90);
		adTypes.put("468x60", IMBanner.INMOBI_AD_UNIT_468X60);
		adTypes.put("120x600", IMBanner.INMOBI_AD_UNIT_120X600);
		adTypes.put("320x50", IMBanner.INMOBI_AD_UNIT_320X50);
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
		InMobi.initialize(sActivity.get(), adsID);
	}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
		SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);

		if(type.equals("interstitial"))
		{
			final IMInterstitial interstitial = new IMInterstitial(sActivity.get(), adsID);
			mngr.set(interstitial, type, new AdsStateChangeListener(){

				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					interstitial.show();
				}
				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
			interstitial.setIMInterstitialListener(new AdsInmobiInterstitialListener(mngr.getState(type)));
			interstitial.loadInterstitial();
		}
		else
		{
			if(adTypes.get(type) != null || type.equals("auto"))
			{
				Integer newType;
				if(type.equals("auto"))
					newType = adTypes.get(getAutoSize());
				else
					newType = adTypes.get(type);
				//if there is an existing ad view
				//destroy it
				if(mngr.get(type) == null)
				{
					// Create the adView with your publisher ID and type
					final IMBanner adView = new IMBanner(sActivity.get(), adsID, newType);
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
							Ads.addAd(AdsInmobi.me, adView);
						}
						@Override
						public void onDestroy() {
							 hideAd(type);
							 adView.destroy();
						}	
						@Override
						public void onHide() {
							Ads.removeAd(AdsInmobi.me, adView);
							Ads.adDismissed(AdsInmobi.me, type);
						}	
					});
					mngr.setAutoKill(type, false);
					adView.setIMBannerListener(new AdsInmobiBannerListener(mngr.getState(type)));
					adView.loadBanner();
				}
			}
			else
			{
				Ads.adError(this, "Unknown type: " + type);
			}
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
		int width = 0;
		if(currentType.equals(IMBanner.INMOBI_AD_UNIT_300X250))
		{
			width = 300;
		}
		else if(currentType.equals(IMBanner.INMOBI_AD_UNIT_728X90))
		{
			width = 728;
		}
		else if(currentType.equals(IMBanner.INMOBI_AD_UNIT_468X60))
		{
			width = 468;
		}
		else if(currentType.equals(IMBanner.INMOBI_AD_UNIT_120X600))
		{
			width = 120;
		}
		else if(currentType.equals(IMBanner.INMOBI_AD_UNIT_320X50))
		{
			width = 320;
		}
		return (int) Ads.DipsToPixels(width);
	}
	
	public int getHeight(){
		int height = 0;
		if(currentType.equals(IMBanner.INMOBI_AD_UNIT_300X250))
		{
			height = 250;
		}
		else if(currentType.equals(IMBanner.INMOBI_AD_UNIT_728X90))
		{
			height = 90;
		}
		else if(currentType.equals(IMBanner.INMOBI_AD_UNIT_468X60))
		{
			height = 60;
		}
		else if(currentType.equals(IMBanner.INMOBI_AD_UNIT_120X600))
		{
			height = 600;
		}
		else if(currentType.equals(IMBanner.INMOBI_AD_UNIT_320X50))
		{
			height = 50;
		}
		return (int) Ads.DipsToPixels(height);
	}
	
	public static String getAutoSize() {
		String[] typearray = {"728x90", "468x60", "320x50"};
		int[][] maparray = { { 0, 728, 90 }, {
		            1, 468, 60 }, {
		            2, 320, 50 } };
		        for (int i = 0; i < maparray.length; i++) {
		                if (Ads.DipsToPixels(maparray[i][1]) <= Ads.screenWidth
		                        && Ads.DipsToPixels(maparray[i][2]) <= Ads.screenHeight) {
		                    return typearray[maparray[i][0]];
		                }
		            }
		        return "320x50";
		    }

	@Override
	public void enableTesting() {
		//has no testing
		InMobi.setLogLevel(LOG_LEVEL.DEBUG);
	}
}

class AdsInmobiInterstitialListener implements IMInterstitialListener {
	
	private AdsState state;
	
	AdsInmobiInterstitialListener(AdsState type){
		state = type;
	}
	
	@Override
	public void onDismissInterstitialScreen(IMInterstitial arg0) {
		Ads.adDismissed(AdsInmobi.me, "interstitial");
	}

	@Override
	public void onInterstitialFailed(IMInterstitial arg0, IMErrorCode error) {
		state.reset();
		Ads.adFailed(AdsInmobi.me, "interstitial", error.name());
	}

	@Override
	public void onInterstitialInteraction(IMInterstitial arg0,
			Map<String, String> arg1) {
		Ads.adActionBegin(AdsInmobi.me, "interstitial");
	}

	@Override
	public void onInterstitialLoaded(IMInterstitial i) {
		state.load();
		Ads.adReceived(AdsInmobi.me, "interstitial");
	}

	@Override
	public void onLeaveApplication(IMInterstitial arg0) {}

	@Override
	public void onShowInterstitialScreen(IMInterstitial arg0) {}
	
}

class AdsInmobiBannerListener implements IMBannerListener{
	
	private AdsState state;
	
	AdsInmobiBannerListener(AdsState type){
		state = type;
	}
	
	@Override
	public void onBannerInteraction(IMBanner arg0, Map<String, String> arg1) {
		Ads.adActionBegin(AdsInmobi.me, state.getType());
	}

	@Override
	public void onBannerRequestFailed(IMBanner arg0, IMErrorCode error) {
		state.reset();
		Ads.adFailed(AdsInmobi.me, state.getType(), error.name());
	}

	@Override
	public void onBannerRequestSucceeded(IMBanner arg0) {
		state.load();
		Ads.adReceived(AdsInmobi.me, state.getType());
	}

	@Override
	public void onDismissBannerScreen(IMBanner arg0) {
		Ads.adDismissed(AdsInmobi.me, state.getType());
	}

	@Override
	public void onLeaveApplication(IMBanner arg0) {}

	@Override
	public void onShowBannerScreen(IMBanner arg0) {}
}
