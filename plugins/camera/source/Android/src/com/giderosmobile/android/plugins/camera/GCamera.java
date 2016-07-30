package com.giderosmobile.android.plugins.camera;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.nio.IntBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import javax.microedition.khronos.egl.EGL;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.opengles.GL10;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.media.MediaPlayer;
import android.net.Uri;
import android.opengl.GLES20;
import android.opengl.GLException;
import android.os.Build;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
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
	}

	private static int             _updateTexImageCounter = 0;
	private static int             _updateTexImageCompare = 0;
	private static SurfaceTexture stex=null;
	private static int[] frames=new int[1];;
	private static int[] frameRender=new int[1];;
	private static int[] camtex=new int[1];
	private static float mat[]=new float[16];
	private static int tgtTexId=0;
	@TargetApi(11)
	public static void frame()
	{
		if (_updateTexImageCounter!=_updateTexImageCompare)
		{
			stex.updateTexImage();
			stex.getTransformMatrix(mat);
			nativeRender(camtex[0],mat);
			_updateTexImageCompare++;
		}
	}

	@SuppressWarnings("JniMissingFunction")
	static native void nativeRender(int camtex, float[] mat);
	private static int GL_TEXTURE_EXTERNAL_OES = 0x8D65;


	@TargetApi(Build.VERSION_CODES.HONEYCOMB)
	public static int[] start(int width,int height,int angle)
	{
		int[] dimret=new int[2];
		if (camera==null)
		{
			camera = Camera.open();
			if (camera!=null)
			{
				if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
					try {
						GLES20.glGenTextures(1,camtex,0);
						GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, camtex[0]);
						GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER,
								GLES20.GL_NEAREST);
						GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER,
								GLES20.GL_LINEAR);
						stex=new SurfaceTexture(camtex[0]);
						stex.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
							@Override
							public void onFrameAvailable(SurfaceTexture surfaceTexture) {
								GCamera._updateTexImageCounter++;
							}
						});


						Camera.Parameters parameters = camera.getParameters();

						if (camera.getParameters().getSupportedPreviewSizes() != null){
							Camera.Size previewSize = getOptimalPreviewSize(camera.getParameters().getSupportedPreviewSizes(), width, height);
							parameters.setPreviewSize(previewSize.width, previewSize.height);
							dimret[0]=previewSize.width;
							dimret[1]=previewSize.height;
						}

						if(parameters.getSupportedFocusModes() != null && parameters.getSupportedFlashModes().contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)){
							parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
						}

						if (parameters.getSupportedFlashModes() != null && parameters.getSupportedFlashModes().contains(Camera.Parameters.FLASH_MODE_AUTO)){
							parameters.setFlashMode(Camera.Parameters.FLASH_MODE_AUTO);
						}

						camera.setParameters(parameters);
						camera.setPreviewTexture(stex);
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
				camera.startPreview();
			}
		}
		return dimret;
	}

	static private Camera.Size getOptimalPreviewSize(List<Camera.Size> sizes, int width, int height)
	{
		// Source: http://stackoverflow.com/questions/7942378/android-camera-will-not-work-startpreview-fails
		Camera.Size optimalSize = null;

		final double ASPECT_TOLERANCE = 0.1;
		double targetRatio = (double) height / width;

		// Try to find a size match which suits the whole screen minus the menu on the left.
		for (Camera.Size size : sizes){

			if (size.height != width) continue;
			double ratio = (double) size.width / size.height;
			if (ratio <= targetRatio + ASPECT_TOLERANCE && ratio >= targetRatio - ASPECT_TOLERANCE){
				optimalSize = size;
			}
		}

		// If we cannot find the one that matches the aspect ratio, ignore the requirement.
		if (optimalSize == null) {
			// TODO : Backup in case we don't get a size.
			return sizes.get(0);
		}

		return optimalSize;
	}

	public static void stop()
	{
		if (camera!=null)
		{
			camera.stopPreview();
			_updateTexImageCompare=0;
			_updateTexImageCounter=0;
			camera.release();
			camera=null;
		}
	}
	
	public static boolean isCameraAvailable(){
		PackageManager pm = sActivity.get().getPackageManager();
		return pm.hasSystemFeature(PackageManager.FEATURE_CAMERA);
	}	
}
