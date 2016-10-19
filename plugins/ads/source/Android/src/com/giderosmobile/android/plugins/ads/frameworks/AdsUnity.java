package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;
import com.unity3d.ads.*;


public class AdsUnity implements AdsInterface, IUnityAdsListener {

	private WeakReference<Activity> sActivity;
	private AdsManager mngr;
	private static AdsUnity me;
	private boolean v4vcMap=true;
	
	private String mapType(String type)
	{
		if (type.equals("rewardedVideo"))
			v4vcMap=false;
		if (v4vcMap&&type.equals("v4vc"))
			return "rewardedVideo";
		return type;
	}
	
	private String unmapType(String type)
	{
		if (v4vcMap&&type.equals("rewardedVideo"))
			return "v4vc";
		return type;		
	}

	public void onCreate(WeakReference<Activity> activity)
	{
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
	}

	public void onResume()
	{
	}

	public boolean onKeyUp(int keyCode, KeyEvent event) {
		return false;
	}

	public void setKey(final Object parameters){
		SparseArray<String> param = (SparseArray<String>)parameters;
		UnityAds.initialize(sActivity.get(), param.get(0), this);
	}


	//load an Ad
	public void loadAd(final Object parameters)
	{
		final SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);
			mngr.set(UnityAds.class, type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					UnityAds.show(sActivity.get(),mapType(type));
				}
				@Override
				public void onDestroy() {}
				@Override
				public void onHide() {}
				@Override
                public void onRefresh() {}
			});
			mngr.load(type);
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
	public void onUnityAdsReady(String s) {
		Ads.adReceived(this,unmapType(s));
	}

	@Override
	public void onUnityAdsStart(String s) {
		Ads.adDisplayed(this,unmapType(s));
	}

	@Override
	public void onUnityAdsFinish(String s, UnityAds.FinishState finishState) {
		if (finishState== UnityAds.FinishState.COMPLETED)
			Ads.adActionEnd(this,unmapType(s));
		else if (finishState== UnityAds.FinishState.SKIPPED)
			Ads.adDismissed(this,unmapType(s));
		else if (finishState== UnityAds.FinishState.ERROR)
			Ads.adFailed(this,unmapType(s),"");
	}

	@Override
	public void onUnityAdsError(UnityAds.UnityAdsError unityAdsError, String s) {
	}
}
