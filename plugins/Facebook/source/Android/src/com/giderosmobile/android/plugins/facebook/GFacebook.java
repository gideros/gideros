package com.giderosmobile.android.plugins.facebook;

import java.io.File;
import java.io.FileNotFoundException;
import java.lang.ref.WeakReference;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import org.json.JSONObject;

import com.facebook.AppLinkData;
import com.facebook.FacebookException;
import com.facebook.FacebookOperationCanceledException;
import com.facebook.FacebookRequestError;
import com.facebook.HttpMethod;
import com.facebook.Request;
import com.facebook.RequestAsyncTask;
import com.facebook.Response;
import com.facebook.Session;
import com.facebook.model.GraphObject;
import com.facebook.widget.WebDialog;
import com.facebook.widget.WebDialog.OnCompleteListener;
import com.giderosmobile.android.plugins.facebook.fbsimple.Permissions;
import com.giderosmobile.android.plugins.facebook.fbsimple.SimpleFacebook;
import com.giderosmobile.android.plugins.facebook.fbsimple.SimpleFacebookConfiguration;
import com.giderosmobile.android.plugins.facebook.fbsimple.SimpleFacebook.OnReopenSessionListener;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Base64;
import android.view.Window;
import android.view.WindowManager;

public class GFacebook
{
	private static WeakReference<Activity> sActivity;
	public static long sData;
	private static String[] PERMISSIONS = {"basic_info", "user_about_me"};
	private static SimpleFacebook sfb;
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
		sfb = SimpleFacebook.getInstance(sActivity.get());
		
