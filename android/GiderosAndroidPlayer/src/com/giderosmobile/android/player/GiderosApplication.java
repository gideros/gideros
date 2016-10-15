package com.giderosmobile.android.player;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.BufferedOutputStream;
import java.lang.reflect.Method;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.Locale;

import dalvik.system.DexClassLoader;
import android.app.Activity;
import android.app.UiModeManager;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.Color;
import android.media.AudioManager;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.os.Vibrator;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.FrameLayout;
import android.widget.ListView;
import android.widget.TextView;

public class GiderosApplication
{
	public Object lock = new Object();
	
	public ZipResourceFile mainFile;
	public ZipResourceFile patchFile;
	
	static private GiderosApplication instance_;	
	static public GiderosApplication getInstance()
	{
		return instance_;
	}
	
	private Accelerometer accelerometer_;
	private boolean isAccelerometerStarted_ = false;

	private Gyroscope gyroscope_;
	private boolean isGyroscopeStarted_ = false;

	private Geolocation geolocation_;
	private boolean isLocationStarted_ = false;
	private boolean isHeadingStarted_ = false;

	private boolean isForeground_ = false;

	private boolean isSurfaceCreated_ = false;

	private ArrayList<Integer> eventQueue_ = new ArrayList<Integer>();
	private static final int PAUSE = 0;
	private static final int RESUME = 1;
	private static final int STOP = 2;
	private static final int START = 3;
	
	private ListView projectList;
	
	private GGMediaPlayerManager mediaPlayerManager_;
	
	//private AudioDevice audioDevice_;
	
	private ArrayList < Class < ? >>	sAvailableClasses = new ArrayList < Class < ? >> ();
	
	private static Class < ? > findClass ( String className ) {
		
		Class < ? > theClass = null;
		try {

			theClass = Class.forName ( className );
		} catch ( Throwable e ) {

		}
		
		return theClass;
	}
	
	private static Object executeMethod ( Class < ? > theClass, Object theInstance, String methodName, Class < ? > [] parameterTypes, Object [] parameterValues ) {
		
		Object result = null;
		if ( theClass != null ) {
			
			try {

				Method theMethod = theClass.getMethod ( methodName, parameterTypes );

				result = theMethod.invoke ( theInstance, parameterValues );
			} catch ( Throwable e ) {
				
			}			
		}
		
		return result;
	}
	
	public GiderosApplication(String[] sExternalClasses)
	{
		for ( String className : sExternalClasses )
		{
			if (className != null)
			{
				Class < ? > theClass = findClass ( className );
				if ( theClass != null ) {				
					sAvailableClasses.add ( theClass );
				}
			}
		}
		
		accelerometer_ = new Accelerometer();
		gyroscope_ = new Gyroscope();
		geolocation_ = new Geolocation();
	
		populateAllFiles();
		getDirectories();
		
		mediaPlayerManager_ = new GGMediaPlayerManager(mainFile, patchFile);

		//audioDevice_ = new AudioDevice();
		Activity activity=WeakActivityHolder.get();
		int sampleRate=0;
		if (android.os.Build.VERSION.SDK_INT >= 17)
		{
			try
			{
				AudioManager am = (AudioManager) activity.getSystemService(Context.AUDIO_SERVICE);
				String sampleRateStr = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
				sampleRate = Integer.parseInt(sampleRateStr);
			}
			catch (Exception e)
			{
			}
		}
		if (sampleRate == 0) sampleRate = 44100; // Use a default value if property not found
		GiderosApplication.nativeOpenALSetup(sampleRate);
				
		synchronized (lock)
		{
			GiderosApplication.nativeCreate(allfiles_ == null);
			GiderosApplication.nativeSetDirectories(externalDir_, internalDir_, cacheDir_);
			if (allfiles_ != null)
				GiderosApplication.nativeSetFileSystem(allfiles_);
		}	
		
		loadLpkPlugins();
	}
	
