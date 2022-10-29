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
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.ProductDetailsResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryPurchasesParams;
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
		billingClient = BillingClient.newBuilder(sActivity.get()).enablePendingPurchases().setListener(this).build();
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

	Map<String, ProductDetails> inventory=new HashMap<String,ProductDetails>();
	Set<String> purchasing=new HashSet<String>();

	@Override
	public void request(Hashtable<String, String> products) {
		if (sdkAvailable == 1) {
			List<QueryProductDetailsParams.Product> skuList = new ArrayList<QueryProductDetailsParams.Product>();
			Enumeration<String> e = products.keys();
			while(e.hasMoreElements())
			{
				String prodName = e.nextElement();
				skuList.add(QueryProductDetailsParams.Product.newBuilder()
						.setProductId(products.get(prodName))
						.setProductType(BillingClient.ProductType.INAPP)
						.build());
			}
			try {
				billingClient.queryProductDetailsAsync(QueryProductDetailsParams.newBuilder().setProductList(skuList).build(),
						new ProductDetailsResponseListener() {
							@Override
							public void onProductDetailsResponse(@NonNull BillingResult result, @Nullable List<ProductDetails> list) {
								if (result.getResponseCode()!=BillingClient.BillingResponseCode.OK) {
									Iab.productsError(this, result.getDebugMessage());
									return;
								}
								Hashtable<String, String> products = Iab.getProducts(this);
								if(products == null){
									Iab.productsError(this, "Request Failed");
									return;
								}
								Map<String, ProductDetails> inv=new HashMap<String, ProductDetails>();
								for (ProductDetails s:list)
									inv.put(s.getProductId(),s);
								inventory=inv;
								SparseArray<Bundle> arr = new SparseArray<Bundle>();
								Enumeration<String> e = products.keys();
								int i = 0;
								while(e.hasMoreElements())
								{
									String prodName = e.nextElement();
									ProductDetails details = inv.get(products.get(prodName));
									if(details != null)
									{
										Bundle map = new Bundle();
										map.putString("productId", products.get(prodName));
										map.putString("title", details.getTitle());
										map.putString("description", details.getDescription());
										map.putString("price", details.getOneTimePurchaseOfferDetails().getFormattedPrice());
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
			ProductDetails sku=inventory.get(productId);
			if (sku==null) {
				Iab.purchaseError(this, "No such product id: "+productId);
				return;
			}

			// Set the parameters for the offer that will be presented
			// in the billing flow creating separate productDetailsParamsList variable
			List<BillingFlowParams.ProductDetailsParams> productDetailsParamsList = new ArrayList<BillingFlowParams.ProductDetailsParams>();
			productDetailsParamsList.add(
							BillingFlowParams.ProductDetailsParams.newBuilder()
									.setProductDetails(sku)
									.build()
					);

			BillingFlowParams billingFlowParams = BillingFlowParams.newBuilder()
					.setProductDetailsParamsList(productDetailsParamsList)
					.build();

			purchasing.add(productId);
			billingClient.launchBillingFlow(sActivity.get(), billingFlowParams);
		}
		catch(Exception e){
			Iab.purchaseError(this, e.getLocalizedMessage());
		}
	}

	private String getSku(Purchase p) {
		return p.getProducts().get(0);
	}

	@Override
	public void restore() {
		try {
			QueryPurchasesParams params = QueryPurchasesParams.newBuilder().setProductType(BillingClient.ProductType.INAPP).build();
			billingClient.queryPurchasesAsync(params, new PurchasesResponseListener() {
				@Override
				public void onQueryPurchasesResponse(@NonNull BillingResult billingResult, @NonNull List<Purchase> plist) {
					if (billingResult.getResponseCode()!=BillingClient.BillingResponseCode.OK) {
						Iab.restoreError(this, "Request Failed");
					}
					else {
						if (plist!=null) {
							for (final Purchase info:plist) {
								if (info.getPurchaseState() == Purchase.PurchaseState.PURCHASED) {
									if (Iab.isConsumable(getSku(info), this)) {
										try {
											billingClient.consumeAsync(ConsumeParams.newBuilder().setPurchaseToken(info.getPurchaseToken()).build(), new ConsumeResponseListener() {
												@Override
												public void onConsumeResponse(@NonNull BillingResult result, @NonNull String s) {
													if (result.getResponseCode() == BillingClient.BillingResponseCode.OK) {
														Iab.purchaseComplete(this, getSku(info), info.getOrderId());
													} else {
														Iab.purchaseError(this, result.getDebugMessage());
													}
												}
											});
										} catch (Exception e2) {

										}
									} else {
										if (info.isAcknowledged())
											Iab.purchaseComplete(this, getSku(info), info.getOrderId());
										else
											billingClient.acknowledgePurchase(AcknowledgePurchaseParams.newBuilder().setPurchaseToken(info.getPurchaseToken()).build(), new AcknowledgePurchaseResponseListener() {
												@Override
												public void onAcknowledgePurchaseResponse(@NonNull BillingResult result) {
													if (result.getResponseCode() == BillingClient.BillingResponseCode.OK) {
														Iab.purchaseComplete(this, getSku(info), info.getOrderId());
													} else {
														Iab.purchaseError(this, result.getDebugMessage());
													}
												}
											});
									}
								}
							}
							Iab.restoreComplete(this);
						}
					}
				}
			});
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
			if (purchasing.contains(getSku(info))) {
				if (info.getPurchaseState() == Purchase.PurchaseState.PURCHASED) {
					if (Iab.isConsumable(getSku(info), this)) {
						try {
							billingClient.consumeAsync(ConsumeParams.newBuilder().setPurchaseToken(info.getPurchaseToken()).build(), new ConsumeResponseListener() {
								@Override
								public void onConsumeResponse(@NonNull BillingResult result, @NonNull String s) {
									if (result.getResponseCode() == BillingClient.BillingResponseCode.OK) {
										Iab.purchaseComplete(this, getSku(info), info.getOrderId());
									} else {
										Iab.purchaseError(this, result.getDebugMessage());
									}
								}
							});
						} catch (Exception e2) {

						}
					} else {
						if (info.isAcknowledged())
							Iab.purchaseComplete(this, getSku(info), info.getOrderId());
						else
							billingClient.acknowledgePurchase(AcknowledgePurchaseParams.newBuilder().setPurchaseToken(info.getPurchaseToken()).build(), new AcknowledgePurchaseResponseListener() {
								@Override
								public void onAcknowledgePurchaseResponse(@NonNull BillingResult result) {
									if (result.getResponseCode() == BillingClient.BillingResponseCode.OK) {
										Iab.purchaseComplete(this, getSku(info), info.getOrderId());
									} else {
										Iab.purchaseError(this, result.getDebugMessage());
									}
								}
							});
					}
				}
			}
			else {
				Iab.purchaseError(this, "PENDING");
			}
		}
		purchasing.clear();
	}
}