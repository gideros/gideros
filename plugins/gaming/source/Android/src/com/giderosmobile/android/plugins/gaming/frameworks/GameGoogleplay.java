package com.giderosmobile.android.plugins.gaming.frameworks;

import java.lang.ref.WeakReference;

import com.giderosmobile.android.plugins.gaming.*;
import com.giderosmobile.android.plugins.gaming.frameworks.googleplay.GameHelper;
import com.giderosmobile.android.plugins.gaming.frameworks.googleplay.GameHelper.GameHelperListener;
import com.google.android.gms.appstate.AppStateManager;
import com.google.android.gms.appstate.AppStateManager.StateDeletedResult;
import com.google.android.gms.appstate.AppStateManager.StateResult;
import com.google.android.gms.appstate.AppStateStatusCodes;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GooglePlayServicesUtil;
import com.google.android.gms.common.api.PendingResult;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.games.Games;
import com.google.android.gms.games.GamesStatusCodes;
import com.google.android.gms.games.achievement.Achievement;
import com.google.android.gms.games.achievement.AchievementBuffer;
import com.google.android.gms.games.achievement.Achievements.LoadAchievementsResult;
import com.google.android.gms.games.achievement.Achievements.UpdateAchievementResult;
import com.google.android.gms.games.leaderboard.LeaderboardScore;
import com.google.android.gms.games.leaderboard.LeaderboardScoreBuffer;
import com.google.android.gms.games.leaderboard.Leaderboards;
import com.google.android.gms.games.leaderboard.Leaderboards.LoadScoresResult;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.SparseArray;

