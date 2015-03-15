package com.giderosmobile.android.plugins.ads;

public class AdsState{
	AdsStateChangeListener listener;
	Object ad;
	String type;
	boolean loaded;
	boolean show;
	boolean autoKill = true;
	
	
	public AdsState(Object adObject, String adType){
		ad = adObject;
		type = adType;
		loaded = false;
		show = false;
	}
	
	public void setObject(Object adObject){
		ad = adObject;
	}
	
	public void setListener(AdsStateChangeListener l){
		listener = l;
	}
	
	public void setAutoKill(boolean kill){
		autoKill = kill;
	}
	
	public Object getObject(){
		return ad;
	}
	
	public String getType(){
		return type;
	}
	
	public void load(){
		loaded = true;
		checkAction();
	}
	
	public void show(){
		show = true;
		checkAction();
	}
	
	public boolean isReady(){
		return (loaded && show);
	}
	
	public boolean isLoaded(){
		return loaded;
	}
	
	public void reset(){
		reset(true);
	}
	
	public void reset(boolean delete){
		hide();
		if(delete)
			ad = null;
		loaded = false;
		show = false;
	}
	
	public void destroy(){
		if(listener != null)
		{
			try{
				listener.onDestroy();
			}
			catch(Exception e){}
		}
	}
	
	public void hide(){
		show = false;
		if(listener != null)
		{
			try{
				listener.onHide();
			}
			catch(Exception e){}
		}
	}
	
	private void checkAction(){
		if(isReady() && ad != null && listener != null)
		{
			try{
				listener.onShow();
			}
			catch(Exception e){}

			if(autoKill)
				reset(true);
		}
	}	
	
}
