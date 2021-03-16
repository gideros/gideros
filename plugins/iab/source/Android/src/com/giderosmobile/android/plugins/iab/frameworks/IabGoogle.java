package com.giderosmobile.android.plugins.iab.frameworks;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;


import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.AcknowledgePurchaseResponseListener;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.SkuDetails;
import com.android.billingclient.api.SkuDetailsParams;
import com.android.billingclient.api.SkuDetailsResponseListener;
import com.giderosmobile.android.plugins.iab.Iab;
import com.giderosmobile.android.plugins.iab.IabInterface;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.SparseArray;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class IabGoogle implements IabInterface, PurchasesUpdatedListener {
	private static WeakReference<Activity> sActivity;
	private BillingClient billingClient;
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
		if (billingClient!=null)
			billingClient.endConnection();
		billingClient=null;
	}
	
	@Override
	public void onStart() {}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
	}

	@Override
	public void init(Object parameters) {
		billingClient = BillingClient.newBuilder(sActivity.get()).setListener(this).build();
		billingClient.startConnection(new BillingClientStateListener() {
			@Override
			public void onBillingSetupFinished(BillingResult result) {
				if (result.getResponseCode()==BillingClient.BillingResponseCode.OK)
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
			public void onBillingServiceDisconnected() {
				sdkAvailable = 0;
				if(wasChecked)
					Iab.notAvailable(this);
			}
		});

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

	Map<String, SkuDetails> inventory=new HashMap<String,SkuDetails>();
	Set<String> purchasing=new HashSet<String>();

	@Override
	public void request(Hashtable<String, String> products) {
		if (sdkAvailable == 1) {
			List<String> skuList = new ArrayList<String>();
			Enumeration<String> e = products.keys();
			while(e.hasMoreElements())
			{
				String prodName = e.nextElement();
				skuList.add(products.get(prodName));
			}
			try {
				billingClient.querySkuDetailsAsync(SkuDetailsParams.newBuilder().setType(BillingClient.SkuType.INAPP).setSkusList(skuList).build(),
						new SkuDetailsResponseListener() {
							@Override
							public void onSkuDetailsResponse(@NonNull BillingResult result, @Nullable List<com.android.billingclient.api.SkuDetails> list) {
								if (result.getResponseCode()!=BillingClient.BillingResponseCode.OK) {
									Iab.productsError(this, result.getDebugMessage());
									return;
								}
								Hashtable<String, String> products = Iab.getProducts(this);
								if(products == null){
									Iab.productsError(this, "Request Failed");
									return;
								}
								Map<String, SkuDetails> inv=new HashMap<String, SkuDetails>();
								for (SkuDetails s:list)
									inv.put(s.getSku(),s);
								inventory=inv;
								SparseArray<Bundle> arr = new SparseArray<Bundle>();
								Enumeration<String> e = products.keys();
								int i = 0;
								while(e.hasMoreElements())
								{
									String prodName = e.nextElement();
									SkuDetails details = inv.get(products.get(prodName));
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
						});
			}
			catch (Exception e2) {
				
			}
		}
	}

	@Override
	public void purchase(String productId) {
		try{
			SkuDetails sku=inventory.get(productId);
			if (sku==null) {
				Iab.purchaseError(this, "No such product id: "+productId);
				return;
			}
			BillingFlowParams purchaseParams =
					BillingFlowParams.newBuilder()
							.setSkuDetails(sku)
							.build();

			billingClient.launchBillingFlow(sActivity.get(), purchaseParams);
			purchasing.add(productId);
		}
		catch(Exception e){
			Iab.purchaseError(this, e.getLocalizedMessage());
		}
	}


	@Override
	public void restore() {
		try {
			Purchase.PurchasesResult purchases = billingClient.queryPurchases(BillingClient.SkuType.INAPP);
			if (purchases.getResponseCode()!=BillingClient.BillingResponseCode.OK) {
				Iab.restoreError(this, "Request Failed");
			}
			else {
				List<Purchase> plist = purchases.getPurchasesList();
				if (plist!=null) {
					for (final Purchase info:plist) {
							if (Iab.isConsumable(info.getSku(), this)) {
								try {
									billingClient.consumeAsync(ConsumeParams.newBuilder().setPurchaseToken(info.getPurchaseToken()).build(), new ConsumeResponseListener() {
										@Override
										public void onConsumeResponse(@NonNull BillingResult result, @NonNull String s) {
											if (result.getResponseCode()==BillingClient.BillingResponseCode.OK) {
												Iab.purchaseComplete(this, info.getSku(), info.getOrderId());
											}
											else {
												Iab.purchaseError(this, result.getDebugMessage());
											}
										}
									});
								} catch (Exception e2) {

								}
							} else {
								Iab.purchaseComplete(this, info.getSku(), info.getOrderId());
							}
					}
					Iab.restoreComplete(this);
				}
			}
		}
		catch (Exception e2) {
		}
	}

	@Override
	public void onPurchasesUpdated(@NonNull BillingResult result, @Nullable List<com.android.billingclient.api.Purchase> list) {
		if (result.getResponseCode()!=BillingClient.BillingResponseCode.OK) {
			Iab.purchaseError(this, result.getDebugMessage());
			return;
		}
		for (final Purchase info:list) {
			if (purchasing.contains(info.getSku())) {
				if (Iab.isConsumable(info.getSku(), this)) {
					try {
						billingClient.consumeAsync(ConsumeParams.newBuilder().setPurchaseToken(info.getPurchaseToken()).build(), new ConsumeResponseListener() {
							@Override
							public void onConsumeResponse(@NonNull BillingResult result, @NonNull String s) {
								if (result.getResponseCode()==BillingClient.BillingResponseCode.OK) {
									Iab.purchaseComplete(this, info.getSku(), info.getOrderId());
								}
								else {
									Iab.purchaseError(this, result.getDebugMessage());
								}
							}
						});
					} catch (Exception e2) {

					}
				} else {
					billingClient.acknowledgePurchase(AcknowledgePurchaseParams.newBuilder().setPurchaseToken(info.getPurchaseToken()).build(), new AcknowledgePurchaseResponseListener() {
						@Override
						public void onAcknowledgePurchaseResponse(@NonNull BillingResult result) {
							if (result.getResponseCode()==BillingClient.BillingResponseCode.OK) {
								Iab.purchaseComplete(this, info.getSku(), info.getOrderId());
							}
							else {
								Iab.purchaseError(this, result.getDebugMessage());
							}
						}
					});
					Iab.purchaseComplete(this, info.getSku(), info.getOrderId());
				}
			}
		}
		purchasing.clear();;
	}
}