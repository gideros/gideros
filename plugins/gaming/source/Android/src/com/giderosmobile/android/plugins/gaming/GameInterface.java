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
	
	public void logout();
	
	public void showLeaderboard(String id);
	
	public void reportScore(String id, long score, int immediate);
	
	public void showAchievements();
	
	public void reportAchievement(String id, int numSteps, int immediate);
	
	public void loadAchievements();
	
	public void loadScores(String id, int span, int collection, int maxResults);
	
	public void loadState(int key);
	public void updateState(int key, byte[] state, int immediate);
	public void resolveState(int key, String version, byte[] state);
	public void deleteState(int key);
}
