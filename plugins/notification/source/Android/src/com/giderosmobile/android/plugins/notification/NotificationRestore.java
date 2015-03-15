package com.giderosmobile.android.plugins.notification;

import java.util.Map;
import java.util.Set;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

public class NotificationRestore extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {

    	// Obtain scheduled notification details form Shared Preferences
    	SharedPreferences alarmSettings = context.getSharedPreferences("NotificationData", Context.MODE_PRIVATE);
    	Map<String, ?> allAlarms = alarmSettings.getAll();
    	Set<String> alarmIds = allAlarms.keySet();

    	//restore each notification
    	for (String alarmId : alarmIds) {
    		try {
    			JSONObject object = new JSONObject(alarmSettings.getString(alarmId, ""));
    			
    			Intent i = new Intent(context, NotificationClass.class);
    			i.setAction("" + object.getInt("id"));
    			
    			PendingIntent sender = PendingIntent.getBroadcast(context, 0, i, PendingIntent.FLAG_UPDATE_CURRENT);

    			// Get the AlarmManager service
    			AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
    			
    			if(object.has("repeat"))
    			{
    				am.setRepeating(AlarmManager.RTC_WAKEUP, object.getLong("time"), object.getLong("repeat"), sender);
    			}
    			else
    			{
    				am.set(AlarmManager.RTC_WAKEUP, object.getLong("time"), sender);
    			}
    			
    		} catch (JSONException e) {}
    	}
    }
}