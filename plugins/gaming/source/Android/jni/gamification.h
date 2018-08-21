#ifndef GAMIFICATION_H
#define GAMIFICATION_H

#include "gideros.h"
#include <string>

enum
{
	GAME_LOGIN_COMPLETE_EVENT,
	GAME_LOGIN_ERROR_EVENT,
	GAME_LOAD_ACHIEVEMENTS_COMPLETE_EVENT,
	GAME_LOAD_ACHIEVEMENTS_ERROR_EVENT,
	GAME_REPORT_ACHIEVEMENT_COMPLETE_EVENT,
	GAME_REPORT_ACHIEVEMENT_ERROR_EVENT,
	GAME_LOAD_SCORES_COMPLETE_EVENT,
	GAME_LOAD_SCORES_ERROR_EVENT,
	GAME_REPORT_SCORE_COMPLETE_EVENT,
	GAME_REPORT_SCORE_ERROR_EVENT,
	GAME_PLAYER_INFORMATION_COMPLETE_EVENT,
	GAME_PLAYER_INFORMATION_ERROR_EVENT,
	GAME_PLAYER_SCORE_COMPLETE_EVENT,
	GAME_PLAYER_SCORE_ERROR_EVENT,
	GAME_PLAYER_SCORES_COMPLETE_EVENT,
	GAME_STATE_LOADED_EVENT,
	GAME_STATE_ERROR_EVENT,
	GAME_STATE_CONFLICT_EVENT,
	GAME_STATE_DELETED_EVENT,
	GAME_GAME_STARTED_EVENT,
	GAME_INVITATION_RECEIVED_EVENT,
	GAME_JOINED_ROOM_EVENT,
	GAME_LEFT_ROOM_EVENT,
	GAME_ROOM_CONNECTED_EVENT,
	GAME_ROOM_CREATED_EVENT,
	GAME_CONNECTED_TO_ROOM_EVENT,
	GAME_DISCONNECTED_FROM_ROOM_EVENT,
	GAME_PEER_DECLINED_EVENT,
	GAME_PEER_INVITED_EVENT,
	GAME_PEER_JOINED_EVENT,
	GAME_PEER_LEFT_EVENT,
	GAME_PEER_CONNECTED_EVENT,
	GAME_PEER_DISCONNECTED_EVENT,
	GAME_ROOM_AUTO_MATCHING_EVENT,
	GAME_ROOM_CONNECTING_EVENT,
	GAME_DATA_RECEIVED_EVENT
};

typedef struct game_Report
{
	const char *caller;
	const char *value;
} game_Report;

typedef struct game_ReportScore
{
	const char *caller;
	const char *value;
	long score;
} game_ReportScore;

typedef struct game_ReportScoreError
{
	const char *caller;
	const char *value;
	const char *error;
	long score;
} game_ReportScoreError;

typedef struct game_LoadError
{
	const char *caller;
	const char *value;
	const char *error;
} game_LoadScore;

typedef struct Achievement
{
	std::string id;
	std::string name;
	std::string description;
	int status;
	int lastUpdate;
	int currentSteps;
	int totalSteps;
} Achievement;

typedef struct game_Achievement
{
	const char *id;
	const char *name;
	const char *description;
	int status;
	int lastUpdate;
	int currentSteps;
	int totalSteps;
} game_Achievement;

typedef struct game_Achievements
{
	const char *caller;
	int count;
	game_Achievement *achievements;
} game_Achievements;

typedef struct game_PlayerEntry
{
	std::string id;
	std::string name;
} game_PlayerEntry;

typedef struct Score
{
	std::string rank;
	std::string score;
	std::string name;
	std::string playerId;
	std::string pic;
	int timestamp;
} Score;

typedef struct game_Score
{
	const char *rank;
	const char *score;
	const char *name;
	const char *playerId;
	const char *pic;
	int timestamp;
} game_Score;

typedef struct game_Leaderboard
{
	const char *caller;
	const char *id;
	const char *name;
	int count;
	game_Score *scores;
} game_Leaderboard;

typedef struct game_Player
{
	const char *caller;
	const char *playerId;
	const char *name;
	const char *pic;
} game_Player;

typedef struct game_PlayerScore
{
	const char *caller;
	const char *formatScore;
	const char *id;
	int rank;
	long score;
	int timestamp;
} game_PlayerScore;

typedef struct game_EmptyEvent
{
	const char *caller;
} game_EmptyEvent;

typedef struct game_SimpleEvent
{
	const char *caller;
	const char *id;
} game_SimpleEvent;

typedef struct game_ReceivedEvent
{
	const char *caller;
	const char *sender;
	const void *data;
	size_t size;
} game_ReceivedEvent;

typedef struct game_Parameter
{
    const char *value;
} game_Parameter;

typedef struct game_StateLoaded
{
	const char *type;
	int key;
	int fresh;
	const void *data;
	size_t size;
} game_StateLoaded;

typedef struct game_StateError
{
	const char *type;
	int key;
	const char* error;
} game_StateError;

typedef struct game_StateDeleted
{
	const char *type;
	int key;
} game_StateDeleted;

typedef struct game_StateConflict
{
	const char *type;
	int key;
	const char* ver;
	const void *localData;
	size_t localSize;
	const void *serverData;
	size_t serverSize;
} game_StateConflict;

#ifdef __cplusplus
extern "C" {
#endif
    
G_API int game_isAvailable();

G_API void game_init();
G_API void game_cleanup();

G_API void game_initialize(const char *game);
G_API void game_destroy(const char *game);
G_API void game_login(const char *game, game_Parameter *params);
G_API void game_logout(const char *game);

G_API void game_reportScore(const char *game, const char *id, long score, int immediate);
G_API void game_showAchievements(const char *game);
G_API void game_getPlayerInfo(const char *game);
G_API void game_showLeaderboard(const char *game, const char *id);
G_API void game_reportAchievement(const char *game, const char *id, double steps, int immediate);
G_API void game_revealAchievement(const char *game, const char *id, int immediate);
G_API void game_loadAchievements(const char *game);
G_API void game_loadScores(const char *game, const char *id, int span, int collection, int maxResults);
G_API void game_loadPlayerScores(const char *game, const char *id, int span, int collection, int maxResults);

G_API void game_loadState(const char *game, int key);
G_API void game_updateState(const char *game, int key, const void* data, size_t size, int immediate);
G_API void game_resolveState(const char *game, int key, const char* ver, const void* data, size_t size);
G_API void game_deleteState(const char *game, int key);

G_API g_id game_addCallback(gevent_Callback callback, void *udata);
G_API void game_removeCallback(gevent_Callback callback, void *udata);
G_API void game_removeCallbackWithGid(g_id gid);

G_API void game_autoMatch(const char *game, int minPlayers, int maxPlayers);
G_API void game_invitePlayers(const char *game, int minPlayers, int maxPlayers);
G_API void game_joinRoom(const char *game, const char* id);
G_API void game_showInvitations(const char *game);
G_API void game_showWaitingRoom(const char *game, int minPlayers);
G_API void game_sendTo(const char *game, const char* id, const void* data, size_t size, int isReliable);
G_API void game_sendToAll(const char *game, const void* data, size_t size, int isReliable);
G_API const char* game_getCurrentPlayer(const char *game);
G_API const char* game_getCurrentPlayerId(const char *game);
G_API const char* game_getCurrentPicture(const char *game, int highRes);
G_API void game_getCurrentScore(const char *game, const char *id, int span, int collection);
G_API game_PlayerEntry* game_getAllPlayers(const char *game);


#ifdef __cplusplus
}
#endif

#endif
