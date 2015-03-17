package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import java.util.Hashtable;

import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;

import com.millennialmedia.android.MMAdView;
import com.millennialmedia.android.MMException;
import com.millennialmedia.android.MMInterstitial;
import com.millennialmedia.android.MMAd;
import com.millennialmedia.android.MMRequest;
import com.millennialmedia.android.MMSDK;
import com.millennialmedia.android.RequestListener;

public class AdsMillenial implements AdsInterface {
	
	private WeakReference<Activity> sActivity;
	private String adsID;
	private AdsMillenialSize currentType;
	private AdsManager mngr;
	private Hashtable<String, AdsMillenialSize> adTypes;
	
	static AdsMillenial me;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		//create ad types
		mngr = new AdsManager();
		currentType = null;
		adTypes = new Hashtable<String, AdsMillenialSize>();
		adTypes.put("banner", new AdsMillenialSize(320, 50));
		adTypes.put("med_banner", new AdsMillenialSize(480, 60));
		adTypes.put("iab_leaderboard", new AdsMillenialSize(728, 90));
		adTypes.put("rectangle",  new AdsMillenialSize(320, 250));
		
		MMSDK.initialize(sActivity.get());
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
				final MMInterstitial interstitial = new MMInterstitial(sActivity.get());
				mngr.set(interstitial, type, new AdsStateChangeListener(){
					@Override
					public void onShow() {
						Ads.adDisplayed(me, type);
						interstitial.display();
					}

					@Override
					public void onDestroy() {}	
					@Override
					public void onHide() {}	
				});

				MMRequest request = new MMRequest();
				interstitial.setMMRequest(request);
				interstitial.setApid(adsID);
				interstitial.setListener(new AdsMillenialListener(mngr.getState(type)));
				interstitial.fetch();
			}
			else
			{
				if(adTypes.get(type) != null || type.equals("auto"))
				{
					if(mngr.get(type) == null)
					{
						// Create the adView with your publisher ID and type
						final MMAdView adView = new MMAdView(sActivity.get());
						mngr.set(adView, type, new AdsStateChangeListener(){

							@Override
							public void onShow() {
								Ads.adDisplayed(me, type);
								if(type.equals("auto"))
								{
									String currentName = getAutoSize();
									currentType = adTypes.get(currentName);
								}
								else
								{
									currentType = adTypes.get(type);
								}
								Ads.addAd(AdsMillenial.me, adView, currentType.width, currentType.height);
							}	
							@Override
							public void onDestroy() {
								 hideAd(type);
							}
							@Override
							public void onHide() {
								Ads.removeAd(AdsMillenial.me, adView);
								Ads.adDismissed(AdsMillenial.me, type);
							}	
						});
						mngr.setAutoKill(type, false);
							
						adView.setApid(adsID);
						MMRequest request = new MMRequest();
						adView.setMMRequest(request);
						adView.setId(MMSDK.getDefaultAdId());
						adView.setListener(new AdsMillenialListener(mngr.getState(type)));
						adView.getAd();
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
			Ads.adError(this, "Provide App ID");
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
		if(currentType != null)
			return Ads.DipsToPixels(currentType.width);
		return 0;
	}
	
	public int getHeight(){
		if(currentType != null)
			return Ads.DipsToPixels(currentType.height);
		return 0;
	}

	@Override
	public void enableTesting() {
		
	}
	
	private String getAutoSize() {
		String[] typearray = {"iab_leaderboard", "med_banner", "banner"};
        int[][] maparray = {
        		{1, 728, 90 }, 
        		{2, 480, 60 },
        		{3, 320, 50 }};

        for (int i = 0; i < maparray.length; i++) {
                if (maparray[i][1] * Ads.screenDensity <= Ads.screenWidth
                        && maparray[i][2] * Ads.screenDensity <= Ads.screenHeight) {
                    return typearray[maparray[i][0]];
                }
            }
        return "banner";
    }
	
}

class AdsMillenialSize{
	public int width;
	public int height;
	AdsMillenialSize(int widthSize, int heightSize){
		width = widthSize;
		height = heightSize;
	}
}

class AdsMillenialListener implements RequestListener{
	
	private AdsState state;
	
	AdsMillenialListener(AdsState type){
		state = type;
	}
	
	@Override
	public void requestCompleted(MMAd mmAd) {
		state.load();
		Ads.adReceived(AdsMillenial.me, state.getType());
	}
	
	@Override
	public void requestFailed(MMAd arg0, MMException arg1) {
		state.reset();
		Ads.adFailed(AdsMillenial.me, state.getType(), arg1.getLocalizedMessage());
	}

	@Override
	public void MMAdOverlayClosed(MMAd arg0) {
		Ads.adDismissed(AdsMillenial.me, state.getType());
	}

	@Override
	public void MMAdOverlayLaunched(MMAd arg0) {}

	@Override
	public void MMAdRequestIsCaching(MMAd arg0) {}

	@Override
	public void onSingleTap(MMAd arg0) {
		Ads.adActionBegin(AdsMillenial.me, state.getType());
	}

}
