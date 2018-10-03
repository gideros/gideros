package com.giderosmobile.android.plugins.megacool;

import java.lang.ref.WeakReference;
import java.util.List;

import android.app.Activity;
import android.util.Log;
import android.os.Build;
import android.graphics.Point;
import android.view.Display;
import com.giderosmobile.android.GiderosSettings;
import co.megacool.megacool.Event;
import co.megacool.megacool.EventType;
import co.megacool.megacool.Megacool;
import co.megacool.megacool.ShareConfig;

public class GMegaCool
{
	private static WeakReference<Activity> sActivity;
	private static final int GMEGACOOL_EVENT_RECEIVED_SHARE_OPENED=1;
	private static final int GMEGACOOL_EVENT_SENT_SHARE_OPENED=2;
	private static final String TAG="Megacool";

	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
	    // Initialize the Megacool SDK
        Megacool.start(activity, "MEGACOOLKEY", new Megacool.OnEventsReceivedListener() {
            @Override
            public void onEventsReceived(List<Event> events) {
                for (Event event : events) {
                    if (event.type == EventType.RECEIVED_SHARE_OPENED 
                        && event.isFirstSession()) {
                        // This device has received a share and installed the 
                        // app for the first time
                        Log.d(TAG, "This device installed the app.");
        	        	onEvent(GMEGACOOL_EVENT_RECEIVED_SHARE_OPENED);
                    }
                    if (event.type == EventType.SENT_SHARE_OPENED 
                        && event.isFirstSession()) {
                        // A share sent from this device has been opened, and 
                        // the receiver installed the app for the first time
                        Log.d(TAG, "Your friend installed the app. Here's your reward!");
        	        	onEvent(GMEGACOOL_EVENT_SENT_SHARE_OPENED);
                    }
                }
            }
        },"Gideros",getGiderosVersion());
        //View is not laid out yet, use display as we probably run full screen
		Display display = activity.getWindowManager().getDefaultDisplay();
		Point ds=new Point();
		display.getSize(ds);
		int vd=2;
		if (Math.max(ds.x,ds.y)>1500) vd=4;
		Megacool.initCapture(ds.x/vd,ds.y/vd,"OpenGLES3");
		Megacool.setCaptureMethod(Megacool.CaptureMethod.OPENGL);
	}
	
	public static boolean share(String fallback){
		Activity activity=sActivity.get();
		if (fallback!=null) {
			ShareConfig shareConfig = new ShareConfig()
			        .fallbackImageUrl(fallback);
			Megacool.share(activity,shareConfig);
		}
		else
			Megacool.share(activity);
		return true;
	}

	public static boolean startRecording() {
		Activity activity=sActivity.get();
		if (activity!=null) {
			Megacool.startRecording(GiderosSettings.mainView);
		}
		return false;
	}	

	public static void stopRecording() {
		Megacool.stopRecording();
	}	

	static native void onEvent(int event);	
	static native String getGiderosVersion();
	
	static boolean renderInit=false;
	public static void frameEnter() {
		if (!renderInit) {
			Megacool.initRenderThread();
			renderInit=true;
		}
	}
	
	public static void frameLeave() {
		Megacool.notifyRenderComplete();
	}
}