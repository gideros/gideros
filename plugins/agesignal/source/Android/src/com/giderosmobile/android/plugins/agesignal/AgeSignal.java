package com.giderosmobile.android.plugins.agesignal;

import java.util.Date;
import android.app.Activity;
import android.util.Log;

import com.google.android.play.agesignals.AgeSignalsManager;
import com.google.android.play.agesignals.AgeSignalsManagerFactory;
import com.google.android.play.agesignals.AgeSignalsRequest;

public class AgeSignal {
	private static AgeSignalsManager ageSignalsManager;
	private static long sData;
	
	//on create event from Gideros
	//receives reference to current activity
	//just in case if you might need it
	public static void onCreate(Activity activity)
	{
		ageSignalsManager =
			    AgeSignalsManagerFactory.create(activity.getApplicationContext());
	}
	
	public static void onResume(){
	}
	
	public static void onStop(){
	}
	
	//on destroy event
	public static void onDestroy()
	{
	}
	
	public static void init(long data){
		sData = data;
	}
	
	public static void cleanup(){
		sData = 0;
	}
		
	public static void checkAgeSignals(){
		ageSignalsManager
	    .checkAgeSignals(AgeSignalsRequest.builder().build())
	    .addOnSuccessListener(
	        ageSignalsResult -> {
	          Date approval=null;
			  //approval=ageSignalsResult.mostRecentApprovalDate(); XXX doesn't compile, API is broken
	          onAgeSignals(ageSignalsResult.installId(),
                      ageSignalsResult.userStatus().toString(), //XXX Does this return the int value or the string value ?
	        		  (approval==null)?0:approval.getTime(),
	        		  ageSignalsResult.ageLower(),
	        		  ageSignalsResult.ageUpper(),
	        		  sData);
	        })
		.addOnFailureListener( reason -> {
			Log.e("AgeSignals", "checkAgeSignals: failed", reason);
		});
	}
	
	private static native void onAgeSignals(String installId,String userStatus,long approvalDate,int ageLower,int ageUpper,long instance);
}
