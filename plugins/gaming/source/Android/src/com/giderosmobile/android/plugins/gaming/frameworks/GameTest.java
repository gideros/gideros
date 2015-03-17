package com.giderosmobile.android.plugins.gaming.frameworks;

import java.lang.ref.WeakReference;

import com.giderosmobile.android.plugins.gaming.*;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.util.SparseArray;

public class GameTest implements GameInterface {

	@Override
	public void onCreate(WeakReference<Activity> activity) {
		Log.d("GameTest", "onCreate");
	}

	@Override
	public void onDestroy() {
		Log.d("GameTest", "onDestroy");
	}

	@Override
	public void onStart() {
		Log.d("GameTest", "onStart");
	}

	@Override
	public void onStop() {
		Log.d("GameTest", "onStop");
	}

	@Override
	public void onPause() {
		Log.d("GameTest", "onPause");
	}

	@Override
	public void onResume() {
		Log.d("GameTest", "onResume");
	}

	@Override
	public void login(Object parameters) {
		Log.d("GameTest", "login: "+ parameters);
		Game.loginComplete(this);
		Game.loginError(this, "Could not login");
	}
	
	@Override
	public void logout() {
		Log.d("GameTest", "logout");
	}

	@Override
	public void onActivityResult(int request, int response, Intent data) {
		Log.d("GameTest", "onActivityResult: " + request + "; " + response);
	}

	@Override
	public void showLeaderboard(String id) {
		Log.d("GameTest", "showLeaderboard: " + id);
	}

	@Override
	public void reportScore(String id, long score, int immediate) {
		Log.d("GameTest", "reportScore: " + id + ", " + score + ",  " + immediate);
		Game.reportScoreComplete(this, id, score);
		Game.reportScoreError(this, id, score, "Score Error");
	}

	@Override
	public void showAchievements() {
		Log.d("GameTest", "showAchievements");
	}

	@Override
	public void reportAchievement(String id, int numSteps, int immediate) {
		Log.d("GameTest", "reportAchievement: " + id + ", " + numSteps + ", " + immediate);
		Game.reportAchievementComplete(this, id);
		Game.reportAchievementError(this, id, "Achievement Error");
	}

	@Override
	public void loadAchievements() {
		Log.d("GameTest", "loadAchievements");
		SparseArray<Bundle> arr = new SparseArray<Bundle>();
		int size = 3;
		for(int i = 0; i < size; i++){
			Bundle map = new Bundle();
			map.putString("id", "achievement"+i);
			map.putString("description", "Description "+i);
			map.putString("name", "name"+i);
			map.putInt("lastUpdate", (1000*i));
			map.putInt("status", i); //0-unlocked, 1-revealed, 2-hidden
				map.putInt("currentSteps", 10*i);
				map.putInt("totalSteps", 100*i);
			arr.put(i, map);
		}
		Game.loadAchievementsComplete(this, arr);
		Game.loadAchievementsError(this, "Load Achievement Error");
	}

	@Override
	public void loadScores(String id, int span, int collection, int maxResults) {
		Log.d("GameTest", "loadScores: " + id + ", " + span + ", " + collection + ", " + maxResults);
		String leaderboardName = "Leaderbroad";
		SparseArray<Bundle> lscores = new SparseArray<Bundle>();
		int size = 3;
		for(int i = 0; i < size; i++){
			Bundle map = new Bundle();
			map.putString("rank", ""+(i+1));
			map.putString("score", (100-(i*10))+"");
			map.putString("name", "Test user " +(i+1));
			map.putString("playerId", i+"");
			map.putInt("timestamp", (1000*i));
			lscores.put(i, map);
		}
		Game.loadScoresComplete(this, id, leaderboardName, lscores);
		Game.loadScoresError(this, id, "Load Scores Error");
	}

	@Override
	public void loadState(int key) {
		Log.d("GameTest", "load state: " + key);
		Game.stateLoaded(this, key, "state1".getBytes(), 1);
	}

	@Override
	public void updateState(int key, byte[] state, int immediate) {
		Log.d("GameTest", "update state: " + key + ": " + state);
		Game.stateConflict(this, key, "1", "state1".getBytes(), "state2".getBytes());
	}

	@Override
	public void resolveState(int key, String version, byte[] state) {
		Log.d("GameTest", "resolve state: " + key + ": " + state + " for "+ version);
		Game.stateError(this, key, "Error resolving state");
	}

	@Override
	public void deleteState(int key) {
		Log.d("GameTest", "delete state: " + key);
		Game.stateDeleted(this, key);
	}
}
