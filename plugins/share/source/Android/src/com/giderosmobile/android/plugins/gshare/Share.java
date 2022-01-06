package com.giderosmobile.android.plugins.gshare;

import java.io.File;
import java.io.FileOutputStream;
import java.lang.ref.WeakReference;
import java.util.Map;

import android.app.Activity;
import android.net.Uri;
import android.os.Build;
import android.content.Intent;
import android.util.Log;

import androidx.core.content.FileProvider;

import java.io.UnsupportedEncodingException;

class Share
{
	private static WeakReference<Activity> sActivity;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
	}
	
	private static int fileNum=0;
	public static boolean share(Map<String,byte[]> map){
		Activity activity=sActivity.get();
		if (map.isEmpty()) return true;
		String mimeType=map.keySet().iterator().next();
		byte[] data=map.get(mimeType);
		
		Intent shareIntent = new Intent();
		shareIntent.setAction(Intent.ACTION_SEND);
		shareIntent.setType(mimeType);
		if (mimeType.startsWith("text/")) {
		   try {
			shareIntent.putExtra(Intent.EXTRA_TEXT, new String(data,"UTF-8"));
		   } catch (UnsupportedEncodingException e) {
		        return false;
		   }
		}
		else {
			String ext=".bin";
			if (mimeType.equals("image/png")) ext=".png";
			else if (mimeType.equals("image/jpeg")) ext=".jpg";
			File dir= new File(activity.getCacheDir(),"share");
			dir.mkdirs();
		    File file = new File(dir, Integer.toString(++fileNum)+ext);
		    try {
		        if (file.exists())
		        	file.delete();
	            file.createNewFile();
		        FileOutputStream fos = new FileOutputStream(file);
		        fos.write(data);
		        fos.close();
		    } catch (Exception e) {
		        Log.e("GiderosShare", e.getMessage());
		    }
		    Uri contentUri = FileProvider.getUriForFile(activity, activity.getPackageName()+".share", file);
			shareIntent.putExtra(Intent.EXTRA_STREAM, contentUri);
		}
		activity.startActivity(Intent.createChooser(shareIntent, null));
		return true;
	}	
}
