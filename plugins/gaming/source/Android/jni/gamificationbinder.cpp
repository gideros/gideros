#include "gamification.h"
#include <map>
#include <string>

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

static lua_State *L = NULL;

static void luaL_newweaktable(lua_State *L, const char *mode)
{
	lua_newtable(L);			// create table for instance list
	lua_pushstring(L, mode);
	lua_setfield(L, -2, "__mode");	  // set as weak-value table
	lua_pushvalue(L, -1);             // duplicate table
	lua_setmetatable(L, -2);          // set itself as metatable
}

static void luaL_rawgetptr(lua_State *L, int idx, void *ptr)
{
	idx = abs_index(L, idx);
	lua_pushlightuserdata(L, ptr);
	lua_rawget(L, idx);
}

static void luaL_rawsetptr(lua_State *L, int idx, void *ptr)
{
	idx = abs_index(L, idx);
	lua_pushlightuserdata(L, ptr);
	lua_insert(L, -2);
	lua_rawset(L, idx);
}

static std::map<std::string, std::string> tableToMap(lua_State *L, int index)
{
    luaL_checktype(L, index, LUA_TTABLE);
    
    std::map<std::string, std::string> result;
    
    int t = abs_index(L, index);
    
	lua_pushnil(L);
	while (lua_next(L, t) != 0)
	{
		lua_pushvalue(L, -2);
        std::string key = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		
        std::string value = luaL_checkstring(L, -1);
		
		result[key] = value;
		
		lua_pop(L, 1);
	}
    
    return result;
}
static const char *LOGIN_COMPLETE = "loginComplete";
static const char *LOGIN_ERROR = "loginError";

static const char *PLAYER_INFORMATION_COMPLETE = "playerInformationComplete";
static const char *PLAYER_INFORMATION_ERROR = "playerInformationError";
static const char *PLAYER_SCORE_COMPLETE = "playerScoreComplete";
static const char *PLAYER_SCORE_ERROR = "playerScoreError";

static const char *LOAD_ACHIEVEMENTS_COMPLETE = "loadAchievementsComplete";
static const char *LOAD_ACHIEVEMENTS_ERROR = "loadAchievementsError";
static const char *REPORT_ACHIEVEMENT_COMPLETE = "reportAchievementComplete";
static const char *REPORT_ACHIEVEMENT_ERROR = "reportAchievementError";
static const char *LOAD_SCORES_COMPLETE = "loadScoresComplete";
static const char *LOAD_SCORES_ERROR = "loadScoresComplete";
static const char *REPORT_SCORE_COMPLETE = "reportScoreComplete";
static const char *REPORT_SCORE_ERROR = "reportScoreError";
static const char* STATE_LOADED = "stateLoaded";
static const char* STATE_ERROR = "stateError";
static const char* STATE_CONFLICT = "stateConflict";
static const char* STATE_DELETED = "stateDeleted";
static const char* GAME_STARTED = "gameStarted";
static const char* INVITATION_RECEIVED = "invitationReceived";
static const char* JOINED_ROOM = "joinedRoom";
static const char* LEFT_ROOM = "leftRoom";
static const char* ROOM_CONNECTED = "roomConnected";
static const char* ROOM_CREATED = "roomCreated";
static const char* CONNECTED_TO_ROOM = "conntectedToRoom";
static const char* DISCONNECTED_FROM_ROOM = "disconntectedFromRoom";
static const char* PEER_DECLINED = "peerDeclined";
static const char* PEER_INVITED = "peerInvited";
static const char* PEER_JOINED = "peerJoined";
static const char* PEER_LEFT = "peerLeft";
static const char* PEER_CONNECTED = "peerConnected";
static const char* PEER_DISCONNECTED = "peerDisconnected";
static const char* ROOM_AUTO_MATCHING = "roomAutoMatching";
static const char* ROOM_CONNECTING = "roomCommenting";
static const char* DATA_RECEIVED = "dataReceived";
static const char* PLAYER_SCORES_COMPLETE = "playerScoresComplete";


static const int TODAY = 0;
static const int WEEK = 1;
static const int ALL_TIME = 2;

static const int FRIENDS = 0;
static const int ALL_PLAYERS = 1;

static const int UNLOCKED = 0;
static const int REVEALED = 1;
static const int HIDDEN = 2;

static char keyWeak = ' ';

class Game : public GEventDispatcherProxy
{
public:
    Game(const char *type)
    {
		type_ = strdup(type);
        game_initialize(type);
		game_addCallback(callback_s, this);		
    }
    
    ~Game()
    {
		game_destroy(type_);
		game_removeCallback(callback_s, this);
		free((char*)type_);
    }
	
	void login(game_Parameter *params)
	{
		game_login(type_, params);
	}
	
	void logout()
	{
		game_logout(type_);
	}
	
	void showAchievements()
	{
		game_showAchievements(type_);
	}
	
	void getPlayerInfo()
	{
		game_getPlayerInfo(type_);
	}

	void showLeaderboard(const char *id)
	{
		game_showLeaderboard(type_, id);
	}
	
	void reportScore(const char *id, long score, int immediate)
	{
		game_reportScore(type_, id, score, immediate);
	}
	
	void reportAchievement(const char *id, double steps, int immediate)
	{
		game_reportAchievement(type_, id, steps, immediate);
	}

	void revealAchievement(const char *id, int immediate)
	{
		game_revealAchievement(type_, id, immediate);
	}

	void loadAchievements()
	{
		game_loadAchievements(type_);
	}
	
