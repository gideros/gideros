package com.giderosmobile.android.plugins.facebook.fbsimple;

import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import com.facebook.FacebookOperationCanceledException;
import com.facebook.Session;
import com.facebook.SessionState;
import com.giderosmobile.android.plugins.facebook.fbsimple.utils.Errors;
import com.giderosmobile.android.plugins.facebook.fbsimple.utils.Errors.ErrorMsg;
import com.giderosmobile.android.plugins.facebook.fbsimple.utils.Logger;


/**
 * Simple Facebook SDK which wraps original Facebook SDK 3.5
 * 
 * <br>
 * <br>
 * <b>Features:</b>
 * <ul>
 * <li>Simple configuration</li>
 * <li>No need to use LoginButton view</li>
 * <li>Login/logout</li>
 * <li>Publish feed</li>
 * <li>Publish open graph story</li>
 * <li>Publish photo</li>
 * <li>Invite friends</li>
 * <li>Fetch my profile</li>
 * <li>Fetch friends</li>
 * <li>Fetch albums</li>
 * <li>Predefined all possible permissions. See {@link Permissions}</li>
 * <li>No need to care for correct sequence logging with READ and PUBLISH permissions</li>
 * </ul>
 * 
 * @author sromku
 */
public class SimpleFacebook
{
	private static SimpleFacebook mInstance = null;
	private static SimpleFacebookConfiguration mConfiguration = new SimpleFacebookConfiguration.Builder().build();

	private static Activity mActivity;
	private SessionStatusCallback mSessionStatusCallback = null;

	private SimpleFacebook()
	{
		mSessionStatusCallback = new SessionStatusCallback();
	}

	public static void initialize(Activity activity)
	{
		if (mInstance == null)
		{
			mInstance = new SimpleFacebook();
		}

		mActivity = activity;
	}

	public static SimpleFacebook getInstance(Activity activity)
	{
		if (mInstance == null)
		{
			mInstance = new SimpleFacebook();
		}

		mActivity = activity;
		return mInstance;
	}

	public static SimpleFacebook getInstance()
	{
		return mInstance;
	}

	/**
	 * Set facebook configuration
	 * 
	 * @param facebookToolsConfiguration
	 */
	public static void setConfiguration(SimpleFacebookConfiguration facebookToolsConfiguration)
	{
		mConfiguration = facebookToolsConfiguration;
	}
	
	public static SimpleFacebookConfiguration getConfiguration(){
		return mConfiguration;
	}
	
	public void setReopenSessionListener(OnReopenSessionListener listener){
		mSessionStatusCallback.mOnReopenSessionListener = listener;
	}

	/**
	 * Login to Facebook
	 * 
	 * @param onLoginListener
	 */
	public void login(OnLoginListener onLoginListener)
	{
		if (isLogin())
		{
			// log
			logInfo("You were already logged in before calling 'login()' method");

			if (onLoginListener != null)
			{
				onLoginListener.onLogin();
			}
		}
		else
		{
			Session session = Session.getActiveSession();
			if (session == null || session.getState().isClosed())
			{
				session = new Session.Builder(mActivity.getApplicationContext())
					.setApplicationId(mConfiguration.getAppId())
					.build();
				Session.setActiveSession(session);
			}

			mSessionStatusCallback.mOnLoginListener = onLoginListener;
			session.addCallback(mSessionStatusCallback);

			/*
			 * If session is not opened, then open it
			 */
			if (!session.isOpened())
			{
				openSession(session, true);
			}
			else
			{
				if (onLoginListener != null)
				{
					onLoginListener.onLogin();
				}
			}
		}
	}

	/**
	 * Logout from Facebook
	 */
	public void logout(OnLogoutListener onLogoutListener)
	{
		if (isLogin())
		{
			Session session = Session.getActiveSession();
			if (session != null && !session.isClosed())
			{
				mSessionStatusCallback.mOnLogoutListener = onLogoutListener;
				session.closeAndClearTokenInformation();
				session.removeCallback(mSessionStatusCallback);

				if (onLogoutListener != null)
				{
					onLogoutListener.onLogout();
				}
			}
		}
		else
		{
			// log
			logInfo("You were already logged out before calling 'logout()' method");

			if (onLogoutListener != null)
			{
				onLogoutListener.onLogout();
			}
		}
	}

	

