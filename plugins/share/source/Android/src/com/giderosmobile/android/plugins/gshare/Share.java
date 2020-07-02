package com.giderosmobile.android.plugins.gshare;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.os.Build;

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
		if (mimeType.startsWith("text/"))
			sendIntent.putExtra(Intent.EXTRA_TEXT, "This is my text to send.");
		else {
/*		    File file = new File(context.getCacheDir(), filename);
		    Uri contentUri = FileProvider.getUriForFile(context, "com.package.example", file);
			shareIntent.putExtra(Intent.EXTRA_STREAM, contentUri);*/
			return false;
		}
		startActivity(Intent.createChooser(shareIntent, getResources().getText(R.string.send_to)));
		return true;
	}
	
	
}