		/**********************
		 * Uncomment and replace "com.yourdomain.yourapp" with your package
		 * to get hash key for FaceBook app
		 **********************/
		//Log.d("FB", getHashKey());
	}
	
	public static void onResume(){
		sfb = SimpleFacebook.getInstance(sActivity.get());
	}
	
	public static void onActivityResult(int requestCode, int resultCode, Intent data){
		sfb.onActivityResult(sActivity.get(), requestCode, resultCode, data); 
	}
	
	static public void init(long data)
	{
		sData = data;
		AppLinkData appLinkData = AppLinkData.createFromActivity(sActivity.get());
	    if (appLinkData != null) {
	        Bundle arguments = appLinkData.getArgumentBundle();
	        if (arguments != null) {
	            String targetUrl = arguments.getString("target_url");
	            if (targetUrl != null && sData != 0) {
	            	GFacebook.onOpenUrl(targetUrl, sData);
	            }
	        }
	    }
	}
	
	static public void cleanup()
	{
		sData = 0;
	}

	static public void login(String appId){
		login(appId, null);
	}
	
	public static void login(String appId, Object[] permissions){
		if(permissions != null)
			PERMISSIONS = (String[])permissions;
		SimpleFacebookConfiguration configuration = new SimpleFacebookConfiguration.Builder()
	    	.setAppId(appId)
	    	.setPermissions(PERMISSIONS)
	    	.build();
		SimpleFacebook.setConfiguration(configuration);
		
		sActivity.get().runOnUiThread(new Runnable() {
			@Override
			 public void run() {
				sfb.login(new LoginCallback());
			}
		});
	}
	
	public static void logout(){
		sActivity.get().runOnUiThread(new Runnable() {
			@Override
			 public void run() {
				sfb.logout(new LogoutCallback());
			}
		});
	}
	
	public static String getAccessToken(){
		if(Session.getActiveSession() != null)
			return Session.getActiveSession().getAccessToken();
		return "";
	}
	
	public static long getExpirationDate(){
		if(Session.getActiveSession() != null)
			return (long)Session.getActiveSession().getExpirationDate().getTime()/1000;
		return 0;
	}
	
	public static void dialog(final String action){
		dialog(action, null);
	}
	
	public static void dialog(final String action, final Object parameters){
		sActivity.get().runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				 if (sfb.isLogin())
	             {
					 WebDialog dialog = (
				        new WebDialog.Builder(sActivity.get(),
				            Session.getActiveSession(), action, (Bundle)parameters))
				            .setOnCompleteListener(new WebDialogCallback(action))
				            .build();
					 Window dialogWindow = dialog.getWindow();
					 dialogWindow.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
					 dialog.show();
	             }
			}
		});
	}
	
	public static void upload(final String path, final String orig){
		sActivity.get().runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				String npath = path;
				if (sfb.isLogin())
				{
					if (SimpleFacebook.getConfiguration().getPublishPermissions().contains(Permissions.PUBLISH_ACTION.getValue()))
					{
						if (!SimpleFacebook.getOpenSessionPermissions().contains(Permissions.PUBLISH_ACTION.getValue()))
						{
							sfb.setReopenSessionListener(new OnReopenSessionListener()
							{
								@Override
								public void onSuccess()
								{
									stageResource(path, orig);
								}
									@Override
								public void onNotAcceptingPermissions()
								{
									// this fail can happen when user doesn't accept the publish permissions
									GFacebook.onRequestError(path, "Does not accept permission", sData);
								}
							});
							// extend publish permissions automatically
							SimpleFacebook.extendPublishPermissions();
						}
					}
					else
					{
						GFacebook.onRequestError(path, "No permission", sData);
						return;
					}
					stageResource(path, orig);
				}
				else
				{
					GFacebook.onRequestError(npath, "Not logged in", sData);
				}
			}
		});
	}
	
	public static void request(String path, int method){
		request(path, method, null);
	}
	
	public static void request(final String path, final int method, final Object parameters){
		sActivity.get().runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				String npath = path;
				if (sfb.isLogin())
				{
					HttpMethod m = convertMethod(method);
					Bundle params =  (Bundle)parameters;
					if(m == HttpMethod.POST)
					{
						if (SimpleFacebook.getConfiguration().getPublishPermissions().contains(Permissions.PUBLISH_ACTION.getValue()))
						{
							if (!SimpleFacebook.getOpenSessionPermissions().contains(Permissions.PUBLISH_ACTION.getValue()))
							{
								sfb.setReopenSessionListener(new OnReopenSessionListener()
								{
									@Override
									public void onSuccess()
									{
										request(path, method, parameters);
									}

									@Override
									public void onNotAcceptingPermissions()
									{
										Session.getActiveSession().refreshPermissions();
										// this fail can happen when user doesn't accept the publish permissions
										GFacebook.onRequestError(path, "Does not accept permission", sData);
									}
								});

								// extend publish permissions automatically
								SimpleFacebook.extendPublishPermissions();
							}
						}
						else
						{
							Session.getActiveSession().refreshPermissions();
							GFacebook.onRequestError(path, "No permission", sData);
							return;
						}
			
						if(params != null && params.containsKey("path")){
							Bitmap photo = loadBitmap(params.getString("path"));
							params.remove("path");
							params.putParcelable("picture", photo);
							if(params.containsKey("album")){
								String album = params.getString("album");
								params.remove("album");
								npath = album + "/photos";
							}
						}
					}
					graphRequest(npath, params, m);
				}
				else
				{
					GFacebook.onRequestError(npath, "Not logged in", sData);
				}
			}
		});
	}
	
	public static native void onLoginComplete(long data);
	public static native void onLoginError(String error, long data);
	public static native void onLogoutComplete(long data);
	public static native void onLogoutError(String error, long data);
	public static native void onOpenUrl(String url, long data);
	public static native void onDialogComplete(String type, String response, long data);
	public static native void onDialogError(String type, String errorDescription, long data);
	public static native void onRequestComplete(String type, String response, long data);
	public static native void onRequestError(String type, String errorDescription, long data);

	private static void graphRequest(final String graphPath, final Bundle paramaters, final HttpMethod method){
		Session session = SimpleFacebook.getOpenSession();
		Request request = new Request(session, graphPath, paramaters, method, new RequestCallback(graphPath));
		RequestAsyncTask task = new RequestAsyncTask(request);
		task.execute();
	}
	
	public static void stageResource(String path, String orig){
		Session session = SimpleFacebook.getOpenSession();
		File file = new File(path);
		Request request;
		try {
			request = Request.newUploadStagingResourceWithImageRequest(session, file,  new RequestCallback(orig));
			RequestAsyncTask task = new RequestAsyncTask(request);
			task.execute();
		} catch (FileNotFoundException e) {
			GFacebook.onRequestError(orig, "File not found", sData);
			e.printStackTrace();
		}
	}
	
	private static Bitmap loadBitmap(String path){
		BitmapFactory.Options options = new BitmapFactory.Options();
		options.inPreferredConfig = Bitmap.Config.ARGB_8888;
		return BitmapFactory.decodeFile(path, options);
	}
	
	private static HttpMethod convertMethod(int method){
		HttpMethod realMethod = HttpMethod.GET;
		if(method == 1)
			realMethod = HttpMethod.POST;
		else if(method == 2)
			realMethod = HttpMethod.DELETE;
		return realMethod;
	}
	
	private static String getHashKey()
	{
		// Add code to print out the key hash
		try
		{
			PackageInfo info = sActivity.get().getPackageManager().getPackageInfo(sActivity.get().getPackageName(), PackageManager.GET_SIGNATURES);
			for (Signature signature: info.signatures)
			{
				MessageDigest md = MessageDigest.getInstance("SHA");
				md.update(signature.toByteArray());
				return Base64.encodeToString(md.digest(), Base64.DEFAULT);
			}
		}
		catch (NameNotFoundException e)
		{

		}
		catch (NoSuchAlgorithmException e)
		{

		}
		return null;
	}
}

