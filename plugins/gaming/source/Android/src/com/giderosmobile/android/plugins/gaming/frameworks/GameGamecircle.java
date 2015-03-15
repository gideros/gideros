package com.giderosmobile.android.plugins.gaming.frameworks;

import java.lang.ref.WeakReference;
import java.util.EnumSet;
import java.util.List;

import com.amazon.ags.api.AGResponseCallback;
import com.amazon.ags.api.AGResponseHandle;
import com.amazon.ags.api.AmazonGamesCallback;
import com.amazon.ags.api.AmazonGamesClient;
import com.amazon.ags.api.AmazonGamesFeature;
import com.amazon.ags.api.AmazonGamesStatus;
import com.amazon.ags.api.achievements.Achievement;
import com.amazon.ags.api.achievements.AchievementsClient;
import com.amazon.ags.api.achievements.GetAchievementsResponse;
import com.amazon.ags.api.achievements.UpdateProgressResponse;
import com.amazon.ags.api.leaderboards.GetScoresResponse;
import com.amazon.ags.api.leaderboards.LeaderboardsClient;
import com.amazon.ags.api.leaderboards.Score;
import com.amazon.ags.api.leaderboards.SubmitScoreResponse;
import com.amazon.ags.api.whispersync.GameDataMap;
import com.amazon.ags.api.whispersync.WhispersyncEventListener;
import com.amazon.ags.api.whispersync.model.SyncableDeveloperString;
import com.amazon.ags.constants.LeaderboardFilter;
import com.giderosmobile.android.plugins.gaming.*;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.SparseArray;

public class GameGamecircle implements GameInterface {
	private static WeakReference<Activity> sActivity;
	GameGamecircle me;
	GameDataMap gameDataMap = null;
	AmazonGamesClient agsClient = null;
	boolean signed = false;
	EnumSet<AmazonGamesFeature> myGameFeatures = EnumSet.of(
		       AmazonGamesFeature.Achievements, AmazonGamesFeature.Leaderboards, AmazonGamesFeature.Whispersync);
	AmazonGamesCallback callback = new AmazonGamesCallback() {
        @Override
        public void onServiceNotReady(AmazonGamesStatus status) {
        	Game.loginError(this, status.name());
        }
        @Override
        public void onServiceReady(AmazonGamesClient amazonGamesClient) {
            agsClient = amazonGamesClient;
            gameDataMap = AmazonGamesClient.getWhispersyncClient().getGameData();
            if(!signed)
            {
            	signed = true;
            	Game.loginComplete(this);
            }
        }
};
	@Override
	public void onCreate(WeakReference<Activity> activity) {
		me = this;
		sActivity =  activity;
		signed = false;
	}

	@Override
	public void onDestroy() {
		if (agsClient != null) {
			AmazonGamesClient.shutdown();
	        agsClient = null;
	    }
	}

	@Override
	public void onStart() {}

	@Override
	public void onStop() {}

	@Override
	public void onPause() {
		if (agsClient != null) {
	        agsClient.release();
	        agsClient = null;
	    }
	}

	@Override
	public void onResume() {
		AmazonGamesClient.initialize(sActivity.get(), callback, myGameFeatures);
	}

	@Override
	public void login(Object parameters) {
		AmazonGamesClient.initialize(sActivity.get(), callback, myGameFeatures);
	}
	
	@Override
	public void logout() {}

	@Override
	public void onActivityResult(int request, int response, Intent data) {}

	@Override
	public void showLeaderboard(String id) {
		if(agsClient != null)
		{
			LeaderboardsClient lbClient = agsClient.getLeaderboardsClient();
			lbClient.showLeaderboardsOverlay();
		}
	}

	@Override
	public void reportScore(final String id, final long score, int immediate) {
		if(agsClient != null)
		{
			LeaderboardsClient lbClient = agsClient.getLeaderboardsClient();
			AGResponseHandle<SubmitScoreResponse> handle = lbClient.submitScore(id, score);
		 
			if(immediate == 1)
			{
				handle.setCallback(new AGResponseCallback<SubmitScoreResponse>() {
					
					@Override
					public void onComplete(SubmitScoreResponse result) {
						if (result.isError()) {
							Game.reportScoreError(me, id, score, result.getError().name());
						} else {
							Game.reportScoreComplete(me, id, score);
						}
					}
				});
			}
		}
	}

	@Override
	public void showAchievements() {
		if(agsClient != null)
		{
			AchievementsClient acClient = agsClient.getAchievementsClient();
			acClient.showAchievementsOverlay();
		}
	}

	@Override
	public void reportAchievement(final String id, int numSteps, int immediate) {
		if(agsClient != null)
		{
			AchievementsClient acClient = agsClient.getAchievementsClient();
			AGResponseHandle<UpdateProgressResponse> handle = acClient.updateProgress(id, numSteps);
			if(immediate == 1)
			{
				handle.setCallback(new AGResponseCallback<UpdateProgressResponse>() {
		 
					@Override
					public void onComplete(UpdateProgressResponse result) {
						if (result.isError()) {
							Game.reportAchievementError(me, id, result.getError().name());
						} else {
							Game.reportAchievementComplete(me, id);
						}
					}
				});
			}
		}
	}

