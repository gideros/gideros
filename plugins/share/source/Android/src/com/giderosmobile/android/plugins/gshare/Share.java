package com.giderosmobile.android.plugins.gshare;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import java.util.Map;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.provider.OpenableColumns;
import android.util.Log;

import androidx.core.content.FileProvider;

class Share
{
	private static final int FILESHARE_IMPORT_CODE = 4701;
	private static final int FILESHARE_EXPORT_CODE = 4702;
	private static long sData;
	private static boolean sActive = false;
	private static byte[] sWriteData=null;
	private static Activity mActivity;

	private static final int STS_OK=1;
	private static final int STS_CANCELED=0;
	private static final int STS_GENERIC_ERR=-1;
	private static final int STS_FILE_NOT_FOUND=-2;
	private static final int STS_BUSY=-3;
	
	public static void onCreate(Activity activity)
	{
		mActivity=activity;
	}
	
	public static void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (requestCode==FILESHARE_IMPORT_CODE) {
			int sts=STS_GENERIC_ERR;
			byte[] res=null;
			String file=null;
			String mime=null;
			switch (resultCode) {
				case Activity.RESULT_CANCELED: sts=STS_CANCELED; break; //CANCELED
				case Activity.RESULT_OK: sts=STS_OK; break;
			}
			if (sts==STS_OK) {
				Uri uri = null;
				if (data != null) {
					uri = data.getData();
					mime = mActivity.getContentResolver().getType(uri);

					Cursor cursor = mActivity.getContentResolver()
							.query(uri, null, null, null, null, null);

					try {
						// moveToFirst() returns false if the cursor has 0 rows. Very handy for
						// "if there's anything to look at, look at it" conditionals.
						if (cursor != null && cursor.moveToFirst()) {
							int didx=cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
							if (didx>=0)
								file= cursor.getString(didx);
						}
					} finally {
						cursor.close();
					}

					try (InputStream inputStream =
								 mActivity.getContentResolver().openInputStream(uri)) {
						ByteArrayOutputStream buffer = new ByteArrayOutputStream();

						int nRead;
						byte[] bdata = new byte[16384];

						while ((nRead = inputStream.read(bdata, 0, bdata.length)) != -1) {
							buffer.write(bdata, 0, nRead);
						}
						res=buffer.toByteArray();
					} catch (FileNotFoundException e) {
						sts=STS_FILE_NOT_FOUND;
					} catch (IOException e) {
						sts=STS_GENERIC_ERR;
					}
				}
			}
			sActive=false;
			onImportResult(sts,file,mime, res);
		}
		if (requestCode==FILESHARE_EXPORT_CODE) {
			int sts=STS_GENERIC_ERR;
			switch (resultCode) {
				case Activity.RESULT_CANCELED: sts=STS_CANCELED; break; //CANCELED
				case Activity.RESULT_OK: sts=STS_OK; break;
			}
			if (sts==STS_OK) {
				Uri uri = null;
				if (data != null) {
					uri = data.getData();

					try (OutputStream outputStream =
								 mActivity.getContentResolver().openOutputStream(uri)) {
						outputStream.write(sWriteData);
					} catch (FileNotFoundException e) {
						sts=STS_FILE_NOT_FOUND;
					} catch (IOException e) {
						sts=STS_GENERIC_ERR;
					}
				}
			}
			sActive=false;
			sWriteData=null;
			onExportResult(sts);
		}
	}

	synchronized public static void Init() {
	}

	synchronized public static void Cleanup() {
	}

	synchronized public static boolean Import(String mime,String extension) {
		if (sActive) {
			onImportResult(STS_BUSY,null,null,null);
			return true;
		}
		sActive=true;
		Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
		intent.addCategory(Intent.CATEGORY_OPENABLE);
		intent.setType(mime);

		mActivity.startActivityForResult(intent, FILESHARE_IMPORT_CODE);
		return true;
	}
	synchronized public static boolean Export(byte[] data,String mime,String filename) {
		if (sActive) {
			onExportResult(STS_BUSY);
			return true;
		}
		sActive=true;
		sWriteData=data;
		Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
		intent.addCategory(Intent.CATEGORY_OPENABLE);
		intent.setType(mime);
		intent.putExtra(Intent.EXTRA_TITLE, filename);

		mActivity.startActivityForResult(intent, FILESHARE_EXPORT_CODE);
		return true;
	}

	native private static void onImportResult(int status,String file,String mime,byte[] data);
	native private static void onExportResult(int status);

	private static int fileNum=0;
	public static boolean share(Map<String,byte[]> map){
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
			File dir= new File(mActivity.getCacheDir(),"share");
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
		    Uri contentUri = FileProvider.getUriForFile(mActivity, mActivity.getPackageName()+".share", file);
			shareIntent.putExtra(Intent.EXTRA_STREAM, contentUri);
		}
		mActivity.startActivity(Intent.createChooser(shareIntent, null));
		return true;
	}	
}
