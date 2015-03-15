package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import java.math.BigInteger;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Hashtable;
import android.app.Activity;
import android.provider.Settings;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;
import com.google.android.gms.ads.*;
import com.google.android.gms.ads.AdRequest.Builder;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GooglePlayServicesUtil;

public class AdsAdmob implements AdsInterface {
	
	private WeakReference<Activity> sActivity;
	private String adsID = "";
	private AdSize currentType = AdSize.BANNER;
	private String currentName = "banner";
	private String testID = "";
	private AdsManager mngr;
	
	static AdsAdmob me;
	
	//all AdMob banner types
	private Hashtable<String, AdSize> adTypes;
	
	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;
		
		adsID = "";
		currentType = AdSize.BANNER;
		currentName = "banner";
		testID = "";
		
		//create ad types
		mngr = new AdsManager();
		adTypes = new Hashtable<String, AdSize>();
		adTypes.put("banner", AdSize.BANNER);
		adTypes.put("iab_banner", AdSize.FULL_BANNER);
		adTypes.put("iab_leaderboard", AdSize.LEADERBOARD);
		adTypes.put("iab_mrect", AdSize.MEDIUM_RECTANGLE);
		adTypes.put("iab_skyscaper", AdSize.WIDE_SKYSCRAPER);
		adTypes.put("smart_banner", AdSize.SMART_BANNER);
		adTypes.put("auto", AdSize.SMART_BANNER);
	}
	
	//on destroy event
	public void onDestroy()
	{	
		mngr.destroy();
	}
	
	public void onStart(){}

	public void onStop(){}
	
	public void onPause(){
		if(mngr.get(currentName) != null)
			((AdView) mngr.get(currentName)).pause();
	}
		
	public void onResume(){
		if(mngr.get(currentName) != null)
			((AdView) mngr.get(currentName)).resume();
	}
	
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
			String adPlace = adsID;
			if(param.get(1) != null)
			{
				adPlace = param.get(1);
			}
				if(type.equals("interstitial"))
				{
					final InterstitialAd interstitial = new InterstitialAd(sActivity.get());
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
					interstitial.setAdUnitId(adPlace);
			
					Builder adRequest = new AdRequest.Builder();
					if(!testID.equals(""))
					{
						adRequest.addTestDevice(testID);
					}
					interstitial.setAdListener(new AdsAdmobListener(mngr.getState(type)));
					interstitial.loadAd(adRequest.build());
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
							final AdView adView = new AdView(sActivity.get());
							mngr.set(adView, type, new AdsStateChangeListener(){

								@Override
								public void onShow() {
									Ads.adDisplayed(me, type);
									currentName = type;
									currentType = adTypes.get(type);
									Ads.addAd(AdsAdmob.me, adView);
								}	
								@Override
								public void onDestroy() {
									 hideAd(type);
									 adView.destroy();
								}
								@Override
								public void onHide() {
									Ads.removeAd(AdsAdmob.me, adView);
									Ads.adDismissed(AdsAdmob.me, type);
								}	
							});
							mngr.setAutoKill(type, false);
							adView.setAdUnitId(adPlace);
							adView.setAdSize(adTypes.get(type));
						
							Builder adRequest = new AdRequest.Builder();
							if(!testID.equals(""))
								adRequest.addTestDevice(testID);

							adView.setAdListener(new AdsAdmobListener(mngr.getState(type)));
							adView.loadAd(adRequest.build());
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
		return currentType.getWidthInPixels(sActivity.get());
	}
	
	public int getHeight(){
		return currentType.getHeightInPixels(sActivity.get());
	}
	
	private boolean checkAvailable(){
		return GooglePlayServicesUtil.isGooglePlayServicesAvailable(sActivity.get())== ConnectionResult.SUCCESS; 
	}

	@Override
	public void enableTesting() {
		String aid = Settings.Secure.getString(sActivity.get().getContentResolver(), "android_id");

		Object obj = null;
		try {
		    ((MessageDigest) (obj = MessageDigest.getInstance("MD5"))).update(
		                                   aid.getBytes(), 0, aid.length());

		    obj = String.format("%032X", new Object[] { new BigInteger(1,
		                                   ((MessageDigest) obj).digest()) });
		} catch (NoSuchAlgorithmException localNoSuchAlgorithmException) {
		    obj = aid.substring(0, 32);
		}
		
		testID = obj.toString();
		
	}
	
}

class AdsAdmobListener extends AdListener{
	
	private AdsState state;
	
	AdsAdmobListener(AdsState type){
		state = type;
	}
	
	public void onAdLoaded(){
		Ads.adReceived(AdsAdmob.me, state.getType());
		state.load();
	}
	
	public void onAdFailedToLoad(int errorCode){
		if(AdRequest.ERROR_CODE_INTERNAL_ERROR == errorCode)
			Ads.adFailed(AdsAdmob.me, state.getType(), "Internal error");
		else if(AdRequest.ERROR_CODE_INVALID_REQUEST == errorCode)
			Ads.adFailed(AdsAdmob.me, state.getType(), "Invalid request");
		else if(AdRequest.ERROR_CODE_NETWORK_ERROR == errorCode)
			Ads.adFailed(AdsAdmob.me, state.getType(), "Network error");
		else if(AdRequest.ERROR_CODE_NO_FILL == errorCode)
			Ads.adFailed(AdsAdmob.me, state.getType(), "No fill");
		
		state.reset();
	}
	
	public void onAdOpened(){
		Ads.adActionBegin(AdsAdmob.me, state.getType());
	}
	
	public void onAdClosed(){
		Ads.adActionEnd(AdsAdmob.me, state.getType());
	}
	
	public void onAdLeftApplication(){
		
	}
}
