package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.List;

import android.app.Activity;
import android.util.SparseArray;
import android.view.KeyEvent;


import androidx.annotation.NonNull;

import com.giderosmobile.android.plugins.ads.*;
import com.google.android.gms.ads.*;
import com.google.android.gms.ads.AdRequest.Builder;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;
import com.google.android.gms.ads.interstitial.InterstitialAd;
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;
import com.google.android.gms.ads.rewarded.RewardItem;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;

public class AdsAdmob implements AdsInterface, OnInitializationCompleteListener {
	
	private WeakReference<Activity> sActivity;
	private String adsID = "";
	private AdSize currentType = AdSize.BANNER;
	private String currentName = "banner";
	private String testID = "";
	private AdsManager mngr;
	//rewarded ads and interstitial instance can reused, make as members to access easily
	private RewardedAd mRewardedAd = null;
    private  String mRewardedVideoAdId = "";
	private InterstitialAd interstitial = null;
	
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
    
        if(param.get(1) != null)  //the second is the appid, no longer required but left for compatibility
        {
			if(param.get(2) != null)  //third is the test id
				testID = param.get(2);
            MobileAds.initialize(sActivity.get().getApplicationContext(), this);
        }

    }

	public void adFailed(int errorCode,String type) {
		if(AdRequest.ERROR_CODE_INTERNAL_ERROR == errorCode)
			Ads.adFailed(AdsAdmob.me, type, "Internal error");
		else if(AdRequest.ERROR_CODE_INVALID_REQUEST == errorCode)
			Ads.adFailed(AdsAdmob.me, type, "Invalid request");
		else if(AdRequest.ERROR_CODE_NETWORK_ERROR == errorCode)
			Ads.adFailed(AdsAdmob.me, type, "Network error");
		else if(AdRequest.ERROR_CODE_NO_FILL == errorCode)
			Ads.adFailed(AdsAdmob.me, type, "No fill");
	}

	class InterstitialContext extends InterstitialAdLoadCallback implements AdsStateChangeListener {
		public String adPlace;
		public String type;
		public InterstitialAd ad;

		@Override
		public void onShow() {
			ad.show(sActivity.get());
		}

		@Override
		public void onDestroy() {

		}

		@Override
		public void onHide() {

		}

		@Override
		public void onRefresh() {
			ad=null;
			Builder adRequest = new AdRequest.Builder();
			InterstitialAd.load(sActivity.get().getApplicationContext(),adPlace,adRequest.build(),this);
		}

		@Override
		public void onAdFailedToLoad (LoadAdError adError) {
			adFailed(adError.getCode(),type);
		}

		@Override
		public void onAdLoaded (InterstitialAd adT) {
			ad=adT;
			ad.setFullScreenContentCallback(new FullScreenContentCallback() {
				@Override
				public void onAdClicked() {
					Ads.adActionBegin(AdsAdmob.me, type);
				}

				@Override
				public void onAdDismissedFullScreenContent() {
					Ads.adDismissed(AdsAdmob.me, type);
					mngr.getState(type).refresh();
				}

				@Override
				public void onAdFailedToShowFullScreenContent(@NonNull AdError adError) {
					adFailed(adError.getCode(),type);
				}

				@Override
				public void onAdImpression() {
					Ads.adDisplayed(AdsAdmob.me, type);
				}

				@Override
				public void onAdShowedFullScreenContent() {
					Ads.adActionEnd(AdsAdmob.me, type);
				}
			});
			Ads.adReceived(AdsAdmob.me, type);
			mngr.getState(type).load();
		}
	}

	class RewardedContext extends RewardedAdLoadCallback implements AdsStateChangeListener, OnUserEarnedRewardListener {
		public String adPlace;
		public String type;
		public RewardedAd ad;

		@Override
		public void onShow() {
			ad.show(sActivity.get(), this);
		}

		@Override
		public void onDestroy() {

		}

		@Override
		public void onHide() {

		}

		@Override
		public void onRefresh() {
			ad=null;
			Builder adRequest = new AdRequest.Builder();
			RewardedAd.load(sActivity.get().getApplicationContext(),adPlace,adRequest.build(),this);
		}

		@Override
		public void onAdFailedToLoad (LoadAdError adError) {
			adFailed(adError.getCode(),type);
		}

		@Override
		public void onAdLoaded (RewardedAd adT) {
			ad=adT;
			ad.setFullScreenContentCallback(new FullScreenContentCallback() {
				@Override
				public void onAdClicked() {
					Ads.adActionBegin(AdsAdmob.me, type);
				}

				@Override
				public void onAdDismissedFullScreenContent() {
					Ads.adDismissed(AdsAdmob.me, type);
					mngr.getState(type).refresh();
				}

				@Override
				public void onAdFailedToShowFullScreenContent(@NonNull AdError adError) {
					adFailed(adError.getCode(),type);
				}

				@Override
				public void onAdImpression() {
					Ads.adDisplayed(AdsAdmob.me, type);
				}

				@Override
				public void onAdShowedFullScreenContent() {
					Ads.adActionEnd(AdsAdmob.me, type);
				}
			});
			Ads.adReceived(AdsAdmob.me, type);
			mngr.getState(type).load();
		}

		@Override
		public void onUserEarnedReward(@NonNull RewardItem rewardItem) {
			Ads.adRewarded(AdsAdmob.me, type, rewardItem.getAmount());
		}
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
			InterstitialContext ctx= (InterstitialContext) mngr.get(type);
			 if (ctx == null) {
				 ctx=new InterstitialContext();
				 ctx.adPlace=adPlace;
				 ctx.type=type;
				 mngr.set(ctx, type, ctx);
				 // interstitial should be reused
				 mngr.setAutoKill(type, false);
				 mngr.setPreLoad(type, true);
				 ctx.onRefresh();
			} else{
				 ctx.adPlace=adPlace;
				 ctx.onRefresh();
		    }
		}
		else if(type.equals("rewarded"))
		{
			RewardedContext ctx= (RewardedContext) mngr.get(type);
			if (ctx == null) {
				ctx=new RewardedContext();
				ctx.adPlace=adPlace;
				ctx.type=type;
				mngr.set(ctx, type, ctx);
				// interstitial should be reused
				mngr.setAutoKill(type, false);
				mngr.setPreLoad(type, true);
				ctx.onRefresh();
			} else {
				ctx.adPlace=adPlace;
				ctx.onRefresh();
			}
		}
		else  //banner
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

						@Override
						public void onRefresh() {

						}

					});
					mngr.setAutoKill(type, false);
					adView.setAdUnitId(adPlace);
					adView.setAdSize(adTypes.get(type));

					Builder adRequest = new AdRequest.Builder();

					adView.setAdListener(new AdListener() {
						@Override
						public void onAdClicked() {
							Ads.adActionBegin(AdsAdmob.me, type);
						}

						@Override
						public void onAdImpression() {
							Ads.adDisplayed(AdsAdmob.me, type);
						}

						@Override
						public void onAdClosed() {
							Ads.adDismissed(AdsAdmob.me, type);
							mngr.getState(type).refresh();
							super.onAdClosed();
						}

						@Override
						public void onAdFailedToLoad(@NonNull LoadAdError loadAdError) {
							adFailed(loadAdError.getCode(),type);
						}

						@Override
						public void onAdLoaded() {
							super.onAdLoaded();
							Ads.adReceived(AdsAdmob.me, type);
							mngr.getState(type).load();
						}

						@Override
						public void onAdOpened() {
							super.onAdOpened();
						}

						@Override
						public void onAdSwipeGestureClicked() {
							super.onAdSwipeGestureClicked();
						}
					});
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
		//return GooglePlayServicesUtil.isGooglePlayServicesAvailable(sActivity.get())== ConnectionResult.SUCCESS;
        GoogleApiAvailability googleApiAvailability = GoogleApiAvailability.getInstance();
        int result = googleApiAvailability.isGooglePlayServicesAvailable(sActivity.get());
        if (result != ConnectionResult.SUCCESS) {
            if (googleApiAvailability.isUserResolvableError(result)) {
                googleApiAvailability.getErrorDialog(sActivity.get(), result, 9000).show();
            }

            return false;
        }

        return true;
	}

	@Override
	public void enableTesting() {
		List<String> testDeviceIds = Arrays.asList(testID);
		RequestConfiguration configuration =
				new RequestConfiguration.Builder().setTestDeviceIds(testDeviceIds).build();
		MobileAds.setRequestConfiguration(configuration);
	}

	@Override
	public void onInitializationComplete(@NonNull InitializationStatus initializationStatus) {
		//TODO
	}
}
