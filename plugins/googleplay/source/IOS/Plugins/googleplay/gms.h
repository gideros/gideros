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
	std::string score;
	std::string name;
	std::string playerId;
	int timestamp;
} Score;

typedef struct gms_Score
{
	const char *rank;
	const char *score;
	const char *name;
	const char *playerId;
	int timestamp;
} gms_Score;

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
	GMS_REPORT_SCORE_COMPLETE_EVENT,
	GMS_STATE_LOADED_EVENT,
	GMS_STATE_ERROR_EVENT,
	GMS_STATE_CONFLICT_EVENT,
	GMS_STATE_DELETED_EVENT
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
G_API const char* gms_getCurrentPlayer();
G_API const char* gms_getCurrentPlayerId();

G_API g_id gms_addCallback(gevent_Callback callback, void *udata);
G_API void gms_removeCallback(gevent_Callback callback, void *udata);
G_API void gms_removeCallbackWithGid(g_id gid);
    
G_API void gms_onSignInFailed();
G_API void gms_onSignInSucceeded();
G_API void gms_onAchievementUpdated(const char* Id);
G_API void gms_onScoreSubmitted();
G_API void gms_onAchievementsLoaded(Achievement* ach);
G_API void gms_onLeaderboardScoresLoaded(const char* jId, const char* jName, Score* score);
G_API void gms_onStateLoaded(int key, const void* data, size_t size, int fresh);
G_API void gms_onStateError(int key, const char* error);
G_API void gms_onStateConflict(int key, const char* ver, const void* localState, size_t localSize, const void* serverState, size_t serverSizes);
G_API void gms_onStateDeleted(int key);

#ifdef __cplusplus
}
#endif

#endif