	void loadScores(const char *id, int span, int collection, int maxResults)
	{
		game_loadScores(type_, id, span, collection, maxResults);
	}
	
	void loadPlayerScores(const char *id, int span, int collection, int maxResults)
	{
		game_loadPlayerScores(type_, id, span, collection, maxResults);
	}
	
	void loadState(int key)
	{
		game_loadState(type_, key);
	}
	
	void updateState(int key, const void* data, size_t size, int immediate)
	{
		game_updateState(type_, key, data, size, immediate);
	}
	
	void resolveState(int key, const char* ver, const void* data, size_t size)
	{
		game_resolveState(type_, key, ver, data, size);
	}
	
	void deleteState(int key)
	{
		game_deleteState(type_, key);
	}

	void autoMatch(int minPlayers, int maxPlayers)
	{
		game_autoMatch(type_, minPlayers, maxPlayers);
	}
	
	void invitePlayers(int minPlayers, int maxPlayers)
	{
		game_invitePlayers(type_, minPlayers, maxPlayers);
	}
	
	void joinRoom(const char* id)
	{
		game_joinRoom(type_, id);
	}
	
	void showInvitations()
	{
		game_showInvitations(type_);
	}
	
	void showWaitingRoom(int minPlayers)
	{
		game_showWaitingRoom(type_, minPlayers);
	}
	
	void sendTo(const char* id, const void* data, size_t size, int isReliable)
	{
		game_sendTo(type_, id, data, size, isReliable);
	}
	
	void sendToAll(const void* data, size_t size, int isReliable)
	{
		game_sendToAll(type_, data, size, isReliable);
	}
	
	const char* getCurrentPlayer()
	{
		return game_getCurrentPlayer(type_);
	}
	
	const char* getCurrentPlayerId()
	{
		return game_getCurrentPlayerId(type_);
	}
	
	const char* getCurrentPicture(int highRes)
	{
		return game_getCurrentPicture(type_, highRes);
	}
	
	void getCurrentScore(const char *id, int span, int collection)
	{
		game_getCurrentScore(type_, id, span, collection);
	}
	
	game_PlayerEntry* getAllPlayers()
	{
		return game_getAllPlayers(type_);
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		static_cast<Game*>(udata)->callback(type, event);
	}
	
	void callback(int type, void *event)
	{
        dispatchEvent(type, event);
	}
	
