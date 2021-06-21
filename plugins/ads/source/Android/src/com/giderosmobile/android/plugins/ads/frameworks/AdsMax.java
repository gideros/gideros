package com.giderosmobile.android.plugins.ads.frameworks;

import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.applovin.mediation.MaxAd;
import com.applovin.mediation.MaxAdFormat;
import com.applovin.mediation.MaxAdListener;
import com.applovin.mediation.MaxAdViewAdListener;
import com.applovin.mediation.MaxErrorCodes;
import com.applovin.mediation.MaxReward;
import com.applovin.mediation.MaxRewardedAdListener;
import com.applovin.mediation.ads.MaxAdView;
import com.applovin.mediation.ads.MaxInterstitialAd;
import com.applovin.mediation.ads.MaxRewardedAd;
import com.applovin.sdk.AppLovinSdk;
import com.applovin.sdk.AppLovinSdkConfiguration;
import com.giderosmobile.android.plugins.ads.Ads;
import com.giderosmobile.android.plugins.ads.AdsInterface;
import com.giderosmobile.android.plugins.ads.AdsManager;
import com.giderosmobile.android.plugins.ads.AdsState;
import com.giderosmobile.android.plugins.ads.AdsStateChangeListener;

import java.lang.ref.WeakReference;
import java.util.Hashtable;

public class AdsMax implements AdsInterface {
	
	private WeakReference<Activity> sActivity;
	private MaxAdFormat currentType = MaxAdFormat.BANNER;
	private String currentName = "banner";
	private AdsManager mngr;
	
	static AdsMax me;
	
	//all AdMob banner types
	private Hashtable<String, MaxAdFormat> adTypes;

	public void onCreate(WeakReference<Activity> activity)
	{
		me = this;
		sActivity = activity;

		AppLovinSdk.getInstance( sActivity.get() ).setMediationProvider( "max" );
		AppLovinSdk.initializeSdk( sActivity.get(), new AppLovinSdk.SdkInitializationListener() {
			@Override
			public void onSdkInitialized(AppLovinSdkConfiguration config) {

			}
		} );
		AppLovinSdk.initializeSdk(sActivity.get());
		currentType = MaxAdFormat.BANNER;

		//create ad types
		mngr = new AdsManager();
		adTypes = new Hashtable<String, MaxAdFormat>();
		adTypes.put("banner", MaxAdFormat.BANNER);
		adTypes.put("leader", MaxAdFormat.LEADER);
		adTypes.put("mrec", MaxAdFormat.MREC);
		adTypes.put("auto", MaxAdFormat.BANNER);
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
			((MaxAdView) mngr.get(currentName)).loadAd();
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		return false;
	}
	
	public void setKey(final Object parameters){}

	MaxInterstitialAd ADInter;
	MaxRewardedAd ADReward;
	//load an Ad
	public void loadAd(final Object parameters)
	{
		SparseArray<String> param = (SparseArray<String>)parameters;
		final String type = param.get(0);
		final String aid = param.get(1);
				if(type.equals("interstitial"))
				{
					AppLovinSdk sdk = AppLovinSdk.getInstance(sActivity.get());
					if (ADInter==null) {
						ADInter = new MaxInterstitialAd(aid, sActivity.get());
						mngr.set(null, type, new AdsStateChangeListener() {

									@Override
									public void onShow() {
										Ads.adDisplayed(me, type);
										ADInter.showAd();
									}

									@Override
									public void onDestroy() {
									}

									@Override
									public void onHide() {
									}

									@Override
									public void onRefresh() {
									}
								});
						AdsMaxListener listener = new AdsMaxListener(mngr.getState(type));
						ADInter.setListener(listener);
					}
					ADInter.loadAd();
				}
				else if(type.equals("v4vc"))
				{
					if (ADReward==null) {
						ADReward = MaxRewardedAd.getInstance(aid, sActivity.get());
						mngr.set(null, type, new AdsStateChangeListener() {

							@Override
							public void onShow() {
								Ads.adDisplayed(me, type);
								ADReward.showAd();
							}

							@Override
							public void onDestroy() {
							}

							@Override
							public void onHide() {
							}

							@Override
							public void onRefresh() {
							}
						});
						AdsMaxListener listener = new AdsMaxListener(mngr.getState(type));
						ADReward.setListener(listener);
					}
					ADReward.loadAd();
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

							final MaxAdView adView = new MaxAdView( aid, adTypes.get(type), sActivity.get() );
							mngr.set(adView, type, new AdsStateChangeListener(){

								@Override
								public void onShow() {
									Ads.adDisplayed(me, type);
									currentType = adTypes.get(type);
									currentName = type;
									Ads.addAd(AdsMax.me, adView);
								}	
								@Override
								public void onDestroy() {
									 hideAd(type);
									 adView.destroy();
								}
								@Override
								public void onHide() {
									Ads.removeAd(AdsMax.me, adView);
									Ads.adDismissed(AdsMax.me, type);
								}	
								@Override
			                    public void onRefresh() {}
							});
							mngr.setAutoKill(type, false);
							AdsMaxListener listener = new AdsMaxListener(mngr.getState(type));
							adView.setListener(listener);
							adView.loadAd();
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
		return currentType.getSize().getWidth();
	}
	
	public int getHeight(){
		return currentType.getSize().getHeight();
	}

	@Override
	public void enableTesting() {
		
	}
	
}

class AdsMaxListener implements MaxAdListener, MaxRewardedAdListener, MaxAdViewAdListener {
	
