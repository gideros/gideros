package com.giderosmobile.android.plugins.iab.frameworks;

import java.lang.ref.WeakReference;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Map;
import java.util.Set;

import com.amazon.device.iap.PurchasingService;
import com.amazon.device.iap.PurchasingListener;
import com.amazon.device.iap.model.FulfillmentResult;
import com.amazon.device.iap.model.Product;
import com.amazon.device.iap.model.ProductDataResponse;
import com.amazon.device.iap.model.PurchaseResponse;
import com.amazon.device.iap.model.PurchaseUpdatesResponse;
import com.amazon.device.iap.model.Receipt;
import com.amazon.device.iap.model.UserDataResponse;
import com.giderosmobile.android.plugins.iab.Iab;
import com.giderosmobile.android.plugins.iab.IabInterface;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.SparseArray;

public class IabAmazon implements IabInterface {
	public static WeakReference<Activity> sActivity;
	public static boolean sdkAvailable = false;
	public static boolean wasChecked = false;
	
	public static Boolean isInstalled(){
		if(android.os.Build.MANUFACTURER == "Amazon" || Iab.isPackageInstalled("com.amazon.venezia") || checkLoader())
			return true;
		return false;
	}
	
	public static boolean checkLoader(){
		boolean res = false;
		try {
            ClassLoader localClassLoader = IabAmazon.class.getClassLoader();
            localClassLoader.loadClass("com.amazon.android.Kiwi");
            res = true;
        } catch (Throwable localThrowable) {
        }
		return res;
	}

	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity = activity;
		PurchasingService.registerListener(sActivity.get().getApplicationContext(), new IabAmazonListener(this));
		PurchasingService.getUserData();
	}

	@Override
	public void onDestroy() {}
	
	@Override
	public void onStart() {}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {}

	@Override
	public void init(Object parameters) {
		
	}

	@Override
	public void check() {
		if(sdkAvailable == true){
			Iab.available(this);
		}
		else
		{
			wasChecked = true;
		}
	}
	

	@Override
	public void request(Hashtable<String, String> products) {
		Set<String> skuSet = new HashSet<String>();
    	Enumeration<String> e = products.keys();
		while(e.hasMoreElements())
		{
			String prodName = e.nextElement();
        	skuSet.add(products.get(prodName));
        }
        PurchasingService.getProductData(skuSet);
	}

	@Override
	public void purchase(String productId) {
		PurchasingService.purchase(productId);
	}

	@Override
	public void restore() {
		PurchasingService.getPurchaseUpdates(true);
	}
}

class IabAmazonListener implements PurchasingListener {
	
	private IabAmazon caller;
	
	public IabAmazonListener(IabAmazon iabAmazon) {
		caller = iabAmazon;
	}

	@Override
	public void onProductDataResponse(ProductDataResponse productDataResponse) {
		switch (productDataResponse.getRequestStatus()) { 
     	case SUCCESSFUL:
     		final Map<String, Product> products = productDataResponse.getProductData();
     		SparseArray<Bundle> arr = new SparseArray<Bundle>();
     		int i = 0; 
     		for (final String key : products.keySet()) {
     			Product item = products.get(key);
     	        Bundle map = new Bundle();
     	        map.putString("productId", item.getSku());
     	        map.putString("title", item.getTitle());
     	        map.putString("description", item.getDescription());
     	        map.putString("price", item.getPrice());
     	        arr.put(i, map);
     	        i++;
     		}
     		Iab.productsComplete(caller, arr);
     		break;
     	case FAILED:
     		Iab.restoreError(caller, "Failed");
     		break;
     }
	}

	@Override
	public void onPurchaseResponse(PurchaseResponse purchaseResponse) {
	 
		switch (purchaseResponse.getRequestStatus()) {
			case SUCCESSFUL:
				final Receipt receipt = purchaseResponse.getReceipt();
				Iab.purchaseComplete(caller, receipt.getSku(), receipt.getReceiptId());
				PurchasingService.notifyFulfillment(receipt.getReceiptId(), FulfillmentResult.FULFILLED);
				break;
			case FAILED:
				Iab.purchaseError(caller, "Purchase Failed");
				break;
			case INVALID_SKU:
				Iab.purchaseError(caller, "Invalid SKU");
				break;
			case ALREADY_PURCHASED:
				Iab.purchaseError(caller, "Item was already purchased");
				break;
			case NOT_SUPPORTED:
				Iab.purchaseError(caller, "Not Supported");
				break;
			default:
				break;
         }
	 
	}

	@Override 
    public void onPurchaseUpdatesResponse(final PurchaseUpdatesResponse purchaseUpdatesResponse) {
		switch (purchaseUpdatesResponse.getRequestStatus()) { 
        	case SUCCESSFUL:
        		for (final Receipt receipt : purchaseUpdatesResponse.getReceipts()) {
        			final String sku = receipt.getSku();
        			switch (receipt.getProductType()) {
        				case ENTITLED:
        					Iab.purchaseComplete(caller, sku, receipt.getReceiptId());
        					break;
        				default:
        					break;
        			}
        		}
        		if (purchaseUpdatesResponse.hasMore()) {
        			PurchasingService.getPurchaseUpdates(false);
        		}
        		else
        		{
        			Iab.restoreComplete(caller);
        		}
        		break;
        	case FAILED:
        		Iab.restoreError(caller, "Request Failed");
        	case NOT_SUPPORTED:
        		Iab.restoreError(caller, "Not supported");
        		break;
        	default:
        		break;
        }
	}

	@Override
	public void onUserDataResponse(UserDataResponse user) {
		IabAmazon.sdkAvailable = true;
		if(IabAmazon.wasChecked)
		{
			Iab.available(caller);
			IabAmazon.wasChecked = false;
		}
	}
} 
