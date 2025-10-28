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
				Date approval=ageSignalsResult.mostRecentApprovalDate();
				Integer ageLower=ageSignalsResult.ageLower();
				Integer ageUpper=ageSignalsResult.ageUpper();
				Integer userStatus=ageSignalsResult.userStatus();
	          	onAgeSignals(ageSignalsResult.installId(),
						(userStatus==null)?"":userStatus.toString(),
	        		  	(approval==null)?0:approval.getTime(),
						(ageLower==null)?-1:ageLower,
						(ageUpper==null)?-1:ageUpper,
	        		  sData);
	        })
		.addOnFailureListener( reason -> {
			Log.e("AgeSignals", "checkAgeSignals: failed", reason);
			onAgeSignals("",reason.toString(),0,-1,-1,sData);
		});
	}
	
	private static native void onAgeSignals(String installId,String userStatus,long approvalDate,int ageLower,int ageUpper,long instance);
}
