package com.giderosmobile.android.plugins.exampleplugin;

import com.giderosmobile.android.player.WeakActivityHolder;

import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.content.pm.PackageManager;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.content.Context;
import android.app.Activity;
import android.util.Log;
import android.Manifest;

import java.lang.String;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

public class GExampleplugin {

	private static WeakReference<Activity> sActivity;
	private static Context context;
	private static WifiManager wifiManager;
	private static BroadcastReceiver receiver;
	private static int start=1;
	private static int stop=0;
	private static boolean running = false;
	private static ArrayList<String> permissions_checked;

	public static void onCreate(Activity activity) {
		sActivity =  new WeakReference<Activity>(activity);
		context = activity.getApplicationContext();
		wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
	}
	public static void onDestroy() {
		stop_java();
	}
	//(if needed)  onPause stop()
	//(if needed)  onResume start()
	public static void start_java() {
		running=true;
		onState(start,"START !");
		if (wifiManager != null) {
			IntentFilter filter = new IntentFilter();
			filter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
			receiver = new BroadcastReceiver() {
				@Override
				public void onReceive(Context context, Intent intent) {
					String action = WifiManager.SCAN_RESULTS_AVAILABLE_ACTION;
					if (intent.getAction().equals(action)) {
						int count = 0;
						boolean granted = true;
						String[] permissions = {
							Manifest.permission.CHANGE_WIFI_STATE,		//0
							Manifest.permission.ACCESS_COARSE_LOCATION,	//1
							Manifest.permission.ACCESS_FINE_LOCATION	//2
						};
						permissions_checked = new ArrayList<String>();
						permissions_checked.add(permissions[0]);
						if (android.os.Build.VERSION.SDK_INT >= 23) {
							granted=false;
							Activity activity = WeakActivityHolder.get();
							// if proj targeted <23 then checkSelfPermission will always return PERMISSION_GRANTED
							if (activity.checkSelfPermission(permissions[1])==PackageManager.PERMISSION_GRANTED){
								permissions_checked.add(permissions[1]);
								granted=true;
							}
							if (!granted) {
								if (activity.checkSelfPermission(permissions[2])==PackageManager.PERMISSION_GRANTED){
									permissions_checked.add(permissions[2]);
									granted=true;
								}
							}
							if (!granted){
								activity.requestPermissions(//request at run time !
									new String[]{
										permissions[1],
										permissions[2]
									}, 0);
							}
						}
						if (granted) {
							// if API >= 23 then scans empty if no permissions ACCESS_COARSE_LOCATION (dangerous!) or ACCESS_FINE_LOCATION (dangerous!)
							List<ScanResult> scans = wifiManager.getScanResults();
							if (scans != null) {
								count = scans.size();
								//permissions_checked will be returned by getPermissionsChecked_java()
								onWifi(action,count,granted,permissions);
							}
							else {
								onState(stop,"ScanResult is nil!");
							}
						}
					}
				}
			};
			context.registerReceiver(receiver, filter);
			//need permission CHANGE_WIFI_STATE (not dangerous)
			wifiManager.startScan();
		}
		else {
			onState(stop,"WifiManager is nil !");
		}
	}
	public static void stop_java() {
		running=false;
		onState(stop,"STOP !");
		if (receiver != null) {
			context.unregisterReceiver(receiver);
			receiver = null;
		}
	}
	public static boolean test_java() {
		return running;
	}
	public static ArrayList<String> getPermissionsChecked_java() {
		return permissions_checked;
	};
	@SuppressWarnings("JniMissingFunction")
	public static native void onState(int state, String description);
	@SuppressWarnings("JniMissingFunction")
	public static native void onWifi(String a, int c, boolean g, String[] ps_all);
}
