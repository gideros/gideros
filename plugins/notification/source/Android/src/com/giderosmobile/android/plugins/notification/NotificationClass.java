package com.giderosmobile.android.plugins.notification;

import java.lang.ref.WeakReference;
import java.util.Calendar;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.json.JSONException;
import org.json.JSONObject;

import com.google.android.gcm.GCMRegistrar;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.AssetFileDescriptor;
import android.content.res.Resources;
import android.media.MediaPlayer;
import android.os.Bundle;

import android.util.SparseArray;

public class NotificationClass extends BroadcastReceiver 
{
	private static WeakReference<Activity> sActivity;
	private static SparseArray<GNotification> mBuilders;
	private static Map<String, Integer> addDate;
	private static int icon;
	private static String mainClass;
	private static String appName;
	private static String pathPrefix;
	private static NotificationManager mNotificationManager;
	private static volatile boolean canDispatch = false;
	private static volatile boolean inBackground = true;
	private static volatile long sData = 0;
	
	//application events
	public static void onCreate(Activity activity)
	{
		//reference to activity
		sActivity =  new WeakReference<Activity>(activity);
		//create a map to store notifications
		mBuilders = new SparseArray<GNotification>();
		
		icon = sActivity.get().getApplicationInfo().icon;
		mainClass = sActivity.get().getClass().getName();
		Resources appR = sActivity.get().getResources();
		appName = (String) appR.getText(appR.getIdentifier("app_name", "string", sActivity.get().getPackageName()));
		pathPrefix = transformPath("");

		Editor alarmSettingsEditor = sActivity.get().getSharedPreferences("NotificationClassData", Context.MODE_PRIVATE).edit();
		alarmSettingsEditor.putInt("icon", icon);
		alarmSettingsEditor.putString("mainClass", mainClass);
		alarmSettingsEditor.putString("appName", appName);
		alarmSettingsEditor.putString("pathPrefix", pathPrefix);
		alarmSettingsEditor.commit();
		
		addDate = new HashMap<String, Integer>();
		addDate.put("year", Calendar.YEAR);
		addDate.put("month", Calendar.MONTH);
		addDate.put("day", Calendar.DAY_OF_MONTH);
		addDate.put("hour", Calendar.HOUR_OF_DAY);
		addDate.put("min", Calendar.MINUTE);
		addDate.put("sec", Calendar.SECOND);
		
	}
	
	public static void onDestroy()
	{
		int size= mBuilders.size();

		for(int i = 0; i < size; i++) {
		    cleanup(mBuilders.keyAt(i));
		}
	}
		
	@Override
	public void onReceive(Context context, Intent intent) {
		Bundle bundle = intent.getExtras();
		int id = bundle.getInt("id");
		JSONObject object = GNPersistent.get(context, "NotificationData", id+"");
		if(object != null)
		{
			GNotification mBuilder = new GNotification();
			mBuilder.id = id;
			mBuilder.isScheduled = true;
			try {
				mBuilder.title = object.getString("title");
				mBuilder.message = object.getString("message");
				mBuilder.number = object.getInt("number");
				mBuilder.sound = object.getString("sound");
				mBuilder.customData = object.getString("custom");
				mBuilder.soundPrefix = object.getString("soundPrefix");
				mBuilder.mainClass = object.getString("mainClass");
				mBuilder.icon = object.getInt("icon");
				if(object.has("repeat"))
    			{
					mBuilder.shouldRepeat = true;
    			}
				dispatch(context, mBuilder, false);
			} catch (JSONException e) {}
		}
		
	}

	public static void onPause(){
		inBackground = true;
	}
	
