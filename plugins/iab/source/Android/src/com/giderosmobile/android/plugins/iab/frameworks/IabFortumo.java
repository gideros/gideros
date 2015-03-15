package com.giderosmobile.android.plugins.iab.frameworks;

import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.List;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.SparseArray;

import com.fortumo.android.Fortumo;
import com.fortumo.android.PaymentRequest;
import com.fortumo.android.PaymentRequestBuilder;
import com.fortumo.android.PaymentResponse;
import com.giderosmobile.android.plugins.iab.Iab;
import com.giderosmobile.android.plugins.iab.IabInterface;

public class IabFortumo implements IabInterface {
	private static WeakReference<Activity> sActivity;
	private static String SERVICE_ID = "";
	private static String APP_SECRET = "";
	private static final int REQUEST_CODE = 1016;
	
	public static Boolean isInstalled(){
		return true;
	}

	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity = activity;
		Fortumo.enablePaymentBroadcast(sActivity.get(), "com.giderosmobile.android.plugins.iab.fortumo.PAYMENT_BROADCAST_PERMISSION");
	}

	@Override
	public void onDestroy() {
	}
	
	@Override
	public void onStart() {
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (requestCode == REQUEST_CODE) {
			if(data == null) {
				return;
			}
			
			// OK
			if (resultCode == Activity.RESULT_OK) {
				PaymentResponse response = new PaymentResponse(data);
				
				switch (response.getBillingStatus()) {
					case Fortumo.MESSAGE_STATUS_BILLED:  
					Iab.purchaseComplete(this, response.getProductName(), response.getMessageId()+"");
					break;
				case Fortumo.MESSAGE_STATUS_FAILED:
					Iab.purchaseError(this, "Purchase failed");
					break;
				case Fortumo.MESSAGE_STATUS_NOT_SENT:
					Iab.purchaseError(this, "Purchase not sent");
					break;
				case Fortumo.MESSAGE_STATUS_USE_ALTERNATIVE_METHOD:
					Iab.purchaseError(this, "User alternative method");
					break;
				case Fortumo.MESSAGE_STATUS_PENDING:
					// ...
					break;	
				}
			// Cancel
			} else {
				Iab.purchaseError(this, "Purchase canceled");
			}
		} 
	}

	@Override
	public void init(Object parameters) {
		SparseArray<byte[]> p = (SparseArray<byte[]>)parameters;
		try {
			SERVICE_ID = new String(p.get(0), "UTF-8");
			APP_SECRET = new String(p.get(1), "UTF-8");
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
		
	}

	@Override
	public void check() {
		Iab.available(this);
	}
	
	@Override
	public void request(Hashtable<String, String> products) {
		SparseArray<Bundle> arr = new SparseArray<Bundle>();
    	int i = 0; 
    	Enumeration<String> e = products.keys();
		while(e.hasMoreElements())
		{
			String prodName = e.nextElement();
        	Bundle map = new Bundle();
        	map.putString("productId", prodName);
        	map.putString("title", products.get(prodName));
        	map.putString("description", products.get(prodName));
        	map.putString("price", "0.00");
        	arr.put(i, map);
        	i++;
        }
        Iab.productsComplete(this, arr);
	}

	@Override
	public void purchase(String productId) {
		PaymentRequestBuilder builder = new PaymentRequestBuilder();
		builder.setDisplayString(productId);
		builder.setService(SERVICE_ID, APP_SECRET);
        builder.setProductName(productId);
        if(Iab.isConsumable(productId, this))
        {
        	builder.setConsumable(true);
        }
        else
        {
        	builder.setConsumable(false);
        }
        PaymentRequest payment = builder.build();
        sActivity.get().startActivityForResult(payment.toIntent(sActivity.get()), REQUEST_CODE);
	}

	@Override
	public void restore() {
		List<PaymentResponse> products = Fortumo.getPurchaseHistory(sActivity.get(), SERVICE_ID, APP_SECRET, 1000);
		 for(PaymentResponse p : products) {
			 Iab.purchaseComplete(this, p.getProductName(), p.getMessageId()+"");
		 }
	}
}