	public void loadLpkPlugins()
	{
		Activity activity=WeakActivityHolder.get();
		AssetManager assetManager = activity.getAssets();
		try {
			String[] dexs = assetManager.list("dex");
			String libraryPath = activity.getApplicationContext().getApplicationInfo().nativeLibraryDir;
			Log.v("LPK","Looking for LPK plugins");
			for (String dexn:dexs)
			{
				String dex=dexn.substring(0, dexn.indexOf("."));
				Log.v("LPK","Found "+dexn+" ("+dex+")");
				File dexInternalStoragePath = new File(activity.getDir("dex", Context.MODE_PRIVATE),dex+".dex");
				if (!dexInternalStoragePath.exists())
				{
					//Copy dex to private area
			    BufferedInputStream bis = null;
				OutputStream dexWriter = null;

				  final int BUF_SIZE = 8 * 1024;
				  try {
				      bis = new BufferedInputStream(assetManager.open("dex/"+dex+".dex"));
				      dexWriter = new BufferedOutputStream(new FileOutputStream(dexInternalStoragePath));
				      byte[] buf = new byte[BUF_SIZE];
				      int len;
				      while((len = bis.read(buf, 0, BUF_SIZE)) > 0) {
				          dexWriter.write(buf, 0, len);
				      }
				      dexWriter.close();
				      bis.close();				      
				  } catch (Exception ge)
				  {
						ge.printStackTrace();
					  dexInternalStoragePath.delete();
				  }
				}
				if (dexInternalStoragePath.exists())
				{
					//Load plugin from dex
					  // Internal storage where the DexClassLoader writes the optimized dex file to
					  final File optimizedDexOutputPath = activity.getDir("optdex", Context.MODE_PRIVATE);

					  DexClassLoader cl = new DexClassLoader(dexInternalStoragePath.getAbsolutePath(),
					                                         optimizedDexOutputPath.getAbsolutePath(),
					                                         libraryPath,
					                                         activity.getClassLoader());
					  try {
						Class<?> k=cl.loadClass("com.giderosmobile.android.plugins."+dex+".Loader");
						try {
							k.newInstance();
							Log.v("LPK","Loaded "+dex+" :"+k.getName());
							try {
								Method mtd = k.getMethod("onLoad", Activity.class);
								mtd.invoke(null, activity);
								Log.v("LPK","Initialized "+dex+" :"+k.getName());
							} catch (NoSuchMethodException le) {
							} catch (Exception le) {
								le.printStackTrace();
							}
						} catch (InstantiationException e) {
							e.printStackTrace();
						} catch (IllegalAccessException e) {
							e.printStackTrace();
						}
					} catch (ClassNotFoundException e) {
						e.printStackTrace();
					}
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		} catch (Error e) {
			e.printStackTrace();
		}
	}

	private String allfiles_ = null;

	private void populateAllFiles()
	{
		allfiles_ = null;
		
		AssetManager assetManager = WeakActivityHolder.get().getAssets();

		ArrayList<String> lines = new ArrayList<String>();

		try
		{
			InputStream in = assetManager.open("assets/allfiles.txt");
			BufferedReader br = new BufferedReader(new InputStreamReader(in));
			while (true)
			{
				String line = br.readLine();
				if (line == null)
					break;
				lines.add(line);
			}
			in.close();
		} catch (IOException e)
		{
			loadProjects();
			Logger.log("player mode");
			return;
		}		
				
		StringBuilder sb = new StringBuilder();

		ApplicationInfo applicationInfo = WeakActivityHolder.get().getApplicationInfo();
		sb.append(applicationInfo.sourceDir).append("|");

		lines.add("properties.bin*");
		lines.add("luafiles.txt*");
					
		String mainPath = null;
		String patchPath = null;
		mainFile = null;
		patchFile = null;
		try
		{
			if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED))
			{
				Activity activity = WeakActivityHolder.get();

				String packageName = activity.getPackageName();
				String root = Environment.getExternalStorageDirectory().getAbsolutePath();
				String expPath = root + "/Android/obb/" + packageName;

				int versionCode = activity.getPackageManager().getPackageInfo(activity.getPackageName(), 0).versionCode;

				mainPath = expPath + "/" + "main." + versionCode + "." + packageName + ".obb";
				if ((new File(mainPath)).isFile())
					mainFile = new ZipResourceFile(mainPath);

				patchPath = expPath + "/" + "patch." + versionCode + "." + packageName + ".obb";
				if ((new File(patchPath)).isFile())
					patchFile = new ZipResourceFile(patchPath);
			}
		} catch (Exception e)
		{
		}
		
		if (mainFile != null)
			sb.append(mainPath);
		sb.append("|");			

		if (patchFile != null)
			sb.append(patchPath);
		sb.append("|");			

		for (int i = 0; i < lines.size(); ++i)
		{
			String line = lines.get(i);				
			
			String suffix = "";
			
			if (line.endsWith("*"))
			{
				line = line.substring(0, line.length() - 1);
				suffix = ".jet";
			}
			
			try
			{
				int zipFile = 0;
				AssetFileDescriptor fd = null;

				if (patchFile != null)
				{
					fd = patchFile.getAssetFileDescriptor(line + suffix);
					zipFile = 2;
				}

				if (fd == null)
					if (mainFile != null)
					{
						fd = mainFile.getAssetFileDescriptor(line + suffix);
						zipFile = 1;
					}

				if (fd == null)
				{
					fd = assetManager.openFd("assets/" + line + suffix);
					zipFile = 0;
				}

				if (fd != null)
				{
					sb.append(line).append("|").append(zipFile).append("|").append(fd.getStartOffset()).append("|").append(fd.getLength()).append("|");
					fd.close();
				}
			} catch (IOException e)
			{
				Logger.log(e.toString());
			}
		}

		sb.deleteCharAt(sb.length() - 1);
		
		allfiles_ = sb.toString();
	}
	