	public static void onResume()
	{	
		inBackground = false;
		Intent intent = sActivity.get().getIntent();
		if(intent != null)
		{
			Bundle bundle = intent.getExtras();
			if(bundle != null){	
				if(bundle.getBoolean("push_notification"))
				{
					int id = bundle.getInt("id");
					String title = bundle.getString("title");
					String text = bundle.getString("message");
					int number = bundle.getInt("number");
					String sound = bundle.getString("sound");
					String custom = bundle.getString("custom");
					if(canDispatch){
						onPushNotification(id, title, text, number, sound, custom, true);
					}
					else
					{
						GNPersistent.saveEvent(sActivity.get(), "NotificationPushEvent", id, title, text, number, sound, custom, true);
					}
				}
				else
				{
					
					int id = bundle.getInt("id");
					String title = bundle.getString("title");
					String text = bundle.getString("message");
					int number = bundle.getInt("number");
					String sound = bundle.getString("sound");
					String custom = bundle.getString("custom");
					if(canDispatch){
						onLocalNotification(id, title, text, number, sound, custom, true);
					}
					else
					{
						GNPersistent.saveEvent(sActivity.get(), "NotificationLocalEvent", id, title, text, number, sound, custom, true);
					}
				}
			}
		}	
	}
	
	static public void construct(long data)
	{
		sData = data;
		canDispatch = false;
	}
	
	static public void destruct()
	{
		canDispatch = false;
    	sData = 0;
	}
	
	//class shared methods
	public static void init(int id){
		if(mBuilders.get(id) == null)
		{
			GNotification mBuilder = new GNotification();
			mBuilder.icon = icon;
			mBuilder.mainClass = mainClass;
			mBuilder.id = id;
			mBuilder.title = appName;
			mBuilder.soundPrefix = transformPath("");
			JSONObject object = GNPersistent.get(sActivity.get(), "NotificationData", id+"");
			if(object != null)
			{
				mBuilder.isScheduled = true;
				try {
					mBuilder.title = object.getString("title");
					mBuilder.message = object.getString("message");
					mBuilder.number = object.getInt("number");
					mBuilder.sound = object.getString("sound");
					mBuilder.customData = object.getString("custom");
					mBuilder.mainClass = object.getString("mainClass");
					mBuilder.icon = object.getInt("icon");
				} catch (JSONException e) {}
			}
			mBuilders.put(id, mBuilder);
		}
	}
		
	public static void cleanup(int id){
		if(mBuilders.get(id) != null)
		{
			mBuilders.remove(id);
		}
	}

	public static void readyForEvents(){
		canDispatch = true;
		dispatchEvents("NotificationLocalEvent");
		dispatchEvents("NotificationPushEvent");
	}
	
	public static void setTitle(int id, String title){
		if(mBuilders.get(id) != null)
		{
			GNotification mBuilder = mBuilders.get(id);
			mBuilder.title = title;
			if(mBuilder.isScheduled)
			{
				GNPersistent.safe(sActivity.get(), mBuilder, "NotificationData");
			}
		}
		else
		{
			JSONObject object = GNPersistent.get(sActivity.get(), "NotificationData", id+"");
			if(object != null)
			{
				init(id);
			}
		}
	}
	
	public static String getTitle(int id){
		String title = "";
		if(mBuilders.get(id) != null)
		{
			GNotification mBuilder = mBuilders.get(id);
			title = mBuilder.title;
		}
		return title;
	}
	
	public static void setBody(int id, String message){
		if(mBuilders.get(id) != null)
		{
			GNotification mBuilder = mBuilders.get(id);
			mBuilder.message = message;
			if(mBuilder.isScheduled)
			{
				GNPersistent.safe(sActivity.get(), mBuilder, "NotificationData");
			}
		}
		else
		{
			JSONObject object = GNPersistent.get(sActivity.get(), "NotificationData", id+"");
			if(object != null)
			{
				init(id);
			}
		}
	}
	
	public static String getBody(int id){
		String message = "";
		if(mBuilders.get(id) != null)
		{
			GNotification mBuilder = mBuilders.get(id);
			message = mBuilder.message;
		}
		return message;
	}
	
	public static void setNumber(int id, int number){
		if(mBuilders.get(id) != null)
		{
			GNotification mBuilder = mBuilders.get(id);
			mBuilder.number = number;
			if(mBuilder.isScheduled)
			{
				GNPersistent.safe(sActivity.get(), mBuilder, "NotificationData");
			}
		}
		else
		{
			JSONObject object = GNPersistent.get(sActivity.get(), "NotificationData", id+"");
			if(object != null)
			{
				init(id);
			}
		}
	}
	
