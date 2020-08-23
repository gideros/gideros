package com.giderosmobile.android.plugins.storereview;

import java.lang.ref.WeakReference;
import java.lang.reflect.Method;
import java.util.Hashtable;
import java.util.Map;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.util.SparseArray;

public class StoreReview {
	
	private static WeakReference<Activity> sActivity;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
	}
	
	public static boolean review()
	{	
		ReviewManager manager = ReviewManagerFactory.create(sActivity.get());
		Task<ReviewInfo> request = manager.requestReviewFlow();
		request.addOnCompleteListener(task -> {
		    if (task.isSuccessful()) {
		        // We can get the ReviewInfo object
		        ReviewInfo reviewInfo = task.getResult();
				Task<Void> flow = manager.launchReviewFlow(activity, reviewInfo);
		    }
		});
		return true;
	}
	
}