	private AdsState state;
	
	AdsMaxListener(AdsState type){
		state = type;
	}
	
	public void setType(AdsState type){
		state = type;
	}
	
	public void failed(int errorCode) {
		if(errorCode == MaxErrorCodes.NO_FILL)
			Ads.adFailed(AdsMax.me, state.getType(), "No fill");
		else if(errorCode == MaxErrorCodes.DONT_KEEP_ACTIVITIES_ENABLED)
			Ads.adFailed(AdsMax.me, state.getType(), "'Don't keep activities' Enabled");
		else if(errorCode == MaxErrorCodes.FULLSCREEN_AD_ALREADY_SHOWING)
			Ads.adFailed(AdsMax.me, state.getType(), "Fullscreen Ad already shwing");
		else if(errorCode == MaxErrorCodes.NO_ACTIVITY)
			Ads.adFailed(AdsMax.me, state.getType(), "No activity");
		else if(errorCode == MaxErrorCodes.INVALID_INTERNAL_STATE)
			Ads.adFailed(AdsMax.me, state.getType(), "Invalid internal state");
		else if(errorCode == MaxErrorCodes.MEDIATION_ADAPTER_LOAD_FAILED)
			Ads.adFailed(AdsMax.me, state.getType(), "Mediation adapter load failed");
		else if(errorCode == MaxErrorCodes.UNSPECIFIED_ERROR)
			Ads.adFailed(AdsMax.me, state.getType(), "Unspecified error");
		else
			Ads.adFailed(AdsMax.me, state.getType(), "Unknown error");
		state.reset();
	}


	@Override
	public void onAdLoaded(MaxAd ad) {
		if(state.getType().equals("interstitial") || state.getType().equals("v4vc")){
			state.setObject(ad);
		}
		Ads.adReceived(AdsMax.me, state.getType());
		state.load();
	}

	@Override
	public void onAdLoadFailed(String adUnitId, int errorCode) {
		failed(errorCode);
	}

	@Override
	public void onAdDisplayed(MaxAd ad) {

	}

	@Override
	public void onAdHidden(MaxAd ad) {
		Ads.adDismissed(AdsMax.me, state.getType());
	}

	@Override
	public void onAdClicked(MaxAd ad) {
		Ads.adActionBegin(AdsMax.me, state.getType());
	}

	@Override
	public void onAdDisplayFailed(MaxAd ad, int errorCode) {
		failed(errorCode);
	}

	@Override
	public void onRewardedVideoStarted(MaxAd ad) {
		Ads.adActionBegin(AdsMax.me, state.getType());
	}

	@Override
	public void onRewardedVideoCompleted(MaxAd ad) {
		if(!state.getType().equals("v4vc")){
			Ads.adActionEnd(AdsMax.me, state.getType());
		}
	}

	@Override
	public void onUserRewarded(MaxAd ad, MaxReward reward) {
		Ads.adRewarded(AdsMax.me, state.getType(),reward.getAmount());
	}

	@Override
	public void onAdExpanded(MaxAd ad) {

	}

	@Override
	public void onAdCollapsed(MaxAd ad) {

	}
}