	public static int getNumber(int id){
		int number = 0;
		if(mBuilders.get(id) != null)
		{
			GNotification mBuilder = mBuilders.get(id);
			number = mBuilder.number;
		}
		return number;
	}
	
	public static void setSound(int id, String sound){
		if(mBuilders.get(id) != null)
		{
			GNotification mBuilder = mBuilders.get(id);
			if(sound.startsWith("./"))
			{
				sound = sound.substring(2);
			}
			mBuilder.sound = sound;
			if(mBuilder.isScheduled)
			{
				GNPersistent.safe(sActivity.get(), mBuilder, "NotificationData");
			}
		}
		else
		{
			JSONObject object = GNPersistent.get(sActivity.get(), "NotificationData", id+"");
			if(object != null)
			{
				init(id);
			}
		}
	}
	
	public static String getSound(int id){
		String sound = "";
		if(mBuilders.get(id) != null)
		{
			GNotification mBuilder = mBuilders.get(id);
			sound = mBuilder.sound;
		}
		return sound;
	}
	
	public static void setCustom(int id, String custom){
		if(mBuilders.get(id) != null)
		{
			GNotification mBuilder = mBuilders.get(id);
			mBuilder.customData = custom;
			if(mBuilder.isScheduled)
			{
				GNPersistent.safe(sActivity.get(), mBuilder, "NotificationData");
			}
		}
		else
		{
			JSONObject object = GNPersistent.get(sActivity.get(), "NotificationData", id+"");
			if(object != null)
			{
				init(id);
			}
		}
	}
	
	public static String getCustom(int id){
		String custom = "";
		if(mBuilders.get(id) != null)
		{
			GNotification mBuilder = mBuilders.get(id);
			custom = mBuilder.customData;
		}
		return custom;
	}
	
	public static void cancel(int id){
		internalCancel(id);
		cleanup(id);
	}

	public static void cancelAll(){
		int size= mBuilders.size();

		for(int i = 0; i < size; i++) {
			cancel(mBuilders.keyAt(i));
		}
		
		// Clear all notification
		NotificationManager nMgr = (NotificationManager) sActivity.get().getSystemService(Context.NOTIFICATION_SERVICE);
		nMgr.cancelAll();
		
		GNPersistent.deleteAll(sActivity.get(), "NotificationData");
	}
	
	public static void registerForPushNotifications(String projectID){
		final String regId = GCMRegistrar.getRegistrationId(sActivity.get());
		if (regId.equals("")) {
			GCMRegistrar.register(sActivity.get(), projectID);
		}
		else
		{
			onPushRegistration(regId);
		}
	}
	
	public static void unregisterForPushNotifications(){
		GCMRegistrar.unregister(sActivity.get());
	}
	
	public static Object getScheduledNotifications(){
		return GNPersistent.getAll(sActivity.get(), "NotificationData");
	}
	
	public static Object getLocalNotifications(){
		return GNPersistent.getAll(sActivity.get(), "NotificationLocal");
	}
	
	public static Object getPushNotifications(){
		return GNPersistent.getAll(sActivity.get(), "NotificationPush");
	}
	
	public static void clearLocalNotifications(){
		GNPersistent.deleteAll(sActivity.get(), "NotificationLocal");
	}
	
	public static void clearPushNotifications(){
		GNPersistent.deleteAll(sActivity.get(), "NotificationPush");
	}

	public static void dispatchNow(int id){
		if(mBuilders.get(id) != null)
		{
			
			GNotification mBuilder = mBuilders.get(id);
			dispatch(sActivity.get(), mBuilder, false);
		}
	}
	
