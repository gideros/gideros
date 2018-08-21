package com.giderosmobile.android.plugins.gaming;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.content.Intent;

public interface GameInterface {
	
	public void onCreate(WeakReference<Activity> activity);
	
	public void onDestroy();
	
	public void onStart();
	
	public void onStop();
	
	public void onPause();
		
	public void onResume();
	
	public void onActivityResult(int request, int response, Intent data);
	
	public void login(final Object parameters);
	
	public boolean loggedin();
	
	public void logout();
	
	public void showLeaderboard(String id);
	
	public void reportScore(String id, long score, int immediate);
	
	public void showAchievements();
	
	public void getPlayerInfo();
	
	public void reportAchievement(String id, int numSteps, int immediate);
	
	public void revealAchievement(String id, int immediate);
	
	public void loadAchievements();
	
	public void loadScores(String id, int span, int collection, int maxResults);
	public void loadPlayerScores(String id, int span, int collection, int maxResults);
	
	public void loadState(int key);
	public void updateState(int key, byte[] state, int immediate);
	public void resolveState(int key, String version, byte[] state);
	public void deleteState(int key);
	
	//Multiplayer
    public void autoMatch(int minPlayers, int maxPlayers);
    public void invitePlayers(int minPlayers, int maxPlayers);
    public void joinRoom(String id);
    public void showInvitations();
    public void showWaitingRoom(int minPlayers);
    public void sendTo(String id, byte[] message, int isReliable);
    public void sendToAll(byte[] message, int isReliable);
    public String getCurrentPlayer();
    public String getCurrentPlayerId();
    public String getPlayerPicture(String id, int highRes);
    public void getCurrentPlayerScore(String leaderboardId, int span, int leaderboardCollection);
    public Object getAllPlayers();

}
