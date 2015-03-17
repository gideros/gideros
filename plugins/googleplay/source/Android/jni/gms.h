#ifndef GMS_H
#define GMS_H

#include <gglobal.h>
#include <gevent.h>
#include <string>


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

typedef struct gms_Achievement
{
	const char *id;
	const char *name;
	const char *description;
	int status;
	int lastUpdate;
	int currentSteps;
	int totalSteps;
} gms_Achievement;

typedef struct gms_Achievements
{
	int count;
	gms_Achievement *achievements;
} gms_Achievements;

typedef struct gms_Player
{
	std::string id;
	std::string name;
} gms_Player;

typedef struct Score
{
	std::string rank;
	std::string formatScore;
	std::string name;
	std::string playerId;
	long score;
	int timestamp;
} Score;

typedef struct gms_Score
{
	const char *rank;
	const char *formatScore;
	const char *name;
	const char *playerId;
	long score;
	int timestamp;
} gms_Score;

typedef struct gms_PlayerScore
{
	const char *rank;
	const char *formatScore;
	long score;
	int timestamp;
} gms_PlayerScore;

typedef struct gms_Leaderboard
{
	const char *id;
	const char *name;
	int count;
	gms_Score *scores;
} gms_Leaderboard;

typedef struct gms_SimpleEvent
{
	const char *id;
} gms_SimpleEvent;

typedef struct gms_ReceivedEvent
{
	const char *sender;
	const void *data;
	size_t size;
} gms_ReceivedEvent;

typedef struct gms_StateLoaded
{
	int key;
	int fresh;
	const void *data;
	size_t size;
} gms_StateLoaded;

typedef struct gms_StateError
{
	int key;
	const char* error;
} gms_StateError;

typedef struct gms_StateDeleted
{
	int key;
} gms_StateDeleted;

typedef struct gms_StateConflict
{
	int key;
	const char* ver;
	const void *localData;
	size_t localSize;
	const void *serverData;
	size_t serverSize;
} gms_StateConflict;

enum
{
	GMS_LOGIN_ERROR_EVENT,
	GMS_LOGIN_COMPLETE_EVENT,
	GMS_LOAD_ACHIEVEMENTS_COMPLETE_EVENT,
	GMS_REPORT_ACHIEVEMENT_COMPLETE_EVENT,
	GMS_LOAD_SCORES_COMPLETE_EVENT,
	GMS_PLAYER_SCORES_COMPLETE_EVENT,
	GMS_REPORT_SCORE_COMPLETE_EVENT,
	GMS_STATE_LOADED_EVENT,
	GMS_STATE_ERROR_EVENT,
	GMS_STATE_CONFLICT_EVENT,
	GMS_STATE_DELETED_EVENT,
	GMS_GAME_STARTED_EVENT,
	GMS_INVITATION_RECEIVED_EVENT,
	GMS_JOINED_ROOM_EVENT,
	GMS_LEFT_ROOM_EVENT,
	GMS_ROOM_CONNECTED_EVENT,
	GMS_ROOM_CREATED_EVENT,
	GMS_CONNECTED_TO_ROOM_EVENT,
	GMS_DISCONNECTED_FROM_ROOM_EVENT,
	GMS_PEER_DECLINED_EVENT,
	GMS_PEER_INVITED_EVENT,
	GMS_PEER_JOINED_EVENT,
	GMS_PEER_LEFT_EVENT,
	GMS_PEER_CONNECTED_EVENT,
	GMS_PEER_DISCONNECTED_EVENT,
	GMS_ROOM_AUTO_MATCHING_EVENT,
	GMS_ROOM_CONNECTING_EVENT,
	GMS_DATA_RECEIVED_EVENT
};


#ifdef __cplusplus
extern "C" {
#endif

G_API void gms_init();
G_API void gms_cleanup();

G_API bool gms_isAvailable();
G_API void gms_login();
G_API void gms_logout();
G_API void gms_showSettings();
G_API void gms_showLeaderboard(const char *id);
G_API void gms_reportScore(const char *id, long score, int immediate);
G_API void gms_showAchievements();
G_API void gms_reportAchievement(const char *id, int steps, int immediate);
G_API void gms_loadAchievements();
G_API void gms_loadScores(const char *id, int span, int collection, int maxResults);
G_API void gms_loadPlayerScores(const char *id, int span, int collection, int maxResults);
G_API void gms_loadState(int key);
G_API void gms_updateState(int key, const void* data, size_t size, int immediate);
G_API void gms_resolveState(int key, const char* ver, const void* data, size_t size);
G_API void gms_deleteState(int key);
G_API void gms_autoMatch(int minPlayers, int maxPlayers);
G_API void gms_invitePlayers(int minPlayers, int maxPlayers);
G_API void gms_joinRoom(const char* id);
G_API void gms_showInvitations();
G_API void gms_showWaitingRoom(int minPlayers);
G_API void gms_sendTo(const char* id, const void* data, size_t size, int isReliable);
G_API void gms_sendToAll(const void* data, size_t size, int isReliable);
G_API const char* gms_getCurrentPlayer();
G_API const char* gms_getCurrentPlayerId();
G_API const char* gms_getCurrentPicture(int highRes);
G_API void gms_getCurrentScore(const char *id, int span, int collection);
G_API gms_Player* gms_getAllPlayers();

G_API g_id gms_addCallback(gevent_Callback callback, void *udata);
G_API void gms_removeCallback(gevent_Callback callback, void *udata);
G_API void gms_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif