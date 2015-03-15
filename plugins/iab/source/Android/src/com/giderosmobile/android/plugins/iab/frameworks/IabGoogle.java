package com.giderosmobile.android.plugins.iab.frameworks;

import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.List;


import com.giderosmobile.android.plugins.iab.Iab;
import com.giderosmobile.android.plugins.iab.IabInterface;
import com.giderosmobile.android.plugins.iab.frameworks.google.IabHelper;
import com.giderosmobile.android.plugins.iab.frameworks.google.IabResult;
import com.giderosmobile.android.plugins.iab.frameworks.google.Inventory;
import com.giderosmobile.android.plugins.iab.frameworks.google.Purchase;
import com.giderosmobile.android.plugins.iab.frameworks.google.SkuDetails;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.SparseArray;

public class IabGoogle implements IabInterface, IabHelper.OnIabSetupFinishedListener, IabHelper.QueryInventoryFinishedListener, IabHelper.OnIabPurchaseFinishedListener, IabHelper.OnConsumeFinishedListener {
	private static WeakReference<Activity> sActivity;
	IabHelper mHelper;
	boolean wasChecked = false;
	int sdkAvailable = -1;
	
	public static Boolean isInstalled(){
		if(Iab.isPackageInstalled("com.android.vending") || 
			Iab.isPackageInstalled("com.google.vending") ||
			Iab.isPackageInstalled("com.google.market"))
			return true;
		return false;
	}

	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity = activity;
	}

	@Override
	public void onDestroy() {
		if (mHelper != null) 
			mHelper.dispose();
		mHelper = null;
	}
	
	@Override
	public void onStart() {}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		mHelper.handleActivityResult(requestCode, resultCode, data);
	}

	@Override
	public void init(Object parameters) {
		SparseArray<byte[]> p = (SparseArray<byte[]>)parameters;
		try {
			mHelper = new IabHelper(sActivity.get(), new String(p.get(0), "UTF-8"));
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
		mHelper.startSetup(this);
	}

	@Override
	public void check() {
		if(sdkAvailable == 1)
			Iab.available(this);
		else if(sdkAvailable == 0)
			Iab.notAvailable(this);
		else
			wasChecked = true;
	}
	
	@Override
	public void request(Hashtable<String, String> products) {
		mHelper.flagEndAsync();
		List<String> skuList = new ArrayList<String>();
    	Enumeration<String> e = products.keys();
		while(e.hasMoreElements())
		{
			String prodName = e.nextElement();
        	skuList.add(products.get(prodName));
        }
        mHelper.queryInventoryAsync(true, skuList, this);
	}

	@Override
	public void purchase(String productId) {
		mHelper.flagEndAsync();
		mHelper.launchPurchaseFlow(sActivity.get(), productId, 10001, this);
	}


	@Override
	public void restore() {
		mHelper.flagEndAsync();
		mHelper.queryInventoryAsync(new IabGooglePurchased(this));
	}

	@Override
	public void onIabSetupFinished(IabResult result) {
		 if (result.isSuccess())
			 sdkAvailable = 1;
		 else
			 sdkAvailable = 0;
	     if(wasChecked)
	     {
	    	 if(sdkAvailable == 1)
	    		 Iab.available(this);
	    	 else
	    		 Iab.notAvailable(this);
	     }
	}

	@Override
	public void onQueryInventoryFinished(IabResult result, Inventory inv) {
		if (result.isFailure()) {
			Iab.productsError(this, "Request Failed");
			return;
		}
		Hashtable<String, String> products = Iab.getProducts(this);
		SparseArray<Bundle> arr = new SparseArray<Bundle>();
		Enumeration<String> e = products.keys();
		int i = 0;
		while(e.hasMoreElements())
		{
			String prodName = e.nextElement();
        	SkuDetails details = inv.getSkuDetails(products.get(prodName));
        	if(details != null)
        	{
        		Bundle map = new Bundle();
        		map.putString("productId", products.get(prodName));
        		map.putString("title", details.getTitle());
        		map.putString("description", details.getDescription());
        		map.putString("price", details.getPrice());
        		arr.put(i, map);
        		i++;
        	}
        }
        Iab.productsComplete(this, arr);
	}

	@Override
	public void onIabPurchaseFinished(IabResult result, Purchase info) {
		if (result.isFailure()) {
			Iab.purchaseError(this, result.getMessage());
			return;
		}
		if(Iab.isConsumable(info.getSku(), this))
		{
			mHelper.consumeAsync(info, this);
		}
		else
		{
			Iab.purchaseComplete(this, info.getSku(), info.getOrderId());
		}
	}

	@Override
	public void onConsumeFinished(Purchase purchase, IabResult result) {
		if (result.isSuccess()) {
			Iab.purchaseComplete(this, purchase.getSku(), purchase.getOrderId());
	    }
	    else {
	    	Iab.purchaseError(this, result.getMessage());
	    }
	}	
}

class IabGooglePurchased implements IabHelper.QueryInventoryFinishedListener{
	IabGoogle caller;
	public IabGooglePurchased(IabGoogle iabGoogle) {
		caller = iabGoogle;
	}
	
	@Override
	public void onQueryInventoryFinished(IabResult result, Inventory inv) {
		if (result.isFailure()) {
			Iab.restoreError(caller, "Request Failed");
		}
		else
		{
			Hashtable<String, String> products = Iab.getProducts(caller);
			Enumeration<String> e = products.keys();
			while(e.hasMoreElements())
			{
				String prodName = e.nextElement();
				if(inv.hasPurchase(products.get(prodName)))
				{
					Purchase info = inv.getPurchase(products.get(prodName));
					Iab.purchaseComplete(caller, products.get(prodName), info.getOrderId());
				}
			}
			Iab.restoreComplete(caller);
		}
	}

	
}
