package com.giderosmobile.android.plugins.bridge;

import java.lang.ref.WeakReference;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.Proxy;
import java.util.HashMap;
import java.util.StringTokenizer;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.FrameLayout;

public class GBridge{
	
	private static WeakReference<Activity> sActivity;
	
	private static HashMap<Class, Method[]> sMethodCache = new HashMap<Class, Method[]>();
	private static HashMap<Class, Field[]> sFieldCache = new HashMap<Class, Field[]>();
	private static HashMap<Class, Constructor[]> sConstructorCache = new HashMap<Class, Constructor[]>();
	private static FrameLayout layout;
	private static long sData = 0;
	private static boolean wasRequested = false;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
		layout = (FrameLayout)sActivity.get().getWindow().getDecorView().findViewById(android.R.id.content);
		layout.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
            		if(!wasRequested)
            		{
            			wasRequested = true;
            			layout.postDelayed(new Runnable(){
            			       @Override
            			       public void run() {
            			    	   layout.requestLayout();
            			       }            
            			}, 500);
            		}
            		else
            		{
            			wasRequested = false;
            		}
            }
        });
	}
	
	//on destroy event
	public static void onDestroy()
	{	
		cleanup();
	}
	
	public static void init(long data){
		sData = data;
	}
	
	public static void cleanup()
	{
		sData = 0;
		clearLayout();
	}
	
	public static Activity getActivity(){
		return sActivity.get();
	}
		
	public static FrameLayout getLayout(){
		return layout;
	}
	
	public static GLSurfaceView getSurface(){
		return (GLSurfaceView) layout.getChildAt(0);
	}
	
	public static void hideKeyboard(){
		InputMethodManager inputManager = (InputMethodManager)sActivity.get().getSystemService(Context.INPUT_METHOD_SERVICE);
		View view = sActivity.get().getCurrentFocus();
		if(view != null)
		{
			inputManager.hideSoftInputFromWindow(view.getWindowToken(), 0);
			view.clearFocus();
		}
		sActivity.get().getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
		layout.requestFocus();
	}
	
	public static String getClassName(Object obj){
		return obj.getClass().getName();
	}

	public static void clearLayout(){
		sActivity.get().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				for (int i = layout.getChildCount()-1; i > 0; i--) {
					layout.removeViewAt(i);
				}
			}
		});
	}

	private static Method[] getMethods(Class cls)
	{
		if (sMethodCache.containsKey(cls))
			return sMethodCache.get(cls);
		
		Method[] methods = cls.getMethods();
		
		sMethodCache.put(cls, methods);
		
		return methods;
	}
	
	private static Field[] getFields(Class cls)
	{
		if (sFieldCache.containsKey(cls))
			return sFieldCache.get(cls);
		
		Field[] fields = cls.getFields();
		
		sFieldCache.put(cls, fields);
		
		return fields;
	}

	private static Constructor[] getConstructors(Class cls)
	{
		if (sConstructorCache.containsKey(cls))
			return sConstructorCache.get(cls);
		
		Constructor[] constructors = cls.getConstructors();
		
		sConstructorCache.put(cls, constructors);
		
		return constructors;
	}

	public static int getClassConstructorsLength(Class cls)
	{
		Constructor[] constructors = getConstructors(cls);
		return constructors.length;
	}

	public static String[] getClassConstructorParameters(Class cls, int index)
	{
		Constructor[] constructors = getConstructors(cls);
		Class[] parameters = constructors[index].getParameterTypes();

		String[] result = new String[parameters.length];
		for (int i = 0; i < parameters.length; ++i)
		{
			result[i] = parameters[i].getName();
		}

		return result;
	}
	
	public static String[] getClassFields(Class cls)
	{
		Field fields[] = getFields(cls);
		String[] result = new String[fields.length];
		for (int i = 0; i < fields.length; ++i)
		{
			boolean isStatic = Modifier.isStatic(fields[i].getModifiers());
			result[i] = (isStatic ? "+" : "-") + fields[i].getName();
		}
		return result;
	}
	
	public static String[] getClassFieldTypes(Class cls)
	{
		Field fields[] = getFields(cls);
		String[] result = new String[fields.length];
		for (int i = 0; i < fields.length; ++i)
		{
			result[i] = fields[i].getType().getName();
		}
		return result;
	}

	public static String[] getClassMethods(Class cls)
	{
		Method methods[] = getMethods(cls);
		String[] result = new String[methods.length];
		for (int i = 0; i < methods.length; ++i)
		{
			boolean isStatic = Modifier.isStatic(methods[i].getModifiers());
			result[i] = (isStatic ? "+" : "-") + methods[i].getName();
		}
		return result;
	}

	public static String[] getClassMethodParameters(Class cls, int index)
	{
		Method methods[] = getMethods(cls);
		Method method = methods[index];
		Class returnType = method.getReturnType();
		Class[] parameters = method.getParameterTypes();

		String[] result = new String[parameters.length + 1];
		result[0] = returnType.getName();
		for (int i = 0; i < parameters.length; ++i)
		{
			result[i + 1] = parameters[i].getName();
		}

		return result;
	}
	
	public static Object getFieldObject(Class cls, int index, Object instance)
	{
		Field fields[] = getFields(cls);
		Field field = fields[index];
		field.setAccessible(true);
		try{
			return field.get(instance);
		} catch(Exception e){
			
		}
		return null;
	}
	
	public static void getFieldVoid(Class cls, int index, Object instance)
	{
		getFieldObject(cls, index, instance);
	}

	public static boolean getFieldBoolean(Class cls, int index, Object instance)
	{
		return ((Boolean)getFieldObject(cls, index, instance)).booleanValue();
	}
	
	public static byte getFieldByte(Class cls, int index, Object instance)
	{
		return ((Byte)getFieldObject(cls, index, instance)).byteValue();
	}

	public static char getFieldChar(Class cls, int index, Object instance)
	{
		return ((Character)getFieldObject(cls, index, instance)).charValue();
	}

	public static short getFieldShort(Class cls, int index, Object instance)
	{
		return ((Short)getFieldObject(cls, index, instance)).shortValue();
	}

	public static int getFieldInt(Class cls, int index, Object instance)
	{
		return ((Integer)getFieldObject(cls, index, instance)).intValue();
	}

	public static long getFieldLong(Class cls, int index, Object instance)
	{
		return ((Long)getFieldObject(cls, index, instance)).longValue();
	}

	public static float getFieldFloat(Class cls, int index, Object instance)
	{
		return ((Float)getFieldObject(cls, index, instance)).floatValue();
	}

	public static double getFieldDouble(Class cls, int index, Object instance)
	{
		return ((Double)getFieldObject(cls, index, instance)).doubleValue();
	}

	public static String getFieldString(Class cls, int index, Object instance)
	{
		return (String)getFieldObject(cls, index, instance);
	}
	
	public static void setFieldObject(Class cls, int index, Object instance, Object value)
	{
		Field fields[] = getFields(cls);
		Field field = fields[index];
		field.setAccessible(true);
		try {
			field.set(instance, value);
		} catch (Exception e) {

		}
	}
	
	public static void setFieldVoid(Class cls, int index, Object instance)
	{
		
	}

	public static void setFieldBoolean(Class cls, int index, Object instance, boolean value)
	{
		setFieldObject(cls, index, instance, Boolean.valueOf(value));
	}
	
	public static void setFieldByte(Class cls, int index, Object instance, byte value)
	{
		setFieldObject(cls, index, instance, Byte.valueOf(value));
	}

	public static void setFieldChar(Class cls, int index, Object instance, char value)
	{
		setFieldObject(cls, index, instance, Character.valueOf(value));
	}

	public static void setFieldShort(Class cls, int index, Object instance, short value)
	{
		setFieldObject(cls, index, instance, Short.valueOf(value));
	}

	public static void setFieldInt(Class cls, int index, Object instance, int value)
	{
		setFieldObject(cls, index, instance, Integer.valueOf(value));
	}

	public static void setFieldLong(Class cls, int index, Object instance, long value)
	{
		setFieldObject(cls, index, instance, Long.valueOf(value));
	}

	public static void setFieldFloat(Class cls, int index, Object instance, float value)
	{
		setFieldObject(cls, index, instance, Float.valueOf(value));
	}

	public static void setFieldDouble(Class cls, int index, Object instance, double value)
	{
		setFieldObject(cls, index, instance, Double.valueOf(value));
	}

	public static void setFieldString(Class cls, int index, Object instance, String value)
	{
		setFieldObject(cls, index, instance, value);
	}

	public static Object callFunctionOnUI(final Class cls, final int index, final Object instance, final Object[] parameters)
	{
		final Object[] ret = new Object[1];
		/*Log.d("Bridge", "Method call start");
		if(instance != null)
		{
			Log.d("Bridge", "Calling instance: " + instance.toString());
		}
		else
		{
			Log.d("Bridge", "Calling class: " + cls.toString());
		}
		
		Method methods[] = getMethods(cls);
		Method method = methods[index];
		Log.d("Bridge", "For method: " + method.toString());
		
		String[] params = getClassMethodParameters(cls, index);
		for (int i = 0; i < params.length; ++i)
		{
			if(i == 0)
				Log.d("Bridge", "Returning: " + params[i]);
			else
				Log.d("Bridge", "Accepting param (" + i + "): " + params[i]);
			
		}
		
		if(parameters != null)
		{
			for (int i = 0; i < parameters.length; ++i)
			{
				if(parameters[i] != null)
					Log.d("Bridge", "With parameter (" + i + "): " + parameters[i].getClass());
				else
					Log.d("Bridge", "With parameter (" + i + "): null");
			}
		}
		else
		{
			Log.d("Bridge", "Without parameters");
		}*/
		
		Runnable myRunnable = new Runnable(){

			@Override
			public void run() {
				Method methods[] = getMethods(cls);
				Method method = methods[index];
				method.setAccessible(true);
				try {
					ret[0] = method.invoke(instance, parameters);
				} catch (Exception e) {
					Log.e("Bridge", e.getMessage());
					ret[0] = null;
				}
				synchronized ( this )
                {
                    this.notify();
                }
			}
		
		};
		
		synchronized( myRunnable ) {
			sActivity.get().runOnUiThread(myRunnable) ;

			try {
				myRunnable.wait() ;
			} catch (InterruptedException e) {
				Log.e("Bridge", e.getMessage());
			}
		}
		/*if(ret[0] != null)
		{
			Log.d("Bridge", "Actually returns: " + ret[0].toString());
		}
		else
		{
			Log.d("Bridge", "Actually returns void");
		}
		Log.d("Bridge", "Method call ended");*/
		return ret[0];
	}

	public static Object callStaticFunctionObject(Class cls, int index, Object instance, Object[] parameters)
	{
		/*Method methods[] = getMethods(cls);
		Method method = methods[index];
		method.setAccessible(true);
		try {
			return method.invoke(instance, parameters);
		} catch (Exception e) {
		}
		return null;
		*/
		return callFunctionOnUI(cls, index, instance, parameters);
	}

	public static void callStaticFunctionVoid(Class cls, int index, Object instance, Object[] parameters)
	{
		//callStaticFunctionObject(cls, index, instance, parameters);
		callFunctionOnUI(cls, index, instance, parameters);
	}

	public static boolean callStaticFunctionBoolean(Class cls, int index, Object instance, Object[] parameters)
	{
		//return ((Boolean)callStaticFunctionObject(cls, index, instance, parameters)).booleanValue();
		return ((Boolean)callFunctionOnUI(cls, index, instance, parameters)).booleanValue();
	}
	
	public static byte callStaticFunctionByte(Class cls, int index, Object instance, Object[] parameters)
	{
		//return ((Byte)callStaticFunctionObject(cls, index, instance, parameters)).byteValue();
		return ((Byte)callFunctionOnUI(cls, index, instance, parameters)).byteValue();
	}

	public static char callStaticFunctionChar(Class cls, int index, Object instance, Object[] parameters)
	{
		//return ((Character)callStaticFunctionObject(cls, index, instance, parameters)).charValue();
		return ((Character)callFunctionOnUI(cls, index, instance, parameters)).charValue();
	}

	public static short callStaticFunctionShort(Class cls, int index, Object instance, Object[] parameters)
	{
		//return ((Short)callStaticFunctionObject(cls, index, instance, parameters)).shortValue();
		return ((Short)callFunctionOnUI(cls, index, instance, parameters)).shortValue();
	}

	public static int callStaticFunctionInt(Class cls, int index, Object instance, Object[] parameters)
	{
		//return ((Integer)callStaticFunctionObject(cls, index, instance, parameters)).intValue();
		return ((Integer)callFunctionOnUI(cls, index, instance, parameters)).intValue();
	}

	public static long callStaticFunctionLong(Class cls, int index, Object instance, Object[] parameters)
	{
		//return ((Long)callStaticFunctionObject(cls, index, instance, parameters)).longValue();
		return ((Long)callFunctionOnUI(cls, index, instance, parameters)).longValue();
	}

	public static float callStaticFunctionFloat(Class cls, int index, Object instance, Object[] parameters)
	{
		//return ((Float)callStaticFunctionObject(cls, index, instance, parameters)).floatValue();
		return ((Float)callFunctionOnUI(cls, index, instance, parameters)).floatValue();
	}

	public static double callStaticFunctionDouble(Class cls, int index, Object instance, Object[] parameters)
	{
		//return ((Double)callStaticFunctionObject(cls, index, instance, parameters)).doubleValue();
		return ((Double)callFunctionOnUI(cls, index, instance, parameters)).doubleValue();
	}

	public static String callStaticFunctionString(Class cls, int index, Object instance, Object[] parameters)
	{
		//return (String)callStaticFunctionObject(cls, index, instance, parameters);
		return (String)callFunctionOnUI(cls, index, instance, parameters);
	}
	public static Object createObject(final Class cls, final int index, final Object[] parameters)
	{
		final Object[] ret = new Object[1];
		
		/*Log.d("Bridge", "Constructor call start");
		Log.d("Bridge", "Calling for class: " + cls.toString());
		
		Constructor[] methods = getConstructors(cls);
		Constructor method = methods[index];
		Log.d("Bridge", "For method: " + method.toString());
		
		String[] params = getClassConstructorParameters(cls, index);
		for (int i = 0; i < params.length; ++i)
		{
			Log.d("Bridge", "Accepting param (" + i + "): " + params[i]);			
		}
		
		if(parameters != null)
		{
			for (int i = 0; i < parameters.length; ++i)
			{
				if(parameters[i] != null)
					Log.d("Bridge", "With parameter (" + i + "): " + parameters[i].getClass());
				else
					Log.d("Bridge", "With parameter (" + i + "): null");
			}
		}
		else
		{
			Log.d("Bridge", "Without parameters");
		}*/
		Runnable myRunnable = new Runnable(){

			@Override
			public void run() {
				Constructor[] constructors = getConstructors(cls);
				Constructor constructor = constructors[index];
				try {
					ret[0] = constructor.newInstance(parameters);
				} catch (Exception e) {
					Log.e("Bridge", "Can't create object for: " + cls.toString() + " because " + e.getMessage());
					ret[0] = null;
				}
				synchronized ( this )
                {
                    this.notify();
                }
			}
		
		};
		
		synchronized( myRunnable ) {
			sActivity.get().runOnUiThread(myRunnable) ;

			try {
				myRunnable.wait() ;
			} catch (InterruptedException e) {
				Log.e("Bridge", e.getMessage());
			}
		}
		
		/*if(ret[0] != null)
		{
			Log.d("Bridge", "Actually returns: " + ret[0].toString());
		}
		else
		{
			Log.d("Bridge", "Actually returns void");
		}
		Log.d("Bridge", "Constructor call ended");*/
		return ret[0];
	}

	public static boolean isInstance(Class cls, Object obj)
	{
		return cls.isInstance(obj);
	}
	
	public static boolean isSubclass(String strcls, String strsubcls)
	{
		Class cls;
		try {
			cls = Class.forName(strcls);
		} catch (ClassNotFoundException e) {
			return false;
		}
		Class subcls;
		try {
			subcls = Class.forName(strsubcls);
		} catch (ClassNotFoundException e) {
			return false;
		}
		return cls.isAssignableFrom(subcls);
	}

	public static boolean getBoolean(int index, Object[] args)
	{
		return ((Boolean)args[index]).booleanValue();
	}
	
	public static byte getByte(int index, Object[] args)
	{
		return ((Byte)args[index]).byteValue();
	}

	public static char getChar(int index, Object[] args)
	{
		return ((Character)args[index]).charValue();
	}

	public static short getShort(int index, Object[] args)
	{
		return ((Short)args[index]).shortValue();
	}

	public static int getInt(int index, Object[] args)
	{
		return ((Integer)args[index]).intValue();
	}

	public static long getLong(int index, Object[] args)
	{
		return ((Long)args[index]).longValue();
	}

	public static float getFloat(int index, Object[] args)
	{
		return ((Float)args[index]).floatValue();
	}

	public static double getDouble(int index, Object[] args)
	{
		return ((Double)args[index]).doubleValue();
	}
	
	public static Object createProxy(String implem, long data)
	{
		StringTokenizer st = new StringTokenizer(implem, ",");
		Class[] interfaces = new Class[st.countTokens()];
		for (int i = 0; st.hasMoreTokens(); i++)
			try {
				interfaces[i] = Class.forName(st.nextToken());
			} catch (ClassNotFoundException e) {}

		InvocationHandler handler = new LuaInvocationHandler(data);

		Object proxy = null;
		try
		{
			proxy = Proxy.newProxyInstance(GBridge.class.getClassLoader(), interfaces, handler);
			//Log.d("Bridge", "Proxy value: " + proxy);
		}
		catch(Exception e)
		{
			Log.e("Birdge", e.getMessage());
		}
		
		return proxy;
	}
	
	public static void onInvoke(String methodName, Object[] args, String[] types, long data)
	{
		if(sData != 0)
			onInvoke(methodName, args, types, data, sData);
	}
	
	public static native void onInvoke(String methodName, Object[] args, String[] types, long data, long sData);
}

