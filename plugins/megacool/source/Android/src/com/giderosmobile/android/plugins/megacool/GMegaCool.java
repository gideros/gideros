package com.giderosmobile.android.plugins.megacool;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.os.Build;

class GMegaCool
{
	private static WeakReference<Activity> sActivity;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
	}
	
	public static boolean share(String mimeType,byte[] data){
	    File file = new File(context.getCacheDir(), filename);
	    Uri contentUri = FileProvider.getUriForFile(context, "com.package.example", file);

		Intent shareIntent = new Intent();
		shareIntent.setAction(Intent.ACTION_SEND);
		shareIntent.putExtra(Intent.EXTRA_STREAM, contentUri);
		shareIntent.setType(mimeType);
		startActivity(Intent.createChooser(shareIntent, getResources().getText(R.string.send_to)));
		return true;
	}
	
	
}