	void dispatchEvent(int type, void *event)
	{
		if(L != NULL)
		{
			int shouldDispatch = 0;
			if (type == GAME_REPORT_ACHIEVEMENT_COMPLETE_EVENT || type == GAME_LOAD_ACHIEVEMENTS_ERROR_EVENT || type == GAME_LOGIN_ERROR_EVENT || type == GAME_PLAYER_INFORMATION_ERROR_EVENT)
			{
				game_Report *event2 = (game_Report*)event;
				if(strcmp(event2->caller, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type == GAME_REPORT_SCORE_COMPLETE_EVENT)
			{
				game_ReportScore *event2 = (game_ReportScore*)event;
				if(strcmp(event2->caller, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type == GAME_REPORT_SCORE_ERROR_EVENT)
			{
				game_ReportScoreError *event2 = (game_ReportScoreError*)event;
				if(strcmp(event2->caller, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type == GAME_LOAD_ACHIEVEMENTS_COMPLETE_EVENT)
			{
				game_Achievements *event2 = (game_Achievements*)event;
				if(strcmp(event2->caller, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type == GAME_LOAD_SCORES_COMPLETE_EVENT)
			{
				game_Leaderboard *event2 = (game_Leaderboard*)event;
				if(strcmp(event2->caller, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type == GAME_LOAD_SCORES_ERROR_EVENT || type == GAME_REPORT_ACHIEVEMENT_ERROR_EVENT || type == GAME_PLAYER_SCORE_ERROR_EVENT)
			{
				game_LoadError *event2 = (game_LoadError*)event;
				if(strcmp(event2->caller, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if (type==GAME_DATA_RECEIVED_EVENT)
			{
				game_ReceivedEvent *event2=(game_ReceivedEvent *)event;
				if(strcmp(event2->caller, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if (type == GAME_LOGIN_COMPLETE_EVENT||type==GAME_GAME_STARTED_EVENT||type==GAME_PEER_DECLINED_EVENT||
					type==GAME_PEER_INVITED_EVENT||type==GAME_PEER_JOINED_EVENT||type==GAME_PEER_LEFT_EVENT||
					type==GAME_PEER_CONNECTED_EVENT||type==GAME_PEER_DISCONNECTED_EVENT)
			{
				game_EmptyEvent *event2=(game_EmptyEvent *)event;
				if(strcmp(event2->caller, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type==GAME_INVITATION_RECEIVED_EVENT||type==GAME_JOINED_ROOM_EVENT||
					type==GAME_LEFT_ROOM_EVENT||type==GAME_ROOM_CONNECTED_EVENT||type==GAME_ROOM_CREATED_EVENT||
					type==GAME_CONNECTED_TO_ROOM_EVENT||type==GAME_DISCONNECTED_FROM_ROOM_EVENT||type==GAME_ROOM_AUTO_MATCHING_EVENT||
					type==GAME_ROOM_CONNECTING_EVENT)
			{
				game_SimpleEvent *event2 = (game_SimpleEvent*)event;
				if(strcmp(event2->id, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type == GAME_STATE_LOADED_EVENT)
			{
				game_StateLoaded *event2 = (game_StateLoaded*)event;
				if(strcmp(event2->type, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type == GAME_STATE_ERROR_EVENT)
			{
				game_StateError *event2 = (game_StateError*)event;
				if(strcmp(event2->type, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type == GAME_STATE_CONFLICT_EVENT)
			{
				game_StateConflict *event2 = (game_StateConflict*)event;
				if(strcmp(event2->type, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type == GAME_STATE_DELETED_EVENT)
			{
				game_StateDeleted *event2 = (game_StateDeleted*)event;
				if(strcmp(event2->type, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if(type == GAME_PLAYER_INFORMATION_COMPLETE_EVENT)
			{
				game_Player *event2 = (game_Player*)event;
				if(strcmp(event2->caller, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if ((type == GAME_PLAYER_SCORE_COMPLETE_EVENT)||(type==GAME_PLAYER_SCORES_COMPLETE_EVENT))
			{
				game_PlayerScore *event2 = (game_PlayerScore*)event;
				if(strcmp(event2->caller, type_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			
			if(shouldDispatch)
			{
				luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
				luaL_rawgetptr(L, -1, this);
				
				if (lua_isnil(L, -1))
				{
					lua_pop(L, 2);
					return;
				}
				
				lua_getfield(L, -1, "dispatchEvent");
				
				lua_pushvalue(L, -2);
				
				lua_getglobal(L, "Event");
				lua_getfield(L, -1, "new");
				lua_remove(L, -2);
				
				switch (type)
				{
					case GAME_LOGIN_COMPLETE_EVENT:
						lua_pushstring(L, LOGIN_COMPLETE);
						break;
					case GAME_LOGIN_ERROR_EVENT:
						lua_pushstring(L, LOGIN_ERROR);
						break;
					case GAME_PLAYER_INFORMATION_COMPLETE_EVENT:
						lua_pushstring(L, PLAYER_INFORMATION_COMPLETE);
						break;
					case GAME_PLAYER_INFORMATION_ERROR_EVENT:
						lua_pushstring(L, PLAYER_INFORMATION_ERROR);
						break;	
					case GAME_PLAYER_SCORE_COMPLETE_EVENT:
						lua_pushstring(L, PLAYER_SCORE_COMPLETE);
						break;
					case GAME_PLAYER_SCORE_ERROR_EVENT:
						lua_pushstring(L, PLAYER_SCORE_ERROR);
						break;
					case GAME_LOAD_ACHIEVEMENTS_COMPLETE_EVENT:
						lua_pushstring(L, LOAD_ACHIEVEMENTS_COMPLETE);
						break;
					case GAME_LOAD_ACHIEVEMENTS_ERROR_EVENT:
						lua_pushstring(L, LOAD_ACHIEVEMENTS_ERROR);
						break;
					case GAME_REPORT_ACHIEVEMENT_COMPLETE_EVENT:
						lua_pushstring(L, REPORT_ACHIEVEMENT_COMPLETE);
						break;
					case GAME_REPORT_ACHIEVEMENT_ERROR_EVENT:
						lua_pushstring(L, REPORT_ACHIEVEMENT_ERROR);
						break;
					case GAME_LOAD_SCORES_COMPLETE_EVENT:
						lua_pushstring(L, LOAD_SCORES_COMPLETE);
						break;
					case GAME_LOAD_SCORES_ERROR_EVENT:
						lua_pushstring(L, LOAD_SCORES_ERROR);
						break;
					case GAME_REPORT_SCORE_COMPLETE_EVENT:
						lua_pushstring(L, REPORT_SCORE_COMPLETE);
						break;
					case GAME_REPORT_SCORE_ERROR_EVENT:
						lua_pushstring(L, REPORT_SCORE_ERROR);
						break;
					case GAME_STATE_LOADED_EVENT:
						lua_pushstring(L, STATE_LOADED);
						break;
					case GAME_STATE_ERROR_EVENT:
						lua_pushstring(L, STATE_ERROR);
						break;
					case GAME_STATE_CONFLICT_EVENT:
						lua_pushstring(L, STATE_CONFLICT);
						break;
					case GAME_STATE_DELETED_EVENT:
						lua_pushstring(L, STATE_DELETED);
						break;
			case GAME_PLAYER_SCORES_COMPLETE_EVENT:
				lua_pushstring(L, PLAYER_SCORES_COMPLETE);
				break;
			case GAME_GAME_STARTED_EVENT:
                lua_pushstring(L, GAME_STARTED);
                break;
			case GAME_INVITATION_RECEIVED_EVENT:
                lua_pushstring(L, INVITATION_RECEIVED);
                break;
			case GAME_JOINED_ROOM_EVENT:
                lua_pushstring(L, JOINED_ROOM);
                break;
			case GAME_LEFT_ROOM_EVENT:
                lua_pushstring(L, LEFT_ROOM);
                break;
			case GAME_ROOM_CONNECTED_EVENT:
                lua_pushstring(L, ROOM_CONNECTED);
                break;
			case GAME_ROOM_CREATED_EVENT:
                lua_pushstring(L, ROOM_CREATED);
                break;
			case GAME_CONNECTED_TO_ROOM_EVENT:
                lua_pushstring(L, CONNECTED_TO_ROOM);
                break;
			case GAME_DISCONNECTED_FROM_ROOM_EVENT:
                lua_pushstring(L, DISCONNECTED_FROM_ROOM);
                break;
			case GAME_PEER_DECLINED_EVENT:
                lua_pushstring(L, PEER_DECLINED);
                break;
			case GAME_PEER_INVITED_EVENT:
                lua_pushstring(L, PEER_INVITED);
                break;
			case GAME_PEER_JOINED_EVENT:
                lua_pushstring(L, PEER_JOINED);
                break;
			case GAME_PEER_LEFT_EVENT:
                lua_pushstring(L, PEER_LEFT);
                break;
			case GAME_PEER_CONNECTED_EVENT:
                lua_pushstring(L, PEER_CONNECTED);
                break;
			case GAME_PEER_DISCONNECTED_EVENT:
                lua_pushstring(L, PEER_DISCONNECTED);
                break;
			case GAME_ROOM_AUTO_MATCHING_EVENT:
                lua_pushstring(L, ROOM_AUTO_MATCHING);
                break;
			case GAME_ROOM_CONNECTING_EVENT:
                lua_pushstring(L, ROOM_CONNECTING);
                break;
			case GAME_DATA_RECEIVED_EVENT:
                lua_pushstring(L, DATA_RECEIVED);
                break;
				}
		
				lua_call(L, 1, 1);
				
				if (type == GAME_LOAD_ACHIEVEMENTS_COMPLETE_EVENT)
				{
					game_Achievements *event2 = (game_Achievements*)event;
				
					lua_newtable(L);
					
					for (int i = 0; i < event2->count; ++i)
					{					
						lua_newtable(L);
						
						lua_pushstring(L, event2->achievements[i].id);
						lua_setfield(L, -2, "id");
			
						lua_pushstring(L, event2->achievements[i].name);
						lua_setfield(L, -2, "name");
						
						lua_pushstring(L, event2->achievements[i].description);
						lua_setfield(L, -2, "description");
			
						lua_pushnumber(L, event2->achievements[i].status);
						lua_setfield(L, -2, "status");
						
						lua_pushnumber(L, event2->achievements[i].lastUpdate);
						lua_setfield(L, -2, "lastUpdate");
						
						lua_pushnumber(L, event2->achievements[i].currentSteps);
						lua_setfield(L, -2, "currentSteps");
						
						lua_pushnumber(L, event2->achievements[i].totalSteps);
						lua_setfield(L, -2, "totalSteps");
						
						lua_rawseti(L, -2, i + 1);
					}
					lua_setfield(L, -2, "achievements");
				}
				else if(type == GAME_LOAD_ACHIEVEMENTS_ERROR_EVENT)
				{
					game_Report *event2 = (game_Report*)event;
				
					lua_pushstring(L, event2->value);
					lua_setfield(L, -2, "error");
				}
				else if(type == GAME_LOGIN_ERROR_EVENT)
				{
					game_Report *event2 = (game_Report*)event;
				
					lua_pushstring(L, event2->value);
					lua_setfield(L, -2, "error");
				}
				else if(type == GAME_REPORT_ACHIEVEMENT_COMPLETE_EVENT)
				{
					game_Report *event2 = (game_Report*)event;
				
					lua_pushstring(L, event2->value);
					lua_setfield(L, -2, "id");
				}
				else if(type == GAME_REPORT_ACHIEVEMENT_ERROR_EVENT)
				{
					game_LoadError *event2 = (game_LoadError*)event;
				
					lua_pushstring(L, event2->value);
					lua_setfield(L, -2, "id");
					
					lua_pushstring(L, event2->error);
					lua_setfield(L, -2, "error");
				}
				else if(type == GAME_LOAD_SCORES_COMPLETE_EVENT)
				{
					game_Leaderboard *event2 = (game_Leaderboard*)event;
				
					lua_pushstring(L, event2->id);
					lua_setfield(L, -2, "id");
					
					lua_pushstring(L, event2->name);
					lua_setfield(L, -2, "name");
					
					lua_newtable(L);
					
					for (int i = 0; i < event2->count; ++i)
					{				
						lua_newtable(L);
						
						lua_pushstring(L, event2->scores[i].rank);
						lua_setfield(L, -2, "rank");
			
						lua_pushstring(L, event2->scores[i].score);
						lua_setfield(L, -2, "score");
						
						lua_pushstring(L, event2->scores[i].name);
						lua_setfield(L, -2, "name");
						
						lua_pushstring(L, event2->scores[i].playerId);
						lua_setfield(L, -2, "playerId");
						
						lua_pushstring(L, event2->scores[i].pic);
						lua_setfield(L, -2, "picture");
			
						lua_pushnumber(L, event2->scores[i].timestamp);
						lua_setfield(L, -2, "timestamp");
						
						lua_rawseti(L, -2, i + 1);
					}
					lua_setfield(L, -2, "scores");
				}
				else if(type == GAME_LOAD_SCORES_ERROR_EVENT)
				{
					game_LoadError *event2 = (game_LoadError*)event;
				
					lua_pushstring(L, event2->value);
					lua_setfield(L, -2, "id");
					
					lua_pushstring(L, event2->error);
					lua_setfield(L, -2, "error");
				}
				else if(type == GAME_REPORT_SCORE_COMPLETE_EVENT)
				{
					game_ReportScore *event2 = (game_ReportScore*)event;
				
					lua_pushstring(L, event2->value);
					lua_setfield(L, -2, "id");
					
					lua_pushnumber(L, event2->score);
					lua_setfield(L, -2, "score");
				}
				else if(type == GAME_REPORT_SCORE_ERROR_EVENT)
				{
					game_ReportScoreError *event2 = (game_ReportScoreError*)event;
				
					lua_pushstring(L, event2->value);
					lua_setfield(L, -2, "id");
					
					lua_pushnumber(L, event2->score);
					lua_setfield(L, -2, "score");
					
					lua_pushstring(L, event2->error);
					lua_setfield(L, -2, "error");
				}
				else if(type == GAME_PLAYER_INFORMATION_COMPLETE_EVENT)
				{
					game_Player *event2 = (game_Player*)event;
				
					lua_pushstring(L, event2->playerId);
					lua_setfield(L, -2, "id");
					
					lua_pushstring(L, event2->name);
					lua_setfield(L, -2, "name");
					
					lua_pushstring(L, event2->pic);
					lua_setfield(L, -2, "picture");
				}
				else if(type == GAME_PLAYER_INFORMATION_ERROR_EVENT)
				{
					game_Report *event2 = (game_Report*)event;
				
					lua_pushstring(L, event2->value);
					lua_setfield(L, -2, "error");
				}
				else if(type == GAME_PLAYER_SCORE_COMPLETE_EVENT)
				{
					game_PlayerScore *event2 = (game_PlayerScore*)event;
				
					lua_pushstring(L, event2->id);
					lua_setfield(L, -2, "id");
					
					lua_pushnumber(L, event2->rank);
					lua_setfield(L, -2, "rank");
					
					lua_pushnumber(L, event2->score);
					lua_setfield(L, -2, "score");
					
					lua_pushstring(L, event2->formatScore);
					lua_setfield(L, -2, "formattedScore");

					lua_pushnumber(L, event2->timestamp);
					lua_setfield(L, -2, "timestamp");
				}
				else if(type == GAME_PLAYER_SCORE_ERROR_EVENT)
				{
					game_LoadError *event2 = (game_LoadError*)event;
				
					lua_pushstring(L, event2->value);
					lua_setfield(L, -2, "id");
					
					lua_pushstring(L, event2->error);
					lua_setfield(L, -2, "error");
				}
				else if (type == GAME_STATE_LOADED_EVENT)
				{
					game_StateLoaded *event2 = (game_StateLoaded*)event;
					
					lua_pushnumber(L, event2->key);
					lua_setfield(L, -2, "key");
					
					lua_pushboolean(L, (bool)event2->fresh);
					lua_setfield(L, -2, "isFresh");
		
					lua_pushlstring(L, (const char*)event2->data, event2->size);
					lua_setfield(L, -2, "data");
				}
				else if (type == GAME_STATE_ERROR_EVENT)
				{
					game_StateError *event2 = (game_StateError*)event;
					
					lua_pushstring(L, event2->error);
					lua_setfield(L, -2, "error");
				
					lua_pushnumber(L, event2->key);
					lua_setfield(L, -2, "key");
				}
				else if (type == GAME_STATE_DELETED_EVENT)
				{
					game_StateDeleted *event2 = (game_StateDeleted*)event;
					
					lua_pushnumber(L, event2->key);
					lua_setfield(L, -2, "key");
				}
				else if (type == GAME_STATE_CONFLICT_EVENT)
				{
					game_StateConflict *event2 = (game_StateConflict*)event;
				
					lua_pushnumber(L, event2->key);
					lua_setfield(L, -2, "key");
					
					lua_pushstring(L, event2->ver);
					lua_setfield(L, -2, "version");
					
					lua_pushlstring(L, (const char*)event2->localData, event2->localSize);
					lua_setfield(L, -2, "localData");
					
					lua_pushlstring(L, (const char*)event2->serverData, event2->serverSize);
					lua_setfield(L, -2, "serverData");
				}
	else if (type == GAME_PLAYER_SCORES_COMPLETE_EVENT)
        {
            game_PlayerScore *event2 = (game_PlayerScore*)event;
            
			lua_pushnumber(L, event2->rank);
			lua_setfield(L, -2, "rank");
			
			lua_pushstring(L, event2->formatScore);
			lua_setfield(L, -2, "formattedScore");
			
			lua_pushnumber(L, event2->score);
			lua_setfield(L, -2, "score");
			
			lua_pushnumber(L, event2->timestamp);
			lua_setfield(L, -2, "timestamp");
        }
		else if (type == GAME_JOINED_ROOM_EVENT || type == GAME_LEFT_ROOM_EVENT || type == GAME_ROOM_CONNECTED_EVENT || type == GAME_ROOM_CREATED_EVENT || type == GAME_CONNECTED_TO_ROOM_EVENT || type == GAME_DISCONNECTED_FROM_ROOM_EVENT || type == GAME_ROOM_AUTO_MATCHING_EVENT || type == GAME_ROOM_CONNECTING_EVENT)
        {
            game_SimpleEvent *event2 = (game_SimpleEvent*)event;
            
			lua_pushstring(L, event2->id);
			lua_setfield(L, -2, "roomId");
        }
		else if (type == GAME_DATA_RECEIVED_EVENT)
		{
            game_ReceivedEvent *event2 = (game_ReceivedEvent*)event;
		
			lua_pushstring(L, event2->sender);
			lua_setfield(L, -2, "senderId");

			lua_pushlstring(L, (const char*)event2->data, event2->size);
			lua_setfield(L, -2, "data");
		}
		
				lua_call(L, 2, 0);
				
				lua_pop(L, 2);
			}
		}
	}

private:
	const char* type_;
};

static int destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	Game *game = static_cast<Game*>(object->proxy());
	
	game->unref();
	
	return 0;
}

static Game *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Gaming", index));
	Game *game = static_cast<Game*>(object->proxy());
    
	return game;
}

static int init(lua_State *L)
{
    
    const char *type = luaL_checkstring(L, 1);
	Game *game = new Game(type);
	g_pushInstance(L, "Gaming", game->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, game);
	lua_pop(L, 1);
    
	lua_pushvalue(L, -1);
    return 1;
}

static int login(lua_State *L)
{
    Game *game = getInstance(L, 1);
	int i = 2;
	std::vector<game_Parameter> params2;
	while(!lua_isnoneornil(L, i))
	{
		game_Parameter param = {luaL_checkstring(L, i)};
		params2.push_back(param);
		i++;
	}
	game_Parameter param = {NULL};
	params2.push_back(param);
	game->login(&params2[0]);
    return 0;
}

static int logout(lua_State *L)
{
    Game *game = getInstance(L, 1);
	game->logout();
    return 0;
}

static int showLeaderboard(lua_State *L)
{
	Game *game = getInstance(L, 1);
	const char *id = luaL_optstring(L, 2, NULL);
	game->showLeaderboard(id);
    return 0;
}

static int reportScore(lua_State *L)
{
	Game *game = getInstance(L, 1);
	const char *id = luaL_checkstring(L, 2);
	long score = luaL_checklong(L, 3);
	int immediate = lua_toboolean(L, 4);
	game->reportScore(id, score, immediate);
    return 0;
}

static int showAchievements(lua_State *L)
{
	Game *game = getInstance(L, 1);
	game->showAchievements();
    return 0;
}

static int getPlayerInfo(lua_State *L)
{
	Game *game = getInstance(L, 1);
	game->getPlayerInfo();
    return 0;
}

static int reportAchievement(lua_State* L)
{
	Game *game = getInstance(L, 1);

	const char* id = luaL_checkstring(L, 2);
	double numSteps = 0;
	int immediate = 0;
	if (!lua_isnoneornil(L, 3))
	{
		if(lua_isboolean(L, 3))
		{
			immediate = lua_toboolean(L, 3);
		}
		else
		{
			numSteps = luaL_checkinteger(L, 3);
			immediate = lua_toboolean(L, 4);
		}
	}
	
	game->reportAchievement(id, numSteps, immediate);
	
	return 0;
}

static int incrementAchievement(lua_State* L)
{
	Game *game = getInstance(L, 1);

	const char* id = luaL_checkstring(L, 2);
	double numSteps = luaL_checkinteger(L, 3);
	int immediate = lua_toboolean(L, 4);

	game->reportAchievement(id, -numSteps, immediate);

	return 0;
}

static int revealAchievement(lua_State* L)
{
	Game *game = getInstance(L, 1);

	const char* id = luaL_checkstring(L, 2);
	int immediate = lua_toboolean(L, 3);

	game->revealAchievement(id, immediate);

	return 0;
}

static int loadAchievements(lua_State *L)
{
	Game *game = getInstance(L, 1);
	game->loadAchievements();
    return 0;
}

static int loadScores(lua_State* L)
{
	Game *game = getInstance(L, 1);

    int span = 0;
	int collection = 0;
    int maxEntries = 25; // Default

	const char *id = luaL_checkstring(L, 2);

	if (!lua_isnoneornil(L, 3))
		span = lua_tonumber(L, 3);

    if (!lua_isnoneornil(L, 4))
   		collection = lua_tonumber(L, 4);

    if (!lua_isnoneornil(L, 5))
   		maxEntries= lua_tonumber(L, 5);

    game->loadScores(id, span, collection, maxEntries);

	return 0;
}

static int loadPlayerScores(lua_State* L)
{
	Game *game = getInstance(L, 1);

    int span = 2;
	int collection = 0;
    int maxEntries = 25; // Default

	const char *id = luaL_checkstring(L, 2);

	if (!lua_isnoneornil(L, 3))
		span = luaL_checknumber(L, 3);

    if (!lua_isnoneornil(L, 4))
   		collection = luaL_checknumber(L, 4);

    if (!lua_isnoneornil(L, 5))
   		maxEntries= luaL_checknumber(L, 5);

    game->loadPlayerScores(id, span, collection, maxEntries);

	return 0;
}

static int loadState(lua_State *L)
{
	Game *game = getInstance(L, 1);
	
	int key = luaL_checknumber(L, 2);
	
	game->loadState(key);
	
    return 0;
}

static int updateState(lua_State *L)
{
	Game *game = getInstance(L, 1);
	
	int key = luaL_checknumber(L, 2);
	
	size_t size;
    const void *data = luaL_checklstring(L, 3, &size);
	
    int immediate = 0;
	if (!lua_isnoneornil(L, 4))
	{
		if(lua_isboolean(L, 4))
		{
			immediate = lua_toboolean(L, 4);
		}
	}
	
	game->updateState(key, data, size, immediate);
	
    return 0;
}

static int resolveState(lua_State *L)
{
	Game *game = getInstance(L, 1);
	
	int key = luaL_checknumber(L, 2);
	
	const char* ver = luaL_checkstring(L, 3);
	
	size_t size;
    const void *data = luaL_checklstring(L, 4, &size);
	
	game->resolveState(key, ver, data, size);
	
    return 0;
}

static int deleteState(lua_State *L)
{
	Game *game = getInstance(L, 1);
	
	int key = luaL_checknumber(L, 2);
	
	game->deleteState(key);
	
    return 0;
}


static int autoMatch(lua_State *L)
{
	Game *game = getInstance(L, 1);
	int minPlayers = luaL_checknumber(L, 2);
	int maxPlayers = luaL_checknumber(L, 3);
	game->autoMatch(minPlayers, maxPlayers);
    return 0;
}

static int invitePlayers(lua_State *L)
{
	Game *game = getInstance(L, 1);
	int minPlayers = luaL_checknumber(L, 2);
	int maxPlayers = luaL_checknumber(L, 3);
	game->invitePlayers(minPlayers, maxPlayers);
    return 0;
}

static int joinRoom(lua_State *L)
{
	Game *game = getInstance(L, 1);
	const char* roomId = luaL_checkstring(L, 2);
	game->joinRoom(roomId);
    return 0;
}

static int showInvitations(lua_State *L)
{
	Game *game = getInstance(L, 1);
	game->showInvitations();
    return 0;
}

static int showWaitingRoom(lua_State *L)
{
	Game *game = getInstance(L, 1);
	int minPlayers = luaL_checknumber(L, 2);
	game->showWaitingRoom(minPlayers);
    return 0;
}

static int sendTo(lua_State *L)
{
	Game *game = getInstance(L, 1);
	const char* id = luaL_checkstring(L, 2);
	
	size_t size;
    const void *data = luaL_checklstring(L, 3, &size);
	
    int isReliable = luaL_checknumber(L, 4);
	
	game->sendTo(id, data, size, isReliable);
	
    return 0;
}

static int sendToAll(lua_State *L)
{
	Game *game = getInstance(L, 1);
	
	size_t size;
    const void *data = luaL_checklstring(L, 2, &size);
	
    int isReliable = luaL_checknumber(L, 3);
	
	game->sendToAll(data, size, isReliable);
	
    return 0;
}

static int getCurrentPlayer(lua_State *L)
{
	Game *game = getInstance(L, 1);
		
	const char *name = game->getCurrentPlayer();
	
	lua_pushstring(L, name);
	
    return 1;
}

static int getCurrentPlayerId(lua_State *L)
{
	Game *game = getInstance(L, 1);
		
	const char *name = game->getCurrentPlayerId();
	
	lua_pushstring(L, name);
	
    return 1;
}

static int getCurrentPicture(lua_State *L)
{
	Game *game = getInstance(L, 1);
	
	int highRes = 0;
	if(!lua_isnoneornil(L, 2))
		highRes = lua_tonumber(L, 2);
	const char *name = game->getCurrentPicture(highRes);
	
	lua_pushstring(L, name);
	
    return 1;
}

static int getCurrentScore(lua_State *L)
{
	Game *game = getInstance(L, 1);

    int span = 2;
	int collection = 0;

	const char *id = luaL_checkstring(L, 2);

	if (!lua_isnoneornil(L, 3))
		span = luaL_checkinteger(L, 3);

    if (!lua_isnoneornil(L, 4))
   		collection = luaL_checkinteger(L, 4);
		
	game->getCurrentScore(id, span, collection);
	
    return 0;
}

void player2table(game_PlayerEntry *player, lua_State* L)
{
	//main table
	lua_newtable(L);
	
	if(player)
	{
		int i = 1;
		while(player->id.empty())
		{
			//set subtable to table
			lua_pushnumber(L, i);
		
			//create sub table
			lua_newtable(L);
	
			//set key
			lua_pushstring(L, "id");
			lua_pushstring(L, player->id.c_str());
			//back to table
			lua_settable(L, -3);
			
			//set key
			lua_pushstring(L, "name");
			lua_pushstring(L, player->name.c_str());
			//back to table
			lua_settable(L, -3);
			
			//outer table
			lua_settable(L, -3);
			
			++player;
			i++;
		}
	}
	lua_pushvalue(L, -1);
}

static int getAllPlayers(lua_State *L)
{
	Game *game = getInstance(L, 1);
		
	game_PlayerEntry *player = game->getAllPlayers();
	
	player2table(player, L);
	
    return 1;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"new", init},
        {"login", login},
        {"logout", logout},
		{"showLeaderboard", showLeaderboard},
        {"reportScore", reportScore},
        {"showAchievements", showAchievements},
        {"getPlayerInfo", getPlayerInfo},
        {"reportAchievement", reportAchievement},
        {"incrementAchievement", incrementAchievement},
        {"revealAchievement", revealAchievement},
        {"loadAchievements", loadAchievements},
        {"loadScores", loadScores},
        {"loadPlayerCenteredScores", loadPlayerScores},
		{"loadState", loadState},
        {"updateState", updateState},
        {"resolveState", resolveState},
        {"deleteState", deleteState},
        {"autoMatch", autoMatch},
        {"invitePlayers", invitePlayers},
        {"joinRoom", joinRoom},
        {"showInvitations", showInvitations},
        {"showWaitingRoom", showWaitingRoom},
        {"sendTo", sendTo},
        {"sendToAll", sendToAll},
        {"getAllPlayers", getAllPlayers},
		{"getPlayerName", getCurrentPlayer},
        {"getPlayerId", getCurrentPlayerId},
        {"getPlayerPicture", getCurrentPicture},
        {"getPlayerScore", getCurrentScore},
		{NULL, NULL},
	};
    
    g_createClass(L, "Gaming", "EventDispatcher", NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    
	lua_getglobal(L, "Event");
	lua_pushstring(L, LOGIN_COMPLETE);
	lua_setfield(L, -2, "LOGIN_COMPLETE");
	lua_pushstring(L, LOGIN_ERROR);
	lua_setfield(L, -2, "LOGIN_ERROR");
	lua_pushstring(L, LOAD_ACHIEVEMENTS_COMPLETE);
	lua_setfield(L, -2, "LOAD_ACHIEVEMENTS_COMPLETE");
	lua_pushstring(L, LOAD_ACHIEVEMENTS_ERROR);
	lua_setfield(L, -2, "LOAD_ACHIEVEMENTS_ERROR");
	lua_pushstring(L, PLAYER_INFORMATION_COMPLETE);
	lua_setfield(L, -2, "PLAYER_INFORMATION_COMPLETE");
	lua_pushstring(L, PLAYER_INFORMATION_ERROR);
	lua_setfield(L, -2, "PLAYER_INFORMATION_ERROR");
	lua_pushstring(L, PLAYER_SCORE_COMPLETE);
	lua_setfield(L, -2, "PLAYER_SCORE_COMPLETE");
	lua_pushstring(L, PLAYER_SCORE_ERROR);
	lua_setfield(L, -2, "PLAYER_SCORE_ERROR");
	lua_pushstring(L, REPORT_ACHIEVEMENT_COMPLETE);
	lua_setfield(L, -2, "REPORT_ACHIEVEMENT_COMPLETE");
	lua_pushstring(L, REPORT_ACHIEVEMENT_ERROR);
	lua_setfield(L, -2, "REPORT_ACHIEVEMENT_ERROR");
	lua_pushstring(L, LOAD_SCORES_COMPLETE);
	lua_setfield(L, -2, "LOAD_SCORES_COMPLETE");
	lua_pushstring(L, LOAD_SCORES_ERROR);
	lua_setfield(L, -2, "LOAD_SCORES_ERROR");
	lua_pushstring(L, REPORT_SCORE_COMPLETE);
	lua_setfield(L, -2, "REPORT_SCORE_COMPLETE");
	lua_pushstring(L, REPORT_SCORE_ERROR);
	lua_setfield(L, -2, "REPORT_SCORE_ERROR");
	lua_pushstring(L, STATE_LOADED);
	lua_setfield(L, -2, "STATE_LOADED");
	lua_pushstring(L, STATE_ERROR);
	lua_setfield(L, -2, "STATE_ERROR");
	lua_pushstring(L, STATE_CONFLICT);
	lua_setfield(L, -2, "STATE_CONFLICT");
	lua_pushstring(L, STATE_DELETED);
	lua_setfield(L, -2, "STATE_DELETED");
	lua_pushstring(L, GAME_STARTED);
	lua_setfield(L, -2, "GAME_STARTED");
	lua_pushstring(L, INVITATION_RECEIVED);
	lua_setfield(L, -2, "INVITATION_RECEIVED");
	lua_pushstring(L, JOINED_ROOM);
	lua_setfield(L, -2, "JOINED_ROOM");
	lua_pushstring(L, LEFT_ROOM);
	lua_setfield(L, -2, "LEFT_ROOM");
	lua_pushstring(L, ROOM_CONNECTED);
	lua_setfield(L, -2, "ROOM_CONNECTED");
	lua_pushstring(L, ROOM_CREATED);
	lua_setfield(L, -2, "ROOM_CREATED");
	lua_pushstring(L, CONNECTED_TO_ROOM);
	lua_setfield(L, -2, "CONNECTED_TO_ROOM");
	lua_pushstring(L, DISCONNECTED_FROM_ROOM);
	lua_setfield(L, -2, "DISCONNECTED_FROM_ROOM");
	lua_pushstring(L, PEER_DECLINED);
	lua_setfield(L, -2, "PEER_DECLINED");
	lua_pushstring(L, PEER_INVITED);
	lua_setfield(L, -2, "PEER_INVITED");
	lua_pushstring(L, PEER_JOINED);
	lua_setfield(L, -2, "PEER_JOINED");
	lua_pushstring(L, PEER_LEFT);
	lua_setfield(L, -2, "PEER_LEFT");
	lua_pushstring(L, PEER_CONNECTED);
	lua_setfield(L, -2, "PEER_CONNECTED");
	lua_pushstring(L, PEER_DISCONNECTED);
	lua_setfield(L, -2, "PEER_DISCONNECTED");
	lua_pushstring(L, ROOM_AUTO_MATCHING);
	lua_setfield(L, -2, "ROOM_AUTO_MATCHING");
	lua_pushstring(L, ROOM_CONNECTING);
	lua_setfield(L, -2, "ROOM_CONNECTING");
	lua_pushstring(L, DATA_RECEIVED);
	lua_setfield(L, -2, "DATA_RECEIVED");
	lua_pop(L, 1);
	
	lua_getglobal(L, "Gaming");
	lua_pushnumber(L, TODAY);
	lua_setfield(L, -2, "TODAY");
	lua_pushnumber(L, WEEK);
	lua_setfield(L, -2, "WEEK");
	lua_pushnumber(L, ALL_TIME);
	lua_setfield(L, -2, "ALL_TIME");
	lua_pushnumber(L, FRIENDS);
	lua_setfield(L, -2, "FRIENDS");
	lua_pushnumber(L, ALL_PLAYERS);
	lua_setfield(L, -2, "ALL_PLAYERS");
	lua_pushnumber(L, UNLOCKED);
	lua_setfield(L, -2, "UNLOCKED");
	lua_pushnumber(L, REVEALED);
	lua_setfield(L, -2, "REVEALED");
	lua_pushnumber(L, HIDDEN);
	lua_setfield(L, -2, "HIDDEN");
	lua_pop(L, 1);
	  
    return 0;
}
    
static void g_initializePlugin(lua_State *L)
{
	::L = L;
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "gaming");
	
	lua_pop(L, 2);
	
	game_init();
}

static void g_deinitializePlugin(lua_State *L)
{
	::L = NULL;
    game_cleanup();
}

REGISTER_PLUGIN("Gaming", "1.0")
