package com.giderosmobile.android.plugins.gaming.frameworks;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.giderosmobile.android.plugins.gaming.*;
import com.google.android.gms.auth.api.signin.*;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.drive.Drive;
import com.google.android.gms.games.AnnotatedData;
import com.google.android.gms.games.Games;
import com.google.android.gms.games.GamesCallbackStatusCodes;
import com.google.android.gms.games.GamesClient;
import com.google.android.gms.games.InvitationsClient;
import com.google.android.gms.games.RealTimeMultiplayerClient;
import com.google.android.gms.games.SnapshotsClient;
import com.google.android.gms.games.AchievementsClient;
import com.google.android.gms.games.LeaderboardsClient;
import com.google.android.gms.games.PlayersClient;
import com.google.android.gms.games.Player;
import com.google.android.gms.games.achievement.Achievement;
import com.google.android.gms.games.achievement.AchievementBuffer;
import com.google.android.gms.games.leaderboard.LeaderboardScore;
import com.google.android.gms.games.leaderboard.LeaderboardScoreBuffer;
import com.google.android.gms.games.leaderboard.ScoreSubmissionData;
import com.google.android.gms.games.multiplayer.Invitation;
import com.google.android.gms.games.multiplayer.InvitationCallback;
import com.google.android.gms.games.multiplayer.Multiplayer;
import com.google.android.gms.games.multiplayer.Participant;
import com.google.android.gms.games.multiplayer.realtime.OnRealTimeMessageReceivedListener;
import com.google.android.gms.games.multiplayer.realtime.RealTimeMessage;
import com.google.android.gms.games.multiplayer.realtime.Room;
import com.google.android.gms.games.multiplayer.realtime.RoomConfig;
import com.google.android.gms.games.multiplayer.realtime.RoomStatusUpdateCallback;
import com.google.android.gms.games.multiplayer.realtime.RoomUpdateCallback;
import com.google.android.gms.games.snapshot.Snapshot;
import com.google.android.gms.games.snapshot.SnapshotMetadata;
import com.google.android.gms.games.snapshot.SnapshotMetadataChange;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.util.SparseArray;

public class GameGoogleplay implements GameInterface {
	private static WeakReference<Activity> sActivity;
	protected GoogleSignInClient mSignInClient;
	protected GoogleSignInAccount mAccount;
	final static int RC_UNUSED = 9002;
	final static int RC_SIGN_IN = 9000;
	boolean signed = false;
	GameGoogleplay me;
	
	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity =  activity;
		me = this;
		signed = false;
		if(isAvailable())
		{
			GoogleSignInOptions options = new GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_SIGN_IN)
					.requestScopes(Drive.SCOPE_APPFOLDER)
					.requestScopes(Games.SCOPE_GAMES)
					.build();

