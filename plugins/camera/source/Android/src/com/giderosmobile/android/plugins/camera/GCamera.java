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
import android.Manifest;

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

	public static class CamInfo {
		public String name;
		public String description;
		public int position;
	}

	public static CamInfo[] availableDevices() {
		int ncams=Camera.getNumberOfCameras();
		CamInfo[] cams=new CamInfo[ncams];
		Camera.CameraInfo cami=new Camera.CameraInfo();
		for (int k=0;k<ncams;k++) {
			Camera.getCameraInfo(k,cami);
			CamInfo cam=new CamInfo();
			cam.name=""+k;
			cam.description="Builtin "+
					((cami.facing==Camera.CameraInfo.CAMERA_FACING_FRONT)?"front":"back")+
					" camera #"+k;
			cam.position=(cami.facing==Camera.CameraInfo.CAMERA_FACING_FRONT)?1:2;
			cams[k]=cam;
		}
		return cams;
	}

	@TargetApi(Build.VERSION_CODES.HONEYCOMB)
	public static int[] start(int width,int height,int angle,String device)
	{
		int[] dimret=new int[4];
		int camId=-1;
		try {
			camId=Integer.parseInt(device);
		} catch (Exception e)
		{
		}
		if ((camId==-1)||(camId>=Camera.getNumberOfCameras()))
		{
			int ncams=Camera.getNumberOfCameras();
			Camera.CameraInfo cami=new Camera.CameraInfo();
			camId=0;
			for (int k=0;k<ncams;k++) {
				Camera.getCameraInfo(k,cami);
				if (cami.facing==Camera.CameraInfo.CAMERA_FACING_BACK) {
					camId = k;
					break;
				}
			}
		}

		if (camera==null)
		{
			camera = Camera.open(camId);
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
							Camera.Size previewSize = getOptimalPreviewSize2(camera.getParameters().getSupportedPreviewSizes(), width, height);
							parameters.setPreviewSize(previewSize.width, previewSize.height);
							dimret[0]=previewSize.width;
							dimret[1]=previewSize.height;
						}

						List<String> focusmodes=parameters.getSupportedFocusModes();
						if(focusmodes!=null)
						{
							if (focusmodes.contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE))
								parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
							else if (focusmodes.contains(Camera.Parameters.FOCUS_MODE_AUTO))
								parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
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
				android.hardware.Camera.CameraInfo info =
						new android.hardware.Camera.CameraInfo();
				android.hardware.Camera.getCameraInfo(camId, info);

				if ((angle%180)!=(info.orientation%180)) { int c=dimret[0]; dimret[0]=dimret[1]; dimret[1]=c; }
				if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT)
					dimret[2] = (angle - info.orientation + 360) % 360;
				else
					dimret[2] = (angle + info.orientation + 360) % 360;
				dimret[3] = 1;
				camera.setDisplayOrientation(0);
				camera.startPreview();
			}
		}
		return dimret;
	}

	static private Camera.Size getOptimalPreviewSize2(List<Camera.Size> sizes, int w, int h) {
        final double ASPECT_TOLERANCE = 0.1;
        double targetRatio=(double)h / w;

        if (sizes == null) return null;

        Camera.Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;

        int targetHeight = h;

        for (Camera.Size size : sizes) {
            double ratio = (double) size.width / size.height;
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) continue;
            if (Math.abs(size.height - targetHeight) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(size.height - targetHeight);
            }
        }

        if (optimalSize == null) {
            minDiff = Double.MAX_VALUE;
            for (Camera.Size size : sizes) {
                if (Math.abs(size.height - targetHeight) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.height - targetHeight);
                }
            }
        }
        return optimalSize;
    }
	
	static private Camera.Size getOptimalPreviewSize1(List<Camera.Size> sizes, int width, int height)
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

		int mw=100000,mh=100000,mi=-1;
		if (optimalSize == null) {
			//Get the smallest size bigger than requested
			int li=0;
			while (li<sizes.size())
			{
				Camera.Size sz=sizes.get(li);
				if ((sz.width>=width)&&(sz.height>=height)
						&&(sz.width<mw)&&(sz.height<mh)) {
					mw = sz.width;
					mh = sz.height;
					mi = li;
				}
				li++;
			}
			if (mi>=0)
				return sizes.get(mi);
		}

		mw=0;
		mh=0;
		mi=-1;
		if (optimalSize == null) {
			//Last resort get the biggest texture
			int li=0;
			while (li<sizes.size())
			{
				Camera.Size sz=sizes.get(li);
				if ((sz.width>=mw)&&(sz.height>=mh)) {
					mw = sz.width;
					mh = sz.height;
					mi = li;
				}
				li++;
			}
			return sizes.get(mi);
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
		if (!pm.hasSystemFeature(PackageManager.FEATURE_CAMERA))
			return false;
		if (android.os.Build.VERSION.SDK_INT >= 23) {
			Activity activity = sActivity.get();
			if ((activity!=null)&&activity.checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
					activity.requestPermissions(new String[]{Manifest.permission.CAMERA},	0);
					return false;
			}				
		}
		return true;
	}	
}
