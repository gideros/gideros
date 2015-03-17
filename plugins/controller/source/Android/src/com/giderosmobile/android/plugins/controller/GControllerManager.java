package com.giderosmobile.android.plugins.controller;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import android.app.Activity;

import android.view.WindowManager;

public class GControllerManager
{
	public static WeakReference<Activity> sActivity;
	private static long sData = 0;
	private static HashMap<String, GControllerInterface> types = new HashMap<String,GControllerInterface>();
	private static HashMap<String, Integer> allDevices = new HashMap<String,Integer>();
	private static HashMap<String, GController> devices = new HashMap<String,GController>();
	private static HashMap<Integer, String> playerIds = new HashMap<Integer, String>();
     
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
	}
    
    static public void onDestroy()
    {
    	cleanup();
    }
    
    static public void onPause()
    {
    	for (GControllerInterface value : types.values()) {
			value.onPause();
		}
    }
    
    static public void onResume()
    {
    	for (GControllerInterface value : types.values()) {
			value.onResume();
		}
    }
	
	/*
	 * Gideros methods
	 */
	
	static public void init(long data)
	{
		sActivity.get().runOnUiThread(new Runnable(){
			public void run(){
				sActivity.get().getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			}
		});
		sData = data;
		addInterface("GControllerDefault");
		addInterface("GControllerMoga");
		addInterface("GControllerGreenThrottle");
	}
	
	
	static public void cleanup()
	{
   		sData = 0;
   		for (GControllerInterface value : types.values()) {
			value.onDestroy();
		}
    	types.clear();
    	devices.clear();
    	allDevices.clear();
    	playerIds.clear();
	}
	
	static public int isAnyAvailable()
	{
		if(devices.size() > 0)
		{
			return 1;
		}
		return 0;
	}
	
	static public int getPlayerCount(){
		return devices.size();
	}
	
	static public String getControllerName(int id){
		if(playerIds.containsKey(id)){
			String internalId = getKeyByValue(id);
			return types.get(playerIds.get(id)).getControllerName(internalId);
		}
		return "No such controller";
	}
	
	static public int[] getPlayers(){
		int players[] = new int[getPlayerCount()];
		Iterator<Map.Entry<String,GController>> iter = devices.entrySet().iterator();
		int i = 0;
		while (iter.hasNext()) {
			Map.Entry<String,GController> entry = iter.next();
			players[i] = entry.getValue().getPlayerId();
			i++;
		}
		return players;
	}
	
	static public void vibrate(int id, long ms){
		if(playerIds.containsKey(id)){
			String internalId = getKeyByValue(id);
			types.get(playerIds.get(id)).vibrate(internalId, ms);
		}
	}
	
	/*
	 * Gideros Events
	 */
	
	static public void onKeyDownEvent(int keyCode, int playerId)
	{
		if(sData != 0)
			onKeyDownEvent(keyCode, playerId, sData);
	}
	
	static public void onKeyUpEvent(int keyCode, int playerId)
	{
		if(sData != 0)
			onKeyUpEvent(keyCode, playerId, sData);
	}
	
	static public void onRightJoystick(float x, float y, double angle, double strength, int playerId)
	{
		if(sData != 0)
			onRightJoystick(x, y, angle, strength, playerId, sData);
	}
	
	static public void onLeftJoystick(float x, float y, double angle, double strength, int playerId)
	{
		if(sData != 0)
			onLeftJoystick(x, y, angle, strength, playerId, sData);
	}
	
	static public void onRightTrigger(double strength, int playerId)
	{
		if(sData != 0)
			onRightTrigger(strength, playerId, sData);
	}
	
	static public void onLeftTrigger(double strength, int playerId)
	{
		if(sData != 0)
			onLeftTrigger(strength, playerId, sData);
	}
	
	static public void onConnectedEvent(int playerId)
	{
		if(sData != 0)
			onConnected(playerId, sData);
	}
	
	static public void onDisconnectedEvent(int playerId)
	{
		if(sData != 0)
			onDisconnected(playerId, sData);
	}
    
	private static native void onConnected(int playerId, long data);
	private static native void onDisconnected(int playerId, long data);
	private static native void onKeyDownEvent(int keyCode, int playerId, long data);
	private static native void onKeyUpEvent(int keyCode, int playerId, long data);
	private static native void onRightJoystick(float x, float y, double angle, double strength, int playerId, long data);
	private static native void onLeftJoystick(float x, float y, double angle, double strength, int playerId, long data);
	private static native void onRightTrigger(double strength, int playerId, long data);
	private static native void onLeftTrigger(double strength, int playerId, long data);
	
	/*
	 * Helping methods
	 */
	
	private static void addInterface(String name){
		String className = "com.giderosmobile.android.plugins.controller." + name;
		Class classz = null;
		boolean isClass = true;
		try {
			classz = Class.forName(className);
		} catch (ClassNotFoundException e) {
			isClass = false;
		}
		if(isClass)
		{
			try {
				GControllerInterface controllerType = (GControllerInterface)classz.newInstance();
				controllerType.onCreate(sActivity);
				types.put(name, controllerType);
			} catch (InstantiationException e) {
			} catch (IllegalAccessException e) {
			}
		}
	}
	
	static public void addDevice(String id, String type){
		if(!devices.containsKey(id))
		{
			if(!allDevices.containsKey(id))
			{
				int playerId = allDevices.size()+1;
				allDevices.put(id, playerId);
				playerIds.put(playerId, type);
			}
			devices.put(id, new GController(allDevices.get(id)));
		}
	}
	
	static public void removeDevice(String id){
		if(devices.containsKey(id))
		{
			devices.get(id).destroy();
			devices.remove(id);
		}
	}
	
	static public GController getController(String id, String type){
		addDevice(id, type);
		return devices.get(id);
	}
	
	static public String getKeyByValue(Integer value) {
	    for (Map.Entry<String, Integer> entry : allDevices.entrySet()) {
	        if (value.equals(entry.getValue())) {
	            return entry.getKey();
	        }
	    }
	    return null;
	}

};
