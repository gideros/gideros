package com.giderosmobile.android.plugins.googlelicensing;

import java.lang.ref.WeakReference;
import java.util.Random;

import com.google.android.vending.expansion.downloader.DownloadProgressInfo;
import com.google.android.vending.expansion.downloader.DownloaderClientMarshaller;
import com.google.android.vending.expansion.downloader.DownloaderServiceMarshaller;
import com.google.android.vending.expansion.downloader.IDownloaderClient;
import com.google.android.vending.expansion.downloader.IDownloaderService;
import com.google.android.vending.expansion.downloader.IStub;
import com.google.android.vending.licensing.AESObfuscator;
import com.google.android.vending.licensing.LicenseChecker;
import com.google.android.vending.licensing.LicenseCheckerCallback;
import com.google.android.vending.licensing.Policy;
import com.google.android.vending.licensing.ServerManagedPolicy;
import com.google.android.vending.licensing.util.Base64;
import com.google.android.vending.licensing.util.Base64DecoderException;

import android.os.Messenger;
import android.provider.Settings.Secure;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager.NameNotFoundException;

public class GoogleLVL implements LicenseCheckerCallback, IDownloaderClient {
	//reference to main activity
	private static WeakReference<Activity> sActivity;
	private static LicenseChecker mChecker;
	private static String deviceID;
	private static IStub mDownloaderClientStub;
	private static IDownloaderService mRemoteService;
	private static long sData;
	private static boolean useCellurar = false;
	static String KEY;
	static byte[] SALT; 
	
	/****************
	 * Here we will register to all
	 * Application life cycle events
	 ****************/
	
	//on create event from Gideros
	//receives reference to current activity
	//just in case if you might need it
	public static void onCreate(Activity activity)
	{
		//reference to activity
		sActivity =  new WeakReference<Activity>(activity);
		deviceID = Secure.getString(sActivity.get().getContentResolver(), Secure.ANDROID_ID);
		//get salt
		SALT = loadData();
		if(SALT.length == 0)
		{
			//if no salts
			SALT = new byte[20];
			//generate new one
			new Random().nextBytes(SALT);
			//and save it
			saveData(SALT);
		}
	}
	
	public static void onResume(){
		if (null != mDownloaderClientStub) {
	        mDownloaderClientStub.connect(sActivity.get());
	    }
	}
	
	public static void onStop(){
		 if (null != mDownloaderClientStub) {
			 mDownloaderClientStub.disconnect(sActivity.get());
		 }
	}
	
	//on destroy event
	public static void onDestroy()
	{
		 mChecker.onDestroy();	
	}
	
	public static void init(long data){
		sData = data;
	}
	
	public static void cleanup(){
		sData = 0;
	}
	
	@Override
	public void allow(int reason) {
		if(sData != 0)
		{
			GoogleLVL.onLicenseAllowed(sData);
		}
	}

	@Override
	public void dontAllow(int reason) {
		if(sData != 0)
		{
			if (reason == Policy.RETRY) {
				GoogleLVL.onLicenseRetry(sData);
			} else {
				GoogleLVL.onLicenseDisallowed(sData);
			}
		}
	}

	@Override
	public void applicationError(int errorCode) {
		if(sData != 0)
		{
			//Developer errors go here (usually incorrectly set up environment)
			String error = "Unknown error";
			if(errorCode == ERROR_NOT_MARKET_MANAGED){
				error = "The application (package name) was not recognized by Google Play";
			}
			else if(errorCode == ERROR_INVALID_PACKAGE_NAME){
				error = "The application requested a license check for a package that is not installed on the device";
			}
			else if(errorCode == ERROR_INVALID_PUBLIC_KEY){
				error = "Invalid public key";
			}
			else if(errorCode == ERROR_MISSING_PERMISSION){
				error = "Missing CHECK_LICENSE permission in AndroidManifest";
			}
			else if(errorCode == ERROR_NON_MATCHING_UID){
				error = "The application requested a license check for a package whose UID (package, user ID pair) does not match that of the requesting application";
			}
			else if(errorCode == ERROR_CHECK_IN_PROGRESS){
				error = "Check is already in progress";
			}
			GoogleLVL.onError(error, sData);
		}
	}
	
