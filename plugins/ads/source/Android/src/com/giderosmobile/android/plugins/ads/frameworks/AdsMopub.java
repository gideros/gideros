package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;

import com.giderosmobile.android.plugins.ads.*;
import com.mopub.mobileads.MoPubErrorCode;
import com.mopub.mobileads.MoPubInterstitial;
import com.mopub.mobileads.MoPubInterstitial.InterstitialAdListener;
import com.mopub.mobileads.MoPubView;
import com.mopub.mobileads.MoPubView.BannerAdListener;

import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;


public class AdsMopub implements AdsInterface{
	
	private WeakReference<Activity> sActivity;
	private String adsID = "";
	private String currentType = "banner";
	private AdsManager mngr;
	static AdsMopub me;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		sActivity = activity;
		mngr = new AdsManager();
		me = this;
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
		SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);
		String tag = param.get(1);
		if(type.equals("interstitial"))
		{
			final MoPubInterstitial mInterstitial;
			if(tag != null)
				mInterstitial = new MoPubInterstitial(sActivity.get(), param.get(1));
			else
				mInterstitial = new MoPubInterstitial(sActivity.get(), adsID);
			mngr.set(mInterstitial, type, new AdsStateChangeListener(){

				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					mInterstitial.show();
				}

				@Override
				public void onDestroy() {
					mInterstitial.destroy();
				}	
				@Override
				public void onHide() {}	
			});
		    mInterstitial.setInterstitialAdListener(new AdsMopubInterstitialListener(mngr.getState(type)));
		    mInterstitial.load();
		}
		else
		{
			if(mngr.get(type) == null)
			{
				final MoPubView adView = new MoPubView(sActivity.get());
				mngr.set(adView, type, new AdsStateChangeListener(){

					@Override
					public void onShow() {
						Ads.adDisplayed(me, type);
						currentType = type;
						Ads.addAd(AdsMopub.me, adView);
					}

					@Override
					public void onDestroy() {
						hideAd(type);
						adView.destroy();
					}	
					@Override
					public void onHide() {
						Ads.removeAd(AdsMopub.me, adView);
						Ads.adDismissed(AdsMopub.me, type);
					}		
				});
				mngr.setAutoKill(type, false);
				if(tag != null)
					adView.setAdUnitId(param.get(1));
				else
					adView.setAdUnitId(adsID);
				adView.setBannerAdListener(new AdsMopubBannerListener(mngr.getState(type)));
				adView.loadAd();
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
		if(mngr.get(currentType) != null)
			return (int) (((MoPubView) mngr.get(currentType)).getAdWidth());
		return 0;
	}
	
	public int getHeight(){
		if(mngr.get(currentType) != null)
			return (int) (((MoPubView) mngr.get(currentType)).getAdHeight());
		return 0;
	}

	@Override
	public void enableTesting() {
		
	}
}

class AdsMopubBannerListener implements BannerAdListener{
	
	private AdsState state;
	
	AdsMopubBannerListener(AdsState type){
		state = type;
	}
	
	@Override
	public void onBannerClicked(MoPubView arg0) {
	}

	@Override
	public void onBannerCollapsed(MoPubView arg0) {
		Ads.adActionEnd(AdsMopub.me, state.getType());
	}

	@Override
	public void onBannerExpanded(MoPubView arg0) {
		Ads.adActionBegin(AdsMopub.me, state.getType());
	}

	@Override
	public void onBannerFailed(MoPubView arg0, MoPubErrorCode error) {
		state.reset();
		Ads.adFailed(AdsMopub.me, state.getType(), error.name());
	}

	@Override
	public void onBannerLoaded(MoPubView arg0) {
		state.load();
		Ads.adReceived(AdsMopub.me, state.getType());
	}
}

class AdsMopubInterstitialListener implements InterstitialAdListener{
	
	private AdsState state;
	
	AdsMopubInterstitialListener(AdsState type){
		state = type;
	}
	
	@Override
	public void onInterstitialClicked(MoPubInterstitial arg0) {
		Ads.adActionBegin(AdsMopub.me, state.getType());
	}

	@Override
	public void onInterstitialDismissed(MoPubInterstitial arg0) {
		Ads.adDismissed(AdsMopub.me, state.getType());
	}

	@Override
	public void onInterstitialFailed(MoPubInterstitial arg0, MoPubErrorCode error) {
		state.reset();
		Ads.adFailed(AdsMopub.me, state.getType(), error.name());
	}

	@Override
	public void onInterstitialLoaded(MoPubInterstitial interstitial) {
		state.load();
		Ads.adReceived(AdsMopub.me, state.getType());
	}

	@Override
	public void onInterstitialShown(MoPubInterstitial arg0) {
	}
}
