package com.giderosmobile.android.plugins.googlelicensing;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;

import com.google.android.vending.expansion.downloader.DownloaderClientMarshaller;

public class GiderosDownloadReceiver extends BroadcastReceiver{

	@Override
	public void onReceive(Context context, Intent intent) {
		try {
			DownloaderClientMarshaller.startDownloadServiceIfRequired(context, intent,
					GiderosAPKDownloader.class);
		} catch (NameNotFoundException e) {
			e.printStackTrace();
		}
	}
	
}