public class GameGoogleplay implements GameInterface, GameHelperListener {
	private static WeakReference<Activity> sActivity;
	protected static GameHelper mHelper;
	protected static int mRequestedClients = GameHelper.CLIENT_APPSTATE|GameHelper.CLIENT_GAMES|GameHelper.CLIENT_PLUS;
	final static int RC_UNUSED = 9002;
	boolean signed = false;
	GameGoogleplay me;
	
	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity =  activity;
		me = this;
		signed = false;
		if(isAvailable())
		{
			mHelper = new GameHelper(sActivity.get(), mRequestedClients);
			mHelper.setup(me);
		}
	}

	@Override
	public void onDestroy() {}

	@Override
	public void onStart() {
		if(mHelper != null)
		{
           	mHelper.onStart(sActivity.get());
		}
	}

	@Override
	public void onStop() {
		if(mHelper != null)
			mHelper.onStop();
	}

	@Override
	public void onPause() {}

	@Override
	public void onResume() {}

	@Override
	public void login(Object parameters) {
		if(mHelper != null && !mHelper.isSignedIn())
	    {
			mHelper.beginUserInitiatedSignIn();
	    }
	}
	
	@Override
	public void logout() {
		signed = false;
       	if(mHelper != null && mHelper.isSignedIn())
       	{
       		mHelper.signOut();
        }
	}

	@Override
	public void onActivityResult(int request, int response, Intent data) {
		if(mHelper != null)
			mHelper.onActivityResult(request, response, data);
	}

	@Override
	public void showLeaderboard(final String id) {
       	if(mHelper != null && mHelper.isSignedIn())
       		sActivity.get().startActivityForResult(Games.Leaderboards.getLeaderboardIntent(mHelper.getApiClient(), id), RC_UNUSED);
	}

	@Override
	public void reportScore(final String id, final long score, int immediate) {
		if(isAvailable())
		{
			if(mHelper != null && mHelper.isSignedIn())
	    	{
	    		if(immediate == 1)
	    		{
	    			PendingResult<Leaderboards.SubmitScoreResult> result = Games.Leaderboards.submitScoreImmediate(mHelper.getApiClient(), id, score);
	    			ResultCallback<Leaderboards.SubmitScoreResult> mResultCallback = new
	    		            ResultCallback<Leaderboards.SubmitScoreResult>() {
	    		        @Override
	    		        public void onResult(Leaderboards.SubmitScoreResult result) {
	    	    			if(result.getStatus().getStatusCode() == GamesStatusCodes.STATUS_OK){
	    	    				Game.reportScoreComplete(me, id, score);
	    		    		}
	    	    			else
	    	    			{
	    	    				Game.reportScoreError(me, id, score, "Error: "+result.getStatus().getStatusCode());
	    	    			}
	    		        }
	    		    };
	    			result.setResultCallback(mResultCallback);
	    		}
	    		else
	    		{
	    			Games.Leaderboards.submitScore(mHelper.getApiClient(), id, score);
	    		}
	    	}
	       	else
	    		Game.reportScoreError(this, id, score, Game.NOT_LOG_IN);
		}
		else
			Game.reportScoreError(this, id, score, Game.LIBRARY_NOT_FOUND);
	}

	@Override
	public void showAchievements() {
       	if(mHelper != null && mHelper.isSignedIn())
       		sActivity.get().startActivityForResult(Games.Achievements.getAchievementsIntent(mHelper.getApiClient()), RC_UNUSED);
	}

	@Override
	public void reportAchievement(final String id, int numSteps, int immediate) {
		if(isAvailable())
		{
			if(mHelper != null && mHelper.isSignedIn())
			{
				if(immediate == 1)
				{
					PendingResult<UpdateAchievementResult> result = Games.Achievements.unlockImmediate(mHelper.getApiClient(), id);
					ResultCallback<UpdateAchievementResult> mResultCallback = new
							ResultCallback<UpdateAchievementResult>() {
						@Override
						public void onResult(UpdateAchievementResult result) {
							if(result.getStatus().getStatusCode() == GamesStatusCodes.STATUS_OK || result.getStatus().getStatusCode() == GamesStatusCodes.STATUS_ACHIEVEMENT_UNLOCKED){
								Game.reportAchievementComplete(me, id);
							}
							else
							{
								Game.reportAchievementError(me, id, "Error: "+result.getStatus().getStatusCode());
							}
						}
					};
					result.setResultCallback(mResultCallback);
				}
				else
				{
					Games.Achievements.unlock(mHelper.getApiClient(), id);
				}
			}
			else
	    		Game.reportAchievementError(this, id, Game.NOT_LOG_IN);
		}
		else
		{
			Game.reportAchievementError(this, id, Game.LIBRARY_NOT_FOUND);
		}
	}

	@Override
	public void loadAchievements() {
		if(isAvailable())
		{
			if(mHelper != null && mHelper.isSignedIn())
	    	{
	    		PendingResult<LoadAchievementsResult> result = Games.Achievements.load(mHelper.getApiClient(), true);
	    		ResultCallback<LoadAchievementsResult> mResultCallback = new
			            ResultCallback<LoadAchievementsResult>() {
			        @Override
			        public void onResult(LoadAchievementsResult result) {
			    		if(result.getStatus().getStatusCode() == GamesStatusCodes.STATUS_OK){
			    			SparseArray<Bundle> arr = new SparseArray<Bundle>();
			    			AchievementBuffer buffer = result.getAchievements();
			    			int size = buffer.getCount();
			    			for(int i = 0; i < size; i++){
			    				Achievement ach = buffer.get(i);
			    				Bundle map = new Bundle();
			    				map.putString("id", ach.getAchievementId());
			    				map.putString("description", ach.getDescription());
			    				map.putString("name", ach.getName());
			    				map.putInt("lastUpdate", (int)(ach.getLastUpdatedTimestamp()/1000));
			    				map.putInt("status", ach.getState()); //0-unlocked, 1-revealed, 2-hidden
			    				if(ach.getType() == Achievement.TYPE_INCREMENTAL)
			    				{
			    					map.putInt("currentSteps", ach.getCurrentSteps());
			    					map.putInt("totalSteps", ach.getTotalSteps());
			    				}
			    				arr.put(i, map);
			    			}
			    			Game.loadAchievementsComplete(me, arr);
			    			buffer.close();
			    		}
			    		else
			    		{
			    			Game.loadAchievementsError(me, "Error: "+result.getStatus().getStatusCode());
			    		}
			        }
			    };
	    		result.setResultCallback(mResultCallback);
	    	}
			else
	    		Game.loadAchievementsError(this, Game.NOT_LOG_IN);
		}
		else
		{
			Game.loadAchievementsError(this, Game.LIBRARY_NOT_FOUND);
		}
	}	

	@Override
	public void loadScores(final String id, int span, int collection, int maxResults) {
		if(maxResults > 25){
			maxResults = 25;
		}
		if(isAvailable())
		{
			if(mHelper != null && mHelper.isSignedIn())
	    	{
	    		PendingResult<LoadScoresResult> result = Games.Leaderboards.loadTopScores(mHelper.getApiClient(), id, span, collection, maxResults);
	    		ResultCallback<LoadScoresResult> mResultCallback = new
			            ResultCallback<LoadScoresResult>() {
			        @Override
			        public void onResult(LoadScoresResult result) {
			    		if(result.getStatus().getStatusCode() == GamesStatusCodes.STATUS_OK){
			    			String leaderboardId =  result.getLeaderboard().getLeaderboardId();
			    			String leaderboardName = result.getLeaderboard().getDisplayName();
			    			LeaderboardScoreBuffer scores = result.getScores();
			    			SparseArray<Bundle> lscores = new SparseArray<Bundle>();
			    			int size = scores.getCount();
			    			for(int i = 0; i < size; i++){
			    				LeaderboardScore l = scores.get(i);
			    				Bundle map = new Bundle();
			    				map.putString("rank", l.getDisplayRank());
			    				map.putString("score", l.getDisplayScore());
			    				map.putString("name", l.getScoreHolderDisplayName());
			    				map.putString("playerId", l.getScoreHolder().getPlayerId());
			    				map.putInt("timestamp", (int)(l.getTimestampMillis()/1000));
			    				lscores.put(i, map);
			    			}
			    			Game.loadScoresComplete(me, leaderboardId, leaderboardName, lscores);
			    			scores.close();
			    		}
			    		else
			    		{
			    			Game.loadScoresError(this, id, "Error: "+result.getStatus().getStatusCode());
			    		}
			        }
			    };
	    		result.setResultCallback(mResultCallback);
	    	}
			else
	    		Game.loadScoresError(this, id, Game.NOT_LOG_IN);
		}
		else
		{
			Game.loadScoresError(this, id, Game.LIBRARY_NOT_FOUND);
		}
	}
	
	@Override
	public void loadState(final int key) {
		if (mHelper != null && mHelper.isSignedIn()) {
    		PendingResult<StateResult> result = AppStateManager.load(mHelper.getApiClient(), key);
    		ResultCallback<StateResult> mResultCallback = new
		            ResultCallback<StateResult>() {
		        @Override
		        public void onResult(StateResult result) {
		        	int statusCode = result.getStatus().getStatusCode();
		        	int stateKey = key;
		        	result.getLoadedResult();
		        	if (statusCode == AppStateStatusCodes.STATUS_OK || statusCode == AppStateStatusCodes.STATUS_NETWORK_ERROR_STALE_DATA) {
		        		
		        		AppStateManager.StateConflictResult conflictResult = result.getConflictResult();
		                AppStateManager.StateLoadedResult loadedResult = result.getLoadedResult();
		                if (loadedResult != null) {
		    				Game.stateLoaded(this, stateKey, loadedResult.getLocalData(), 1);
		                } else if (conflictResult != null) {
		                	Game.stateConflict(this, stateKey, conflictResult.getResolvedVersion(), conflictResult.getLocalData(), conflictResult.getServerData());
		                }
		    		} else {

	    				String error = "unknown error";
	    				if(statusCode == AppStateStatusCodes.STATUS_CLIENT_RECONNECT_REQUIRED)
	    				{
	    					error = "need to reconnect client";
	    				}
	    				else if(statusCode == AppStateStatusCodes.STATUS_NETWORK_ERROR_OPERATION_FAILED)
	    				{
	    					error = "could not connect server";
	    				}
	    				else if(statusCode == AppStateStatusCodes.STATUS_NETWORK_ERROR_NO_DATA)
	    				{
	    					error = "could not connect server";
	    				}
	    				else if(statusCode == AppStateStatusCodes.STATUS_STATE_KEY_NOT_FOUND)
	    				{
	    					error = "no data";
	    				}
	    				else if(statusCode == AppStateStatusCodes.STATUS_STATE_KEY_LIMIT_EXCEEDED)
	    				{
	    					error = "key limit exceeded";
	    				}
	    				else if(statusCode == AppStateStatusCodes.STATUS_WRITE_SIZE_EXCEEDED)
	    				{
	    					error = "too much data passed";
	    				}
	    				Game.stateError(this, stateKey, error);
	    			}
		        }
		    };
    		result.setResultCallback(mResultCallback);
    	}
		else
    		Game.stateError(this, key, Game.NOT_LOG_IN);
	}

	@Override
	public void updateState(final int key, byte[] state, int immediate) {
		if (mHelper != null && mHelper.isSignedIn()) {
    		if(immediate == 1)
    		{
    			PendingResult<StateResult> result = AppStateManager.updateImmediate(mHelper.getApiClient(), key, state);
    			ResultCallback<StateResult> mResultCallback = new
    		            ResultCallback<StateResult>() {
    		        @Override
    		        public void onResult(StateResult result) {
    		        	int statusCode = result.getStatus().getStatusCode();
    		        	int stateKey = key;
    		        	result.getLoadedResult();
    		        	if (statusCode == AppStateStatusCodes.STATUS_OK || statusCode == AppStateStatusCodes.STATUS_NETWORK_ERROR_STALE_DATA) {
    		        		
    		        		AppStateManager.StateConflictResult conflictResult = result.getConflictResult();
    		                if (conflictResult != null) {
    		                	Game.stateConflict(this, stateKey, conflictResult.getResolvedVersion(), conflictResult.getLocalData(), conflictResult.getServerData());
    		                }
    		    		} else {
   		    				String error = "unknown error";
   		    				if(statusCode == AppStateStatusCodes.STATUS_CLIENT_RECONNECT_REQUIRED)
   		    				{
   		    					error = "need to reconnect client";
   		    				}
   		    				else if(statusCode == AppStateStatusCodes.STATUS_NETWORK_ERROR_OPERATION_FAILED)
   		    				{
   		    					error = "could not connect server";
   		    				}
   		    				else if(statusCode == AppStateStatusCodes.STATUS_NETWORK_ERROR_NO_DATA)
   		    				{
   		    					error = "could not connect server";
   		    				}
   		    				else if(statusCode == AppStateStatusCodes.STATUS_STATE_KEY_NOT_FOUND)
   		    				{
   		    					error = "no data";
   		    				}
   		    				else if(statusCode == AppStateStatusCodes.STATUS_STATE_KEY_LIMIT_EXCEEDED)
   		    				{
   		    					error = "key limit exceeded";
   		    				}
   		    				else if(statusCode == AppStateStatusCodes.STATUS_WRITE_SIZE_EXCEEDED)
   		    				{
   		    					error = "too much data passed";
   		    				}
   		    				Game.stateError(this, stateKey, error);
    		    		}
    		        }
    		    };
    			result.setResultCallback(mResultCallback);
    		}
    		else
    		{
    			AppStateManager.update(mHelper.getApiClient(), key, state);
    		}
    	}
		else
    		Game.stateError(this, key, Game.NOT_LOG_IN);
	}

	@Override
	public void resolveState(int key, String version, byte[] state) {
		if (mHelper != null && mHelper.isSignedIn()) {
    		AppStateManager.resolve(mHelper.getApiClient(),key, version, state);
    	}
	}

	@Override
	public void deleteState(final int key) {
		if (mHelper != null && mHelper.isSignedIn()) {
    		PendingResult<StateDeletedResult> result = AppStateManager.delete(mHelper.getApiClient(), key);
    		ResultCallback<StateDeletedResult> mResultCallback = new
		            ResultCallback<StateDeletedResult>() {
		        @Override
		        public void onResult(StateDeletedResult result) {
		        	int statusCode = result.getStatus().getStatusCode();
		        	int stateKey = key;
		        	if (statusCode == AppStateStatusCodes.STATUS_OK) {
		        		Game.stateDeleted(this, stateKey);
		    		}
		    		else
		    		{
		    			String error = "unknown error";
		    			if(statusCode == AppStateStatusCodes.STATUS_CLIENT_RECONNECT_REQUIRED)
		    			{
		    				error = "need to reconnect client";
		    			}
		    			else if(statusCode == AppStateStatusCodes.STATUS_NETWORK_ERROR_OPERATION_FAILED)
		    			{
		    				error = "could not connect server";
		    			}
		    			Game.stateError(this, stateKey, error);
		    		}
		        }
		    };
    		result.setResultCallback(mResultCallback);
    	}
		else
    		Game.stateError(this, key, Game.NOT_LOG_IN);
	}
	
	private boolean isAvailable(){
    	int result = GooglePlayServicesUtil.isGooglePlayServicesAvailable(sActivity.get());
    	if(result == ConnectionResult.SUCCESS)
    	{
    		return true;
    	}
    	return false;
    }

	@Override
	public void onSignInFailed() {
		Game.loginError(this, "Could not sign in");
	}

	@Override
	public void onSignInSucceeded() {
		if(!signed)
		{
			signed = true;
			Game.loginComplete(this);
		}
	}
}
