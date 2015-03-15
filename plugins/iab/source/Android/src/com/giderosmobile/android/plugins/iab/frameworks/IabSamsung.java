package com.giderosmobile.android.plugins.iab.frameworks;

import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Hashtable;

import com.giderosmobile.android.plugins.iab.Iab;
import com.giderosmobile.android.plugins.iab.IabInterface;
import com.giderosmobile.android.plugins.iab.frameworks.samsung.SamsungIapHelper;
import com.giderosmobile.android.plugins.iab.frameworks.samsung.SamsungIapHelper.OnGetInboxListListener;
import com.giderosmobile.android.plugins.iab.frameworks.samsung.SamsungIapHelper.OnGetItemListListener;
import com.giderosmobile.android.plugins.iab.frameworks.samsung.SamsungIapHelper.OnIapBindListener;
import com.giderosmobile.android.plugins.iab.frameworks.samsung.SamsungIapHelper.OnInitIapListener;
import com.giderosmobile.android.plugins.iab.frameworks.samsung.vo.InBoxVO;
import com.giderosmobile.android.plugins.iab.frameworks.samsung.vo.ItemVO;
import com.giderosmobile.android.plugins.iab.frameworks.samsung.vo.PurchaseVO;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.SparseArray;

public class IabSamsung implements IabInterface, OnInitIapListener, OnGetItemListListener, OnGetInboxListListener, OnIapBindListener {
	private static WeakReference<Activity> sActivity;
	private SamsungIapHelper mSamsungIapHelper = null;
	private static final int IAP_MODE = SamsungIapHelper.IAP_MODE_TEST_SUCCESS;
	public static boolean sdkAvailable = false;
	public static boolean wasChecked = false;
	public static boolean finishedChecking = false;
	public static String itemGroupId;
	//private static final int IAP_MODE = SamsungIapHelper.IAP_MODE_COMMERCIAL;
	
	public static Boolean isInstalled(){
		if(Iab.isPackageInstalled("com.sec.android.iap"))
			return true;
		return false;
	}

	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity = activity;
	}

	@Override
	public void onDestroy() {
		mSamsungIapHelper = SamsungIapHelper.getInstance(sActivity.get(), IAP_MODE );                                        
        if( null != mSamsungIapHelper )
        {
            mSamsungIapHelper.stopRunningTask();
            mSamsungIapHelper.dispose();
        }
	}
	
	@Override
	public void onStart() {
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		if( requestCode == SamsungIapHelper.REQUEST_CODE_IS_ACCOUNT_CERTIFICATION ) { 
			finishedChecking = true;
			if( resultCode == Activity.RESULT_OK ) {
				mSamsungIapHelper.bindIapService(this);
			} 
			else{
				if(wasChecked)
				{
					Iab.notAvailable(this);
				}
			}
		}
		else if( requestCode == SamsungIapHelper.REQUEST_CODE_IS_IAP_PAYMENT ) 
		{ 
			if( null == data ) 
			{ 
				return; 
			} 
			Bundle extras = data.getExtras(); 
			int statusCode = 1; 
			String errorString = ""; 
			PurchaseVO purchaseVO = null;
			if( null != extras ) { 
				statusCode = extras.getInt( "STATUS_CODE" ); 
				errorString = extras.getString( "ERROR_STRING" ); 
				purchaseVO = new PurchaseVO(extras.getString(SamsungIapHelper.KEY_NAME_RESULT_OBJECT));
			}
			else { 
				Iab.purchaseError(this, "The payment was not processed successfully");
			}
			if( Activity.RESULT_OK == resultCode ) { 
				if( statusCode == SamsungIapHelper.IAP_ERROR_NONE ) { 
					mSamsungIapHelper.verifyPurchaseResult(sActivity.get(), purchaseVO);
				}
				else
				{
					Iab.purchaseError(this, errorString);
				}
			}
		}
	}

	@Override
	public void init(Object parameters) {
		SparseArray<byte[]> p = (SparseArray<byte[]>)parameters;
		try {
			itemGroupId = new String(p.get(0), "UTF-8");
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
		mSamsungIapHelper = SamsungIapHelper.getInstance(sActivity.get(), IAP_MODE );
		mSamsungIapHelper.setOnInitIapListener( this );
		mSamsungIapHelper.setOnGetItemListListener( this );
		mSamsungIapHelper.setOnGetInboxListListener( this );
		if( true == mSamsungIapHelper.isInstalledIapPackage(sActivity.get()) )
		{
			if( true == mSamsungIapHelper.isValidIapPackage(sActivity.get()) )
			{
				mSamsungIapHelper.startAccountActivity(sActivity.get());
			}
			else
			{
				finishedChecking = true;
			}
		}
		else
		{
			finishedChecking = true;
		}
	}

	@Override
	public void check() {
		if(finishedChecking)
		{
			if(sdkAvailable)
			{
				Iab.available(this);  
			}
			else
			{
				Iab.notAvailable(this);
			}
		}
		else
		{
			wasChecked = true;
		}
	}
	
	@Override
	public void request(Hashtable<String, String> products) {
		mSamsungIapHelper.safeGetItemList(sActivity.get(), itemGroupId, 0, products.size(), SamsungIapHelper.ITEM_TYPE_ALL);
	}

	@Override
	public void purchase(String productId) {
		mSamsungIapHelper.startPurchase(sActivity.get(), SamsungIapHelper.REQUEST_CODE_IS_IAP_PAYMENT, itemGroupId, productId);
		Iab.purchaseComplete(this, "test_product", "test_receipt");
		Iab.purchaseError(this, "Test Purchase Error");
	}
	
	@Override
	public void restore() {
		Calendar c = Calendar.getInstance();
		SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd");
		String formattedDate = df.format(c.getTime());
		mSamsungIapHelper.safeGetItemInboxTask(sActivity.get(), itemGroupId, 0, 1000, "20130101", formattedDate);
	}

	@Override
	public void onSucceedGetItemList(ArrayList<ItemVO> _itemList) {
		SparseArray<Bundle> arr = new SparseArray<Bundle>();
    	int i = 0; 
    	for(ItemVO p : _itemList)
		{
        	Bundle map = new Bundle();
        	map.putString("productId", p.getItemId());
        	map.putString("title", p.getItemName());
        	map.putString("description", p.getItemDesc());
        	map.putString("price", p.getItemPriceString());
        	arr.put(i, map);
        	i++;
        }
        Iab.productsComplete(this, arr);
	}

	@Override
	public void OnSucceedGetInboxList(ArrayList<InBoxVO> _inboxList) {
    	for(InBoxVO p : _inboxList)
		{
    		Iab.purchaseComplete(this, p.getItemId(), p.getPaymentId());
        }
		Iab.restoreComplete(this);
	}
	
	@Override
	public void onSucceedInitIap() {
		sdkAvailable = true;
		if(wasChecked)
		{
			Iab.available(this);
		}
	}

	@Override
	public void onBindIapFinished(int result) {
		mSamsungIapHelper.safeInitIap(sActivity.get());
	}
	
}
