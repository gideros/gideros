package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import android.app.Activity;
import androidx.annotation.NonNull;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;

import com.vungle.publisher.AdConfig;
import com.vungle.publisher.VungleAdEventListener;
import com.vungle.publisher.VungleInitListener;
import com.vungle.publisher.VunglePub;


public class AdsVungle implements AdsInterface {

	private WeakReference<Activity> sActivity;
	private AdsManager mngr;
	private static AdsVungle me;// get the VunglePub instance
	final com.vungle.publisher.VunglePub vunglePub = com.vungle.publisher.VunglePub.getInstance();
	private String user;

	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		mngr = new AdsManager();
	}

	private final VungleAdEventListener vungleListener = new VungleAdEventListener() {
		@Override
		public void onAdEnd(@NonNull String s, boolean wasViewed, boolean wasClicked) {
			if (wasViewed) {
				Ads.adActionEnd(this, s);
			} else {
				Ads.adDismissed(this, s);
			}
		}

		@Override
		public void onAdStart(@NonNull String s) {
			Ads.adDisplayed(this, s);
		}

		@Override
		public void onUnableToPlayAd(@NonNull String s, String reason) {
			Ads.adError(this, reason);
		}

		@Override
		public void onAdAvailabilityUpdate(@NonNull String s, boolean isAdPlayable) {
			if (isAdPlayable) {
				Ads.adReceived(this, s);
				mngr.load(s);
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
		if (param.size()>1)
			user=param.get(1);
		String[] placements=new String[] { "Default" };
		if (param.size()>2)
		{
			placements=new String[param.size()-2];
			for (int k=0;k<placements.length;k++)
				placements[k]=param.get(k+2);
		}
		vunglePub.init(sActivity.get(), param.get(0), placements, new VungleInitListener() {
			@Override
			public void onSuccess() {
			}

			@Override
			public void onFailure(Throwable throwable) {
				Ads.adError(this, throwable.getMessage());
			}
		});
		vunglePub.clearAndSetEventListeners(vungleListener);
	}

	private void playAd(String s,String user) {
		// create a new AdConfig object
		final AdConfig overrideConfig = new AdConfig();

		// set incentivized option on
		overrideConfig.setIncentivizedUserId(user);
		overrideConfig.setIncentivizedCancelDialogTitle("Careful!");
		overrideConfig.setIncentivizedCancelDialogBodyText("If the video isn't completed you won't get your reward! Are you sure you want to close early?");
		overrideConfig.setIncentivizedCancelDialogCloseButtonText("Close");
		overrideConfig.setIncentivizedCancelDialogKeepWatchingButtonText("Keep Watching");

		// the overrideConfig object will only affect this ad play.
		vunglePub.playAd(s,overrideConfig);
	}

	//load an Ad
	public void loadAd(final Object parameters)
	{
		final SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);
		String user=this.user;
		if (param.size()>1)
			user=param.get(1);
		final String adUser=user;
			mngr.set(VunglePub.class, type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					playAd(type,adUser);
				}
				@Override
				public void onDestroy() {}
				@Override
				public void onHide() {}
				@Override
                public void onRefresh() {}
			});
		vunglePub.loadAd(type);
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