class LoginCallback implements SimpleFacebook.OnLoginListener{

	@Override
	public void onThinking() {}

	@Override
	public void onException(Throwable throwable) {
		GFacebook.onLoginError(throwable.getLocalizedMessage(), GFacebook.sData);
	}

	@Override
	public void onFail(String reason) {
		GFacebook.onLoginError(reason, GFacebook.sData);
	}

	@Override
	public void onLogin() {
		Session.getActiveSession().refreshPermissions();
		GFacebook.onLoginComplete(GFacebook.sData);
	}

	@Override
	public void onNotAcceptingPermissions() {
		GFacebook.onLoginError("User didn't accept read permissions", GFacebook.sData);
	}
	
}

class LogoutCallback implements SimpleFacebook.OnLogoutListener{

	@Override
	public void onThinking() {}

	@Override
	public void onException(Throwable throwable) {
		GFacebook.onLogoutError(throwable.getLocalizedMessage(), GFacebook.sData);
	}

	@Override
	public void onFail(String reason) {
		GFacebook.onLogoutError(reason, GFacebook.sData);
	}

	@Override
	public void onLogout() {
		GFacebook.onLogoutComplete(GFacebook.sData);
	}
	
}

class RequestCallback implements Request.Callback {
	private String path;
	
	RequestCallback(String graphPath){
		path = graphPath;
	}
    public void onCompleted(Response response) {
    	String result = "[]";
    	GraphObject graphObject = response.getGraphObject();
    	if(graphObject != null)
    	{
    		JSONObject graphResponse = graphObject.getInnerJSONObject();
    		result = graphResponse.toString();
    	}
        FacebookRequestError error = response.getError();
        if (error != null) {
            GFacebook.onRequestError(path, error.getErrorMessage(), GFacebook.sData);
        } else {
        	GFacebook.onRequestComplete(path, result, GFacebook.sData);
        }
    }
};

class WebDialogCallback implements OnCompleteListener {

	String action;
	WebDialogCallback(String type){
		action = type;
	}
	@Override
	public void onComplete(Bundle values, FacebookException error) {
		if (error != null) {
            if (error instanceof FacebookOperationCanceledException) {
                GFacebook.onDialogError(action, "Canceled", GFacebook.sData);
            } else {
            	GFacebook.onDialogError(action, error.getLocalizedMessage(), GFacebook.sData);
            }
        } else {
            if (values.containsKey("post_id")) {
                GFacebook.onDialogComplete(action, values.getString("post_id"), GFacebook.sData);
            }
            else if(values.containsKey("request")){
            	GFacebook.onDialogComplete(action, values.getString("request"), GFacebook.sData);
            } else {
            	GFacebook.onDialogError(action, "Canceled", GFacebook.sData);
            }
        }   
    }
				
}
