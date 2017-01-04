package com.giderosmobile.android.plugins.gaming.frameworks;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

import com.giderosmobile.android.plugins.gaming.*;
import com.giderosmobile.android.plugins.gaming.frameworks.googleplay.GameHelper;
import com.giderosmobile.android.plugins.gaming.frameworks.googleplay.GameHelper.GameHelperListener;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
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
import com.google.android.gms.games.snapshot.Snapshot;
import com.google.android.gms.games.snapshot.SnapshotMetadataChange;
import com.google.android.gms.games.snapshot.Snapshots.CommitSnapshotResult;
import com.google.android.gms.games.snapshot.Snapshots.DeleteSnapshotResult;
import com.google.android.gms.games.snapshot.Snapshots.OpenSnapshotResult;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.SparseArray;

import static com.google.android.gms.games.Games.Snapshots;

public class GameGoogleplay implements GameInterface, GameHelperListener {
	private static WeakReference<Activity> sActivity;
	protected static GameHelper mHelper;
	protected static int mRequestedClients = GameHelper.CLIENT_DRIVE|GameHelper.CLIENT_GAMES|GameHelper.CLIENT_PLUS;
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
       	{
    		if (id!=null)
    			sActivity.get().startActivityForResult(Games.Leaderboards.getLeaderboardIntent(mHelper.getApiClient(), id), RC_UNUSED);
    		else
    			sActivity.get().startActivityForResult(Games.Leaderboards.getAllLeaderboardsIntent(mHelper.getApiClient()), RC_UNUSED);
       	}
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
					PendingResult<UpdateAchievementResult> result;
					if (numSteps>0)
						result = Games.Achievements.setStepsImmediate(mHelper.getApiClient(), id, numSteps);
					else if (numSteps<0)
						result = Games.Achievements.incrementImmediate(mHelper.getApiClient(), id, -numSteps);					
					else
						result = Games.Achievements.unlockImmediate(mHelper.getApiClient(), id);
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
					if (numSteps>0)
						Games.Achievements.setSteps(mHelper.getApiClient(), id, numSteps);
					else if (numSteps<0)
						Games.Achievements.increment(mHelper.getApiClient(), id, -numSteps);
					else
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
	public void revealAchievement(final String id, int immediate) {
		if(isAvailable())
		{
			if(mHelper != null && mHelper.isSignedIn())
			{
				if(immediate == 1)
				{
					PendingResult<UpdateAchievementResult> result;
					result = Games.Achievements.revealImmediate(mHelper.getApiClient(), id);
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
					Games.Achievements.reveal(mHelper.getApiClient(), id);
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

	Map<Integer,Snapshot> snapShots=new HashMap<Integer,Snapshot>();
	@Override
	public void loadState(final int key) {
		if (mHelper != null && mHelper.isSignedIn()) {
    		PendingResult<OpenSnapshotResult> result = Snapshots.open(mHelper.getApiClient(), ""+key,true);
    		ResultCallback<OpenSnapshotResult> mResultCallback = new
		            ResultCallback<OpenSnapshotResult>() {
		        @Override
		        public void onResult(OpenSnapshotResult result) {

		        	int statusCode = result.getStatus().getStatusCode();
		        	int stateKey = key;
		        	if (statusCode == GamesStatusCodes.STATUS_OK || statusCode == GamesStatusCodes.STATUS_SNAPSHOT_CONTENTS_UNAVAILABLE
							|| statusCode ==GamesStatusCodes.STATUS_SNAPSHOT_CONFLICT) {
						snapShots.put(stateKey,result.getSnapshot());
		        		if (statusCode ==GamesStatusCodes.STATUS_SNAPSHOT_CONFLICT)
						{
							try {
								Game.stateConflict(this, stateKey,
										result.getConflictId(),
										result.getConflictingSnapshot().getSnapshotContents().readFully(),
										result.getSnapshot().getSnapshotContents().readFully());
							} catch (IOException e) {
								Game.stateError(this, stateKey, "error while loading state");
							}
						}
						else
						{
							try {
								Game.stateLoaded(this, stateKey, result.getSnapshot().getSnapshotContents().readFully(),1);
							} catch (IOException e) {
								Game.stateError(this, stateKey, "error while loading state");
							}
						}
		    		} else {

	    				String error = "unknown error";
	    				if(statusCode == GamesStatusCodes.STATUS_CLIENT_RECONNECT_REQUIRED)
	    				{
	    					error = "need to reconnect client";
	    				}
	    				else if(statusCode == GamesStatusCodes.STATUS_NETWORK_ERROR_OPERATION_FAILED)
	    				{
	    					error = "could not connect server";
	    				}
	    				else if(statusCode == GamesStatusCodes.STATUS_NETWORK_ERROR_NO_DATA)
	    				{
	    					error = "could not connect server";
	    				}
	    				else if(statusCode == GamesStatusCodes.STATUS_SNAPSHOT_NOT_FOUND)
	    				{
	    					error = "no data";
	    				}
	    				else if(statusCode == GamesStatusCodes.STATUS_SNAPSHOT_CREATION_FAILED)
	    				{
	    					error = "creation failed";
	    				}
	    				else if(statusCode == GamesStatusCodes.STATUS_INTERNAL_ERROR)
	    				{
	    					error = "internal error";
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
				Snapshot s=snapShots.get(key);
				if (s==null) {
					loadState(key);
					s=snapShots.get(key);
					if (s==null) return;
				}
				s.getSnapshotContents().writeBytes(state);
				SnapshotMetadataChange metadataChange = new SnapshotMetadataChange.Builder().build();
			    PendingResult<CommitSnapshotResult> result = Snapshots.commitAndClose(mHelper.getApiClient(),s,metadataChange);
				snapShots.remove(key);

    			ResultCallback<CommitSnapshotResult> mResultCallback = new
    		            ResultCallback<CommitSnapshotResult>() {
    		        @Override
    		        public void onResult(CommitSnapshotResult result) {
    		        	int statusCode = result.getStatus().getStatusCode();
    		        	int stateKey = key;
    		        	if (statusCode == GamesStatusCodes.STATUS_OK || statusCode == GamesStatusCodes.STATUS_SNAPSHOT_CONFLICT) {
							//Conflicts will be dealt with on next 'load'
    		    		} else {
   		    				String error = "unknown error";
   		    				if(statusCode == GamesStatusCodes.STATUS_CLIENT_RECONNECT_REQUIRED)
   		    				{
   		    					error = "need to reconnect client";
   		    				}
   		    				else if(statusCode == GamesStatusCodes.STATUS_NETWORK_ERROR_OPERATION_FAILED)
   		    				{
   		    					error = "could not connect server";
   		    				}
   		    				else if(statusCode == GamesStatusCodes.STATUS_NETWORK_ERROR_NO_DATA)
   		    				{
   		    					error = "could not connect server";
   		    				}
   		    				else if(statusCode == GamesStatusCodes.STATUS_SNAPSHOT_NOT_FOUND)
   		    				{
   		    					error = "no data";
   		    				}
   		    				else if(statusCode == GamesStatusCodes.STATUS_INTERNAL_ERROR)
   		    				{
   		    					error = "internal error";
   		    				}
   		    				else if(statusCode == GamesStatusCodes.STATUS_SNAPSHOT_COMMIT_FAILED)
   		    				{
   		    					error = "commit failed";
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
	public void resolveState(int key, String version, byte[] state) {
		if (mHelper != null && mHelper.isSignedIn()) {
			Snapshot s=snapShots.get(key);
			if (s==null) {
				loadState(key);
				s=snapShots.get(key);
				if (s==null) return;
			}
			s.getSnapshotContents().writeBytes(state);
			Games.Snapshots.resolveConflict(mHelper.getApiClient(), version, s);
			snapShots.remove(key);
    	}
	}

	@Override
	public void deleteState(final int key) {
		if (mHelper != null && mHelper.isSignedIn()) {
			Snapshot s=snapShots.get(key);
			if (s==null) {
				loadState(key);
				s=snapShots.get(key);
				if (s==null) return;
			}
    		PendingResult<DeleteSnapshotResult> result = Snapshots.delete(mHelper.getApiClient(), s.getMetadata());
			snapShots.remove(key);
    		ResultCallback<DeleteSnapshotResult> mResultCallback = new
		            ResultCallback<DeleteSnapshotResult>() {
		        @Override
		        public void onResult(DeleteSnapshotResult result) {
		        	int statusCode = result.getStatus().getStatusCode();
		        	int stateKey = key;
		        	if (statusCode == GamesStatusCodes.STATUS_OK || statusCode == GamesStatusCodes.STATUS_SNAPSHOT_NOT_FOUND) {
		        		Game.stateDeleted(this, stateKey);
		    		}
		    		else
		    		{
		    			String error = "unknown error";
		    			if(statusCode == GamesStatusCodes.STATUS_CLIENT_RECONNECT_REQUIRED)
		    			{
		    				error = "need to reconnect client";
		    			}
		    			else if(statusCode == GamesStatusCodes.STATUS_NETWORK_ERROR_OPERATION_FAILED)
		    			{
		    				error = "could not connect server";
		    			}
						else if(statusCode == GamesStatusCodes.STATUS_INTERNAL_ERROR)
						{
							error = "internal error";
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
    	int result = GoogleApiAvailability.getInstance().isGooglePlayServicesAvailable(sActivity.get());
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
