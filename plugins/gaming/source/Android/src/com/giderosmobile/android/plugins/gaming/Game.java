package com.giderosmobile.android.plugins.gaming;

import java.lang.ref.WeakReference;
import java.util.Hashtable;

import android.app.Activity;
import android.content.Intent;
import android.util.SparseArray;
/*
 * TODO
 * Match achievement IDs
 * Match leader board IDs
 * Standardize achievement progress
 * deal with achievement increment provided
 * Add Cloud saves
 * Add profile images if available, to scores
 * Add player info method
 * Add player score for leaderboard
 * Advance achievement progress syncing
 */


public class Game {
	
	private static WeakReference<Activity> sActivity;
	private static long sData = 0;
	private static Hashtable<String, GameInterface> games;
	
	public static String FEATURE_NOT_SUPPORTED = "Feature Not Supported";
	public static String LIBRARY_NOT_FOUND = "Needed library was not found";
	public static String NOT_LOG_IN = "You are not logged in to use this feature";
	
	public static void onCreate(Activity activity)
	{
		sActivity =  new WeakReference<Activity>(activity);
		
		games = new Hashtable<String, GameInterface>();
	}
	
	//on destroy event
	public static void onDestroy()
	{	
		for (GameInterface value : games.values()) {
			value.onDestroy();
		}
		games.clear();
	}
	
	//on destroy event
	public static void onStart()
	{	
		for (GameInterface value : games.values()) {
			value.onStart();
		}
	}
	
	//on destroy event
	public static void onStop()
	{	
		for (GameInterface value : games.values()) {
			value.onStop();
		}
	}
	
	public static void onPause()
	{	
		for (GameInterface value : games.values()) {
			value.onPause();
		}
	}
		
	public static void onResume()
	{	
		for (GameInterface value : games.values()) {
			value.onResume();
		}
	}
	
	public static void onActivityResult(int request, int response, Intent data) {
		for (GameInterface value : games.values()) {
			value.onActivityResult(request, response, data);
		}
	}
	
	public static void init(long data){
		sData = data;
	}
	
