package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import java.util.Hashtable;

import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;
import android.view.View;

import com.applovin.adview.AppLovinAdView;
import com.applovin.adview.AppLovinInterstitialAd;
import com.applovin.adview.AppLovinInterstitialAdDialog;
import com.applovin.sdk.AppLovinAd;
import com.applovin.sdk.AppLovinAdClickListener;
import com.applovin.sdk.AppLovinAdLoadListener;
import com.applovin.sdk.AppLovinAdSize;
import com.applovin.sdk.AppLovinAdVideoPlaybackListener;
import com.applovin.sdk.AppLovinErrorCodes;
import com.applovin.sdk.AppLovinSdk;
import com.giderosmobile.android.plugins.ads.*;
import com.google.android.gms.ads.AdView;

import android.util.Log;

public class AdsApplovin implements AdsInterface {
	
	private WeakReference<Activity> sActivity;
	private AppLovinAdSize currentType = AppLovinAdSize.BANNER;
	private String currentName = "banner";
	private AdsManager mngr;
	
	static AdsApplovin me;
	
	//all AdMob banner types
	private Hashtable<String, AppLovinAdSize> adTypes;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		
		AppLovinSdk.initializeSdk(sActivity.get());
		currentType = AppLovinAdSize.BANNER;
		
		//create ad types
		mngr = new AdsManager();
		adTypes = new Hashtable<String, AppLovinAdSize>();
		adTypes.put("banner", AppLovinAdSize.BANNER);
		adTypes.put("leader", AppLovinAdSize.LEADER);
		adTypes.put("mrec", AppLovinAdSize.MREC);
		adTypes.put("auto", AppLovinAdSize.BANNER);
	}
	
	//on destroy event
	public void onDestroy()
	{	
		mngr.destroy();
	}
	
	public void onStart(){}

	public void onStop(){}
	
	public void onPause(){}
		
	public void onResume(){
		if(mngr.get(currentName) != null)
			((AppLovinAdView) mngr.get(currentName)).loadNextAd();
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		return false;
	}
	
	public void setKey(final Object parameters){}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
			SparseArray<String> param = (SparseArray<String>)parameters;
			final String type = param.get(0);
				if(type.equals("interstitial"))
				{
					AppLovinSdk sdk = AppLovinSdk.getInstance(sActivity.get());
					final AppLovinInterstitialAdDialog interstitial = AppLovinInterstitialAd.create(sdk, sActivity.get());
					mngr.set(null, type, new AdsStateChangeListener(){

						@Override
						public void onShow() {
							Ads.adDisplayed(me, type);
							interstitial.showAndRender((AppLovinAd) mngr.get(type));
							//interstitial.show();
						}

						@Override
						public void onDestroy() {}	
						@Override
						public void onHide() {}	
					});
					AdsApplovinListener listener = new AdsApplovinListener(mngr.getState(type));
					interstitial.setAdClickListener(listener);
					//interstitial.setAdVideoPlaybackListener(listener);
					sdk.getAdService().loadNextAd(AppLovinAdSize.INTERSTITIAL, listener);
				}
				else
				{
					if(adTypes.get(type) != null)
					{
						//if there is an existing ad view
						//destroy it
						if(mngr.get(type) == null)
						{
							// Create the adView with your publisher ID and type
							AppLovinSdk sdk = AppLovinSdk.getInstance(sActivity.get());

							final AppLovinAdView adView = new AppLovinAdView( sdk, adTypes.get(type), sActivity.get() );
							mngr.set(adView, type, new AdsStateChangeListener(){

								@Override
								public void onShow() {
									Log.d("app lovin", "show");
									Ads.adDisplayed(me, type);
									currentType = adTypes.get(type);
									currentName = type;
									Ads.addAd(AdsApplovin.me, adView);
								}	
								@Override
								public void onDestroy() {
									Log.d("app lovin", "destroy");
									 hideAd(type);
									 adView.destroy();
								}
								@Override
								public void onHide() {
									Log.d("app lovin", "hide");
									Ads.removeAd(AdsApplovin.me, adView);
									Ads.adDismissed(AdsApplovin.me, type);
								}	
							});
							mngr.setAutoKill(type, false);
							AdsApplovinListener listener = new AdsApplovinListener(mngr.getState(type));
							adView.setAdLoadListener(listener);
							adView.setAdClickListener(listener);
							adView.setAutoDestroy(false);
							adView.loadNextAd();
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
		return currentType.getWidth();
	}
	
	public int getHeight(){
		return currentType.getHeight();
	}

	@Override
	public void enableTesting() {
		
	}
	
}

class AdsApplovinListener implements AppLovinAdLoadListener, AppLovinAdVideoPlaybackListener, AppLovinAdClickListener{
	
	private AdsState state;
	
	AdsApplovinListener(AdsState type){
		state = type;
	}
	
	@Override
	public void adReceived(AppLovinAd ad) {
		if(state.getType().equals("interstitial")){
			state.setObject(ad);
		}
		Ads.adReceived(AdsApplovin.me, state.getType());
		state.load();
	}

	@Override
	public void failedToReceiveAd(int errorCode) {
		if(errorCode == AppLovinErrorCodes.NO_FILL)
			Ads.adFailed(AdsApplovin.me, state.getType(), "No fill");
		else if(errorCode == AppLovinErrorCodes.NO_INCENTIVIZED_AD_PRELOADED)
			Ads.adFailed(AdsApplovin.me, state.getType(), "No incentivized ad preloaded");
		else if(errorCode == AppLovinErrorCodes.SERVER_TIMEOUT)
			Ads.adFailed(AdsApplovin.me, state.getType(), "Server Timeout");
		else if(errorCode == AppLovinErrorCodes.UNABLE_TO_RENDER_AD)
			Ads.adFailed(AdsApplovin.me, state.getType(), "Unable to render ad");
		else if(errorCode == AppLovinErrorCodes.UNKNOWN_SERVER_ERROR)
			Ads.adFailed(AdsApplovin.me, state.getType(), "Unknown server error");
		else if(errorCode == AppLovinErrorCodes.UNSPECIFIED_ERROR)
			Ads.adFailed(AdsApplovin.me, state.getType(), "Unspecified error");
		else if(errorCode == AppLovinErrorCodes.USER_CLOSED_VIDEO)
			Ads.adFailed(AdsApplovin.me, state.getType(), "User closed video");
		else
			Ads.adFailed(AdsApplovin.me, state.getType(), "Unknown error");
		state.reset();
	}

	@Override
	public void adClicked(AppLovinAd arg0) {
		Ads.adActionBegin(AdsApplovin.me, state.getType());
	}

	@Override
	public void videoPlaybackBegan(AppLovinAd arg0) {
		Ads.adActionBegin(AdsApplovin.me, state.getType());
	}

	@Override
	public void videoPlaybackEnded(AppLovinAd ad, double percentViewed, boolean fullyWatched) {
		if(state.getType().equals("v4vc")){
			if(fullyWatched)
				Ads.adActionEnd(AdsApplovin.me, state.getType());
		}
		else
			Ads.adActionEnd(AdsApplovin.me, state.getType());
	}
}
