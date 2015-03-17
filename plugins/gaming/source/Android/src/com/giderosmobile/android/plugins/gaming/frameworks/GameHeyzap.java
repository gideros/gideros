package com.giderosmobile.android.plugins.gaming.frameworks;

import java.lang.ref.WeakReference;

import com.giderosmobile.android.plugins.gaming.*;
import com.heyzap.sdk.HeyzapLib;

import android.app.Activity;
import android.content.Intent;

public class GameHeyzap implements GameInterface {

	private static WeakReference<Activity> sActivity;
	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity =  activity;
		HeyzapLib.load(sActivity.get(), false);
	}

	@Override
	public void onDestroy() {}

	@Override
	public void onStart() {}

	@Override
	public void onStop() {}

	@Override
	public void onPause() {}

	@Override
	public void onResume() {}

	@Override
	public void login(Object parameters) {
		Game.loginComplete(this);
	}
	
	@Override
	public void logout() {}

	@Override
	public void onActivityResult(int request, int response, Intent data) {}

	@Override
	public void showLeaderboard(String id) {
		HeyzapLib.showLeaderboards(sActivity.get(), id);
	}

	@Override
	public void reportScore(String id, long score, int immediate) {
		if(immediate == 1)
		{
			HeyzapLib.submitScore(sActivity.get(), score+"", score+"", id);
			Game.reportScoreComplete(this, id, score);
		}
		else
			HeyzapLib.submitScore(sActivity.get(), score+"", score+"", id, false);
	}

	@Override
	public void showAchievements() {
		HeyzapLib.showAchievements(sActivity.get());
	}

	@Override
	public void reportAchievement(String id, int numSteps, int immediate) {
		HeyzapLib.unlockAchievement(sActivity.get(), id);
		if(immediate == 1)
			Game.reportAchievementComplete(this, id);
	}

	@Override
	public void loadAchievements() {
		Game.loadAchievementsError(this, Game.FEATURE_NOT_SUPPORTED);
	}

	@Override
	public void loadScores(String id, int span, int collection, int maxResults) {
		Game.loadScoresError(this, id, Game.FEATURE_NOT_SUPPORTED);
	}

	@Override
	public void loadState(int key) {
		Game.stateError(this, key, Game.FEATURE_NOT_SUPPORTED);
	}

	@Override
	public void updateState(int key, byte[] state, int immediate) {
		Game.stateError(this, key, Game.FEATURE_NOT_SUPPORTED);
	}

	@Override
	public void resolveState(int key, String version, byte[] state) {
		Game.stateError(this, key, Game.FEATURE_NOT_SUPPORTED);
	}

	@Override
	public void deleteState(int key) {
		Game.stateError(this, key, Game.FEATURE_NOT_SUPPORTED);
	}
}