	public static void cleanup()
	{
		sData = 0;
		try
		{
			Runnable myRunnable = new Runnable() {
				@Override
				public void run() {
					onDestroy();
				}
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static void initialize(String type){
		final String adp = modifyName(type);
		if(games.get(adp) == null)
		{
			String className = "com.giderosmobile.android.plugins.gaming.frameworks.Game"+adp;
			Class classz = null;
			try {
				classz = Class.forName(className);
			} catch (ClassNotFoundException e) {
				e.printStackTrace();
			}
			try {
				games.put(adp, (GameInterface)classz.newInstance());
			} catch (InstantiationException e) {
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
			}

			try
			{
				Runnable myRunnable = new Runnable() {
					@Override
					public void run() {
						if(games.containsKey(adp))
						{
							games.get(adp).onCreate(sActivity);
						}
					}
					
				};
				sActivity.get().runOnUiThread(myRunnable) ;
			}
			catch(Exception ex)	{}
		}
	}
	
	public static void destroy(String type){
		final String adp = modifyName(type);
		if(games.containsKey(adp))
		{
			try
			{
				Runnable myRunnable = new Runnable() {
					@Override
					public void run() {
						if(games.containsKey(adp))
						{
							games.get(adp).onDestroy();
							games.remove(adp);
						}
					}
				};
				sActivity.get().runOnUiThread(myRunnable) ;
			}
			catch(Exception ex)	{}
		}
	}
	
	public static void login(String type, final Object parameters){
		final String adp = modifyName(type);
		try
		{
			Runnable myRunnable = new Runnable() {
				@Override
				public void run() {
					if(games.containsKey(adp))
					{
						games.get(adp).login(parameters);
					}
				}
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static void logout(String type){
		final String adp = modifyName(type);
		try
		{
			Runnable myRunnable = new Runnable() {
				@Override
				public void run() {
					if(games.containsKey(adp))
						games.get(adp).logout();
				}
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}

	public static boolean loggedin(String type){
		final String adp = modifyName(type);
		try
		{
			if(games.containsKey(adp))
				return games.get(adp).loggedin();
		}
		catch(Exception ex)	{}
		return false;
	}

	public static void showLeaderboard(String type, final String id)
	{
		final String adp = modifyName(type);
		try
		{
			Runnable myRunnable = new Runnable() {
				@Override
				public void run() {
					if(games.containsKey(adp))
						games.get(adp).showLeaderboard(id);
				}
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}
	
	public static void reportScore(String type, String id, long score, int immediate)
	{
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).reportScore(id, score, immediate);
	}
	
	public static void showAchievements(String type)
	{
		final String adp = modifyName(type);
		try
		{
			Runnable myRunnable = new Runnable() {
				@Override
				public void run() {
					if(games.containsKey(adp))
						games.get(adp).showAchievements();
				}
			};
			sActivity.get().runOnUiThread(myRunnable) ;
		}
		catch(Exception ex)	{}
	}

	public static void getPlayerInfo(String type)
	{
		final String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).getPlayerInfo();
	}

	public static void reportAchievement(String type, String id, double numSteps, int immediate)
	{
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).reportAchievement(id, (int)numSteps, immediate);
	}

	public static void revealAchievement(String type, String id, int immediate)
	{
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).revealAchievement(id, immediate);
	}

	public static void loadAchievements(String type)
	{
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).loadAchievements();
	}
	
	public static void loadScores(String type, String id, int span, int collection, int maxResults)
	{
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).loadScores(id, span, collection, maxResults);
	}
	
	static public void loadState(String type, int key){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).loadState(key);
	}
	
	static public void updateState(String type, int key, byte[] state, int immediate){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).updateState(key, state, immediate);
	}
	
	static public void resolveState(String type, int key, String version, byte[] state){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).resolveState(key, version, state);
	}
	
	static public void deleteState(String type, int key){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).deleteState(key);
	}
	
