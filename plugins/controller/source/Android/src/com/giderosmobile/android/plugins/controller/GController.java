package com.giderosmobile.android.plugins.controller;

import android.view.KeyEvent;

public class GController{
	private int id;
	private GControllerStick rightStick;
	private GControllerStick leftStick;
	private GControllerTrigger rightTrigger;
	private GControllerTrigger leftTrigger;
	private GControllerButton L2Button;
	private GControllerButton R2Button;
	private GControllerButton DPadUpButton;
	private GControllerButton DPadDownButton;
	private GControllerButton DPadLeftButton;
	private GControllerButton DPadRightButton;
	
	public static float STICK_DEADZONE = 0.25f;
	
	public GController(int playerId)
	{
		GControllerManager.onConnectedEvent(playerId);
		id = playerId;
		rightStick = new GControllerStick(id, false);
		leftStick = new GControllerStick(id, true);
		rightTrigger = new GControllerTrigger(id, false);
		leftTrigger = new GControllerTrigger(id, true);
		L2Button = new GControllerButton(KeyEvent.KEYCODE_BUTTON_L2, id);
		R2Button = new GControllerButton(KeyEvent.KEYCODE_BUTTON_R2, id);
		DPadUpButton = new GControllerButton(KeyEvent.KEYCODE_DPAD_UP, id);
		DPadDownButton = new GControllerButton(KeyEvent.KEYCODE_DPAD_DOWN, id);
		DPadLeftButton = new GControllerButton(KeyEvent.KEYCODE_DPAD_LEFT, id);
		DPadRightButton = new GControllerButton(KeyEvent.KEYCODE_DPAD_RIGHT, id);
	}
	
	public void destroy(){
		GControllerManager.onDisconnectedEvent(id);
		rightStick = null;
		leftStick = null;
		rightTrigger = null;
		leftTrigger = null;
		L2Button = null;
		R2Button = null;
		DPadUpButton = null;
		DPadDownButton = null;
		DPadLeftButton = null;
		DPadRightButton = null;
	}
	
	public int getPlayerId(){
		return id;
	}
	
	public void onKeyDown(int keyCode){
		keyCode = checkKeyCode(keyCode);
		GControllerManager.onKeyDownEvent(keyCode, id);
	}
	
	public void onKeyUp(int keyCode){
		keyCode = checkKeyCode(keyCode);
		GControllerManager.onKeyUpEvent(keyCode, id);
	}
	
	public void handleRightStick(float x, float y){
		rightStick.handleAxis(x, y);
	}
	
	public void handleLeftStick(float x, float y){
		leftStick.handleAxis(x, y);
	}
	
	public void handleRightTrigger(float value)
	{
		rightTrigger.handleTrigger(value);
	}
	
	public void handleLeftTrigger(float value)
	{
		leftTrigger.handleTrigger(value);
	}
	
	public void handleL2Button(float value){
		L2Button.handlePositiveButton(value);
	}
	
	public void handleR2Button(float value){
		R2Button.handlePositiveButton(value);
	}
	
	public void handleDPadUpButton(float value){
		DPadUpButton.handleNegativeButton(value);
	}
	
	public void handleDPadDownButton(float value){
		DPadDownButton.handlePositiveButton(value);
	}
	
	public void handleDPadLeftButton(float value){
		DPadLeftButton.handleNegativeButton(value);
	}
	
	public void handleDPadRightButton(float value){
		DPadRightButton.handlePositiveButton(value);
	}
	
	private static int checkKeyCode(int keyCode){
		switch(keyCode){
			case KeyEvent.KEYCODE_BUTTON_MODE:
			case KeyEvent.KEYCODE_BUTTON_START:
				keyCode = KeyEvent.KEYCODE_MENU;
				break;
			case KeyEvent.KEYCODE_BUTTON_SELECT:
				keyCode = KeyEvent.KEYCODE_BACK;
				break;
			case KeyEvent.KEYCODE_DPAD_CENTER:
				keyCode = KeyEvent.KEYCODE_BUTTON_A;
				break;
		}
		return keyCode;
	}
}

class GControllerStick{
	private int id;
	private float lastX = 0;
	private float lastY = 0;
	private boolean isLeft;
	
	public GControllerStick(int playerId, boolean left){
		id = playerId;
		isLeft = left;
	}
	
	public void handleAxis(float x, float y){
		if(x*x + y*y <= GController.STICK_DEADZONE * GController.STICK_DEADZONE)
	    {
	    	x = 0;
	    	y = 0;
	    }
	    
	    if(lastX != x || lastY != y)
	    {
	    	lastX = x;
	    	lastY = y;
	    	double strength = Math.sqrt(x*x + y*y);
	    	double angle = Math.acos(x/strength);
	    	if(y>0)
	    	{
	    		angle= -angle+2*Math.PI;
	    	}
	    	angle = -angle+2*Math.PI;
	    	if(isLeft)
	    	{
	    		GControllerManager.onLeftJoystick(x, y, angle, strength, id);
	    	}
	    	else
	    	{
	    		GControllerManager.onRightJoystick(x, y, angle, strength, id);
	    	}
	    }
	}
}

class GControllerTrigger{
	private int id;
	private float lastValue = 0;
	private boolean isLeft;
	
	public GControllerTrigger(int playerId, boolean left){
		id = playerId;
		isLeft = left;
	}
	
	public void handleTrigger(float value){
		if(value < GController.STICK_DEADZONE){
			value = 0;
 	    }
 	    
 	    if(lastValue != value){
 	    	lastValue = value;
 	    	if(isLeft)
 	    		GControllerManager.onLeftTrigger(value, id);
 	    	else
 	    		GControllerManager.onRightTrigger(value, id);
 	    }
	}
}

class GControllerButton{
	private int id;
	private boolean isDown = false;
	private int keyCode;
	
	public GControllerButton(int code, int playerId){
		keyCode = code;
		id = playerId;
	}
	
	public void handlePositiveButton(float value){
		if(!isDown && value > 0.5)
    	{
    		isDown = true;
    		GControllerManager.onKeyDownEvent(keyCode, id);
    	}
    	else if(isDown && value < 0.5)
    	{
    		isDown = false;
    		GControllerManager.onKeyUpEvent(keyCode, id);
    	}
	}
	
	public void handleNegativeButton(float value){
		if(!isDown && value < -0.5)
    	{
    		isDown = true;
    		GControllerManager.onKeyDownEvent(keyCode, id);
    	}
    	else if(isDown && value > -0.5)
    	{
    		isDown = false;
    		GControllerManager.onKeyUpEvent(keyCode, id);
    	}
	}
}