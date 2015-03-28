package com.giderosmobile.android.plugins.media;

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

public class GMedia {
	private static WeakReference<Activity> sActivity;
	static long sData = 0;
	private static boolean takeScreenshot = false;
	static final int REQUEST_IMAGE_CAPTURE = 1;
	static final int REQUEST_TAKE_PHOTO = 2;
	static final int SELECT_PICTURE = 3;
	static String returnPath = "";
	static int screenWidth;
	static int screenHeight;
	static double screenDensity;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
		DisplayMetrics metrics = new DisplayMetrics();
		sActivity.get().getWindowManager().getDefaultDisplay().getMetrics(metrics);
		screenWidth = metrics.widthPixels;
		screenHeight = metrics.heightPixels;
		screenDensity = metrics.density;
	}
	
	//on destroy event
	public static void onDestroy()
	{
		cleanup();
	}
	
	public static void onActivityResult(int requestCode, int resultCode, Intent data) {
	    if (requestCode == REQUEST_TAKE_PHOTO && resultCode == Activity.RESULT_OK) {
	    	if(resultCode != Activity.RESULT_OK)
	    	{
	    		onMediaCanceled();
	    		return;
	    	}
	    	onMediaReceived();
	    }
	    else if (requestCode == SELECT_PICTURE && resultCode == Activity.RESULT_OK) {
	    	if(resultCode != Activity.RESULT_OK)
	    	{
	    		onMediaCanceled();
	    		return;
	    	}
	    	Uri selectedImage = data.getData();
			String[] filePathColumn = { MediaStore.Images.Media.DATA };

			Cursor cursor = sActivity.get().getContentResolver().query(selectedImage,
					filePathColumn, null, null, null);
			cursor.moveToFirst();

			int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
			String path = cursor.getString(columnIndex);
			Bitmap source = loadBitmap(path);
			cursor.close();
		    saveBitmap(createImageFile(), source);
			onMediaReceived();
	    }
	}
	
	public static void init(long data){
		sData = data;
	}
	
	public static void cleanup(){
		sData = 0;
	}
	
	public static boolean isCameraAvailable(){
		PackageManager pm = sActivity.get().getPackageManager();
		return pm.hasSystemFeature(PackageManager.FEATURE_CAMERA);
	}
	
	public static void takeScreenshot(){
		takeScreenshot = true;
	}
	
	public static void takePicture() {
	    Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
	    if (takePictureIntent.resolveActivity(sActivity.get().getPackageManager()) != null) {
	        File photoFile  = new File(createImageFile());

	        if (photoFile != null) {
	            takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT,
	                    Uri.fromFile(photoFile));
	            sActivity.get().startActivityForResult(takePictureIntent, REQUEST_TAKE_PHOTO);
	        }
	    }
	}
	
	public static void getPicture(){
		Intent i = new Intent(
				Intent.ACTION_PICK,
				android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
		
		sActivity.get().startActivityForResult(i, SELECT_PICTURE);
	}
	
	public static void savePicture(String path) {
		//copy to public directory
		Bitmap source = loadBitmap(path);
		Environment.getExternalStoragePublicDirectory(
	            Environment.DIRECTORY_PICTURES).mkdirs();
		String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
		String imageFileName = timeStamp + "_gideros.jpg";
	    String destination = Environment.getExternalStoragePublicDirectory(
	            Environment.DIRECTORY_PICTURES).toString() + "/" + imageFileName;
	    saveBitmap(destination, source);
		
	    //let media scanner scan it
	    Intent mediaScanIntent = new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
	    File f = new File(destination);
	    
	    Uri contentUri = Uri.fromFile(f);
	    mediaScanIntent.setData(contentUri);
	    sActivity.get().sendBroadcast(mediaScanIntent);
	}
	
	public static void playVideo(final String path, final boolean force){
		sActivity.get().runOnUiThread(new Runnable() {
			@Override
			 public void run() {
				final FrameLayout frame = (FrameLayout)sActivity.get().getWindow().getDecorView().findViewById(android.R.id.content);
				final GVideoView video = new GVideoView(sActivity.get(), path, force);
				frame.addView(video);
			}
		});
	}
	
	public static void onScreenshotTaken(Bitmap bitmap){
		saveBitmap(createImageFile(), bitmap);
		onMediaReceived();
	}
	
	public static void onDraw(final GL10 gl){
		if(takeScreenshot)
		{
			takeScreenshot = false;
			final Bitmap bitmap = createBitmapFromGLSurface(0, 0, screenWidth, screenHeight, gl);
			new Thread(new Runnable() {
				@Override
				 public void run() {
					onScreenshotTaken(bitmap);
				}
			}).start();
		}
	}
	
	private static void onMediaReceived(){
		if (sData != 0)
			onMediaReceived(returnPath, sData);
	}
	
	private static void onMediaCanceled(){
		if (sData != 0)
			onMediaCanceled(sData);
	}
	
	static void onPlaybackComplete(){
		if (sData != 0)
			onMediaCompleted(sData);
	}
	
	static native void onMediaReceived(String path, long data);
	static native void onMediaCanceled(long data);
	static native void onMediaCompleted(long data);
	
	private static String createImageFile() {
	    // Create an image file name
	    String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
	    String imageFileName = timeStamp + "_gideros.jpg";
	    String storageDir = sActivity.get().getExternalFilesDir(null).toString();
	    returnPath = storageDir + "/" + imageFileName;
	    return returnPath;
	}
	
	private static Bitmap loadBitmap(String path){
		BitmapFactory.Options options = new BitmapFactory.Options();
		options.inPreferredConfig = Bitmap.Config.ARGB_8888;
		return BitmapFactory.decodeFile(path, options);
	}
	
	private static void saveBitmap(String path, Bitmap image){
		try {
            FileOutputStream out = new FileOutputStream(path);
            image.compress(Bitmap.CompressFormat.JPEG, 90, out);
            out.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
	}
	
	private static Bitmap createBitmapFromGLSurface(int x, int y, int w, int h, GL10 gl)
	        throws OutOfMemoryError {
	    int bitmapBuffer[] = new int[w * h];
	    int bitmapSource[] = new int[w * h];
	    IntBuffer intBuffer = IntBuffer.wrap(bitmapBuffer);
	    intBuffer.position(0);

	    try {
	        gl.glReadPixels(x, y, w, h, GL10.GL_RGBA, GL10.GL_UNSIGNED_BYTE, intBuffer);
	        int offset1, offset2;
	        for (int i = 0; i < h; i++) {
	            offset1 = i * w;
	            offset2 = (h - i - 1) * w;
	            for (int j = 0; j < w; j++) {
	                int texturePixel = bitmapBuffer[offset1 + j];
	                int blue = (texturePixel >> 16) & 0xff;
	                int red = (texturePixel << 16) & 0x00ff0000;
	                int pixel = (texturePixel & 0xff00ff00) | red | blue;
	                bitmapSource[offset2 + j] = pixel;
	            }
	        }
	    } catch (GLException e) {
	        return null;
	    }

	    return Bitmap.createBitmap(bitmapSource, w, h, Bitmap.Config.ARGB_8888);
	}
}

class GVideoView extends FrameLayout{
	GVideoView me;
	VideoView video;
	boolean force = false;
	boolean isClosed = false;
	public GVideoView(Context context, String path, boolean shouldForce) {
		super(context);
		force = shouldForce;
		me = this;
		FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
				FrameLayout.LayoutParams.MATCH_PARENT,
				FrameLayout.LayoutParams.MATCH_PARENT);
		setLayoutParams(params);
		setBackgroundColor(0xFF000000);
		video = new VideoView(context);
		addView(video);
		
		video.setZOrderMediaOverlay(true);
		FrameLayout.LayoutParams params2 = new FrameLayout.LayoutParams(
				FrameLayout.LayoutParams.MATCH_PARENT,
				FrameLayout.LayoutParams.MATCH_PARENT);
		params2.gravity = Gravity.CENTER;
		video.setLayoutParams(params2);
		video.setVideoPath(path);
		video.requestFocus();
		video.setOnCompletionListener(new MediaPlayer.OnCompletionListener(){
			@Override
			public void onCompletion(MediaPlayer mp) {
				close();
			}
		});
		video.start();
	}
	
	private void close(){
		if(!isClosed)
		{
			isClosed = true;
			video.stopPlayback();
			((FrameLayout)me.getParent()).removeView(me);
			GMedia.onPlaybackComplete();
		}
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event){
		if(keyCode == KeyEvent.KEYCODE_BACK && !force)
			close();
		return true;
	}
	@Override
	public boolean onTouchEvent(MotionEvent ev){
		if(!force)
			close();
		return true;
	}
	@Override
	public boolean onTrackballEvent(MotionEvent ev){
		return true;
	}
}