	/**
	 * 
	 * Requests {@link Permissions#PUBLISH_ACTION} and nothing else. Useful when you just want to request the
	 * action and won't be publishing at the time, but still need the updated <b>access token</b> with the
	 * permissions (possibly to pass back to your backend). You must add {@link Permissions#PUBLISH_ACTION} to
	 * your SimpleFacebook configuration before calling this.
	 * 
	 * <b>Must be logged to use.</b>
	 * 
	 * @param onPermissionListener The listener for the request permission action
	 */
	public void requestPublish(final OnPermissionListener onPermissionListener)
	{
		if (isLogin())
		{
			if (mConfiguration.getPublishPermissions().contains(Permissions.PUBLISH_ACTION.getValue()))
			{
				if (onPermissionListener != null)
				{
					onPermissionListener.onThinking();
				}
				/*
				 * Check if session to facebook has 'publish_action' permission. If not, we will ask user for
				 * this permission.
				 */
				if (!getOpenSessionPermissions().contains(Permissions.PUBLISH_ACTION.getValue()))
				{
					mSessionStatusCallback.mOnReopenSessionListener = new OnReopenSessionListener()
					{
						@Override
						public void onSuccess()
						{
							if (onPermissionListener != null)
							{
								onPermissionListener.onSuccess(getAccessToken());
							}
						}

						@Override
						public void onNotAcceptingPermissions()
						{
							// this fail can happen when user doesn't accept the publish permissions
							String reason = Errors.getError(ErrorMsg.CANCEL_PERMISSIONS_PUBLISH, String.valueOf(mConfiguration.getPublishPermissions()));
							logError(reason, null);
							if (onPermissionListener != null)
							{
								onPermissionListener.onFail(reason);
							}
						}
					};
					// extend publish permissions automatically
					extendPublishPermissions();
				}
				else
				{
					// We already have the permission.
					if (onPermissionListener != null)
					{
						onPermissionListener.onSuccess(getAccessToken());
					}
				}
			}
		}
		else
		{
			// callback with 'fail' due to not being loged
			if (onPermissionListener != null)
			{
				String reason = Errors.getError(ErrorMsg.LOGIN);
				logError(reason, null);
				onPermissionListener.onFail(reason);
			}
		}
	}

	/**
	 * Call this inside your activity in {@link Activity#onActivityResult} method
	 * 
	 * @param activity
	 * @param requestCode
	 * @param resultCode
	 * @param data
	 * @return
	 */
	public boolean onActivityResult(Activity activity, int requestCode, int resultCode, Intent data)
	{
		if (Session.getActiveSession() != null)
		{
			return Session.getActiveSession().onActivityResult(activity, requestCode, resultCode, data);
		}
		else
		{
			return false;
		}
	}

