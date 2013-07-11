package com.giderosmobile.android.plugins.googlebilling;

import java.lang.ref.WeakReference;

import com.giderosmobile.android.plugins.googlebilling.Consts.PurchaseState;
import com.giderosmobile.android.plugins.googlebilling.Consts.ResponseCode;
import com.giderosmobile.android.plugins.googlebilling.BillingService.RequestPurchase;
import com.giderosmobile.android.plugins.googlebilling.BillingService.RestoreTransactions;
import com.giderosmobile.android.plugins.googlebilling.BillingService.ConfirmNotifications;

import android.app.Activity;
import android.os.Handler;

class GGoogleBilling
{
	private static WeakReference<Activity> sActivity;
	private static GGoogleBilling sInstance;
	private static long sData;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
	}
	
	static public void init(long data)
	{
		sData = data;
		sInstance = new GGoogleBilling(sActivity.get());
	}
	
	static public void cleanup()
	{
    	if (sInstance != null)
    	{
    		sData = 0;
    		ResponseHandler.unregister(sInstance.mPurchaseObserver);
    		sInstance.mBillingService.unbind();
    		sInstance = null;
    	}
	}

	private BillingService mBillingService;
    private Handler mHandler;
	private PurchaseObserver mPurchaseObserver;
	
    private class MyPurchaseObserver extends PurchaseObserver
    {
        public MyPurchaseObserver(Activity activity)
        {
            super(activity);
        }

        @Override
        public void onBillingSupported(ResponseCode responseCode, String type)
        {
        	if (sData != 0)
        		GGoogleBilling.onBillingSupported(responseCode.ordinal(), type, sData);
        }

        @Override
        public void onPurchaseStateChange(PurchaseState purchaseState, String itemId, String notificationId, long purchaseTime, String developerPayload)
        {
        	if (sData != 0)
        		GGoogleBilling.onPurchaseStateChange(purchaseState.ordinal(), itemId, notificationId, purchaseTime, developerPayload, sData);
        }

        @Override
        public void onRequestPurchaseResponse(RequestPurchase request, ResponseCode responseCode)
        {
        	if (sData != 0)
        		GGoogleBilling.onRequestPurchaseResponse(responseCode.ordinal(), request.mProductId, request.mProductType, request.mDeveloperPayload, sData);
        }

        @Override
        public void onRestoreTransactionsResponse(RestoreTransactions request, ResponseCode responseCode)
        {
        	if (sData != 0)
        		GGoogleBilling.onRestoreTransactionsResponse(responseCode.ordinal(), sData);
        }

		@Override
		public void onConfirmNotificationsResponse(ConfirmNotifications request, ResponseCode responseCode)
		{
        	if (sData != 0)
        	{
        		for (String notificationId:request.mNotifyIds)
        			GGoogleBilling.onConfirmNotificationsResponse(responseCode.ordinal(), notificationId, sData);	
        	}
		}
    }

    public GGoogleBilling(Activity activity)
	{
        mPurchaseObserver = new MyPurchaseObserver(activity);
        mBillingService = new BillingService();
        mBillingService.setContext(activity);
        ResponseHandler.register(mPurchaseObserver);
	}

    static public void onDestroy()
    {
    	cleanup(); 
    }
    
    public static void setApiVersion(int apiVersion)
    {
		sInstance.mBillingService.setApiVersion(apiVersion);
    }
    
    public static void setPublicKey(String publicKey)
    {
		Security.setPublicKey(publicKey);
    }

    public static boolean checkBillingSupported(String itemType)
	{
		return sInstance.mBillingService.checkBillingSupported(itemType);
	}
	
	public static boolean requestPurchase(String productId, String itemType, String developerPayload)
	{
		return sInstance.mBillingService.requestPurchase(productId, itemType, developerPayload);
	}
	
	public static boolean restoreTransactions()
	{
		return sInstance.mBillingService.restoreTransactions();
	}
	
	public static boolean confirmNotification(String notificationId)
	{
		return sInstance.mBillingService.confirmNotifications(-1, new String[] {notificationId});
	}
	
	private static native void onBillingSupported(int responseCode, String type, long data);
	private static native void onPurchaseStateChange(int purchaseState, String itemId, String notificationId, long purchaseTime, String developerPayload, long data);
	private static native void onRequestPurchaseResponse(int responseCode, String productId, String productType, String developerPayload, long data);
	private static native void onRestoreTransactionsResponse(int responseCode, long data);
	private static native void onConfirmNotificationsResponse(int responseCode, String notificationId, long data);
}
