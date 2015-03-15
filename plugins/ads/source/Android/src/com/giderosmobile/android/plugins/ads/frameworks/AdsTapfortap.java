package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;
import com.tapfortap.AppWall;
import com.tapfortap.Banner;
import com.tapfortap.Interstitial;
import com.tapfortap.TapForTap;

public class AdsTapfortap implements AdsInterface, Banner.BannerListener, Interstitial.InterstitialListener, AppWall.AppWallListener{
	
	private WeakReference<Activity> sActivity;
	private int BANNER_WIDTH = 320;
	private int BANNER_HEIGHT = 50;
	private AdsManager mngr;
	private static AdsTapfortap me;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		mngr = new AdsManager();
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
		TapForTap.initialize(sActivity.get(), param.get(0));
	}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
		SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);

		if(type.equals("interstitial"))
		{
			final Interstitial interstitial = Interstitial.create(sActivity.get(), this);
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
			
		}
		else if(type.equals("moreapps"))
		{
			final AppWall appWall = AppWall.create(sActivity.get(), this);
			mngr.set(appWall, type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					appWall.show();
				}
				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
		}	
		else
		{
			if(mngr.get(type) == null)
			{
				final Banner adView = Banner.create(sActivity.get(), this);
				mngr.set(adView, type, new AdsStateChangeListener(){
					@Override
					public void onShow() {
						Ads.adDisplayed(me, type);
						Ads.addAd(AdsTapfortap.me, adView, BANNER_WIDTH, BANNER_HEIGHT);
					}
					@Override
					public void onDestroy() {
						hideAd(type);
					}	
					@Override
					public void onHide() {
						Ads.removeAd(AdsTapfortap.me, adView);
						Ads.adDismissed(AdsTapfortap.me, type);
					}	
				});
				mngr.setAutoKill(type, false);
				adView.startShowingAds();
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
		return (int) Ads.DipsToPixels(BANNER_WIDTH);
	}
	
	public int getHeight(){
		return (int) Ads.DipsToPixels(BANNER_HEIGHT);
	}

	@Override
	public void enableTesting() {
		TapForTap.enableTestMode();
	}

	@Override
	public void bannerOnFail(Banner Banner, String s, Throwable throwable) {
		mngr.reset("banner");
		Ads.adFailed(this, "banner", s);
	}

	@Override
	public void bannerOnReceive(Banner arg0) {
		mngr.load("banner");
		Ads.adReceived(this, "banner");
	}

	@Override
	public void bannerOnTap(Banner arg0) {
		Ads.adActionBegin(this, "banner");
	}

	@Override
	public void interstitialOnDismiss(Interstitial arg0) {
		Ads.adDismissed(this, "interstitial");
	}

	@Override
	public void interstitialOnFail(Interstitial arg0, String arg1,
			Throwable arg2) {
		mngr.reset("interstitial");
		Ads.adFailed(this, "interstitial", arg1);
	}

	@Override
	public void interstitialOnReceive(Interstitial arg0) {
		mngr.load("interstitial");
		Ads.adReceived(this, "interstitial");
	}

	@Override
	public void interstitialOnShow(Interstitial arg0) {
	}

	@Override
	public void interstitialOnTap(Interstitial arg0) {
		Ads.adActionBegin(this, "interstitial");
	}

	@Override
	public void appWallOnDismiss(AppWall arg0) {
		Ads.adDismissed(this, "moreapps");
	}

	@Override
	public void appWallOnFail(AppWall arg0, String arg1, Throwable arg2) {
		mngr.reset("moreapps");
		Ads.adFailed(this, "moreapps", arg1);
	}

	@Override
	public void appWallOnReceive(AppWall arg0) {
		mngr.load("moreapps");
		Ads.adReceived(this, "moreapps");
	}

	@Override
	public void appWallOnShow(AppWall arg0) {}

	@Override
	public void appWallOnTap(AppWall arg0) {
		Ads.adActionBegin(this, "moreapps");
	}
	
}