	public static void setKey(String id){
		KEY = id;
        // Construct the LicenseChecker with a Policy.
        mChecker = new LicenseChecker(
        	sActivity.get(), new ServerManagedPolicy(sActivity.get(),
                new AESObfuscator(SALT, sActivity.get().getPackageName(), deviceID)),
            id
        );
	}
	
	public static void checkExpansion(){
		 // Build an Intent to start this activity from the Notification
        Intent notifierIntent = new Intent(sActivity.get(), sActivity.get().getClass());
        notifierIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
                                Intent.FLAG_ACTIVITY_CLEAR_TOP);
        PendingIntent pendingIntent = PendingIntent.getActivity(sActivity.get(), 0,
                notifierIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        // Start the download service (if required)
        int startResult = -1;
		try {
			startResult = DownloaderClientMarshaller.startDownloadServiceIfRequired(sActivity.get(),
			                pendingIntent, GiderosAPKDownloader.class);
		} catch (NameNotFoundException e) {
			if(sData != 0)
				onError(e.getLocalizedMessage(), sData);
			e.printStackTrace();
		}
        // If download has started, initialize this activity to show download progress
        if (startResult != DownloaderClientMarshaller.NO_DOWNLOAD_REQUIRED) {
        	sActivity.get().runOnUiThread(new Runnable() {
    			@Override
    			 public void run() {
    				mDownloaderClientStub = DownloaderClientMarshaller.CreateStub(new GoogleLVL(),
    	                    GiderosAPKDownloader.class);
    				mDownloaderClientStub.connect(sActivity.get());
    			}
    		});
        	if(sData != 0)
        		onDownloadRequired(sData);
        }
        else
        {
        	if(sData != 0)
        		onDownloadNotRequired(sData);
        }
	}
	
	public static void checkLicense(){
		if(mChecker != null)
		{
			mChecker.checkAccess(new GoogleLVL());
		}
	}
	
	public static void cellularDownload(int use){
		if(use == 1)
		{
			//if service is already initiated
			if(mRemoteService != null)
				mRemoteService.setDownloadFlags(IDownloaderService.FLAGS_DOWNLOAD_OVER_CELLULAR);
			useCellurar = true;
		}
		else
		{
			//if service is already initiated
			if(mRemoteService != null)
				mRemoteService.setDownloadFlags(0);
			useCellurar = false;
		}
	}
	
	
	private static native void onLicenseAllowed(long data);
	private static native void onLicenseDisallowed(long data);
	private static native void onLicenseRetry(long data);
	private static native void onDownloadRequired(long data);
	private static native void onDownloadNotRequired(long data);
	private static native void onDownloadProgress(float speed, long time, long progress, long total, long data);
	private static native void onDownloadState(String state, String message, long data);
	private static native void onError(String error, long data);
	
	private static boolean saveData(byte[] sKey)
	{
		SharedPreferences sets = sActivity.get().getSharedPreferences("GoogleLVL", Context.MODE_PRIVATE);
		SharedPreferences.Editor mEdit = sets.edit();
		mEdit.putString("SALT", Base64.encode(sKey)); 

	    return mEdit.commit();     
	}
	
	public static byte[] loadData() {  
		SharedPreferences sets = sActivity.get().getSharedPreferences("GoogleLVL", Context.MODE_PRIVATE);
		String salt = sets.getString("SALT", "");
		byte[] sKey = null;
		try {
			sKey = Base64.decode(salt);
		} catch (Base64DecoderException e) {}
		return sKey;
		
	}

	@Override
	public void onServiceConnected(Messenger m) {
		mRemoteService = DownloaderServiceMarshaller.CreateProxy(m);
	    mRemoteService.onClientUpdated(mDownloaderClientStub.getMessenger());
	    if(useCellurar){
	    	mRemoteService.setDownloadFlags(IDownloaderService.FLAGS_DOWNLOAD_OVER_CELLULAR);
	    }
	}

