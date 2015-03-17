package com.giderosmobile.android.plugins.iab.frameworks;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;

import org.json.JSONException;
import org.json.JSONObject;

import com.giderosmobile.android.plugins.iab.Iab;
import com.giderosmobile.android.plugins.iab.IabInterface;
import com.nokia.payment.iap.aidl.INokiaIAPService;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentSender.SendIntentException;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.SparseArray;

public class IabNokia implements IabInterface {
	private static WeakReference<Activity> sActivity;
	public static final String ITEM_TYPE_INAPP = "inapp";
	INokiaIAPService mService;
	boolean wasChecked = false;
	int sdkAvailable = -1;
	int RC_PURCHASE = 1001;
	ServiceConnection mServiceConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = INokiaIAPService.Stub.asInterface(service);        
		    int response = -1;
			try {
				response = mService.isBillingSupported(3, sActivity.get().getPackageName(), ITEM_TYPE_INAPP);
			} catch (RemoteException e) {
				e.printStackTrace();
			}

			 if (response == 0)
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
		public void onServiceDisconnected(ComponentName name) {
			mService = null;
		}
	};
	
	public static Boolean isInstalled(){
		if(Iab.isPackageInstalled("com.nokia.payment.iapenabler"))
			return true;
		return false;
	}

	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity = activity;
	    Intent paymentEnabler = new Intent("com.nokia.payment.iapenabler.InAppBillingService.BIND");
	    paymentEnabler.setPackage("com.nokia.payment.iapenabler"); 
	    sActivity.get().bindService(paymentEnabler, mServiceConnection, Context.BIND_AUTO_CREATE);
	}

	@Override
	public void onDestroy() {
		if (mServiceConnection != null) {
	        sActivity.get().unbindService(mServiceConnection);
	    }   
	}
	
	@Override
	public void onStart() {}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (requestCode == RC_PURCHASE) {           
	        String purchaseData = data.getStringExtra("INAPP_PURCHASE_DATA");
	        
	        if (resultCode == Activity.RESULT_OK) {
	            try {
	                JSONObject jo = new JSONObject(purchaseData);
	                if(Iab.isConsumable(jo.getString("productId"), this))
	        		{
	                	int response = mService.consumePurchase(3, sActivity.get().getPackageName(), jo.getString("productId"), jo.getString("purchaseToken"));
	                	if (response == 0) {
	                		Iab.purchaseComplete(this, jo.getString("productId"), jo.getString("purchaseToken"));
	                	}
	                	else
	                	{
	                		Iab.purchaseError(this, "Can't consume purchase");
	                	}
	        		}
	                else
	                {
	                	Iab.purchaseComplete(this, jo.getString("productId"), jo.getString("purchaseToken"));
	                }
	            }
	            catch (JSONException e) {
	            	Iab.purchaseError(this, "Can't parse purchase data");
	                e.printStackTrace();
	            } catch (RemoteException e) {
	            	Iab.purchaseError(this, "Can't consume purchase");
					e.printStackTrace();
				}
	        }
	        else
	        {
	        	Iab.purchaseError(this, "Purchase canceled");
	        }
	    }
	}

	@Override
	public void init(Object parameters) {
		
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
		ArrayList<String> productList = new ArrayList<String>();
		Enumeration<String> e = products.keys();
		while(e.hasMoreElements())
		{
			String prodName = e.nextElement();
			productList.add(products.get(prodName));
        }
		Bundle queryProducts = new Bundle();
		queryProducts.putStringArrayList("ITEM_ID_LIST", productList);
		
		Bundle productDetails;
		try {
			productDetails = mService.getProductDetails(3, sActivity.get().getPackageName(), ITEM_TYPE_INAPP, queryProducts);
			int response = productDetails.getInt("RESPONSE_CODE");

			if (response == 0) {
			    ArrayList<String> responseList = productDetails.getStringArrayList("DETAILS_LIST");
			    SparseArray<Bundle> arr = new SparseArray<Bundle>();
			    int i = 0;
			    for (String thisResponse : responseList) {
			        JSONObject object = new JSONObject(thisResponse);		        
			        Bundle map = new Bundle();
		        	map.putString("productId", products.get(object.getString("productId")));
		        	map.putString("title", object.getString("title"));
		        	map.putString("description", object.getString("description"));
		        	map.putString("price",  object.getString("price"));
		        	arr.put(i, map);
		        	i++;
			    }
			    Iab.productsComplete(this, arr);
			}
			else
			{
				Iab.productsError(this, "Can't retrieve products");
			}
		} catch (RemoteException e1) {
			e1.printStackTrace();
			Iab.productsError(this, "Can't retrieve products");
		} catch (JSONException e1) {
			e1.printStackTrace();
			Iab.productsError(this, "Can't retrieve products");
		}
	}

	@Override
	public void purchase(String productId) {
		try {
			Bundle buyIntentBundle = mService.getBuyIntent(3, sActivity.get().getPackageName(),
					productId, ITEM_TYPE_INAPP, "");
			int response = buyIntentBundle.getInt("RESPONSE_CODE");
			if (response == 0) {
				PendingIntent pendingIntent = buyIntentBundle.getParcelable("BUY_INTENT");
				try {
					sActivity.get().startIntentSenderForResult(pendingIntent.getIntentSender(), RC_PURCHASE, new Intent(),
						    Integer.valueOf(0), Integer.valueOf(0), Integer.valueOf(0));
				} catch (SendIntentException e) {
					e.printStackTrace();
				}
			}
			else{
				Iab.purchaseError(this, "Can't make purchase");
			}
		} catch (RemoteException e) {
			e.printStackTrace();
			Iab.purchaseError(this, "Can't make purchase");
		}
	}

	@Override
	public void restore() {
		ArrayList<String> productList = new ArrayList<String>();
		Hashtable<String, String> products = Iab.getProducts(this);
		Enumeration<String> e = products.keys();
		while(e.hasMoreElements())
		{
			String prodName = e.nextElement();
			productList.add(products.get(prodName));
        }
		Bundle queryProducts = new Bundle();
		queryProducts.putStringArrayList("ITEM_ID_LIST", productList);

		String continuationToken = null;

		try {
			Bundle ownedItems = mService.getPurchases(3, sActivity.get().getPackageName(), ITEM_TYPE_INAPP, queryProducts, continuationToken);
			int response = ownedItems.getInt("RESPONSE_CODE");

			if (response == 0) {
			    ArrayList<String> ownedProducts = ownedItems.getStringArrayList("INAPP_PURCHASE_ITEM_LIST");
			    ArrayList<String> purchaseDataList = ownedItems.getStringArrayList("INAPP_PURCHASE_DATA_LIST");
			    
			    for (int i = 0; i < purchaseDataList.size(); ++i) {
			        JSONObject purchaseData = new JSONObject(purchaseDataList.get(i));
			        JSONObject product = new JSONObject(ownedProducts.get(i));
			        
			        Iab.purchaseComplete(this, products.get(product.getString("productId")), purchaseData.getString("purchaseToken"));
			    } 
			    Iab.restoreComplete(this);
			}
			else{
				Iab.restoreError(this, "Can't restore purchases");
			}
		} catch (RemoteException e1) {
			Iab.restoreError(this, "Can't restore purchases");
			e1.printStackTrace();
		} catch (JSONException e1) {
			Iab.restoreError(this, "Can't restore purchases");
			e1.printStackTrace();
		}
	}
	
}
