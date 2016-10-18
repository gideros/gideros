package com.giderosmobile.android.plugins.ads;

import java.util.Enumeration;
import java.util.Hashtable;

public class AdsManager {
	
	private Hashtable<String, AdsState> adViews;
	
	public AdsManager(){
		adViews = new Hashtable<String, AdsState>();
	}
	
	public Object get(String type){
		if(adViews != null && type != null && adViews.get(type) != null)
			return adViews.get(type).getObject();
		return null;
	}
	
	public AdsState getState(String type){
		if(adViews != null && type != null)
			return adViews.get(type);
		return null;
	}
	
	public void set(Object adObject, String type){
		if(adViews != null && type != null)
		{
			AdsState adState = new AdsState(adObject, type);
			adViews.put(type, adState);
		}
	}
	
	public void set(Object adObject, String type, AdsStateChangeListener l){
		if(adViews != null && type != null)
		{
			AdsState adState = new AdsState(adObject, type);
			adState.setListener(l);
			adViews.put(type, adState);
		}
	}
	
	public void setObject(String type, Object o){
		if(adViews != null && type != null && adViews.get(type) != null)
			adViews.get(type).setObject(o);
	}
	
	public void setListener(String type, AdsStateChangeListener l){
		if(adViews != null && type != null && adViews.get(type) != null)
			adViews.get(type).setListener(l);
	}
	
	public void setAutoKill(String type, boolean kill){
		if(adViews != null && type != null && adViews.get(type) != null)
			adViews.get(type).setAutoKill(kill);
	}

	public void setPreLoad(String type, boolean preload){
		if(adViews != null && type != null && adViews.get(type) != null)
			adViews.get(type).setPreLoad(preload);
	}
	
	public void remove(String type){
		if(adViews != null && type != null && adViews.get(type) != null)
			adViews.remove(type);
	}
	
	public void hide(String type){
		if(adViews != null && type != null && adViews.get(type) != null)
			adViews.get(type).hide();
	}
	
	public void destroy(){
		if(adViews != null)
		{
			Enumeration<String> enumKey = adViews.keys();
			while(enumKey.hasMoreElements()) {
				String key = enumKey.nextElement();
				adViews.get(key).destroy();
				adViews.remove(key);
			}
			adViews = null;
		}
	}
	
	public void reset(String type){
		reset(type, true);
	}
	
	public void reset(String type, boolean delete){
		if(adViews != null && type != null && adViews.get(type) != null)
		{
			adViews.get(type).reset(delete);
			if(delete)
				adViews.remove(type);
		}
	}
	
	public void show(String type){
		if(adViews != null && type != null && adViews.get(type) != null)
			adViews.get(type).show();
	}
	
	public void load(String type){
		if(adViews != null && type != null && adViews.get(type) != null)
			adViews.get(type).load();
	}
	
	public boolean isReady(String type){
		if(adViews != null && type != null && adViews.get(type) != null)
			return adViews.get(type).isReady();
		return false;
	}
	
	public boolean isLoaded(String type){
		if(adViews != null && type != null && adViews.get(type) != null)
			return adViews.get(type).isLoaded();
		return false;
	}
}
