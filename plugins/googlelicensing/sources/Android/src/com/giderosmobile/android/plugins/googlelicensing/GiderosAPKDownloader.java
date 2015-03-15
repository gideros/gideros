package com.giderosmobile.android.plugins.googlelicensing;

import com.google.android.vending.expansion.downloader.impl.DownloaderService;

public class GiderosAPKDownloader extends DownloaderService{

	@Override
	public String getPublicKey() {
		return GoogleLVL.KEY;
	}

	@Override
	public byte[] getSALT() {
		return GoogleLVL.SALT;
	}

	@Override
	public String getAlarmReceiverClassName() {
		return GiderosDownloadReceiver.class.getName();
	}
	
}