	public static void dispatchAfter(int id, Object parameters){
		if(mBuilders.get(id) != null)
		{
			Bundle map = (Bundle)parameters;
			
			// get current Calendar object and add time
			Calendar cal = Calendar.getInstance();
			
			Set<String> keys = map.keySet();
		    for (String key : keys) {
			    Integer t = Integer.valueOf(map.getString(key));
			    cal.add(addDate.get(key), t);
			}
			
			
			GNotification mBuilder = mBuilders.get(id);
			if(mBuilder.isScheduled)
			{
				internalCancel(id);
			}
			mBuilder.time = cal;
			Intent intent = prepare(mBuilder);
			
			PendingIntent sender = PendingIntent.getBroadcast(sActivity.get(), 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);

			// Get the AlarmManager service
			AlarmManager am = (AlarmManager) sActivity.get().getSystemService(Context.ALARM_SERVICE);
			am.set(AlarmManager.RTC_WAKEUP, cal.getTimeInMillis(), sender);
		}
	}
	
	public static void dispatchAfter(int id, Object dispatchTime, Object repeatTime){
		if(mBuilders.get(id) != null)
		{
			Bundle dispatchMap = (Bundle)dispatchTime;
			Bundle repeatMap = (Bundle)repeatTime;
			
			// get a Calendar object with current time
			Calendar dispatchCal = Calendar.getInstance();
			
			Set<String> keys = dispatchMap.keySet();
		    for (String key : keys) {
			    Integer t = Integer.valueOf(dispatchMap.getString(key));
			    dispatchCal.add(addDate.get(key), t);
			}
			
			
			// get a Calendar object with repeat time
			Calendar repeatCal = Calendar.getInstance();
			repeatCal.setTimeInMillis(0);	
			Set<String> keys2 = repeatMap.keySet();
		    for (String key : keys2) {
			    Integer t = Integer.valueOf(repeatMap.getString(key));
			    repeatCal.set(addDate.get(key), t);
			}
			
			
			GNotification mBuilder = mBuilders.get(id);
			if(mBuilder.isScheduled)
			{
				internalCancel(id);
			}
			mBuilder.time = dispatchCal;
			mBuilder.repeat = repeatCal;
			Intent intent = prepare(mBuilder);
			
			PendingIntent sender = PendingIntent.getBroadcast(sActivity.get(), 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);
			
			// Get the AlarmManager service
			AlarmManager am = (AlarmManager) sActivity.get().getSystemService(Context.ALARM_SERVICE);
			am.setRepeating(AlarmManager.RTC_WAKEUP, dispatchCal.getTimeInMillis(), repeatCal.getTimeInMillis(), sender);
		}
	}
	
	public static void dispatchOn(int id, Object parameters){
		if(mBuilders.get(id) != null)
		{
			Bundle map = (Bundle)parameters;
			
			// get current Calendar object and add time
			Calendar cal = Calendar.getInstance();
			cal.setTimeInMillis(0);
			Set<String> keys = map.keySet();
		    for (String key : keys) {
			    Integer t = Integer.valueOf(map.getString(key));
			    cal.add(addDate.get(key), t);
			}
			
			
			GNotification mBuilder = mBuilders.get(id);
			if(mBuilder.isScheduled)
			{
				internalCancel(id);
			}
			mBuilder.time = cal;
			Intent intent = prepare(mBuilder);
			
			PendingIntent sender = PendingIntent.getBroadcast(sActivity.get(), 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);

			// Get the AlarmManager service
			AlarmManager am = (AlarmManager) sActivity.get().getSystemService(Context.ALARM_SERVICE);
			am.set(AlarmManager.RTC_WAKEUP, cal.getTimeInMillis(), sender);
		}
	}

