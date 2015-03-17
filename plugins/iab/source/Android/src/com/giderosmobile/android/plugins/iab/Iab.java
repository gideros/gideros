package com.giderosmobile.android.plugins.iab;

import java.lang.ref.WeakReference;
import java.lang.reflect.Method;
import java.util.Hashtable;
import java.util.Map;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.util.SparseArray;

public class Iab {
	
	private static WeakReference<Activity> sActivity;
	private static long sData;
	private static Hashtable<String, IabInterface> iab;
	private static Hashtable<String, Hashtable<String, String>> products;
	private static Hashtable<String, Boolean> consumables;
	private static String[] stores = {"Google", "Amazon", "Ouya", "Samsung", "Fortumo", "Nokia"};
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
		iab = new Hashtable<String, IabInterface>();
		products = new Hashtable<String, Hashtable<String, String>>();
		consumables = new Hashtable<String, Boolean>();
	}
	
	public static void onDestroy()
	{	
		cleanup();
	}
	
	public static void onStart()
	{	
		for (IabInterface value : iab.values()) {
			value.onStart();
		}
	}
	
	public static void onActivityResult(final int requestCode, final int resultCode, final Intent data)
	{	
		for (IabInterface value : iab.values()) {
			value.onActivityResult(requestCode, resultCode, data);
		}
	}
	
	public static Object detectStores(Object parameters){
		SparseArray<String> ret = new SparseArray<String>();
		SparseArray<String> p = (SparseArray<String>)parameters;
		int s = p.size();
		for(int i = 0; i < s; i++)
		{
			String store = p.get(i);
	
			if(checkStore(modifyName(store))){
				ret.put(ret.size(), store.toLowerCase());
			}
		}
		int length = stores.length;
		for(int i = 0; i < length; i++)
		{	
			boolean exists = false;
			int size = ret.size();
			for(int j = 0; j < size; j++)
			{
				if(stores[i].toLowerCase().equals(ret.get(j)))
				{
					exists = true;
					break;
				}
			}
			if(!exists && checkStore(stores[i])){
				ret.put(ret.size(), stores[i].toLowerCase());
			}
		}
		return ret;
	}
	
	private static Boolean checkStore(String store){
		Boolean isInstalled = false;
		String className = "com.giderosmobile.android.plugins.iab.frameworks.Iab"+store;
		Class classz = null;
		try {
			classz = Class.forName(className);
		} catch (ClassNotFoundException e) {
		}
		
		if(classz != null)
		{
			Method method = null;
			try {
				method = classz.getMethod("isInstalled");
			} catch (NoSuchMethodException e) {
			}
			try {
				isInstalled = (Boolean)method.invoke(null);
			} catch (Exception e) {
			} 
		}
		return isInstalled;
	}
	
	public static void init(long data){
		sData = data;
	}
	
	public static void cleanup(){
		sData = 0;
		for (IabInterface value : iab.values()) {
			value.onDestroy();
		}
		iab.clear();
	}
	
	public static void initialize(String iabtype){
		final String adp = modifyName(iabtype);
		if(iab.get(adp) == null)
		{
			String className = "com.giderosmobile.android.plugins.iab.frameworks.Iab"+adp;
			Class classz = null;
			try {
				classz = Class.forName(className);
			} catch (ClassNotFoundException e) {
				e.printStackTrace();
			}
			try {
				iab.put(adp, (IabInterface)classz.newInstance());
				products.put(adp, new Hashtable<String, String>());
			} catch (InstantiationException e) {
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
			}
			
			iab.get(adp).onCreate(sActivity);
		}
	}
	
	public static void destroy(String iabtype){
		final String adp = modifyName(iabtype);
		if(iab.get(adp) != null)
		{
			iab.get(adp).onDestroy();
			iab.remove(adp);
		}
	}
	
	public static void setup(String iabtype, Object parameters){
		String adp = modifyName(iabtype);
		iab.get(adp).init(parameters);
	}
	
	public static void setProducts(String iabtype, Object productNames, Object productSkus){
		String adp = modifyName(iabtype);
		SparseArray<String> pNames = (SparseArray<String>)productNames;
		SparseArray<String> pSkus = (SparseArray<String>)productSkus;
		int size = pNames.size();
		Hashtable<String, String> prods = products.get(adp);
		for(int i = 0; i < size; i++)
		{
			prods.put(pNames.get(i), pSkus.get(i));
		}
	}
	
	public static void setConsumables(String iabtype, Object prods){
		SparseArray<String> products = (SparseArray<String>)prods;
		int size = products.size();
		for(int i = 0; i < size; i++)
		{
			consumables.put(products.get(i), true);
		}
	}
	
	public static void check(String iabtype){
		String adp = modifyName(iabtype);
		iab.get(adp).check();
	}
	
	public static void request(final String iabtype){
		try
		{	
			// Non UI thread
			Runnable myRunnable = new Runnable(){
				
				@Override
				public void run() {
					String adp = modifyName(iabtype);
					iab.get(adp).request(products.get(adp));
				}
				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static void purchase(final String iabtype, final String productId){
		try
		{	
			// Non UI thread
			Runnable myRunnable = new Runnable(){
				
				@Override
				public void run() {
					String adp = modifyName(iabtype);
					Hashtable<String, String> p = products.get(adp);
					iab.get(adp).purchase(p.get(productId));
				}
				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static void restore(final String iabtype){
		try
		{	
			// Non UI thread
			Runnable myRunnable = new Runnable(){
				
				@Override
				public void run() {
					String adp = modifyName(iabtype);
					iab.get(adp).restore();
				}
				
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	//Events
	
	public static void available(Object caller){
		if (sData != 0)
			onAvailable(getCallerName(caller), sData);
	}
	
	public static void notAvailable(Object caller){
		if (sData != 0)
			onNotAvailable(getCallerName(caller), sData);
	}
	
	public static void purchaseComplete(Object caller, String product, String receipt){
		if (sData != 0)
			onPurchaseComplete(getCallerName(caller), getKey(product, caller), receipt, sData);
	}
	
	public static void purchaseError(Object caller, String error){
		if (sData != 0)
			onPurchaseError(getCallerName(caller), error, sData);
	}
	
	public static void productsComplete(Object caller, SparseArray<Bundle> arr){
		if (sData != 0)
		{
			int size = arr.size();
			for(int i = 0; i < size; i++)
			{
				Bundle b = arr.get(i);
				String product = b.getString("productId");
				b.putString("productId", getKey(product, caller));
			}
			onProductsComplete(getCallerName(caller), arr, sData);
		}
	}
	
	public static void productsError(Object caller, String error){
		if (sData != 0)
			onProductsError(getCallerName(caller), error, sData);
	}
	
	public static void restoreComplete(Object caller){
		if (sData != 0)
			onRestoreComplete(getCallerName(caller), sData);
	}
	
	public static void restoreError(Object caller, String error){
		if (sData != 0)
			onRestoreError(getCallerName(caller), error, sData);
	}
	
	private static native void onAvailable(String iap, long data);
	private static native void onNotAvailable(String iap, long data);
	private static native void onPurchaseComplete(String iap, String product, String receipt, long data);
	private static native void onPurchaseError(String iap, String error, long data);
	private static native void onProductsComplete(String iap, Object arr, long data);
	private static native void onProductsError(String iap, String error, long data);
	private static native void onRestoreComplete(String iap, long data);
	private static native void onRestoreError(String iap, String error, long data);
	
	public static boolean isPackageInstalled(String packageName) {
		try {
	        sActivity.get().getApplication().getPackageManager().getPackageInfo(packageName, 0);
	    } catch (NameNotFoundException e) {
	        return false;
	    }
	    return true;
	}
	
	public static Hashtable<String, String> getProducts(Object caller)
	{
		return products.get(modifyName(getCallerName(caller)));
	}
	
	private static String getValue(String key, Object caller){
		return getProducts(caller).get(key);
	}
	
	private static String getKey(String Name, Object caller){
	    for (Object o: getProducts(caller).entrySet()) {
	        Map.Entry entry = (Map.Entry) o;
	        if(entry.getValue().equals(Name))
	        {
	            return (String) entry.getKey();
	        }
	    }
	    return null;
	}
	
	public static boolean isConsumable(String name, Object caller){
		return consumables.containsKey(getKey(name, caller));
	}
	
	private static String modifyName(String iabtype){
		return iabtype = iabtype.substring(0,1).toUpperCase() + iabtype.substring(1).toLowerCase();
	}
	
	private static String getCallerName(Object cls){
		 String name = cls.getClass().getName();
         name = name.replace("com.giderosmobile.android.plugins.iab.frameworks.Iab", "");
         if(name.contains("$"))
        	 name = name.substring(0, name.indexOf("$"));
         return name.toLowerCase();
	}
}
