package com.giderosmobile.android.plugins.iab;

import java.lang.ref.WeakReference;
import java.util.Hashtable;

import android.app.Activity;
import android.content.Intent;

public interface IabInterface {
	
	public void onCreate(WeakReference<Activity> activity);
	
	public void onDestroy();
	
	public void onStart();
	
	public void onActivityResult(final int requestCode, final int resultCode, final Intent data);
	
	public void init(Object parameters);
	
	public void check();
	
	public void request(Hashtable<String, String> products);
	
	public void purchase(String productId);
	
	public void restore();
	
}