	public static void dispatchOn(int id, Object dispatchTime, Object repeatTime){
		if(mBuilders.get(id) != null)
		{
			Bundle dispatchMap = (Bundle)dispatchTime;
			Bundle repeatMap = (Bundle)repeatTime;
			
			// get a Calendar object with current time
			Calendar dispatchCal = Calendar.getInstance();
			dispatchCal.setTimeInMillis(0);
			
			Set<String> keys = dispatchMap.keySet();
		    for (String key : keys) {
			    Integer t = Integer.valueOf(dispatchMap.getString(key));
			    dispatchCal.add(addDate.get(key), t);
			}
			
		    // get a Calendar object with repeat time
		    Calendar repeatCal = Calendar.getInstance();
		 	repeatCal.setTimeInMillis(0);	
		 	Set<String> keys2 = repeatMap.keySet();
		 	for (String key : keys2) {
		 	    Integer t = Integer.valueOf(repeatMap.getString(key));
		 	    repeatCal.set(addDate.get(key), t);
		 	}
			
			
			GNotification mBuilder = mBuilders.get(id);
			if(mBuilder.isScheduled)
			{
				internalCancel(id);
			}
			mBuilder.time = dispatchCal;
			mBuilder.repeat = repeatCal;
			Intent intent = prepare(mBuilder);
			
			PendingIntent sender = PendingIntent.getBroadcast(sActivity.get(), 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);

			// Get the AlarmManager service
			AlarmManager am = (AlarmManager) sActivity.get().getSystemService(Context.ALARM_SERVICE);
			am.setRepeating(AlarmManager.RTC_WAKEUP, dispatchCal.getTimeInMillis(), repeatCal.getTimeInMillis(), sender);
		}
	}
	
	//internal methods
	public static void dispatch(Context context, GNotification mBuilder, boolean push){
		Class c = null;
		try {
			c = Class.forName(mBuilder.mainClass);
		} catch (ClassNotFoundException e) {}

		// Creates an explicit intent for main Activity
		Intent resultIntent = new Intent(context, c);
		resultIntent.setAction("Event" + mBuilder.id);
		resultIntent.putExtra("id", mBuilder.id);
		resultIntent.putExtra("title", mBuilder.title);
		resultIntent.putExtra("message", mBuilder.message);
		resultIntent.putExtra("number", mBuilder.number);
		resultIntent.putExtra("sound", mBuilder.sound);
		resultIntent.putExtra("custom", mBuilder.customData);

		if(push)
		{
			resultIntent.putExtra("push_notification", true);
		}
		PendingIntent resultPendingIntent = PendingIntent.getActivity(context, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);

		if(push)
		{
			GNPersistent.safe(context, mBuilder, "NotificationPush");
		}
		else
		{
			if(!mBuilder.shouldRepeat)
			{
				GNPersistent.delete(context, mBuilder.id, "NotificationData");
			}
			GNPersistent.safe(context, mBuilder, "NotificationLocal");
		}
		
		if(!inBackground && canDispatch){
			if(push)
			{
				onPushNotification(mBuilder.id, mBuilder.title, mBuilder.message, mBuilder.number, mBuilder.sound, mBuilder.customData, false);
			}
			else
			{
				onLocalNotification(mBuilder.id, mBuilder.title, mBuilder.message, mBuilder.number, mBuilder.sound, mBuilder.customData, false);
			}
		}
		else
		{
			Notification builder = mBuilder.createNotification(context, resultPendingIntent);
			
			if(mNotificationManager == null){
				//get notification manager service
				mNotificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
			}

			mNotificationManager.notify(mBuilder.id, builder);
		}

	}
	
	private static Intent prepare(GNotification mBuilder){
		GNPersistent.safe(sActivity.get(), mBuilder, "NotificationData");
		
		//create Intent and pass info to it
		Intent intent = new Intent(sActivity.get(), NotificationClass.class);
		intent.setAction("" + mBuilder.id);
		intent.putExtra("id", mBuilder.id);
		return intent;
	}
	
	private static void internalCancel(int id){
		Intent intent = new Intent(sActivity.get(), NotificationClass.class);
		intent.setAction("" + id);

		PendingIntent pi = PendingIntent.getBroadcast(sActivity.get(), 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);
		
		AlarmManager am = (AlarmManager) sActivity.get().getSystemService(Context.ALARM_SERVICE);
		try {
		    am.cancel(pi);
		} catch (Exception e) {}
		
		GNPersistent.delete(sActivity.get(), id, "NotificationData");
	}
	
