package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;

import com.vungle.publisher.AdConfig;
import com.vungle.publisher.EventListener;
import com.vungle.publisher.VunglePub;


public class AdsVungle implements AdsInterface {

	private WeakReference<Activity> sActivity;
	private AdsManager mngr;
	private static AdsVungle me;// get the VunglePub instance
	final com.vungle.publisher.VunglePub vunglePub = com.vungle.publisher.VunglePub.getInstance();

	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		mngr = new AdsManager();
	}

	private final EventListener vungleListener = new EventListener() {
		@Override
		public void onVideoView(boolean isCompletedView, int watchedMillis, int videoDurationMillis) {
			// Called each time a video completes.  isCompletedView is true if >= 80% of the video was watched.
			if (isCompletedView) {
				Ads.adActionEnd(this, "");
			} else {
				Ads.adDismissed(this, "");
			}
		}

		@Override
		public void onAdStart() {
			// Called before playing an ad.
			Ads.adDisplayed(this, "");
		}

		@Override
		public void onAdUnavailable(String reason) {
			// Called when VunglePub.playAd() was called but no ad is available to show to the user.
			Ads.adError(this, reason);
		}

		@Override
		public void onAdEnd(boolean wasCallToActionClicked) {
			// Called when the user leaves the ad and control is returned to your application.
			if (wasCallToActionClicked) {
				Ads.adActionEnd(this, "video");
			}
		}

		@Override
		public void onAdPlayableChanged(boolean isAdPlayable) {
			// Called when ad playability changes.
			if (isAdPlayable) {
				Ads.adReceived(this, String.valueOf(isAdPlayable));
			}
		}
	};

	//on destroy event
	public void onDestroy(){}

	public void onStart(){}

	public void onStop(){}

	public void onPause()
	{
		vunglePub.onPause();
	}

	public void onResume()
	{
		vunglePub.onResume();
	}

	public boolean onKeyUp(int keyCode, KeyEvent event) {
		return false;
	}

	public void setKey(final Object parameters){
		SparseArray<String> param = (SparseArray<String>)parameters;
		vunglePub.init(sActivity.get(), param.get(0));
		vunglePub.setEventListeners(vungleListener);
	}

	private void playAdIncentivized() {
		// create a new AdConfig object
		final AdConfig overrideConfig = new AdConfig();

		// set incentivized option on
		overrideConfig.setIncentivized(true);
		overrideConfig.setIncentivizedCancelDialogTitle("Careful!");
		overrideConfig.setIncentivizedCancelDialogBodyText("If the video isn't completed you won't get your reward! Are you sure you want to close early?");
		overrideConfig.setIncentivizedCancelDialogCloseButtonText("Close");
		overrideConfig.setIncentivizedCancelDialogKeepWatchingButtonText("Keep Watching");

		// the overrideConfig object will only affect this ad play.
		vunglePub.playAd(overrideConfig);
	}

	//load an Ad
	public void loadAd(final Object parameters)
	{
		final SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);
		if(type.equals("video"))
		{
			mngr.set(VunglePub.class, type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					vunglePub.playAd();
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
		else if(type.equals("v4vc"))
		{
			mngr.set(VunglePub.class, type, new AdsStateChangeListener(){

				@Override
				public void onShow() {
					playAdIncentivized();
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
}
