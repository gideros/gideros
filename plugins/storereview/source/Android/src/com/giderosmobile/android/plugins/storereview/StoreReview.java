package com.giderosmobile.android.plugins.storereview;

import android.app.Activity;

import com.google.android.play.core.review.ReviewInfo;
import com.google.android.play.core.review.ReviewManager;
import com.google.android.play.core.review.ReviewManagerFactory;
import com.google.android.play.core.tasks.OnCompleteListener;
import com.google.android.play.core.tasks.Task;

import java.lang.ref.WeakReference;

import androidx.annotation.NonNull;

public class StoreReview {
	
	private static WeakReference<Activity> sActivity;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
	}
	
	public static boolean review()
	{	
		final ReviewManager manager = ReviewManagerFactory.create(sActivity.get());
		Task<ReviewInfo> request = manager.requestReviewFlow();
		request.addOnCompleteListener(new OnCompleteListener<ReviewInfo>() {
			@Override
			public void onComplete(@NonNull Task<ReviewInfo> task) {
				if (task.isSuccessful()) {
					// We can get the ReviewInfo object
					ReviewInfo reviewInfo = task.getResult();
					Task<Void> flow = manager.launchReviewFlow(sActivity.get(), reviewInfo);
				}

			}
		});
		return true;
	}
	
}
