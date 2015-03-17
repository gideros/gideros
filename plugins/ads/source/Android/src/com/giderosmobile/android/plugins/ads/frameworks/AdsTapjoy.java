package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import java.util.Hashtable;
import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;
import android.view.View;

import com.giderosmobile.android.plugins.ads.*;
import com.tapjoy.TapjoyConnect;
import com.tapjoy.TapjoyDailyRewardAdNotifier;
import com.tapjoy.TapjoyDailyRewardAdStatus;
import com.tapjoy.TapjoyDisplayAdNotifier;
import com.tapjoy.TapjoyDisplayAdSize;
import com.tapjoy.TapjoyFullScreenAdNotifier;

public class AdsTapjoy implements AdsInterface, TapjoyFullScreenAdNotifier, TapjoyDailyRewardAdNotifier{
	
	WeakReference<Activity> sActivity;
	private TapjoyConnect sInstance;
	private String currentType = TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_320X50;
	private AdsManager mngr;
	
	static AdsTapjoy me;
	
	//all AdMob banner types
	private static Hashtable<String, String> adTypes;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		mngr = new AdsManager();
		
		//create ad types
		adTypes = new Hashtable<String, String>();
		adTypes.put("320x50", TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_320X50);
		adTypes.put("640x100", TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_640X100);
		adTypes.put("768x90", TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_768X90);
	}
	
	//on destroy event
	public void onDestroy()
	{	
		mngr.destroy();
		sInstance.sendShutDownEvent();
	}
	
	public void onStart(){}

	public void onStop(){}
	
	public void onPause(){
		sInstance.appPause();
	}
		
	public void onResume(){
		sInstance.appResume();
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		return false;
	}
	
	public void setKey(final Object parameters){
		SparseArray<String> param = (SparseArray<String>)parameters;
		TapjoyConnect.requestTapjoyConnect(sActivity.get(), param.get(0), param.get(1));
		sInstance = TapjoyConnect.getTapjoyConnectInstance();
	}
	
	//load an Ad
	public void loadAd(final Object parameters)
	{
		SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);

		if(type.equals("interstitial"))
		{
			sInstance.getFullScreenAd(this);
			mngr.set(sInstance, type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					sInstance.showFullScreenAd();
				}
				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
		}
		else if(type.equals("offers"))
		{
			mngr.set(sInstance, type, new AdsStateChangeListener(){
				@Override
				public void onShow() {
					Ads.adDisplayed(me, type);
					sInstance.showOffers();
				}
				@Override
				public void onDestroy() {}	
				@Override
				public void onHide() {}	
			});
			mngr.load(type);
			Ads.adReceived(AdsTapjoy.me, type);
		}
		else
		{
			if(adTypes.get(type) != null || type.equals("auto"))
			{	
				sInstance.enableDisplayAdAutoRefresh(true);
				sInstance.setDisplayAdSize(currentType);
				//if there is an existing ad view
				if(mngr.get(type) == null)
				{
					mngr.set(null, type);
					mngr.setAutoKill(type, false);
					sInstance.getDisplayAd(sActivity.get(), new AdsTapjoyListener(mngr.getState(type))); 
					mngr.setListener(type, new AdsStateChangeListener(){
						@Override
						public void onShow() {
							Ads.adDisplayed(me, type);
							if(mngr.get(type) != null)
							{
								if(type.equals("auto"))
								{
									currentType = getAutoSize();
								}
								else
								{
									currentType = adTypes.get(type);
								}
								Ads.addAd(AdsTapjoy.me, (View)mngr.get(type));
							}
						}
						@Override
						public void onDestroy() {
							hideAd(type);
						}	
						@Override
						public void onHide() {
							Ads.removeAd(AdsTapjoy.me, (View)mngr.get(type));
							Ads.adDismissed(AdsTapjoy.me, type);
						}	
					});
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
		if(currentType.equals(TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_320X50))
			width = 320;
		else if(currentType.equals(TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_640X100))
			width = 640;
		else if(currentType.equals(TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_768X90))
			width = 768;
		return (int) Ads.DipsToPixels(width);
	}
	
	public int getHeight(){
		int height = 0;
		if(currentType.equals(TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_320X50))
			height = 50;
		else if(currentType.equals(TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_640X100))
			height = 100;
		else if(currentType.equals(TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_768X90))
			height = 90;
		return (int) Ads.DipsToPixels(height);
	}

	@Override
	public void getFullScreenAdResponse() {
		mngr.load("interstitial");
		Ads.adReceived(this, "interstitial");
	}

	@Override
	public void getFullScreenAdResponseFailed(int arg0) {
		mngr.reset("interstitial");
		Ads.adFailed(this, "interstitial", "Ad Response Failed");
	}

	@Override
	public void getDailyRewardAdResponse() {
		
	}

	@Override
	public void getDailyRewardAdResponseFailed(int error) {
		mngr.reset("offers");
		String errorMsg = null;
		switch (error)
		{
			case TapjoyDailyRewardAdStatus.STATUS_NETWORK_ERROR:
				errorMsg = "Daily Reward ad network error.";
				break;
                 
			case TapjoyDailyRewardAdStatus.STATUS_NO_ADS_AVAILABLE:
				errorMsg = "No Daily Reward ads available";
				break;
                 
			case TapjoyDailyRewardAdStatus.STATUS_SERVER_ERROR:
				errorMsg = "Daily Reward ad server error";
				break;
		}
		Ads.adFailed(this, "offers", errorMsg);
	}
	
	private String getAutoSize() {
		String[] typearray = {TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_768X90, TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_640X100, TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_320X50};
        int[][] maparray = { { 0, 768, 90 }, {
              1, 640, 100 }, {
              2, 320, 50 } };

        for (int i = 0; i < maparray.length; i++) {
                if (maparray[i][1] * Ads.screenDensity <= Ads.screenWidth
                        && maparray[i][2] * Ads.screenDensity <= Ads.screenHeight) {
                    return typearray[maparray[i][0]];
                }
            }
        return TapjoyDisplayAdSize.TJC_DISPLAY_AD_SIZE_320X50;
    }

	@Override
	public void enableTesting() {
		//can be set up in Tapjoy account
		
	}
}

class AdsTapjoyListener implements TapjoyDisplayAdNotifier{
	
	private AdsState state;
	
	AdsTapjoyListener(AdsState type){
		state = type;
	}
	
	@Override
	public void getDisplayAdResponse(View view) {
		Ads.adReceived(AdsTapjoy.me, state.getType());
		state.setObject(view);
		AdsTapjoy.me.sActivity.get().runOnUiThread(new Runnable() {
			  @Override
			  public void run() {
				  state.load();
			  }
		});
	}

	@Override
	public void getDisplayAdResponseFailed(String arg0) {
		state.reset();
		Ads.adFailed(AdsTapjoy.me, state.getType(), arg0);
	}
}
