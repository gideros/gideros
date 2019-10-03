package com.giderosmobile.android.plugins.media;

import android.Manifest;
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
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.widget.FrameLayout;
import android.widget.Toast;
import android.widget.VideoView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.nio.IntBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;

import javax.microedition.khronos.opengles.GL10;

public class GMedia {
    private static final int PERMISSION_ACCESS_CAMERA = 1;
    private static final int REQUEST_TAKE_PICTURE = 2;
    private static final int REQUEST_SELECT_PICTURE = 3;
    private static final int PERMISSION_READ_FOR_SELECT_PICTURE = 4;
    private static final int PERMISSION_WRITE_FOR_SCREENSHOT = 5;
    private static final int PERMISSION_WRITE_FOR_SAVE_PICTURE = 6;
    private static WeakReference<Activity> sActivity;
    private static long sData = 0;
    private static boolean takeScreenshot = false;
    private static String xpath = "";
    private static String returnPath = "";
    private static int screenWidth;
    private static int screenHeight;

    public static void onCreate(Activity activity) {
        sActivity = new WeakReference<>(activity);
        DisplayMetrics metrics = new DisplayMetrics();
        sActivity.get().getWindowManager().getDefaultDisplay().getMetrics(metrics);
        screenWidth = metrics.widthPixels;
        screenHeight = metrics.heightPixels;
    }

    public static void init(long data) {
        sData = data;
    }

    // functions in gideros
    public static boolean isCameraAvailable() { // OK
        PackageManager pm = sActivity.get().getPackageManager();
        return pm.hasSystemFeature(PackageManager.FEATURE_CAMERA);
    }

    public static void takePicture() { // OK BUT PHOTO THUMBNAIL ONLY!
        if (Build.VERSION.SDK_INT >= 23 && !isPermissionCamera()) {
            requestPermissionCamera();
        } else {
            takePictureFunction();
        }
    }

    public static void getPicture() { // OK
        if (Build.VERSION.SDK_INT >= 23 && !isPermissionWriteES()) {
            requestPermissionWES(PERMISSION_READ_FOR_SELECT_PICTURE);
        } else {
            getPictureFunction();
        }
    }

    public static void savePicture(String path) { // OK
        xpath = path;
        if (Build.VERSION.SDK_INT >= 23 && !isPermissionWriteES()) {
            requestPermissionWES(PERMISSION_WRITE_FOR_SAVE_PICTURE);
        } else {
            savePictureFunction();
        }
    }

    public static void takeScreenshot() { // OK
        if (Build.VERSION.SDK_INT >= 23 && !isPermissionWriteES()) {
            requestPermissionWES(PERMISSION_WRITE_FOR_SCREENSHOT);
        } else {
            showToast("in progress...");
            takeScreenshot = true;
        }
    }