	String externalDir_, internalDir_, cacheDir_;
	private void getDirectories()
	{
		boolean mExternalStorageAvailable = false;
		boolean mExternalStorageWriteable = false;
		String state = Environment.getExternalStorageState();

		if (Environment.MEDIA_MOUNTED.equals(state))
		{
			// We can read and write the media
			mExternalStorageAvailable = mExternalStorageWriteable = true;
		} else if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(state))
		{
			// We can only read the media
			mExternalStorageAvailable = true;
			mExternalStorageWriteable = false;
		} else
		{
			// Something else is wrong. It may be one of many other states, but
			// all we need
			// to know is we can neither read nor write
			mExternalStorageAvailable = mExternalStorageWriteable = false;
		}

		externalDir_ = Environment.getExternalStorageDirectory().getAbsolutePath();
		internalDir_ = WeakActivityHolder.get().getFilesDir().getAbsolutePath();
		cacheDir_ = WeakActivityHolder.get().getCacheDir().getAbsolutePath();

		Logger.log("externalDir: " + externalDir_);
		Logger.log("internalDir: " + internalDir_);
		Logger.log("cacheDir: " + cacheDir_);
	}
	
	private void loadProjects(){
		projectList = new ListView(WeakActivityHolder.get());
		TextView text = new TextView(WeakActivityHolder.get());
		text.setText("Gideros Projects");
		text.setTextColor(Color.BLACK);
		text.setTextSize(25);
		text.setBackgroundColor(Color.WHITE);
		projectList.addHeaderView(text);
		ArrayAdapter<String> modeAdapter = new ArrayAdapter<String>(WeakActivityHolder.get(), android.R.layout.simple_list_item_1, android.R.id.text1, traverse(new File(Environment.getExternalStorageDirectory().toString()+"/gideros"))){
			@Override
	        public View getView(int position, View convertView, ViewGroup parent) {
	            View view =super.getView(position, convertView, parent);

	            TextView textView=(TextView) view.findViewById(android.R.id.text1);
	            textView.setTextColor(Color.BLACK);
	            textView.setBackgroundColor(Color.WHITE);

	            return view;
	        }
		};
		projectList.setAdapter(modeAdapter);
		projectList.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            	TextView textView=(TextView) view.findViewById(android.R.id.text1);
            	if(textView != null){
            		projectList.setVisibility(View.GONE);
            		nativeOpenProject((String) textView.getText());
            	}
            }
        });
		projectList.setVisibility(View.GONE);
		FrameLayout layout = (FrameLayout)WeakActivityHolder.get().getWindow().getDecorView();
		layout.addView(projectList);
	}
	
	public List<String> traverse (File dir) {
		Logger.log("Checking: " + dir.getAbsolutePath());
		List<String> projects = new ArrayList<String>();
	    if (dir.exists()) {
	        File[] files = dir.listFiles();
            if(files != null){
                for (int i = 0; i < files.length; ++i) {
                    File file = files[i];
                    if (file.isDirectory()) {
                        Logger.log("Found: " + file.getName());
                        projects.add(file.getName());
                    }
                }
            }
	    }
	    return projects;
	} 
	
	
	private static GLSurfaceView mGLView_;
	static public void onCreate(String[] externalClasses, GLSurfaceView mGLView)
	{
		mGLView_=mGLView;
		instance_ = new GiderosApplication(externalClasses);
		setKeyboardVisibility(false);
		for ( Class < ? > theClass : instance_.sAvailableClasses ) {
			
			executeMethod ( theClass, null, "onCreate", new Class < ? > [] { Activity.class }, new Object [] { WeakActivityHolder.get() });
		}
	}
	
	static public void onDestroy()
	{
		for ( Class < ? > theClass : instance_.sAvailableClasses ) {

			executeMethod ( theClass, null, "onDestroy", new Class < ? > [] { }, new Object [] { });
		}	

		synchronized (instance_.lock)
		{
			GiderosApplication.nativeDestroy();
		}

		instance_ = null;
	}

	public void onStart()
	{
		if (isSurfaceCreated_ == true) {
			synchronized (eventQueue_) {
				eventQueue_.add(START);
			}
		}
		
		for ( Class < ? > theClass : sAvailableClasses ) {

			executeMethod ( theClass, null, "onStart", new Class < ? > [] { }, new Object [] { });
		}		
	}
	
	public void onRestart()
	{
		for ( Class < ? > theClass : sAvailableClasses ) {

			executeMethod ( theClass, null, "onRestart", new Class < ? > [] { }, new Object [] { });
		}
	}
	
	public void onStop()
	{
		for ( Class < ? > theClass : sAvailableClasses ) {

			executeMethod ( theClass, null, "onStop", new Class < ? > [] { }, new Object [] { });
		}

		if (isSurfaceCreated_ == true) {
			synchronized (eventQueue_) {
				eventQueue_.add(STOP);
			}
		}
	}
	
	
	public void onPause()
	{
		isForeground_ = false;

		for ( Class < ? > theClass : sAvailableClasses ) {

			executeMethod ( theClass, null, "onPause", new Class < ? > [] { }, new Object [] { });
		}	

		if (isSurfaceCreated_ == true) {
			synchronized (eventQueue_) {
				eventQueue_.add(PAUSE);
				try {
					eventQueue_.wait();
				} catch (InterruptedException e) {
				}
			}
		}

		
		accelerometer_.disable();
		gyroscope_.disable();
		geolocation_.stopUpdatingLocation();
		geolocation_.stopUpdatingHeading();
		mediaPlayerManager_.onPause();
		//audioDevice_.stop();
	}
	
	public void onResume()
	{
		isForeground_ = true;
		if (isAccelerometerStarted_)
			accelerometer_.enable();
		if (isGyroscopeStarted_)
			gyroscope_.enable();
		if (isLocationStarted_)
			geolocation_.startUpdatingLocation();
		if (isHeadingStarted_)
			geolocation_.startUpdatingHeading();
		mediaPlayerManager_.onResume();
		//audioDevice_.start();

		for ( Class < ? > theClass : sAvailableClasses ) {

			executeMethod ( theClass, null, "onResume", new Class < ? > [] { }, new Object [] { });
		}	

		if (isSurfaceCreated_ == true) {
			synchronized (eventQueue_) {
				eventQueue_.add(RESUME);
				try {
					eventQueue_.wait();
				} catch (InterruptedException e) {
				}
			}
		}
	}

	public void onLowMemory()
	{
		synchronized (lock)
		{
			nativeLowMemory();
		}
	}
	
	public void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		for ( Class < ? > theClass : sAvailableClasses ) {

			executeMethod ( theClass, null, "onActivityResult", new Class < ? > [] { java.lang.Integer.TYPE, java.lang.Integer.TYPE, Intent.class }, new Object [] { new Integer ( requestCode ), new Integer ( resultCode ), data });
		}	
	}

	public void onSurfaceCreated()
	{
		synchronized (lock)
		{
			GiderosApplication.nativeSurfaceCreated();
			isSurfaceCreated_ = true;
		}		
	}
	
	public void onSurfaceChanged(int w, int h)
	{
		synchronized (lock)
		{
			GiderosApplication.nativeSurfaceChanged(w, h, getRotation(w,h));
		}	
	}

	static private void sleep(long nsec)
	{
		try {
			Thread.sleep(nsec / 1000000, (int)(nsec % 1000000));
		} catch (InterruptedException e) {
		}
	}

	private long startTime = System.nanoTime();

	public void onDrawFrame()
	{		
		long target = 1000000000L / (long)fps_;
		
		long currentTime = System.nanoTime();
		long dt = currentTime - startTime;
		startTime = currentTime;
		if (dt < 0)
			dt = 0;
		if (dt < target)
		{
			sleep(target - dt);
			startTime += target - dt;
		}
	    
		synchronized (lock) {
			synchronized (eventQueue_) {
				while (!eventQueue_.isEmpty()) {
					int event = eventQueue_.remove(0);
					switch (event) {
					case PAUSE:
						GiderosApplication.nativePause();
						break;
					case RESUME:
						GiderosApplication.nativeResume();
						break;
					case START:
						GiderosApplication.nativeStart();
						break;
					case STOP:
						GiderosApplication.nativeStop();
						break;
					}
				}
				eventQueue_.notify();
			}

			GiderosApplication.nativeDrawFrame();
		}
	}

	public void onTouchesBegin(int size, int[] id, int[] x, int[] y, float[] pressure, int actionIndex)
	{
		if(!isRunning()){
			if(projectList != null && projectList.getVisibility() == View.GONE){
				projectList.setVisibility(View.VISIBLE);
			}
		}
		GiderosApplication.nativeTouchesBegin(size, id, x, y, pressure, actionIndex);
	}

	public void onTouchesMove(int size, int[] id, int[] x, int[] y, float[] pressure)
	{	
		GiderosApplication.nativeTouchesMove(size, id, x, y, pressure);
	}

	public void onTouchesEnd(int size, int[] id, int[] x, int[] y, float[] pressure, int actionIndex)
	{
		GiderosApplication.nativeTouchesEnd(size, id, x, y, pressure, actionIndex);
	}

	public void onTouchesCancel(int size, int[] id, int[] x, int[] y, float[] pressure)
	{
		GiderosApplication.nativeTouchesCancel(size, id, x, y, pressure);
	}	
	
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		if(projectList != null && projectList.getVisibility() == View.VISIBLE){
			projectList.setVisibility(View.GONE);
			return true;
		}
		boolean handled = nativeKeyDown(keyCode, event.getRepeatCount());
		if (event.getUnicodeChar()>0)
			nativeKeyChar(Character.toString((char)event.getUnicodeChar()));
		if(keyCode == KeyEvent.KEYCODE_VOLUME_DOWN || keyCode == KeyEvent.KEYCODE_VOLUME_MUTE || keyCode == KeyEvent.KEYCODE_VOLUME_UP || keyCode == KeyEvent.KEYCODE_POWER){
			return false;
		}
		return handled;
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		return nativeKeyUp(keyCode, event.getRepeatCount());
	}
	
	public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) {
		if (keyCode==KeyEvent.KEYCODE_UNKNOWN)
			nativeKeyChar(event.getCharacters());
		else if (event.getUnicodeChar()>0)
		{
			String cs=Character.toString((char)event.getUnicodeChar());
			for (int k=0;k<event.getRepeatCount();k++)
				nativeKeyChar(cs);
		}
				
		return false; //XXX what should be return ?
	}

	public boolean isAccelerometerAvailable()
	{
		return accelerometer_.isAvailable();
	}
	
	public void startAccelerometer()
	{
		isAccelerometerStarted_ = true;
		if (isForeground_)
			accelerometer_.enable();
	}

	public void stopAccelerometer()
	{
		isAccelerometerStarted_ = false;
		accelerometer_.disable();
	}


	public boolean isGyroscopeAvailable()
	{
		return gyroscope_.isAvailable();
	}
	
	public void startGyroscope()
	{
		isGyroscopeStarted_ = true;
		if (isForeground_)
			gyroscope_.enable();
	}
	
	public void stopGyroscope()
	{
		isGyroscopeStarted_ = false;
		gyroscope_.disable();
	}
	

	public boolean isGeolocationAvailable()
	{
		return geolocation_.isAvailable();
	}
	
	public boolean isHeadingAvailable()
	{
		return geolocation_.isHeadingAvailable();
	}
	
	public void setGeolocationAccuracy(double accuracy)
	{
		geolocation_.setAccuracy(accuracy);
	}
	
	public double getGeolocationAccuracy()
	{
		return geolocation_.getAccuracy();
	}
	
	public void setGeolocationThreshold(double threshold)
	{			
		geolocation_.setThreshold(threshold);
	}
	
	public double getGeolocationThreshold()
	{
		return geolocation_.getThreshold();
	}
	
	public void startUpdatingLocation()
	{
		isLocationStarted_ = true;
		if (isForeground_)
			geolocation_.startUpdatingLocation();
	}
	
	public void stopUpdatingLocation()
	{
		isLocationStarted_ = false;
		geolocation_.stopUpdatingLocation();
	}
	
	public void startUpdatingHeading()
	{
		isHeadingStarted_ = true;
		if (isForeground_)
			geolocation_.startUpdatingHeading();
	}

	public void stopUpdatingHeading()
	{
		isHeadingStarted_ = false;
		geolocation_.stopUpdatingHeading();		
	}	


	// static equivalents	
	static public boolean isAccelerometerAvailable_s()
	{
		return instance_.isAccelerometerAvailable();
	}
	static public void startAccelerometer_s()
	{
		instance_.startAccelerometer();
	}
	static public void stopAccelerometer_s()
	{
		instance_.stopAccelerometer();
	}


	static public boolean isGyroscopeAvailable_s()
	{
		return instance_.isGyroscopeAvailable();
	}
	static public void startGyroscope_s()
	{
		instance_.startGyroscope();
	}
	static public void stopGyroscope_s()
	{
		instance_.stopGyroscope();
	}	

	
	static public boolean isGeolocationAvailable_s()
	{
		return instance_.isGeolocationAvailable();
	}
	static public boolean isHeadingAvailable_s()
	{
		return instance_.isHeadingAvailable();
	}
	static public void setGeolocationAccuracy_s(double accuracy)
	{
		instance_.setGeolocationAccuracy(accuracy);
	}
	static public double getGeolocationAccuracy_s()
	{
		return instance_.getGeolocationAccuracy();
	}
	static public void setGeolocationThreshold_s(double threshold)
	{			
		instance_.setGeolocationThreshold(threshold);
	}
	static public double getGeolocationThreshold_s()
	{
		return instance_.getGeolocationThreshold();
	}
	static public void startUpdatingLocation_s()
	{
		instance_.startUpdatingLocation();
	}
	static public void stopUpdatingLocation_s()
	{
		instance_.stopUpdatingLocation();
	}
	static public void startUpdatingHeading_s()
	{
		instance_.startUpdatingHeading();
	}
	static public void stopUpdatingHeading_s()
	{
		instance_.stopUpdatingHeading();		
	}	

	static public long BackgroundMusicCreateFromFile(String fileName, int[] error)
	{
		return instance_.mediaPlayerManager_.BackgroundMusicCreateFromFile(fileName, error);
	}

	static public void BackgroundMusicDelete(long backgroundMusic)
	{
		instance_.mediaPlayerManager_.BackgroundMusicDelete(backgroundMusic);
	}

	static public int BackgroundMusicGetLength(long backgroundMusic)
	{
		return instance_.mediaPlayerManager_.BackgroundMusicGetLength(backgroundMusic);
	}

	static public long BackgroundMusicPlay(long backgroundMusic, boolean paused, long data)
	{
		return instance_.mediaPlayerManager_.BackgroundMusicPlay(backgroundMusic, paused, data);
	}
	
	static public void BackgroundChannelStop(long backgroundChannel)
	{
		instance_.mediaPlayerManager_.BackgroundChannelStop(backgroundChannel);
	}
	
	static public void BackgroundChannelSetPosition(long backgroundChannel, int position)
	{
		instance_.mediaPlayerManager_.BackgroundChannelSetPosition(backgroundChannel, position);		
	}

	static public int BackgroundChannelGetPosition(long backgroundChannel)
	{
		return instance_.mediaPlayerManager_.BackgroundChannelGetPosition(backgroundChannel);
	}
	
	static public void BackgroundChannelSetPaused(long backgroundChannel, boolean paused)
	{
		instance_.mediaPlayerManager_.BackgroundChannelSetPaused(backgroundChannel, paused);		
	}

	static public boolean BackgroundChannelIsPaused(long backgroundChannel)
	{
		return instance_.mediaPlayerManager_.BackgroundChannelIsPaused(backgroundChannel);	
	}

	static public boolean BackgroundChannelIsPlaying(long backgroundChannel)
	{
		return instance_.mediaPlayerManager_.BackgroundChannelIsPlaying(backgroundChannel);
	}

	static public void BackgroundChannelSetVolume(long backgroundChannel, float volume)
	{
		instance_.mediaPlayerManager_.BackgroundChannelSetVolume(backgroundChannel, volume);		
	}

	static public float BackgroundChannelGetVolume(long backgroundChannel)
	{
		return instance_.mediaPlayerManager_.BackgroundChannelGetVolume(backgroundChannel);
	}

	static public void BackgroundChannelSetLooping(long backgroundChannel, boolean looping)
	{
		instance_.mediaPlayerManager_.BackgroundChannelSetLooping(backgroundChannel, looping);		
	}

	static public boolean BackgroundChannelIsLooping(long backgroundChannel)
	{
		return instance_.mediaPlayerManager_.BackgroundChannelIsLooping(backgroundChannel);		
	}

	int fps_ = 60;
	static public void setFps(int fps)
	{
		instance_.fps_ = fps;		
	}	
	
	
	static public void setKeepAwake(boolean awake)
	{
		final Activity activity = WeakActivityHolder.get();
		
		if (awake)
			activity.runOnUiThread(new Runnable() {public void run() {activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);}});
		else
			activity.runOnUiThread(new Runnable() {public void run() {activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);}});
	}
	
	static public boolean setKeyboardVisibility(final boolean visible)
	{
		final Activity activity = WeakActivityHolder.get();
		activity.runOnUiThread(new Runnable() {
		    public void run() {
		    	activity.getWindow().setSoftInputMode(visible?WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE:WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
		    	mGLView_.clearFocus();
		    	if (visible)
		    		mGLView_.requestFocus();
		    	
		    	InputMethodManager imm = (InputMethodManager)
	    			activity.getSystemService(Context.INPUT_METHOD_SERVICE);
		    	if (visible)
		    		imm.showSoftInput(mGLView_ , 0/*InputMethodManager.SHOW_FORCED*/);
		    	else
		    	{
		    		imm.hideSoftInputFromWindow(mGLView_.getWindowToken() ,0); 
		    		activity.onWindowFocusChanged(activity.hasWindowFocus());
		    	}
		    }
		});
		return true;
	}
	
	static public void vibrate(int ms)	
	{
		try
		{
			((Vibrator)WeakActivityHolder.get().getSystemService(Context.VIBRATOR_SERVICE)).vibrate(ms);
		}
		catch (SecurityException e)
		{
		}
	}	

	static public String getLocale()
	{
		Locale locale = Locale.getDefault();
		return locale.getLanguage() + "_" + locale.getCountry();
	}

	static public String getLanguage()
	{
		Locale locale = Locale.getDefault();
		return locale.getLanguage();
	}

	static public String getVersion()
	{
		return android.os.Build.VERSION.RELEASE;
	}

	static public String getManufacturer()
	{
		return android.os.Build.MANUFACTURER;
	}

	static public String getModel()
	{
		return android.os.Build.MODEL;
	}
    
    static public String getDeviceType()
	{
		UiModeManager uiModeManager = (UiModeManager) WeakActivityHolder.get().getSystemService(Context.UI_MODE_SERVICE);
        if (uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_TELEVISION) {
            return "TV";
        } else if (uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_APPLIANCE) {
            return "Appliance";
        } else if (uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_CAR) {
            return "Car";
        } else if (uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_DESK) {
            return "Desk";
        } else if (uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_WATCH) {
            return "Watch";
        } else {
            return "Mobile";
        }
	}

	static public void openUrl(String url)
	{
		Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));

		try
		{
			WeakActivityHolder.get().startActivity(intent);
		}
		catch (ActivityNotFoundException e) 
		{
		}
	}

	static public boolean canOpenUrl(String url)
	{
		try
		{
			Activity activity = WeakActivityHolder.get();
			PackageManager pm = activity.getPackageManager();
			Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
			ComponentName cn = intent.resolveActivity(pm);

			if (cn != null)
				return true;
		}
		catch (Exception e)
		{
		}

		return false;
	}
	
	static public void finishActivity()
	{
		final Activity activity = WeakActivityHolder.get();

		activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
            	activity.finish();
            }
        });
	}
	
	static public int getScreenDensity()
	{
		DisplayMetrics dm = new DisplayMetrics();
		WeakActivityHolder.get().getWindowManager().getDefaultDisplay().getMetrics(dm);
		return dm.densityDpi;		
	}

	public static String getLocalIPs()
	{
		StringBuilder sb = new StringBuilder();

		try
		{
			for (Enumeration<NetworkInterface> en = NetworkInterface
					.getNetworkInterfaces(); en.hasMoreElements();)
			{
				NetworkInterface intf = en.nextElement();
				for (Enumeration<InetAddress> enumIpAddr = intf
						.getInetAddresses(); enumIpAddr.hasMoreElements();)
				{
					InetAddress inetAddress = enumIpAddr.nextElement();
					if (!inetAddress.isLoopbackAddress())
					{
						sb.append(inetAddress.getHostAddress().toString());
						sb.append("|");
					}
				}
			}
		} catch (SocketException ex)
		{
		}

		if (sb.length() != 0)
			sb.deleteCharAt(sb.length() - 1);

		return sb.toString();
	}	
	
	public static int getRotation(int w,int h)
	{
		Activity activity = WeakActivityHolder.get();

		int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
		
		if (w<=h)
		{
			if (rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_270)
				return 0;
			else
				return 180;
		}
		else
		{
			if (rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_90)
				return 90;
			else
				return 270;
		}
	}
	
	static public String getDeviceName() {
		String manufacturer = android.os.Build.MANUFACTURER;
		String model = android.os.Build.MODEL;
		if (model.startsWith(manufacturer)) {
			return model;
		} else {
			return manufacturer + " " + model;
		}
	}
	
	static public void throwLuaException(String error) throws LuaException{
		throw new LuaException(error);
	}

	static private native void nativeOpenProject(String project);
	static private native void nativeLowMemory();
	static private native boolean isRunning();
	static private native boolean nativeKeyDown(int keyCode, int repeatCount);
	static private native boolean nativeKeyUp(int keyCode, int repeatCount);
	static private native void nativeKeyChar(String keyChar);
	static private native void nativeOpenALSetup(int sampleRate);
	static private native void nativeCreate(boolean player);
	static private native void nativeSetDirectories(String externalDir, String internalDir, String cacheDir);
	static private native void nativeSetFileSystem(String files);
	static private native void nativePause();
	static private native void nativeResume();
	static private native void nativeDestroy();
	static private native void nativeSurfaceCreated();
	static private native void nativeSurfaceChanged(int w, int h, int rotation);
	static private native void nativeDrawFrame();
	static private native void nativeTouchesBegin(int size, int[] id, int[] x, int[] y, float[] pressure, int actionIndex);
	static private native void nativeTouchesMove(int size, int[] id, int[] x, int[] y, float[] pressure);
	static private native void nativeTouchesEnd(int size, int[] id, int[] x, int[] y, float[] pressure, int actionIndex);
	static private native void nativeTouchesCancel(int size, int[] id, int[] x, int[] y, float[] pressure);
	static private native void nativeStop();
	static private native void nativeStart();
}