	@Override
	public void loadAchievements() {
		if(agsClient != null)
		{
			AchievementsClient acClient = agsClient.getAchievementsClient();
			AGResponseHandle<GetAchievementsResponse> handle = acClient.getAchievements();
			handle.setCallback(new AGResponseCallback<GetAchievementsResponse>() {
				 
				@Override
				public void onComplete(GetAchievementsResponse result) {
					if (result.isError()) {
						Game.loadAchievementsError(me, result.getError().name());
					} else {
						SparseArray<Bundle> arr = new SparseArray<Bundle>();
						List<Achievement> list = result.getAchievementsList();
						int size = list.size();
						for(int i = 0; i < size; i++){
							Achievement ach = list.get(i);
							Bundle map = new Bundle();
							map.putString("id", ach.getId());
							map.putString("description", ach.getDescription());
							map.putString("name", ach.getTitle());
							if(ach.getDateUnlocked() != null)
								map.putInt("lastUpdate", ach.getDateUnlocked().getSeconds());
							else
								map.putInt("lastUpdate", 0);
							map.putInt("status", (ach.isUnlocked()) ? 0 : ((ach.isHidden()) ? 2 : 1)); //0-unlocked, 1-revealed, 2-hidden
								map.putInt("currentSteps", (int)ach.getProgress());
								map.putInt("totalSteps", 100);
							arr.put(i, map);
						}
						Game.loadAchievementsComplete(me, arr);
					}
				}
			});
		}
	}

	@Override
	public void loadScores(final String id, int span, int collection, int maxResults) {
		if(agsClient != null)
		{
			LeaderboardsClient lbClient = agsClient.getLeaderboardsClient();
			LeaderboardFilter filter;
			if(span == 0)
				filter = LeaderboardFilter.GLOBAL_DAY;
			else if(span == 1)
				filter = LeaderboardFilter.GLOBAL_WEEK;
			else{
				if(collection == 0)
					filter = LeaderboardFilter.FRIENDS_ALL_TIME;
				else
					filter = LeaderboardFilter.GLOBAL_ALL_TIME;
			}
			AGResponseHandle<GetScoresResponse> handle = lbClient.getScores(id, filter);
		 
			handle.setCallback(new AGResponseCallback<GetScoresResponse>() {
					
				@Override
				public void onComplete(GetScoresResponse result) {
					if (result.isError()) {
						Game.loadScoresError(me, id, result.getError().name());
					} else {
						String leaderboardName = result.getLeaderboard().getName();
						SparseArray<Bundle> lscores = new SparseArray<Bundle>();
						List<Score> list = result.getScores();
						int size = list.size();
						for(int i = 0; i < size; i++){
							Score score = list.get(i);
							Bundle map = new Bundle();
							map.putString("rank", score.getRank()+"");
							map.putString("score", score.getScoreString());
							map.putString("name", score.getPlayer().getAlias());
							map.putString("playerId", score.getPlayer().getPlayerId());
							map.putInt("timestamp", 0);
							lscores.put(i, map);
						}
						Game.loadScoresComplete(this, id, leaderboardName, lscores);
					}
				}
			});
		}
	}

	@Override
	public void loadState(int key) {
		if(gameDataMap != null)
		{
			SyncableDeveloperString developerString = gameDataMap.getDeveloperString(String.valueOf(key));
			String data = "";
			if(developerString.getValue() != null)
				data = developerString.getValue();
			Game.stateLoaded(this, key, data.getBytes(), 1);
		}
		else
			Game.stateError(this, key, Game.NOT_LOG_IN);
	}

	@Override
	public void updateState(final int key, byte[] state, int immediate) {
		if(gameDataMap != null)
		{
			SyncableDeveloperString developerString = gameDataMap.getDeveloperString(String.valueOf(key));
			developerString.setValue(new String(state));
	 // 	Set up listeners to handle conflicts after syncing with the cloud
			AmazonGamesClient.getWhispersyncClient().setWhispersyncEventListener(new WhispersyncEventListener() {
				public void onNewCloudData() {
					handlePotentialGameDataConflicts(key);
				}
	 
				public void onDataUploadedToCloud() {
					handlePotentialGameDataConflicts(key);
				}
			});
			if(immediate == 1)
				AmazonGamesClient.getWhispersyncClient().synchronize();
		}
		else
			Game.stateError(this, key, Game.NOT_LOG_IN);
	}

	@Override
	public void resolveState(int key, String version, byte[] state) {
		if(gameDataMap != null)
		{
			SyncableDeveloperString developerString = gameDataMap.getDeveloperString(String.valueOf(key));
			developerString.setValue(new String(state));
			developerString.markAsResolved();
		}
		else
			Game.stateError(this, key, Game.NOT_LOG_IN);
	}

	@Override
	public void deleteState(int key) {
		if(gameDataMap != null)
		{
			SyncableDeveloperString developerString = gameDataMap.getDeveloperString(String.valueOf(key));
			developerString.setValue(null);
		}
		else
			Game.stateError(this, key, Game.NOT_LOG_IN);
	}
	
	private void handlePotentialGameDataConflicts(int key){
		SyncableDeveloperString developerString = gameDataMap.getDeveloperString(String.valueOf(key));
		 
	    if (developerString.inConflict()) {
	    	Game.stateConflict(this, key, "1", developerString.getValue().getBytes(), developerString.getCloudValue().getBytes());
	    }
	}
}