class LuaInvocationHandler implements InvocationHandler
{
	private long sData;
	
	public LuaInvocationHandler(long data)
	{
		sData = data;
	}
	/**
	 * Function called when a proxy object function is invoked.
	 */
 	 public Object invoke(Object proxy, Method method, final Object[] args)
 	 {
	  	final String methodName = method.getName();
	  	final String[] types = new String[args.length];
	  	for(int i = 0; i < args.length; i++)
	  	{
	  		String type = args[i].getClass().getName();
	  		if(type.equals("java.lang.Boolean"))
	  		{
	  			type = "boolean";
	  		}
	  		else if(type.equals("java.lang.Byte"))
	  		{
	  			type = "byte";
	  		}
	  		else if(type.equals("java.lang.Character"))
	  		{
	  			type = "char";
	  		}
	  		else if(type.equals("java.lang.Short"))
	  		{
	  			type = "short";
	  		}
	  		else if(type.equals("java.lang.Integer"))
	  		{
	  			type = "int";
	  		}
	  		else if(type.equals("java.lang.Long"))
	  		{
	  			type = "long";
	  		}
	  		else if(type.equals("java.lang.Float"))
	  		{
	  			type = "float";
	  		}
	  		else if(type.equals("java.lang.Double"))
	  		{
	  			type = "double";
	  		}
	  		types[i] = type;
	  		Log.d("Bridge", "Passing arg(" + i + "): " + type);
	  	}
	  	Log.d("Bridge", "Method: " + methodName + " from :" + sData);

	  	GBridge.getSurface().queueEvent(new Runnable(){

			@Override
			public void run() {
				GBridge.onInvoke(methodName, args, types, sData);
			}
	  		
	  	});

	  	return null;
	  }
}