	/**
	 * Indicate if you are logged in or not.
	 * 
	 * @return <code>True</code> if you is logged in, otherwise return <code>False</code>
	 */
	public boolean isLogin()
	{
		Session session = Session.getActiveSession();
		if (session == null)
		{
			if (mActivity == null)
			{
				// You can't create a session if the activity/context hasn't been initialized
				// This is now possible because the library can be started without context.
				return false;
			}
			session = new Session.Builder(mActivity.getApplicationContext())
				.setApplicationId(mConfiguration.getAppId())
				.build();
			Session.setActiveSession(session);
		}
		if (session.isOpened())
		{
			// mSessionNeedsToReopen = false;
			return true;
		}

		/*
		 * Check if we can reload the session when it will be neccesary. We won't do it now.
		 */
		if (session.getState().equals(SessionState.CREATED_TOKEN_LOADED))
		{
			List<String> permissions = session.getPermissions();
			if (permissions.containsAll(mConfiguration.getReadPermissions()))
			{
				reopenSession();
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	/**
	 * Get access token of open session
	 * 
	 * @return a {@link String} containing the Access Token of the current {@link Session} or null if no
	 *         session.
	 */
	public String getAccessToken()
	{
		Session session = getOpenSession();
		if (session != null)
		{
			return session.getAccessToken();
		}
		return null;
	}

	/**
	 * Get open session
	 * 
	 * @return the open session
	 */
	public static Session getOpenSession()
	{
		return Session.getActiveSession();
	}

	/**
	 * Clean all references like Activity to prevent memory leaks
	 */
	public void clean()
	{
		mActivity = null;
	}

	/**
	 * Fetch invited friends from response bundle
	 * 
	 * @param values
	 * @return list of invited friends
	 */
	@SuppressLint("DefaultLocale")
	private static List<String> fetchInvitedFriends(Bundle values)
	{
		List<String> friends = new ArrayList<String>();

		int size = values.size();
		int numOfFriends = size - 1;
		if (numOfFriends > 0)
		{
			for (int i = 0; i < numOfFriends; i++)
			{
				String key = String.format("to[%d]", i);
				String friendId = values.getString(key);
				if (friendId != null)
				{
					friends.add(friendId);
				}
			}
		}

		return friends;
	}

	/**
	 * Get permissions that are accepted by user for current token
	 * 
	 * @return the list of accepted permissions
	 */
	public static List<String> getOpenSessionPermissions()
	{
		return getOpenSession().getPermissions();
	}

	private void openSession(Session session, boolean isRead)
	{
		Session.OpenRequest request = new Session.OpenRequest(mActivity);
		if (request != null)
		{
			request.setDefaultAudience(mConfiguration.getSessionDefaultAudience());
			request.setLoginBehavior(mConfiguration.getSessionLoginBehavior());

			if (isRead)
			{
				request.setPermissions(mConfiguration.getReadPermissions());

				/*
				 * In case there are also PUBLISH permissions, then we would ask for these permissions second
				 * time (after, user accepted the read permissions)
				 */
				if (mConfiguration.hasPublishPermissions())
				{
					mSessionStatusCallback.askPublishPermissions();
				}

				// Open session with read permissions
				session.openForRead(request);
			}
			else
			{
				request.setPermissions(mConfiguration.getPublishPermissions());
				session.openForPublish(request);
			}
		}
	}

	/**
	 * Call this method only if session really needs to be reopened for read or for publish. <br>
	 * <br>
	 * 
	 * <b>Important</b><br>
	 * Any open method must be called at most once, and cannot be called after the Session is closed. Calling
	 * the method at an invalid time will result in {@link UnsupportedOperationException}.
	 */
	private void reopenSession()
	{
		Session session = Session.getActiveSession();
		if (session != null && session.getState().equals(SessionState.CREATED_TOKEN_LOADED))
		{
			List<String> permissions = session.getPermissions();
			List<String> publishPermissions = mConfiguration.getPublishPermissions();
			if (publishPermissions != null && publishPermissions.size() > 0 && permissions.containsAll(publishPermissions))
			{
				openSession(session, false);
			}
			else if (permissions.containsAll(mConfiguration.getReadPermissions()))
			{
				openSession(session, true);
			}
		}
	}

	/**
	 * Extend and ask user for PUBLISH permissions
	 * 
	 * @param activity
	 */
	public static void extendPublishPermissions()
	{
		Session session = Session.getActiveSession();

		Session.NewPermissionsRequest request = new Session.NewPermissionsRequest(mActivity, mConfiguration.getPublishPermissions());
		session.requestNewPublishPermissions(request);
	}

	/**
	 * Helper method
	 */

	private class SessionStatusCallback implements Session.StatusCallback
	{
		private boolean mAskPublishPermissions = false;
		private boolean mDoOnLogin = false;
		OnLoginListener mOnLoginListener = null;
		OnLogoutListener mOnLogoutListener = null;
		OnReopenSessionListener mOnReopenSessionListener = null;

		@Override
		public void call(Session session, SessionState state, Exception exception)
		{
			/*
			 * These are already authorized permissions
			 */
			List<String> permissions = session.getPermissions();

			if (exception != null)
			{
				// log
				logError("SessionStatusCallback: exception=", exception);

				if (exception instanceof FacebookOperationCanceledException)
				{
					/*
					 * If user canceled the read permissions dialog
					 */
					if (permissions.size() == 0)
					{
						mOnLoginListener.onNotAcceptingPermissions();
					}
					else
					{
						/*
						 * User canceled the WRITE permissions. We do nothing here. Once the user will try to
						 * do some action that require WRITE permissions, the dialog will be shown
						 * automatically.
						 */
					}
				}
				else
				{
					mOnLoginListener.onException(exception);
				}
			}

			// log
			logInfo("SessionStatusCallback: state=" + state.name() + ", session=" + String.valueOf(session));

			switch (state)
			{
			case CLOSED:
				if (mOnLogoutListener != null)
				{
					mOnLogoutListener.onLogout();
				}
				break;

			case CLOSED_LOGIN_FAILED:
				break;

			case CREATED:
				break;

			case CREATED_TOKEN_LOADED:
				break;

			case OPENING:
				if (mOnLoginListener != null)
				{
					mOnLoginListener.onThinking();
				}
				break;

			case OPENED:

				/*
				 * Check if we came from publishing actions where we ask again for publish permissions
				 */
				if (mOnReopenSessionListener != null)
				{
					mOnReopenSessionListener.onNotAcceptingPermissions();
					mOnReopenSessionListener = null;
				}

				/*
				 * Check if WRITE permissions were also defined in the configuration. If so, then ask in
				 * another dialog for WRITE permissions.
				 */
				else if (mAskPublishPermissions && session.getState().equals(SessionState.OPENED))
				{
					if (mDoOnLogin)
					{
						/*
						 * If user didn't accept the publish permissions, we still want to notify about
						 * complete
						 */
						mDoOnLogin = false;
						mOnLoginListener.onLogin();
					}
					else
					{

						mDoOnLogin = true;
						extendPublishPermissions();
						mAskPublishPermissions = false;
					}
				}
				else
				{
					if (mOnLoginListener != null)
					{
						mOnLoginListener.onLogin();
					}
				}
				break;

			case OPENED_TOKEN_UPDATED:

				/*
				 * Check if came from publishing actions and we need to reask for publish permissions
				 */
				if (mOnReopenSessionListener != null)
				{
					mOnReopenSessionListener.onSuccess();
					mOnReopenSessionListener = null;
				}
				else if (mDoOnLogin)
				{
					mDoOnLogin = false;

					if (mOnLoginListener != null)
					{
						mOnLoginListener.onLogin();
					}
				}

				break;

			default:
				break;
			}
		}

		/**
		 * If we want to open another dialog with publish permissions just after showing read permissions,
		 * then this method should be called
		 */
		public void askPublishPermissions()
		{
			mAskPublishPermissions = true;
		}
	}

	private static void logInfo(String message)
	{
		Logger.logInfo(SimpleFacebook.class, message);
	}

	private static void logError(String error, Throwable throwable)
	{
		if (throwable != null)
		{
			Logger.logError(SimpleFacebook.class, error, throwable);
		}
		else
		{
			Logger.logError(SimpleFacebook.class, error);
		}
	}

	public interface OnReopenSessionListener
	{
		void onSuccess();

		void onNotAcceptingPermissions();
	}

	/**
	 * On login/logout actions listener
	 * 
	 * @author sromku
	 */
	public interface OnLoginListener extends OnActionListener
	{
		/**
		 * If user performed {@link FacebookTools#login(Activity)} action, this callback method will be
		 * invoked
		 */
		void onLogin();

		/**
		 * If user pressed 'cancel' in READ (First) permissions dialog
		 */
		void onNotAcceptingPermissions();
	}

	public interface OnLogoutListener extends OnActionListener
	{
		/**
		 * If user performed {@link FacebookTools#logout()} action, this callback method will be invoked
		 */
		void onLogout();
	}

	/**
	 * On permission listener - If the App must request a specific permission (only to obtain the new Access
	 * Token)
	 * 
	 * @author Gryzor
	 * 
	 */
	public interface OnPermissionListener extends OnActionListener
	{
		/**
		 * If the permission was granted, this callback is invoked.
		 */
		void onSuccess(final String accessToken);

		/**
		 * If user pressed 'cancel' in PUBLISH permissions dialog
		 */
		void onNotAcceptingPermissions();
	}

	/**
	 * General interface in this simple sdk
	 * 
	 * @author sromku
	 * 
	 */
	public interface OnActionListener extends OnErrorListener
	{
		void onThinking();
	}

	public interface OnErrorListener
	{
		void onException(Throwable throwable);

		void onFail(String reason);
	}

}