	// MP
    static public void autoMatch(String type, int minPlayers, int maxPlayers) {
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).autoMatch(minPlayers,maxPlayers);
    }
    
    static public void invitePlayers(String type, int minPlayers, int maxPlayers){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).invitePlayers(minPlayers, maxPlayers);
    }
    
    static public void joinRoom(String type, String id){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).joinRoom(id);
    }
    
    static public void showInvitations(String type){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).showInvitations();
    }
    
    static public void showWaitingRoom(String type,int minPlayers){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).showWaitingRoom(minPlayers);
    }
    
    static public void sendTo(String type,String id, byte[] message, int isReliable){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).sendTo(id, message, isReliable);
    }
    
    static public void sendToAll(String type,byte[] message, int isReliable){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).sendToAll(message,isReliable);
    }
    
    static public String getCurrentPlayer(String type){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			return games.get(adp).getCurrentPlayer();
    	return "";
    }
    
    static public String getCurrentPlayerId(String type){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			return games.get(adp).getCurrentPlayerId();
    	return "";
    }
    
    static public String getCurrentPlayerPicture(String type, int highRes){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			return games.get(adp).getCurrentPlayerPicture(highRes);
    	return "";
    }
        
    static public void getCurrentPlayerScore(String type,String leaderboardId, int span, int leaderboardCollection){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			games.get(adp).getCurrentPlayerScore(leaderboardId,span,leaderboardCollection);
    }
    
    static public Object getAllPlayers(String type){
		String adp = modifyName(type);
		if(games.containsKey(adp))
			return games.get(adp).getAllPlayers();
	   	return new SparseArray<String>();
    }

	/*
	 * Events
	 */
	
	public static void loginComplete(Object caller){
		if (sData != 0)
			onLoginComplete(getCallerName(caller), sData);
	}
	
	public static void loginError(Object caller, String error){
		if (sData != 0)
			onLoginError(getCallerName(caller), error, sData);
	}
	
	public static void reportAchievementComplete(Object caller, String id){
		if (sData != 0)
			onReportAchievementComplete(getCallerName(caller), id, sData);
	}
	
	public static void reportAchievementError(Object caller, String id, String error){
		if (sData != 0)
			onReportAchievementError(getCallerName(caller), id, error, sData);
	}
	
	public static void reportScoreComplete(Object caller, String id, long score){
		if (sData != 0)
			onReportScoreComplete(getCallerName(caller), id, score, sData);
	}
	
	public static void reportScoreError(Object caller, String id, long score, String error){
		if (sData != 0)
			onReportScoreError(getCallerName(caller), id, error, score, sData);
	}
	
	public static void loadAchievementsComplete(Object caller, Object arr){
		if (sData != 0)
			onLoadAchievementsComplete(getCallerName(caller), arr, sData);
	}
	
	public static void loadAchievementsError(Object caller, String error){
		if (sData != 0)
			onLoadAchievementsError(getCallerName(caller), error, sData);
	}
	
	public static void loadScoresComplete(Object caller, String id, String name, Object scores){
		if (sData != 0)
			onLoadScoresComplete(getCallerName(caller), id, name, scores, sData);
	}
	
	public static void loadScoresError(Object caller, String id, String error){
		if (sData != 0)
			onLoadScoresError(getCallerName(caller), id, error, sData);
	}

	public static void playerInfoComplete(Object caller, String id, String name, String pic){
		if (sData != 0)
			onPlayerInfoComplete(getCallerName(caller), id, name, pic, sData);
	}
	
	public static void playerInfoError(Object caller, String error){
		if (sData != 0)
			onPlayerInfoError(getCallerName(caller), error, sData);
	}

	public static void stateLoaded(Object caller, int key, byte[] state, int fresh){
		if (sData != 0)
			onStateLoaded(getCallerName(caller), key, state, fresh, sData);
	}
	
	public static void stateError(Object caller, int key, String error){
		if (sData != 0)
			onStateError(getCallerName(caller), key, error, sData);
	}
	
	public static void stateConflict(Object caller, int key, String ver, byte[] localState, byte[] serverState){
		if (sData != 0)
			onStateConflict(getCallerName(caller), key, ver, localState, serverState, sData);
	}
	
	public static void stateDeleted(Object caller, int key){
		if (sData != 0)
			onStateDeleted(getCallerName(caller), key, sData);
	}

	public static void gameStarted(Object caller){
		if (sData != 0)
			onGameStarted(getCallerName(caller), sData);
	}
	public static void invitationReceived(Object caller,String id){
		if (sData != 0)
			onInvitationReceived(getCallerName(caller), id, sData);
	}
	public static void joinedRoom(Object caller,String id){
		if (sData != 0)
			onJoinedRoom(getCallerName(caller), id, sData);
	}
	public static void leftRoom(Object caller,String id){
		if (sData != 0)
			onLeftRoom(getCallerName(caller), id, sData);
	}
	public static void roomConnected(Object caller,String id){
		if (sData != 0)
			onRoomConnected(getCallerName(caller), id, sData);
	}
	public static void roomCreated(Object caller,String id){
		if (sData != 0)
			onRoomCreated(getCallerName(caller), id,sData);
	}
	public static void connectedToRoom(Object caller,String id){
		if (sData != 0)
			onConnectedToRoom(getCallerName(caller), id,sData);
	}
	public static void disconnectedFromRoom(Object caller,String id){
		if (sData != 0)
			onDisconnectedFromRoom(getCallerName(caller), id,sData);
	}
	public static void peerDeclined(Object caller){
		if (sData != 0)
			onPeerDeclined(getCallerName(caller), sData);
	}
	public static void peerInvitedToRoom(Object caller){
		if (sData != 0)
			onPeerInvitedToRoom(getCallerName(caller), sData);
	}
	public static void peerJoined(Object caller){
		if (sData != 0)
			onPeerJoined(getCallerName(caller), sData);
	}
	public static void peerLeft(Object caller){
		if (sData != 0)
			onPeerLeft(getCallerName(caller), sData);
	}
	public static void peersConnected(Object caller){
		if (sData != 0)
			onPeersConnected(getCallerName(caller), sData);
	}
	public static void peersDisconnected(Object caller){
		if (sData != 0)
			onPeersDisconnected(getCallerName(caller), sData);
	}
	public static void roomAutoMatching(Object caller,String id){
		if (sData != 0)
			onRoomAutoMatching(getCallerName(caller), id,sData);
	}
	public static void roomConnecting(Object caller,String id){
		if (sData != 0)
			onRoomConnecting(getCallerName(caller), id,sData);
	}
	public static void dataReceived(Object caller,byte[] message, String sender){
		if (sData != 0)
			onDataReceived(getCallerName(caller), message,sender,sData);
	}
	public static void playerScore(Object caller,String rank, String formatscore, long score, int timestamp){
		if (sData != 0)
			onPlayerScore(getCallerName(caller), rank, formatscore, score, timestamp, sData);
	}

	private static native void onLoginComplete(String caller, long data);
	private static native void onLoginError(String caller, String error, long data);
	
	private static native void onReportAchievementComplete(String caller, String id, long data);
	private static native void onReportAchievementError(String caller, String id, String error, long data);
	
	private static native void onReportScoreComplete(String caller, String id, long score, long data);
	private static native void onReportScoreError(String caller, String id, String error, long score, long data);
	
	private static native void onPlayerInfoComplete(String caller, String id, String name, String pic, long data);
	private static native void onPlayerInfoError(String caller, String error, long data);
	
	private static native void onLoadAchievementsComplete(String caller, Object arr, long data);
	private static native void onLoadAchievementsError(String caller, String error, long data);
	
	private static native void onLoadScoresComplete(String caller, String id, String name, Object scores, long data);
	private static native void onLoadScoresError(String caller, String id, String error, long data);
	
	private static native void onStateLoaded(String caller, int key, byte[] state, int fresh, long data);
	private static native void onStateError(String caller, int key, String error, long data);
	private static native void onStateConflict(String caller, int key, String ver, byte[] localState, byte[] serverState, long data);
	private static native void onStateDeleted(String caller, int key, long data);
	
	private static native void onGameStarted(String caller,long data);
	private static native void onInvitationReceived(String caller,String id, long data);
	private static native void onJoinedRoom(String caller,String id, long data);
	private static native void onLeftRoom(String caller,String id, long data);
	private static native void onRoomConnected(String caller,String id, long data);
	private static native void onRoomCreated(String caller,String id, long data);
	private static native void onConnectedToRoom(String caller,String id, long data);
	private static native void onDisconnectedFromRoom(String caller,String id, long data);
	private static native void onPeerDeclined(String caller,long data);
	private static native void onPeerInvitedToRoom(String caller,long data);
	private static native void onPeerJoined(String caller,long data);
	private static native void onPeerLeft(String caller,long data);
	private static native void onPeersConnected(String caller,long data);
	private static native void onPeersDisconnected(String caller,long data);
	private static native void onRoomAutoMatching(String caller,String id, long data);
	private static native void onRoomConnecting(String caller,String id, long data);
	private static native void onDataReceived(String caller,byte[] message, String sender, long data);
	private static native void onPlayerScore(String caller,String rank, String formatscore, long score, int timestamp, long data);

	private static String modifyName(String type){
		return type.substring(0,1).toUpperCase() + type.substring(1).toLowerCase();
	}
	
	private static String getCallerName(Object cls){
		 String name = cls.getClass().getName();
         name = name.replace("com.giderosmobile.android.plugins.gaming.frameworks.Game", "");
         if(name.contains("$"))
        	 name = name.substring(0, name.indexOf("$"));
         return name.toLowerCase();
	}
	
}
