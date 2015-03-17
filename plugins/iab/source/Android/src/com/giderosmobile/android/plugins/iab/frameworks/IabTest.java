package com.giderosmobile.android.plugins.iab.frameworks;

import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import java.util.Enumeration;
import java.util.Hashtable;

import com.giderosmobile.android.plugins.iab.Iab;
import com.giderosmobile.android.plugins.iab.IabInterface;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.util.SparseArray;

public class IabTest implements IabInterface {
	private static WeakReference<Activity> sActivity;

	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity = activity;
		Log.d("IabTest", "created");
	}

	@Override
	public void onDestroy() {
		Log.d("IabTest", "destroyed");
	}
	
	@Override
	public void onStart() {
		Log.d("IabTest", "onStart");
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		Log.d("IabTest", "onActivityResult");
	}

	@Override
	public void init(Object parameters) {
		Log.d("IabTest", "Initiated with params:");
		SparseArray<byte[]> p = (SparseArray<byte[]>)parameters;
		int size = p.size();
        for(int i = 0; i < size; i++) {
        	try {
				Log.d("IabTest", new String(p.get(i), "UTF-8"));
			} catch (UnsupportedEncodingException e) {
				e.printStackTrace();
			}
        }
        Log.d("IabTest", "Initiated end");
	}

	@Override
	public void check() {
		Log.d("IabTest", "checking");
		Iab.available(this);
		Iab.notAvailable(this);
	}
	
	@Override
	public void request(Hashtable<String, String> products) {
		Log.d("IabTest", "requesting");
		SparseArray<Bundle> arr = new SparseArray<Bundle>();
    	int i = 0; 
    	Enumeration<String> e = products.keys();
		while(e.hasMoreElements())
		{
			String prodName = e.nextElement();
        	Bundle map = new Bundle();
        	Log.d("IabTest", "ProdName: "+prodName + "; sku: " + products.get(prodName));
        	map.putString("productId", products.get(prodName));
        	map.putString("title", "Test product " + i);
        	map.putString("description", "Product for testing");
        	map.putString("price", "1.99");
        	arr.put(i, map);
        	i++;
        }
        Iab.productsComplete(this, arr);
        Iab.productsError(this, "Test Products Error");
	}

	@Override
	public void purchase(String productId) {
		Log.d("IabTest", "purchasing: "+productId);
		Iab.purchaseComplete(this, productId, "test_receipt");
		Iab.purchaseError(this, "Test Purchase Error");
	}

	@Override
	public void restore() {
		Log.d("IabTest", "restoring");
		Iab.restoreComplete(this);
		Iab.restoreError(this, "Test Restore Error");
	}
	
}
