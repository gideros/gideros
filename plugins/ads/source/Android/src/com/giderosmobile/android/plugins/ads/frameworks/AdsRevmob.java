package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;
import com.revmob.RevMob;
import com.revmob.RevMobAdsListener;
import com.revmob.RevMobTestingMode;
import com.revmob.ads.banner.RevMobBanner;
import com.revmob.ads.fullscreen.RevMobFullscreen;
import com.revmob.ads.link.RevMobLink;
import com.revmob.ads.popup.RevMobPopup;

public class AdsRevmob implements AdsInterface{
	
	WeakReference<Activity> sActivity;
	private RevMob revmob;
	private AdsManager mngr;
	private String currentType = "banner";
	
	static AdsRevmob me;
	
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
		revmob = RevMob.start(sActivity.get(), param.get(0));
	}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
		SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);
		String tag = param.get(1);
		if(type.equals("interstitial"))
		{
			mngr.set(null, type);
			
			final RevMobFullscreen fullscreen;
			if(tag != null)
				fullscreen = revmob.createFullscreen(sActivity.get(), tag, new AdsRevmobListener(mngr.getState(type)));
			else
				fullscreen = revmob.createFullscreen(sActivity.get(), new AdsRevmobListener(mngr.getState(type)));
			mngr.setObject(type, fullscreen);
			mngr.setListener(type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					fullscreen.show();
				}
				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
		}
		else if(type.equals("popup"))
		{
			mngr.set(null, type);
			final RevMobPopup popup;
			if(tag != null)
				popup = revmob.createPopup(sActivity.get(), tag, new AdsRevmobListener(mngr.getState(type)));
			else
				popup = revmob.createPopup(sActivity.get(), new AdsRevmobListener(mngr.getState(type)));
			mngr.setObject(type, popup);
			mngr.setListener(type, new AdsStateChangeListener(){

				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					popup.show();
				}

				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
		}
		else if(type.equals("moregames"))
		{
			mngr.set(null, type);
			final RevMobLink link;
			if(tag != null)
				link = revmob.createAdLink(sActivity.get(), tag, new AdsRevmobListener(mngr.getState(type)));
			else
				link = revmob.createAdLink(sActivity.get(), tag, new AdsRevmobListener(mngr.getState(type)));
			mngr.setObject(type, link);
			mngr.setListener(type, new AdsStateChangeListener(){

				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					link.open();
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
				mngr.set(null, type);
				mngr.setAutoKill(type, false);
				final RevMobBanner adView;
				if(tag != null)
					adView = revmob.createBanner(sActivity.get(), tag, new AdsRevmobListener(mngr.getState(type)));
				else
					adView = revmob.createBanner(sActivity.get(), new AdsRevmobListener(mngr.getState(type)));
				mngr.setObject(type, adView);
				mngr.setListener(type, new AdsStateChangeListener(){

					@Override
					public void onShow() {
						Ads.adDisplayed(me, type);
						Ads.addAd(AdsRevmob.me, adView, -1, (int)(40*Ads.screenDensity));
						currentType = type;
					}
					@Override
					public void onDestroy() {
						hideAd(type);
					}	
					@Override
					public void onHide() {
						Ads.removeAd(AdsRevmob.me, adView);
						Ads.adDismissed(AdsRevmob.me, type);
					}	
				});
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
		return Ads.screenWidth;
	}
	
	public int getHeight(){
		if(mngr.get(currentType) != null)
		{
			return ((RevMobBanner) mngr.get(currentType)).getHeight();
		}
		return 0;
	}

	public void enableTesting() {
		revmob.setTestingMode(RevMobTestingMode.WITH_ADS);
	}

}

class AdsRevmobListener implements RevMobAdsListener{
	private AdsState state;
	
	AdsRevmobListener(AdsState type){
		state = type;
	}
	
	@Override
	public void onRevMobAdClicked() {
		Ads.adActionBegin(AdsRevmob.me, state.getType());
	}

	@Override
	public void onRevMobAdDismiss() {
		Ads.adDismissed(AdsRevmob.me, state.getType());
	}

	@Override
	public void onRevMobAdDisplayed() {
		
	}

	@Override
	public void onRevMobAdNotReceived(String error) {
		Ads.adFailed(AdsRevmob.me, error, state.getType());
		state.reset();
	}

	@Override
	public void onRevMobAdReceived() {
		Ads.adReceived(AdsRevmob.me, state.getType());
		
		AdsRevmob.me.sActivity.get().runOnUiThread(new Runnable() {
		  @Override
		  public void run() {
			  state.load();
		  }
		});
	}

	@Override
	public void onRevMobEulaIsShown() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onRevMobEulaWasAcceptedAndDismissed() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onRevMobEulaWasRejected() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onRevMobSessionIsStarted() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onRevMobSessionNotStarted(String arg0) {
		// TODO Auto-generated method stub
		
	}
}
