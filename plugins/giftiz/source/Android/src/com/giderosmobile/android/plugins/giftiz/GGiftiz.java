package com.giderosmobile.android.plugins.giftiz;

import java.lang.ref.WeakReference;

import com.purplebrain.giftiz.sdk.GiftizSDK;
import com.purplebrain.giftiz.sdk.GiftizSDK.Inner.ButtonNeedsUpdateDelegate;
import com.purplebrain.giftiz.sdk.GiftizSDK.Inner.GiftizButtonStatus;

import android.app.Activity;


public class GGiftiz implements ButtonNeedsUpdateDelegate
{
	private static WeakReference<Activity> sActivity;
	private static long sData;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
		GiftizSDK.Inner.setButtonNeedsUpdateDelegate(new GGiftiz());
	}

	public static void onPause() {
		GiftizSDK.onPauseMainActivity(sActivity.get());
	}

	public static void onResume() {
		GiftizSDK.onResumeMainActivity(sActivity.get());
	}
	
	
	
	static public void init(long data){
		sData = data;
	}
	
	
	static public void cleanup(){
		sData = 0;
	}

    static public void onDestroy(){
    	cleanup(); 
    }
    
    static public void missionComplete()
    {
    	GiftizSDK.missionComplete(sActivity.get());
    }
    
    static public void purchaseMade(float amount)
    {
    	GiftizSDK.inAppPurchase(sActivity.get(), amount);
    }
    
    static public int getButtonState()
    {
    	int state = -1;
    	 switch (GiftizSDK.Inner.getButtonStatus(sActivity.get())) {
         case ButtonInvisible : state = 0;break;
         case ButtonNaked : state = 1;break;
         case ButtonBadge : state = 2;break;
         case ButtonWarning : state = 3;break;
         }
    	 return state;
    }
    
    static public void buttonClicked()
    {	
    	GiftizSDK.Inner.buttonClicked(sActivity.get());
    }

	@Override
	public void buttonNeedsUpdate() {
		if (sData != 0)
		{
			int state = getButtonState();
			onButtonUpdate(state, sData);
		}
	}
	
	private static native void onButtonUpdate(int state, long data);
	
}
