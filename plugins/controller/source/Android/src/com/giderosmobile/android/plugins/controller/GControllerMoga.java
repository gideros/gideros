package com.giderosmobile.android.plugins.controller;

import java.lang.ref.WeakReference;

import android.app.Activity;

import com.bda.controller.Controller;
import com.bda.controller.ControllerListener;
import com.bda.controller.KeyEvent;
import com.bda.controller.MotionEvent;
import com.bda.controller.StateEvent;

public class GControllerMoga implements GControllerInterface, ControllerListener{
	Controller mController = null;
	public WeakReference<Activity> sActivity;
	
	public void onCreate(WeakReference<Activity> activity){
		sActivity = activity;
		mController = Controller.getInstance(sActivity.get()); 
		mController.init();
		mController.setListener(this, null);
		mController.onResume();
	}
	
	public void onDestroy(){
		if(mController != null) { 
			mController.exit(); 
		}
	}
	
	public void onPause()
    {
		if(mController != null) { 
			mController.onPause(); 
		}
    }
    
    public void onResume()
    {
    	if(mController != null) { 
    		mController.onResume(); 
    	}
    }
    
    @Override
	public void vibrate(String id, long ms) {}

	@Override
	public String getControllerName(String id) {
		if(mController.getState(Controller.STATE_CURRENT_PRODUCT_VERSION) == Controller.ACTION_VERSION_MOGAPRO)
			return "Moga Pro Controller";
		else
			return "Moga Controller";
	}

	@Override
	public void onKeyEvent(com.bda.controller.KeyEvent event) {
		GController c = getController(event.getControllerId());
		if(event.getAction() == KeyEvent.ACTION_DOWN) {
			c.onKeyDown(event.getKeyCode());
		}
		else if(event.getAction() == KeyEvent.ACTION_UP) {
			c.onKeyUp(event.getKeyCode());
		}
		
	}

	@Override
	public void onMotionEvent(com.bda.controller.MotionEvent event) {
		GController controller = getController(event.getControllerId());
		controller.handleLeftStick(event.getAxisValue(MotionEvent.AXIS_X), event.getAxisValue(MotionEvent.AXIS_Y));
		controller.handleRightStick(event.getAxisValue(MotionEvent.AXIS_Z), event.getAxisValue(MotionEvent.AXIS_RZ));
		controller.handleLeftTrigger(event.getAxisValue(MotionEvent.AXIS_LTRIGGER));
    	controller.handleRightTrigger(event.getAxisValue(MotionEvent.AXIS_RTRIGGER));
	}

	@Override
	public void onStateEvent(StateEvent event) {
		if(event.getState() == StateEvent.STATE_CONNECTION){
			if(event.getAction() == StateEvent.ACTION_CONNECTED){
				getController(event.getControllerId());
			}
			else if(event.getAction() == StateEvent.ACTION_DISCONNECTED)
			{
				removeController(event.getControllerId());
			}
		}
	}
	
	private static String getIndetifier(int id){
		return "Moga_"+id;
	}
	
	private static GController getController(int id){
		return GControllerManager.getController(getIndetifier(id), "GControllerMoga");
	}
	
	private static void removeController(int id){
		GControllerManager.removeDevice(getIndetifier(id));
	}
}