	public static void dispatchEvents(String repo){
		SharedPreferences alarmSettings = sActivity.get().getSharedPreferences(repo, Context.MODE_PRIVATE);
    	Map<String, ?> allAlarms = alarmSettings.getAll();
    	Set<String> alarmIds = allAlarms.keySet();

    	//restore each notification
    	for (String alarmId : alarmIds) {
    		JSONObject object;
			try {
				object = new JSONObject(alarmSettings.getString(alarmId, ""));
	    		int id = object.getInt("id");
	    		String title = object.getString("title");
	    		String text = object.getString("message");
	    		int number = object.getInt("number");
	    		String sound = object.getString("sound");
	    		String custom = object.getString("custom");
	    		boolean didOpen = object.getBoolean("didOpen");
	    		if(repo.equals("NotificationLocalEvent"))
	    		{
	    			onLocalNotification(id, title, text, number, sound, custom, didOpen);
	    		}
	    		else if(repo.equals("NotificationPushEvent"))
	    		{
	    			onPushNotification(id, title, text, number, sound, custom, didOpen);
	    		}
			} catch (JSONException e) {}
    	}
    	Editor alarmSettingsEditor = sActivity.get().getSharedPreferences(repo, Context.MODE_PRIVATE).edit();

		alarmSettingsEditor.clear();

		alarmSettingsEditor.commit();
	}
	
	//Internal event check
	public static void onLocalNotification(int id, String title, String text, int number, String sound, String custom, boolean didOpen){
		if (sData != 0)
			onLocalNotification(id, title, text, number, sound, custom, didOpen, sData);
	}
	public static void onPushNotification(int id, String title, String text, int number, String sound, String custom, boolean didOpen){
		if (sData != 0)
			onPushNotification(id, title, text, number, sound, custom, didOpen, sData);
	}
	public static void onPushRegistration(String registrationId){
		if (sData != 0)
			onPushRegistration(registrationId, sData);
	}
	public static void onPushRegistrationError(String errorId){
		if (sData != 0)
			onPushRegistrationError(errorId, sData);
	}
	public static String transformPath(String path){
		if (sData != 0)
		{
			return transformPath(path, sData);
		}
		else
		{
			return path;
		}
	}
	
	//external methods
	public static native String transformPath(String path, long data);
	
	//Gideros events
	private static native void onLocalNotification(int id, String title, String text, int number, String sound, String custom, boolean didOpen, long data);
	private static native void onPushNotification(int id, String title, String text, int number, String sound, String custom, boolean didOpen, long data);
	private static native void onPushRegistration(String registrationId, long data);
	private static native void onPushRegistrationError(String errorId, long data);
	
}

//Notification persistent storage class
class GNPersistent
{
	public static void safe(Context ctx, GNotification mBuilder, String repo){
		JSONObject object = new JSONObject();
		try {
			//safe object properties
			object.put("id", mBuilder.id);
			object.put("title", mBuilder.title);
			object.put("message", mBuilder.message);
			object.put("number", mBuilder.number);
			object.put("sound", mBuilder.sound);
			object.put("custom", mBuilder.customData);
			object.put("soundPrefix", mBuilder.soundPrefix);
			object.put("mainClass", mBuilder.mainClass);
			object.put("icon", mBuilder.icon);
			
			//save dispatch time
			if(mBuilder.time != null){
				object.put("time", mBuilder.time.getTimeInMillis());
			}
			
			//save repeat time
			if(mBuilder.repeat != null){
				object.put("repeat", mBuilder.repeat.getTimeInMillis());
			}
		} catch (JSONException e) {}
		
		Editor alarmSettingsEditor = ctx.getSharedPreferences(repo, Context.MODE_PRIVATE).edit();

		alarmSettingsEditor.putString(""+mBuilder.id, object.toString());

		alarmSettingsEditor.commit();
		
	}
	
