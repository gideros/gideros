package com.giderosmobile.android.plugins.camera;

import java.io.File;
import java.io.FileOutputStream;
import java.lang.ref.WeakReference;
import java.nio.IntBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;

import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaPlayer;
import android.net.Uri;
import android.opengl.GLException;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.widget.FrameLayout;
import android.widget.VideoView;

public class GCamera {
	private static WeakReference<Activity> sActivity;
	private static Camera camera;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
	}
	
	//on destroy event
	public static void onDestroy()
	{
		stop();
		cleanup();
	}
	
	public static void start(int glid)
	{
		if (camera==null)
		{
			camera = Camera.open();
			if (camera!=null)
			{
				camera.setPreviewtexture(new SurfaceTexture(glid));
				camera.startPreview();
			}
		}
	}
	
	public static void stop()
	{
		if (camera!=null)
		{
			camera.stopPreview();
			camera.release();
			camera=null;
		}
	}
	
	public static boolean isCameraAvailable(){
		PackageManager pm = sActivity.get().getPackageManager();
		return pm.hasSystemFeature(PackageManager.FEATURE_CAMERA);
	}	
}
