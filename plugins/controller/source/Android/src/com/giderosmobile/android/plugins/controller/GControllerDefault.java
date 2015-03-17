package com.giderosmobile.android.plugins.controller;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.content.Context;
import android.hardware.input.InputManager;
import android.os.Handler;
import android.os.Vibrator;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class GControllerDefault implements GControllerInterface {

	private static GControllerListener sInstance;
	public static WeakReference<Activity> sActivity;
	private static HashMap<String,InputDevice> playerDevices = new HashMap<String,InputDevice>();
	private static HashMap<Integer,String> systemDevices = new HashMap<Integer,String>();
	private static Timer timer;
	
	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity = activity;
		int currentapiVersion = android.os.Build.VERSION.SDK_INT;
		if (currentapiVersion >= android.os.Build.VERSION_CODES.JELLY_BEAN){
			sInstance = new GControllerListener();
			sActivity.get().runOnUiThread(new Runnable() 
			{
				public void run() 
				{
					InputManager inp = (InputManager) sActivity.get().getSystemService(Context.INPUT_SERVICE);
					inp.registerInputDeviceListener(sInstance, new Handler());
				}
			});
		}
		else if(currentapiVersion >= android.os.Build.VERSION_CODES.HONEYCOMB_MR1)
		{
			startTask();
		}
		checkForDevices();
	}

	@Override
	public void onDestroy() {
		int currentapiVersion = android.os.Build.VERSION.SDK_INT;
		if (currentapiVersion >= android.os.Build.VERSION_CODES.JELLY_BEAN){
				InputManager inp = (InputManager) sActivity.get().getSystemService(Context.INPUT_SERVICE);
				inp.unregisterInputDeviceListener(sInstance);
		}
		else if(currentapiVersion >= android.os.Build.VERSION_CODES.HONEYCOMB_MR1)
		{
			stopTask();
		}
		playerDevices.clear();
		systemDevices.clear();
	}

	@Override
	public void onPause() {
    	int currentapiVersion = android.os.Build.VERSION.SDK_INT;
    	if (currentapiVersion >= android.os.Build.VERSION_CODES.JELLY_BEAN){
    		
    	}
    	else if(currentapiVersion >= android.os.Build.VERSION_CODES.HONEYCOMB_MR1)
		{
    		stopTask();
		}
	}

	@Override
	public void onResume() {
		int currentapiVersion = android.os.Build.VERSION.SDK_INT;
		if (currentapiVersion >= android.os.Build.VERSION_CODES.JELLY_BEAN){
			
		}
		else if(currentapiVersion >= android.os.Build.VERSION_CODES.HONEYCOMB_MR1)
		{
    		startTask();
		}
	}
	
	public void vibrate(String id, long ms){
		int currentapiVersion = android.os.Build.VERSION.SDK_INT;
		if(currentapiVersion >= android.os.Build.VERSION_CODES.JELLY_BEAN && playerDevices.containsKey(id)){
			InputDevice device = playerDevices.get(id);
			Vibrator v = device.getVibrator();
			if(v != null && v.hasVibrator())
			{
				v.vibrate(ms);
			}
		}
	}
	
	public String getControllerName(String id){
		if(id != null && playerDevices.containsKey(id)){
			return playerDevices.get(id).getName();
		}
		return "Default Controller";
	}
	
	static public boolean onKeyDown(int keyCode, KeyEvent event){
		//Log.d("Controller", "Name: "+event.getDevice().getName());
		//Log.d("Controller", "Down Keycode: " + keyCode);
		if(event.getRepeatCount() == 0)
		{
			InputDevice device = event.getDevice();
			if(device != null && checkDevice(device))
			{
				GController c = getController(device);
				c.onKeyDown(keyCode);
				return true;
			}
		}
		return false;
	}
	
	static public boolean onKeyUp(int keyCode, KeyEvent event){
		//Log.d("Controller", "Up Keycode: " + keyCode);
		if(event.getRepeatCount() == 0)
		{
			InputDevice device = event.getDevice();
			if(device != null && checkDevice(device))
			{
				GController c = getController(device);
				c.onKeyUp(keyCode);
				return true;
			}
		}
		return false;
	}
	
	static public boolean onGenericMotionEvent(MotionEvent event) {
		//Log.d("Controller", "Motion event");
		InputDevice device = event.getDevice();
		if(device != null && checkDevice(device))
		{
			/*for(int i = 0; i < 50; i++)
			{
				if(Math.abs(event.getAxisValue(i))>0.25)
				{
					Log.d("Controller", "Motion for " + i + " with result " + event.getAxisValue(i));
				}
			}*/
			if((event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0) 
			{
				GController controller = getController(device);
				controller.handleLeftStick(event.getAxisValue(MotionEvent.AXIS_X), event.getAxisValue(MotionEvent.AXIS_Y));
				if(checkName("X-Box", event.getDevice()))
        	    {
					controller.handleRightStick(event.getAxisValue(MotionEvent.AXIS_RX), event.getAxisValue(MotionEvent.AXIS_RY));
        	    }
				else
				{
					controller.handleRightStick(event.getAxisValue(MotionEvent.AXIS_Z), event.getAxisValue(MotionEvent.AXIS_RZ));
				}
				
				if(checkName("Moga", event.getDevice()))
        	    {
					controller.handleL2Button(event.getAxisValue(MotionEvent.AXIS_BRAKE));
					controller.handleR2Button(event.getAxisValue(MotionEvent.AXIS_GAS));
					controller.handleDPadUpButton(event.getAxisValue(MotionEvent.AXIS_HAT_Y));
					controller.handleDPadDownButton(event.getAxisValue(MotionEvent.AXIS_HAT_Y));
					controller.handleDPadLeftButton(event.getAxisValue(MotionEvent.AXIS_HAT_X));
					controller.handleDPadRightButton(event.getAxisValue(MotionEvent.AXIS_HAT_X));
					controller.handleLeftTrigger(event.getAxisValue(MotionEvent.AXIS_BRAKE));
					controller.handleRightTrigger(event.getAxisValue(MotionEvent.AXIS_GAS));
        	    }
				else if(checkName("X-Box", event.getDevice()))
	        	{
					controller.handleLeftTrigger((event.getAxisValue(MotionEvent.AXIS_Z) + 1)/2);
					controller.handleRightTrigger((event.getAxisValue(MotionEvent.AXIS_RZ)+1)/2);
					controller.handleL2Button((event.getAxisValue(MotionEvent.AXIS_Z) + 1)/2);
					controller.handleR2Button((event.getAxisValue(MotionEvent.AXIS_RZ)+1)/2);
					controller.handleDPadUpButton(event.getAxisValue(MotionEvent.AXIS_HAT_Y));
					controller.handleDPadDownButton(event.getAxisValue(MotionEvent.AXIS_HAT_Y));
					controller.handleDPadLeftButton(event.getAxisValue(MotionEvent.AXIS_HAT_X));
					controller.handleDPadRightButton(event.getAxisValue(MotionEvent.AXIS_HAT_X));
	        	}
				else if(checkName("Amazon", event.getDevice()))
	        	{
					controller.handleL2Button(event.getAxisValue(MotionEvent.AXIS_BRAKE));
					controller.handleR2Button(event.getAxisValue(MotionEvent.AXIS_GAS));
					controller.handleLeftTrigger(event.getAxisValue(MotionEvent.AXIS_BRAKE));
					controller.handleRightTrigger(event.getAxisValue(MotionEvent.AXIS_GAS));
					controller.handleDPadUpButton(event.getAxisValue(MotionEvent.AXIS_HAT_Y));
					controller.handleDPadDownButton(event.getAxisValue(MotionEvent.AXIS_HAT_Y));
					controller.handleDPadLeftButton(event.getAxisValue(MotionEvent.AXIS_HAT_X));
					controller.handleDPadRightButton(event.getAxisValue(MotionEvent.AXIS_HAT_X));
	        	}
				else
				{
					controller.handleLeftTrigger(event.getAxisValue(MotionEvent.AXIS_LTRIGGER));
        	    	controller.handleRightTrigger(event.getAxisValue(MotionEvent.AXIS_RTRIGGER));
				}
			}
			return true;
		}
		return false;
	}
	
	/*
	 * Helping methods
	 */
	
	public static void addDeviceBySystem(int deviceId){
		InputDevice device = InputDevice.getDevice(deviceId);
		if(checkDevice(device) && !systemDevices.containsKey(deviceId))
		{
			String id = getIdentifier(device);
			systemDevices.put(deviceId, id);
			playerDevices.put(id, device);
			GControllerManager.addDevice(id, "GControllerDefault");
		}
	}
	
	public static void removeDeviceBySystem(int deviceId){
		if(systemDevices.containsKey(deviceId))
		{
			String id = systemDevices.get(deviceId);
			GControllerManager.removeDevice(id);
			playerDevices.remove(id);
			systemDevices.remove(deviceId);
		}
	}
	
	private static void startTask(){
		if(timer == null)
		{
			TimerTask task = new TimerTask () {
				@Override
				public void run () {
					checkForDevices();
				}
			};
			timer = new Timer();
			timer.schedule(task, 0, 2000);
		}
	}
	
	private static void stopTask(){
		if(timer != null)
		{
			timer.cancel();
			timer = null;
		}
	}
	
	private static boolean checkName(String name, InputDevice device){
		//try getVendorId() and getProductId()
		return device.getName().toLowerCase().contains(name.toLowerCase());
	}
	
	private static void checkForDevices(){
		int[] ids = InputDevice.getDeviceIds();
		HashMap<String, Boolean> allIds = new HashMap<String, Boolean>();
		for(int i = 0; i < ids.length; i++)
		{
			InputDevice device = InputDevice.getDevice(ids[i]);
			if(checkDevice(device))
			{
				addDeviceBySystem(ids[i]);
				String id = getIdentifier(device);
				allIds.put(id, true);
			}
		}
		int currentapiVersion = android.os.Build.VERSION.SDK_INT;
		if (currentapiVersion < android.os.Build.VERSION_CODES.JELLY_BEAN){
			Iterator<Map.Entry<Integer,String>> iter = systemDevices.entrySet().iterator();
			while (iter.hasNext()) {
				Map.Entry<Integer,String> entry = iter.next();
				if(!allIds.containsKey(entry.getValue()))
				{
					GControllerManager.removeDevice(entry.getValue());
					playerDevices.remove(entry.getKey());
					iter.remove();
				}
			}
		}
	}
	
	private static boolean checkDevice(InputDevice device){
		int hasControllerFlags = InputDevice.SOURCE_GAMEPAD | InputDevice.SOURCE_JOYSTICK;
		int hasRemoteFlags =  InputDevice.SOURCE_DPAD;
		int hasScreenFlags =  InputDevice.SOURCE_MOUSE | InputDevice.SOURCE_TOUCHPAD;
		boolean isGamepad = (device.getSources() & hasControllerFlags) == hasControllerFlags;
		boolean isRemote = ((device.getSources() & hasRemoteFlags) == hasRemoteFlags)
			    && device.getKeyboardType() == InputDevice.KEYBOARD_TYPE_NON_ALPHABETIC;
		boolean isBlocked = (device.getName().toLowerCase().contains("cec"))
				|| (device.getName().toLowerCase().contains("keyboard"));
		boolean isScreen = (device.getSources() & hasScreenFlags) == hasScreenFlags;
		return (device.getId() != 0 && (isGamepad || isRemote || isScreen)) && !isBlocked;
	}
	
	private static GController getController(InputDevice device){
		addDeviceBySystem(device.getId());
		return GControllerManager.getController(getIdentifier(device), "GControllerDefault");
	}
	
	private static String getIdentifier(InputDevice device){
	//	int currentapiVersion = android.os.Build.VERSION.SDK_INT;
	//	if (currentapiVersion >= android.os.Build.VERSION_CODES.JELLY_BEAN){
	//		return device.getDescriptor();
	//	}
		return "Default_"+device.getId();
	}

}

class GControllerListener implements InputManager.InputDeviceListener{
	@Override
	public void onInputDeviceAdded(int deviceId) {
		GControllerDefault.addDeviceBySystem(deviceId);
	}

	@Override
	public void onInputDeviceChanged(int deviceId) {
	}

	@Override
	public void onInputDeviceRemoved(int deviceId) {
		GControllerDefault.removeDeviceBySystem(deviceId);
	}
}