	public static void saveEvent(Context ctx, String repo, int id, String title, String text, int number, String sound, String custom, boolean didOpen){
		
		JSONObject object = new JSONObject();
		try {
			//safe object properties
			object.put("id", id);
			object.put("title", title);
			object.put("message", text);
			object.put("number", number);
			object.put("sound", sound);
			object.put("custom", custom);
			object.put("didOpen", didOpen);

		} catch (JSONException e) {}
		
		Editor alarmSettingsEditor = ctx.getSharedPreferences(repo, Context.MODE_PRIVATE).edit();

		alarmSettingsEditor.putString(""+id, object.toString());

		alarmSettingsEditor.commit();
		
	}

	public static void delete(Context ctx, int id, String repo){
		Editor alarmSettingsEditor = ctx.getSharedPreferences(repo, Context.MODE_PRIVATE).edit();

		alarmSettingsEditor.remove(""+id);

		alarmSettingsEditor.commit();
	}
	
	public static void deleteAll(Context ctx, String repo){
		Editor alarmSettingsEditor = ctx.getSharedPreferences(repo, Context.MODE_PRIVATE).edit();

		alarmSettingsEditor.clear();

		alarmSettingsEditor.commit();
	}
	
	public static SparseArray<Bundle> getAll(Context ctx, String repo){
		SharedPreferences alarmSettings = ctx.getSharedPreferences(repo, Context.MODE_PRIVATE);
		SparseArray<Bundle> arr = new SparseArray<Bundle>();
		Map<String, ?> allAlarms = alarmSettings.getAll();
    	Set<String> alarmIds = allAlarms.keySet();
    	for (String alarmId : alarmIds) {
    		Bundle temp = new Bundle();
			try {
				JSONObject object = new JSONObject(alarmSettings.getString(alarmId, ""));
				temp.putInt("id", Integer.parseInt(object.getString("id")));
				temp.putString("title", object.getString("title"));
				temp.putString("message", object.getString("message"));
				temp.putInt("number", Integer.parseInt(object.getString("number")));
				temp.putString("sound", object.getString("sound"));
				temp.putString("custom", object.getString("custom"));
			} catch (JSONException e) {}
		    
		    arr.put(Integer.parseInt(alarmId), temp);
		}
    	return arr;
	}
	
	public static JSONObject get(Context ctx, String repo, String Id){
		SharedPreferences alarmSettings = ctx.getSharedPreferences(repo, Context.MODE_PRIVATE);
		JSONObject object = null;
		try {
			object = new JSONObject(alarmSettings.getString(Id, ""));
		} catch (JSONException e) {}

		return object;
	}
}


//notification object class
class GNotification{
	public int id;
	public int number = 0;
	public int icon;
	public String mainClass;
	public String title = "";
	public String message = "";
	public String sound = "";
	public String soundPrefix = "";
	public String customData = "";
	public Calendar time = null;
	public Calendar repeat = null;
	public boolean isScheduled = false;
	public boolean shouldRepeat = false;
	public boolean didOpen = false;
	
	public Notification createNotification(Context context, PendingIntent intent){
		
		//create notification builder
		Notification mBuilder = new Notification(icon, title, System.currentTimeMillis());
		mBuilder.defaults |= Notification.DEFAULT_VIBRATE;
		if(number > 0){
			mBuilder.number = number;
		}
		if(sound.equals("") || sound.equals("default"))
		{
			mBuilder.defaults |= Notification.DEFAULT_SOUND;
		}
		else if(!sound.isEmpty()){
			String soundMod = soundPrefix + sound;

			MediaPlayer m = new MediaPlayer();
			try{
				if(soundMod.startsWith("/"))
				{
					m.setDataSource(soundMod);
				}
				else
				{
					AssetFileDescriptor descriptor = context.getAssets().openFd("assets/"+soundMod);
					m.setDataSource(descriptor.getFileDescriptor(), descriptor.getStartOffset(), descriptor.getLength() );
					descriptor.close();
				}
				m.prepare();
				m.start();
			} catch(Exception e){}
		}
		mBuilder.setLatestEventInfo(context, title, message, intent);
		mBuilder.flags = Notification.FLAG_AUTO_CANCEL;
		return mBuilder;
	}
}