    public static void playVideo(final String path, final boolean force) { // OK
        sActivity.get().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final FrameLayout frame = sActivity.get().getWindow().getDecorView().findViewById(android.R.id.content);
                final GVideoView video = new GVideoView(sActivity.get(), path, force);
                frame.addView(video);
            }
        });
    }

    // permissions granted functions
    public static void takePictureFunction() {
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        if (takePictureIntent.resolveActivity(sActivity.get().getPackageManager()) != null) {
            // used to crash. Passing a file:// URI across a package boundary causes a FileUriExposedException. cf: https://developer.android.com/training/camera/photobasics
//				takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT,
//						Uri.fromFile(photoFile));
//				sActivity.get().startActivityForResult(takePictureIntent, REQUEST_TAKE_PICTURE);

            // new version (only takes thumbnail but at least doesn't crash)
            sActivity.get().startActivityForResult(takePictureIntent, REQUEST_TAKE_PICTURE);
        }
    }

    public static void getPictureFunction() {
        Intent i = new Intent(
                Intent.ACTION_PICK,
                android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        sActivity.get().startActivityForResult(i, REQUEST_SELECT_PICTURE);
    }

    public static void savePictureFunction() {
        // create the bitmap
        Bitmap source = loadBitmap(xpath);
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        // save to public directory (SD/Pictures)
        String storageDir = Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_PICTURES).toString() + "/GIDEROS"; // change name here
        String imageFileName = "img_" + timeStamp + ".png"; // change name here
        // create directory
        File fileimg = new File(storageDir, imageFileName);
        fileimg.getParentFile().mkdirs();
        // then save to disk
        showToast("saving...");
        returnPath = storageDir + "/" + imageFileName;
        savePngBitmap(returnPath, source);
        mediaScanner(returnPath);
        onMediaReceived();
    }

    // check and ask for permissions
    private static boolean isPermissionCamera() {
        int result = ContextCompat.checkSelfPermission(sActivity.get(), Manifest.permission.CAMERA);
        return result == PackageManager.PERMISSION_GRANTED;
    }

    private static void requestPermissionCamera() {
        if (ActivityCompat.shouldShowRequestPermissionRationale(sActivity.get(), Manifest.permission.CAMERA)) {
            ActivityCompat.requestPermissions(sActivity.get(), new String[]{Manifest.permission.CAMERA},
                    PERMISSION_ACCESS_CAMERA);
        } else {
            ActivityCompat.requestPermissions(sActivity.get(), new String[]{Manifest.permission.CAMERA},
                    PERMISSION_ACCESS_CAMERA);
        }
    }

    private static boolean isPermissionWriteES() {
        int result = ContextCompat.checkSelfPermission(sActivity.get(), Manifest.permission.WRITE_EXTERNAL_STORAGE);
        return result == PackageManager.PERMISSION_GRANTED;
    }

    private static void requestPermissionWES(int whatfor) {
        if (ActivityCompat.shouldShowRequestPermissionRationale(sActivity.get(), Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            ActivityCompat.requestPermissions(sActivity.get(), new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, whatfor);
        } else {
            ActivityCompat.requestPermissions(sActivity.get(), new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, whatfor);
        }
    }

    // results functions
    public static void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_TAKE_PICTURE && resultCode == Activity.RESULT_OK) {
            if (resultCode != Activity.RESULT_OK) {
                onMediaCanceled();
                return;
            }
            // take photo (I couldn't get the full photo only the thumbnail!)
            Bundle extras = data.getExtras();
            Bitmap source = (Bitmap) extras.get("data"); // photo thumbnail
            String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
            // gideros in app folder. Deleted when user uninstalls the app.
            String storageDir = sActivity.get().getExternalFilesDir(null).toString();
            String imageFileName = "photo_" + timeStamp + ".jpg";
            returnPath = storageDir + "/" + imageFileName;
            saveBitmap(returnPath, source);
            mediaScanner(returnPath);
            onMediaReceived();
        } else if (requestCode == REQUEST_SELECT_PICTURE && resultCode == Activity.RESULT_OK) {
            if (resultCode != Activity.RESULT_OK) {
                onMediaCanceled();
                return;
            }
            Uri selectedImage = data.getData();
            String[] filePathColumn = {MediaStore.Images.Media.DATA};
            Cursor cursor = sActivity.get().getContentResolver().query(selectedImage,
                    filePathColumn, null, null, null);
            cursor.moveToFirst();

            int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
            String path = cursor.getString(columnIndex);
            Bitmap source = loadBitmap(path);
            cursor.close();

            String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
            // gideros in app folder. Deleted when user uninstalls the app.
            String storageDir = sActivity.get().getExternalFilesDir(null).toString();
            String imageFileName = "select_" + timeStamp + ".png"; // better jpeg?
            returnPath = storageDir + "/" + imageFileName;
            savePngBitmap(returnPath, source);
            mediaScanner(returnPath);
            onMediaReceived();
        }
    }

    public static void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode) {
            // If request is cancelled, the result arrays are empty.
            case GMedia.PERMISSION_ACCESS_CAMERA: {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted
                    GMedia.takePictureFunction();
                } else {
                    // permission was denied
                    showToast("access camera not granted!");
                }
                break;
            }
            case GMedia.PERMISSION_READ_FOR_SELECT_PICTURE: {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted
                    GMedia.getPictureFunction();
                } else {
                    // permission was denied
                    showToast("read for select picture not granted!");
                }
                break;
            }
            case GMedia.PERMISSION_WRITE_FOR_SAVE_PICTURE: {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted
                    GMedia.savePictureFunction();
                } else {
                    // permission was denied
                    showToast("write for save picture not granted!");
                }
                break;
            }
            case GMedia.PERMISSION_WRITE_FOR_SCREENSHOT: {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted
                    showToast("in progress...");
                    GMedia.takeScreenshot = true;
                } else {
                    // permission was denied
                    showToast("write for screenshot not granted!");
                }
                break;
            }
            default: {
                showToast("DEFAULT");
                break;
            }
        }
    }

    public static void onDraw(final GL10 gl) {
        if (takeScreenshot) {
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

    private static Bitmap createBitmapFromGLSurface(int x, int y, int w, int h, GL10 gl)
            throws OutOfMemoryError {
        int[] bitmapBuffer = new int[w * h];
        int[] bitmapSource = new int[w * h];
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

    private static void onScreenshotTaken(Bitmap bitmap) {
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        String storageDir = Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_PICTURES).toString();
        String imageFileName = "capt_" + timeStamp + ".jpg"; // can change name here!
        returnPath = storageDir + "/" + imageFileName;
        saveBitmap(returnPath, bitmap);
        mediaScanner(returnPath);
        onMediaReceived();
    }

    private static void mediaScanner(String destination) {
        //let media scanner scan it
        Intent mediaScanIntent = new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
        File f = new File(destination);
        Uri contentUri = Uri.fromFile(f);
        mediaScanIntent.setData(contentUri);
        sActivity.get().sendBroadcast(mediaScanIntent);
    }

    private static void showToast(final String msg) {
        Thread thread = new Thread() {
            public void run() {
                sActivity.get().runOnUiThread(new Runnable() {
                    public void run() {
                        Toast.makeText(sActivity.get().getApplicationContext(), msg, Toast.LENGTH_SHORT).show();
                    }
                });
            }
        };
        thread.start();
    }

    private static Bitmap loadBitmap(String path) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeFile(path, options);
    }

    private static void saveBitmap(String path, Bitmap image) {
        try {
            FileOutputStream out = new FileOutputStream(path);
            image.compress(Bitmap.CompressFormat.JPEG, 90, out);
            out.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void savePngBitmap(String path, Bitmap image) {
        try {
            FileOutputStream out = new FileOutputStream(path);
            image.compress(Bitmap.CompressFormat.PNG, 100, out);
            out.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // callback listeners
    private static void onMediaReceived() {
        if (sData != 0)
            onMediaReceived(returnPath, sData);
    }

    private static void onMediaCanceled() {
        if (sData != 0)
            onMediaCanceled(sData);
    }

    static void onPlaybackComplete() {
        if (sData != 0)
            onMediaCompleted(sData);
    }

    static native void onMediaReceived(String path, long data);

    static native void onMediaCanceled(long data);

    static native void onMediaCompleted(long data);

    //on destroy event
    public static void onDestroy() {
        cleanup();
    }

    private static void cleanup() {
        sData = 0;
    }
}

// *** CLASS GVideoView *** //
class GVideoView extends FrameLayout {
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

//		video.setVideoPath(path);
//		video.setVideoPath(context.getAssets().open("videos/test.mp4"));
        // hack copy the gideros video to readable storage
        File f = new File(context.getCacheDir() + "/" + path);
        if (!f.exists()) try {
            String xpath = context.getCacheDir() + "/";
            InputStream input = context.getAssets().open("assets/" + path);
            f.getParentFile().mkdirs();
            int size = input.available();
            FileOutputStream output = new FileOutputStream(new File(xpath + path));
            byte data[] = new byte[4096];
            long total = 0;
            int count;
            while ((count = input.read(data)) != -1) {
                output.write(data, 0, count);
                total += count;
                if (size <= total) {
                    break;
                }
            }
            output.flush();
            output.close();
            input.close();
        } catch (Exception e) {
            Toast.makeText(context, "video not created!", Toast.LENGTH_SHORT).show();
        }

        Uri uri = Uri.parse(f.getPath());
        video.setVideoURI(uri);
        video.requestFocus();
        video.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mp) {
                close();
            }
        });
        video.start();
    }

    private void close() {
        if (!isClosed) {
            isClosed = true;
            video.stopPlayback();
            ((FrameLayout) me.getParent()).removeView(me);
            GMedia.onPlaybackComplete();
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && !force)
            close();
        return true;
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        if (!force)
            close();
        return true;
    }

    @Override
    public boolean onTrackballEvent(MotionEvent ev) {
        return true;
    }
}