	@Override
	public void onDownloadStateChanged(int newState) {
		if(sData != 0){
			switch (newState) {
            case IDownloaderClient.STATE_IDLE:
            	onDownloadState("STATE_IDLE", "Waiting for download to start", sData);
            case IDownloaderClient.STATE_FETCHING_URL:
            	onDownloadState("STATE_FETCHING_URL", "Looking for resources to download", sData);
            case IDownloaderClient.STATE_CONNECTING:
            	onDownloadState("STATE_CONNECTING", "Connecting to the download server", sData);
            case IDownloaderClient.STATE_DOWNLOADING:
            	onDownloadState("STATE_DOWNLOADING", "Downloading resources", sData);
            case IDownloaderClient.STATE_COMPLETED:
            	onDownloadState("STATE_COMPLETED", "Download finished", sData);
            case IDownloaderClient.STATE_PAUSED_NETWORK_UNAVAILABLE:
            	onDownloadState("STATE_PAUSED_NETWORK_UNAVAILABLE", "Download paused because no network is available", sData);
            case IDownloaderClient.STATE_PAUSED_BY_REQUEST:
            	onDownloadState("STATE_PAUSED_BY_REQUEST", "Download paused", sData);
            case IDownloaderClient.STATE_PAUSED_WIFI_DISABLED_NEED_CELLULAR_PERMISSION:
            	onDownloadState("STATE_PAUSED_WIFI_DISABLED_NEED_CELLULAR_PERMISSION", "Download paused because wifi is disabled", sData);
            case IDownloaderClient.STATE_PAUSED_NEED_CELLULAR_PERMISSION:
            	onDownloadState("STATE_PAUSED_NEED_CELLULAR_PERMISSION", "Download paused because wifi is unavailable", sData);
            case IDownloaderClient.STATE_PAUSED_WIFI_DISABLED:
            	onDownloadState("STATE_PAUSED_WIFI_DISABLED", "Download paused because wifi is disabled", sData);
            case IDownloaderClient.STATE_PAUSED_NEED_WIFI:
            	onDownloadState("STATE_PAUSED_NEED_WIFI", "Download paused because wifi is unavailable", sData);
            case IDownloaderClient.STATE_PAUSED_ROAMING:
            	onDownloadState("STATE_PAUSED_ROAMING", "Download paused because you are roaming", sData);
            case IDownloaderClient.STATE_PAUSED_NETWORK_SETUP_FAILURE:
            	onDownloadState("STATE_PAUSED_NETWORK_SETUP_FAILURE", "Download paused. Test a website in browser", sData);
            case IDownloaderClient.STATE_PAUSED_SDCARD_UNAVAILABLE:
            	onDownloadState("STATE_PAUSED_SDCARD_UNAVAILABLE", "Download paused because the external storage is unavailable", sData);
            case IDownloaderClient.STATE_FAILED_UNLICENSED:
            	onDownloadState("STATE_FAILED_UNLICENSED", "Download failed because you may not have purchased this app", sData);
            case IDownloaderClient.STATE_FAILED_FETCHING_URL:
            	onDownloadState("STATE_FAILED_FETCHING_URL", "Download failed because the resources could not be found", sData);
            case IDownloaderClient.STATE_FAILED_SDCARD_FULL:
            	onDownloadState("STATE_FAILED_SDCARD_FULL", "Download failed because the external storage is full", sData);
            case IDownloaderClient.STATE_FAILED_CANCELED:
            	onDownloadState("STATE_FAILED_CANCELED", "Download cancelled", sData);
            default:
            	onDownloadState("STATE_UNKNOWN", "Unknown state", sData);
        }
		}
	}

	@Override
	public void onDownloadProgress(DownloadProgressInfo progress) {
		if(sData != 0)
			onDownloadProgress(progress.mCurrentSpeed, progress.mTimeRemaining, progress.mOverallProgress, progress.mOverallTotal, sData);
	}
}
