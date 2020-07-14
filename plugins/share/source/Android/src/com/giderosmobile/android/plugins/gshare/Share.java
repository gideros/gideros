package com.giderosmobile.android.plugins.gshare;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.os.Build;
import android.content.Intent;
import java.io.UnsupportedEncodingException;

class Share
{
	private static WeakReference<Activity> sActivity;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
	}
	
	private static int fileNum=0;
	public static boolean share(String mimeType,byte[] data){
		Activity activity=sActivity.get();

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
		    File file = new File(context.getCacheDir()+"share/"+(++fileNum), filename);
		    try {
		        if (file.exists())
		        	file.delete();
	            file.createNewFile();
		        FileOutputStream fos = new FileOutputStream(file);
		        fos.write(bytes);
		        fos.close();
		    } catch (Exception e) {
		        Log.e(TAG, e.getMessage());
		    }
		    Uri contentUri = FileProvider.getUriForFile(context, activity.getPackageName()+".share", file);
			shareIntent.putExtra(Intent.EXTRA_STREAM, contentUri);
		}
		activity.startActivity(Intent.createChooser(shareIntent, null));
		return true;
	}	
}
