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
	
	public static boolean share(String mimeType,byte[] data){

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
/*		    File file = new File(context.getCacheDir(), filename);
		    Uri contentUri = FileProvider.getUriForFile(context, "com.package.example", file);
			shareIntent.putExtra(Intent.EXTRA_STREAM, contentUri);*/
			return false;
		}
		Activity activity=sActivity.get();
		activity.startActivity(Intent.createChooser(shareIntent, null));
		return true;
	}	
}