			mSignInClient = GoogleSignIn.getClient(sActivity.get(), options);
		}
	}

	@Override
	public void onDestroy() {}

	@Override
	public void onStart() {
		mAccount = GoogleSignIn.getLastSignedInAccount(sActivity.get());
	}

	@Override
	public void onStop() {
	}

	@Override
	public void onPause() {}

	@Override
	public void onResume()
	{
	}

	@Override
	public void login(Object parameters) {
		if ((mSignInClient!=null)&&(mAccount==null))
	    {
			mSignInClient.silentSignIn().addOnCompleteListener(sActivity.get(),
				new OnCompleteListener<GoogleSignInAccount>() {
				@Override
				public void onComplete(@NonNull Task<GoogleSignInAccount> task) {
					if (task.isSuccessful()) {
						mAccount = task.getResult();
						onSignInSucceeded();
					} else {
						Intent signInIntent = mSignInClient.getSignInIntent();
						sActivity.get().startActivityForResult(signInIntent, RC_SIGN_IN);
					}
				}
			});
	    }
	}
	
	@Override
	public void logout() {
		signed = false;
       	if(mSignInClient != null && (mAccount!=null))
       	{
       		mAccount=null;
       		mSignInClient.signOut();
        }
	}

	@Override
	public void onActivityResult(int request, int response, Intent data) {
		if (request == RC_SIGN_IN) {
			// The Task returned from this call is always completed, no need to attach
			// a listener.
			Task<GoogleSignInAccount> task = GoogleSignIn.getSignedInAccountFromIntent(data);
			if (task.isSuccessful())
			{
				mAccount=task.getResult();
				onSignInSucceeded();
			}
			else
				onSignInFailed();
		}
	}

	@Override
	public void showLeaderboard(final String id) {
       	if(mAccount != null)
       	{
       		Task<Intent> t;
    		if (id!=null)
    			t=gLeaderboards.getLeaderboardIntent(id);
    		else
    			t=gLeaderboards.getAllLeaderboardsIntent();
    		t.addOnCompleteListener(sActivity.get(), new OnCompleteListener<Intent>() {
				@Override
				public void onComplete(@NonNull Task<Intent> task) {
					if (task.isSuccessful())
						sActivity.get().startActivityForResult(task.getResult(), RC_UNUSED);
					else
					{
						//XXX Report an error here ?
					}
				}
			});
       	}
	}

	@Override
	public void reportScore(final String id, final long score, int immediate) {
		if(isAvailable())
		{
			if(mAccount != null)
	    	{
	    		if(immediate == 1)
	    		{
	    			gLeaderboards.submitScoreImmediate(id, score).addOnCompleteListener(sActivity.get(), new OnCompleteListener<ScoreSubmissionData>() {
						@Override
						public void onComplete(@NonNull Task<ScoreSubmissionData> task) {
							if (task.isSuccessful()) {
								Game.reportScoreComplete(me, id, score);
							}
							else
							{
								Game.reportScoreError(me, id, score, getTaskError(task));
							}
						}
					});
	    		}
	    		else
	    		{
	    			gLeaderboards.submitScore(id, score);
	    		}
	    	}
	       	else
	    		Game.reportScoreError(this, id, score, Game.NOT_LOG_IN);
		}
		else
			Game.reportScoreError(this, id, score, Game.LIBRARY_NOT_FOUND);
	}

	@Override
	public void getPlayerInfo() {
		if(isAvailable())
		{
			if(mAccount != null)
	    	{
    			gPlayers.getCurrentPlayer().addOnCompleteListener(sActivity.get(), new OnCompleteListener<Player>() {
					@Override
					public void onComplete(@NonNull Task<Player> task) {
						if (task.isSuccessful())
						{
							Player p=task.getResult();
							Game.playerInfoComplete(this,p.getPlayerId(),p.getDisplayName(),(p.hasHiResImage()?p.getHiResImageUri():p.getIconImageUri()).toString());
						}
						else
							Game.playerInfoError(this, getTaskError(task));
					}
				});
	    	}
	       	else
	    		Game.playerInfoError(this, Game.NOT_LOG_IN);
		}
		else
			Game.playerInfoError(this, Game.LIBRARY_NOT_FOUND);
	}

	@Override
	public void showAchievements() {
       	if(mAccount != null)
			gAchievements.getAchievementsIntent().addOnCompleteListener(sActivity.get(), new OnCompleteListener<Intent>() {
				@Override
				public void onComplete(@NonNull Task<Intent> task) {
					if (task.isSuccessful()) {
						sActivity.get().startActivityForResult(task.getResult(), RC_UNUSED);
					}
					else
					{
						Game.reportAchievementError(me, "", task.getException().toString());
					}
				}
			});
	}

	private String getTaskError(Task<?> task) {
		return task.getException().toString();
	}

	@Override
	public void reportAchievement(final String id, int numSteps, int immediate) {
		if(isAvailable())
		{
			if(mAccount != null)
			{
				if(immediate == 1)
				{
					Task result;
					if (numSteps>0)
						result = gAchievements.setStepsImmediate(id, numSteps);
					else if (numSteps<0)
						result = gAchievements.incrementImmediate(id, -numSteps);
					else
						result = gAchievements.unlockImmediate(id);
					result.addOnCompleteListener(sActivity.get(), new OnCompleteListener() {
						@Override
						public void onComplete(@NonNull Task task) {
							if (task.isSuccessful()) {
								Game.reportAchievementComplete(me, id);
							}
							else
							{
								Game.reportAchievementError(me, id, getTaskError(task));
							}
						}
					});
				}
				else
				{
					if (numSteps>0)
						gAchievements.setSteps(id, numSteps);
					else if (numSteps<0)
						gAchievements.increment(id, -numSteps);
					else
						gAchievements.unlock(id);
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
			if(mAccount != null)
			{
				if(immediate == 1)
				{
					gAchievements.revealImmediate(id).addOnCompleteListener(sActivity.get(), new OnCompleteListener<Void>() {
								@Override
								public void onComplete(@NonNull Task<Void> task) {
									if (task.isSuccessful()) {
										Game.reportAchievementComplete(me, id);
									} else {
										Game.reportAchievementError(me, id, getTaskError(task));
									}
								}
							});
				}
				else
					gAchievements.reveal(id);
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
			if(mAccount != null)
	    	{
	    		gAchievements.load(true).addOnCompleteListener(sActivity.get(), new OnCompleteListener<AnnotatedData<AchievementBuffer>>() {
					@Override
					public void onComplete(@NonNull Task<AnnotatedData<AchievementBuffer>> task) {
						if (task.isSuccessful()) {
							SparseArray<Bundle> arr = new SparseArray<Bundle>();
							AchievementBuffer buffer = task.getResult().get();
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
							buffer.release();
						}
						else
						{
							Game.loadAchievementsError(me, getTaskError(task));
						}
					}
				});
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
			if(mAccount != null)
	    	{
	    		gLeaderboards.loadTopScores(id, span, collection, maxResults).addOnCompleteListener(sActivity.get(), new OnCompleteListener<AnnotatedData<LeaderboardsClient.LeaderboardScores>>() {
					@Override
					public void onComplete(@NonNull Task<AnnotatedData<LeaderboardsClient.LeaderboardScores>> task) {
						if(task.isSuccessful()){
							LeaderboardsClient.LeaderboardScores ls=task.getResult().get();
							String leaderboardId =  ls.getLeaderboard().getLeaderboardId();
							String leaderboardName = ls.getLeaderboard().getDisplayName();
							LeaderboardScoreBuffer scores = ls.getScores();
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
							ls.release();
						}
						else
						{
							Game.loadScoresError(this, id, getTaskError(task));
						}
					}
				});
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
		if (mAccount != null) {
			gSnapshots.open("" + key, true).addOnCompleteListener(sActivity.get(), new OnCompleteListener<SnapshotsClient.DataOrConflict<Snapshot>>() {
				@Override
				public void onComplete(@NonNull Task<SnapshotsClient.DataOrConflict<Snapshot>> task) {
					if (!task.isSuccessful()) {
						Game.stateError(this, key, getTaskError(task));
					} else {
						SnapshotsClient.DataOrConflict<Snapshot> ds = task.getResult();
						if (ds.isConflict()) {
							SnapshotsClient.SnapshotConflict sc = ds.getConflict();
							snapShots.put(key, sc.getSnapshot());
							try {
								Game.stateConflict(this, key,
										sc.getConflictId(),
										sc.getConflictingSnapshot().getSnapshotContents().readFully(),
										sc.getSnapshot().getSnapshotContents().readFully());
							} catch (IOException e) {
								Game.stateError(this, key, "error while loading state");
							}
						} else {
							snapShots.put(key, ds.getData());
							try {
								Game.stateLoaded(this, key, ds.getData().getSnapshotContents().readFully(), 1);
							} catch (IOException e) {
								Game.stateError(this, key, "error while loading state");
							}
						}
					}
				}
			});
		}
		else
    		Game.stateError(this, key, Game.NOT_LOG_IN);
	}

	@Override
	public void updateState(final int key, byte[] state, int immediate) {
		if (mAccount != null) {
				Snapshot s=snapShots.get(key);
				if (s==null) {
					loadState(key);
					s=snapShots.get(key);
					if (s==null) return;
				}
				s.getSnapshotContents().writeBytes(state);
				SnapshotMetadataChange metadataChange = new SnapshotMetadataChange.Builder().build();
			    gSnapshots.commitAndClose(s,metadataChange).addOnCompleteListener(sActivity.get(), new OnCompleteListener<SnapshotMetadata>() {
					@Override
					public void onComplete(@NonNull Task<SnapshotMetadata> task) {
						if (task.isSuccessful()) {
						}
						else {
							Game.stateError(this, key, "invalid state on commit");
						}
					}
				});
			snapShots.remove(key);
    	}
		else
    		Game.stateError(this, key, Game.NOT_LOG_IN);
	}

	@Override
	public void resolveState(final int key, String version, byte[] state) {
		if (mAccount != null) {
			Snapshot s=snapShots.get(key);
			if (s==null) {
				loadState(key);
				s=snapShots.get(key);
				if (s==null) return;
			}
			s.getSnapshotContents().writeBytes(state);
			gSnapshots.resolveConflict(version, s).addOnCompleteListener(sActivity.get(), new OnCompleteListener<SnapshotsClient.DataOrConflict<Snapshot>>() {
				@Override
				public void onComplete(@NonNull Task<SnapshotsClient.DataOrConflict<Snapshot>> task) {
					if (!task.isSuccessful()) {
						Game.stateError(this, key, getTaskError(task));
					} else {
						SnapshotsClient.DataOrConflict<Snapshot> ds = task.getResult();
						if (ds.isConflict()) {
							SnapshotsClient.SnapshotConflict sc = ds.getConflict();
							snapShots.put(key, sc.getSnapshot());
							try {
								Game.stateConflict(this, key,
										sc.getConflictId(),
										sc.getConflictingSnapshot().getSnapshotContents().readFully(),
										sc.getSnapshot().getSnapshotContents().readFully());
							} catch (Exception e) {
								Game.stateError(this, key, "error while resolving conflict");
							}
						} else {
							snapShots.put(key, ds.getData());
							try {
								Game.stateLoaded(this, key, ds.getData().getSnapshotContents().readFully(), 1);
							} catch (Exception e) {
								Game.stateError(this, key, "error while resolving conflict");
							}
						}
					}
				}
			});
			snapShots.remove(key);
    	}
	}

	@Override
	public void deleteState(final int key) {
		if (mAccount != null) {
			Snapshot s=snapShots.get(key);
			if (s==null) {
				loadState(key);
				s=snapShots.get(key);
				if (s==null) return;
			}
			snapShots.remove(key);
			gSnapshots.delete(s.getMetadata()).addOnCompleteListener(sActivity.get(), new OnCompleteListener<String>() {
						@Override
						public void onComplete(@NonNull Task<String> task) {
							if (!task.isSuccessful()) {
								Game.stateError(this, key, getTaskError(task));
							} else {
								Game.stateDeleted(this, key);
							}
						}
					});
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

	private void onSignInFailed() {
		Game.loginError(this, "Could not sign in");		
	}
	
	private void onLicenseFailed() {
		Game.loginError(this, "No license.");		
	}

	public SnapshotsClient gSnapshots;
	public LeaderboardsClient gLeaderboards;
	public AchievementsClient gAchievements;
	public RealTimeMultiplayerClient gMultiplayer;
	public PlayersClient gPlayers;
	public InvitationsClient gInvitations;
	public GamesClient gGames;

	public Player currentPlayer;
	
	private void onSignInSucceeded() {
		if(!signed)
		{
			signed = true;
			if(mAccount!=null)
			{
				GoogleSignInAccount sa=mAccount;
				gSnapshots=Games.getSnapshotsClient(sActivity.get(), sa);
				gLeaderboards=Games.getLeaderboardsClient(sActivity.get(), sa);
				gAchievements=Games.getAchievementsClient(sActivity.get(), sa);
				gMultiplayer=Games.getRealTimeMultiplayerClient(sActivity.get(), sa);
				gPlayers=Games.getPlayersClient(sActivity.get(), sa);
				gInvitations=Games.getInvitationsClient(sActivity.get(),sa);
				gInvitations.registerInvitationCallback(invitationListener);
				gGames=Games.getGamesClient(sActivity.get(),sa);
				gGames.getActivationHint().addOnSuccessListener(
					new OnSuccessListener<Bundle>() {
						@Override
						public void onSuccess(Bundle bundle) {
							if (bundle!=null) {
								Invitation invitation = bundle.getParcelable(Multiplayer.EXTRA_INVITATION);
								if (invitation != null)
									Game.invitationReceived(this, invitation.getInvitationId());
							}
						}
					});

				gPlayers.getCurrentPlayer().addOnCompleteListener(sActivity.get(), new OnCompleteListener<Player>() {
					@Override
					public void onComplete(@NonNull Task<Player> task) {
						if (task.isSuccessful())
							currentPlayer=task.getResult();
						Game.loginComplete(this);
					}
				});
			}
			else
				Game.loginComplete(this);
		}
	}
	
   /*MULTIPLAYER STUFF*/
    private Room currentRoom;
	private boolean mWaitingRoomFinishedFromCode = false;
	// The participants in the currently active game
	private ArrayList<Participant> mParticipants = null;
	// My participant ID in the currently active game
	private String mMyId = null;
	final static int RC_INVITATION_INBOX = 10001;
	final static int RC_SELECT_PLAYERS = 10000;
	final static int RC_WAITING_ROOM = 10002;
	@Override
    public void autoMatch(int minPlayers, int maxPlayers) {
    	if(mAccount != null)
    	{
    		// automatch criteria to invite 1 random automatch opponent.  
    		// You can also specify more opponents (up to 3). 
    		Bundle am = RoomConfig.createAutoMatchCriteria(minPlayers, maxPlayers, 0);

    		// build the room config:
			RoomConfig.Builder roomConfigBuilder = RoomConfig.builder(roomUpdateCallback)
					.setOnMessageReceivedListener(messageReceivedListener)
					.setRoomStatusUpdateCallback(roomStatusUpdateCallback);
    		roomConfigBuilder.setAutoMatchCriteria(am);
    		RoomConfig roomConfig = roomConfigBuilder.build();

    		// create room:
    		gMultiplayer.create(roomConfig);
    	}

    }

    private void runTaskIntent(Task<Intent> ti,final int rc) {
		ti.addOnSuccessListener(sActivity.get(), new OnSuccessListener<Intent>() {
			@Override
			public void onSuccess(Intent intent) {
				sActivity.get().startActivityForResult(intent,rc);
			}
		});
	}

	@Override
    public void invitePlayers(final int minPlayers, final int maxPlayers){
		if (mAccount!=null)
       		runTaskIntent(gMultiplayer.getSelectOpponentsIntent(minPlayers, maxPlayers), RC_SELECT_PLAYERS);
    }
    
	@Override
    public void joinRoom(String id){
    	if(mAccount != null)
    	{
    		RoomConfig.Builder roomConfigBuilder = RoomConfig.builder(roomUpdateCallback)
                .setOnMessageReceivedListener(messageReceivedListener)
                .setRoomStatusUpdateCallback(roomStatusUpdateCallback);
    		roomConfigBuilder.setInvitationIdToAccept(id);
    		gMultiplayer.join(roomConfigBuilder.build());
    	}
    }
    
    public void showInvitations(){
		if (mAccount!=null)
          		runTaskIntent(gInvitations.getInvitationInboxIntent(), RC_WAITING_ROOM);
    }
    
	@Override
    public void showWaitingRoom(final int minPlayers){
		if (mAccount!=null)
           		runTaskIntent(gMultiplayer.getWaitingRoomIntent(currentRoom, minPlayers), RC_WAITING_ROOM);
    }
    
	@Override
    public void sendTo(String id, byte[] message, int isReliable){
    	if(mAccount != null)
    	{
    		if(isReliable == 1){
    			gMultiplayer.sendReliableMessage(message, currentRoom.getRoomId(), id,null);
    		}
    		else
    		{
				gMultiplayer.sendUnreliableMessage(message, currentRoom.getRoomId(), id);
    		}
    	}
    }
    
	@Override
    public void sendToAll(byte[] message, int isReliable){
    	if(mAccount != null)
    	{
    		if(isReliable == 1){
    			for (Participant p : mParticipants) {
    				if (!p.getParticipantId().equals(mMyId)) {
    					gMultiplayer.sendReliableMessage(message, currentRoom.getRoomId(), p.getParticipantId(),null);
    				}
    			}
    		}
    		else
    		{
    			List<String> receipients = new ArrayList<String>();
    			for (Participant p : mParticipants) {
    				if (!p.getParticipantId().equals(mMyId)) {
    					receipients.add(p.getParticipantId());
    				}
    			}
    			gMultiplayer.sendUnreliableMessage(message, currentRoom.getRoomId(), receipients);
    		}
    	}
    }
    
	@Override
    public String getCurrentPlayer(){
    	if(currentPlayer!=null)
    		return currentPlayer.getDisplayName();
    	return "";
    }
    
	@Override
    public String getCurrentPlayerId(){
		if(currentPlayer!=null)
			return currentPlayer.getPlayerId();
    	return "";
    }
    
	@Override
    public String getPlayerPicture(String id, int highRes){
    	if(currentPlayer!=null)
    	{
    		if(highRes == 0 && currentPlayer.hasHiResImage())
    			return currentPlayer.getHiResImageUri().toString();
    		if(currentPlayer.hasIconImage())
    			return currentPlayer.getIconImageUri().toString();
    	}
    	return "";
    }
    
	@Override
    public void getCurrentPlayerScore(String leaderboardId, int span, int leaderboardCollection){
    	if(mAccount != null)
    	{
    		gLeaderboards.loadCurrentPlayerLeaderboardScore(leaderboardId, span, leaderboardCollection).addOnCompleteListener(sActivity.get(), new OnCompleteListener<AnnotatedData<LeaderboardScore>>() {
				@Override
				public void onComplete(@NonNull Task<AnnotatedData<LeaderboardScore>> task) {
					if (task.isSuccessful()) {
						LeaderboardScore score = task.getResult().get();
						if(score != null)
							Game.playerScore(me,score.getDisplayRank(), score.getDisplayScore(), score.getRawScore(), (int)(score.getTimestampMillis()/1000));
						else
							Game.playerScore(me,"0", "0", 0, 0);
					}
				}
			});
    	}
    }
    
	@Override
    public Object getAllPlayers(){
    	SparseArray<Bundle> arr = new SparseArray<Bundle>();
    	if(mParticipants != null )
    	{
    		int i = 1;
    		for (Participant p : mParticipants) {
        		Bundle bundle = new Bundle();
        		bundle.putString("id", p.getParticipantId());
        		bundle.putString("name", p.getDisplayName());
    			arr.put(i, bundle);
    			i++;
    		}
    	}
    	return arr;
    }

    public void updateRoom(Room room) {
        mParticipants = room.getParticipants();
    }

    private InvitationCallback invitationListener=new InvitationCallback() {
		@Override
		public void onInvitationReceived(Invitation invitation) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/Invitation.html
			Game.invitationReceived(me,invitation.getInvitationId());
		}

		@Override
		public void onInvitationRemoved(String invitationId) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/Invitation.html
			//Game.invitationReceived(invitation.getInvitationId()); XXX TODO
		}
	};

	public RoomUpdateCallback roomUpdateCallback=new RoomUpdateCallback() {
		@Override
		public void onJoinedRoom(int statusCode, Room room) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/realtime/RoomUpdateListener.html
			if (statusCode != GamesCallbackStatusCodes.OK) {
				// display error
				return;
			}
			currentRoom = room;
			updateRoom(room);
			Game.joinedRoom(me,room.getRoomId());
		}

		@Override
		public void onLeftRoom(int statusCode, String roomId) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/realtime/RoomUpdateListener.html
			if (statusCode != GamesCallbackStatusCodes.OK) {
				// display error
				return;
			}
			currentRoom = null;
			Game.leftRoom(me,roomId);
		}

		@Override
		public void onRoomConnected(int statusCode, Room room) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/realtime/RoomUpdateListener.html
			if (statusCode != GamesCallbackStatusCodes.OK) {
				return;
			}
			updateRoom(room);
			Game.roomConnected(me,room.getRoomId());
		}

		@Override
		public void onRoomCreated(int statusCode, Room room) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/realtime/RoomUpdateListener.html
			if (statusCode != GamesCallbackStatusCodes.OK) {
				// display error
				return;
			}
			currentRoom = room;
			updateRoom(room);
			Game.roomCreated(me,room.getRoomId());
		}
	};

	public RoomStatusUpdateCallback roomStatusUpdateCallback=new RoomStatusUpdateCallback() {
		@Override
		public void onConnectedToRoom(Room room) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/realtime/RoomStatusUpdateListener.html
			mParticipants = room.getParticipants();
			mMyId = room.getParticipantId(currentPlayer.getPlayerId());
			Game.connectedToRoom(me,room.getRoomId());
		}

		@Override
		public void onDisconnectedFromRoom(Room room) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/realtime/RoomStatusUpdateListener.html
			RoomConfig.Builder roomConfigBuilder = RoomConfig.builder(roomUpdateCallback)
					.setOnMessageReceivedListener(messageReceivedListener)
					.setRoomStatusUpdateCallback(roomStatusUpdateCallback);
			gMultiplayer.leave(roomConfigBuilder.build(),room.getRoomId());
			currentRoom = null;
			Game.disconnectedFromRoom(me,room.getRoomId());
		}

		@Override
		public void onPeerDeclined(Room arg0, List<String> arg1) {
			updateRoom(arg0);
			Game.peerDeclined(me);
		}

		@Override
		public void onPeerInvitedToRoom(Room arg0, List<String> arg1) {
			updateRoom(arg0);
			Game.peerInvitedToRoom(me);
		}

		@Override
		public void onPeerJoined(Room arg0, List<String> arg1) {
			updateRoom(arg0);
			Game.peerJoined(me);
		}

		@Override
		public void onPeerLeft(Room arg0, List<String> arg1) {
			updateRoom(arg0);
			Game.peerLeft(me);
		}

		@Override
		public void onPeersConnected(Room arg0, List<String> arg1) {
			updateRoom(arg0);
			Game.peersConnected(me);
		}

		@Override
		public void onPeersDisconnected(Room arg0, List<String> arg1) {
			updateRoom(arg0);
			Game.peersDisconnected(me);
		}

		@Override
		public void onRoomAutoMatching(Room room) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/realtime/RoomStatusUpdateListener.html
			updateRoom(room);
			Game.roomAutoMatching(me, room.getRoomId());
		}

		@Override
		public void onRoomConnecting(Room room) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/realtime/RoomStatusUpdateListener.html
			updateRoom(room);
			Game.roomConnecting(me,room.getRoomId());
		}

		@Override
		public void onP2PConnected(@NonNull String s) {

		}

		@Override
		public void onP2PDisconnected(@NonNull String s) {

		}
	};

	private OnRealTimeMessageReceivedListener messageReceivedListener=new OnRealTimeMessageReceivedListener() {
		@Override
		public void onRealTimeMessageReceived(RealTimeMessage rtm) {
			//http://developer.android.com/reference/com/google/android/gms/games/multiplayer/realtime/RealTimeMessageReceivedListener.html
			// get real-time message
			byte[] b = rtm.getMessageData();
			String message = b.toString();
			if (message.equals("__S")) {
				// someone else started to play -- so dismiss the waiting room and
				// get right to it!
				mWaitingRoomFinishedFromCode = true;
				sActivity.get().finishActivity(RC_WAITING_ROOM);
				Game.gameStarted(me);
			}
			else
			{
				String sender = rtm.getSenderParticipantId();
				Game.dataReceived(me,b, sender);
			}
		}
	};


}
