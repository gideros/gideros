#include "gamification.h"
#import "GameClass.h"

class GGame
{
public:
	GGame()
	{
		gid_ = g_NextId();
        [GameClass init];
	}

	~GGame()
	{
        [GameClass cleanup];
		gevent_RemoveEventsWithGid(gid_);
	}
	
	void init(const char *ad)
	{
		[GameClass initialize:[NSString stringWithUTF8String:ad]];
	}
	
	void destroy(const char *ad)
	{
		[GameClass destroy:[NSString stringWithUTF8String:ad]];
	}
	
	void login(const char *ad, game_Parameter *params)
	{
		NSMutableArray *arr = [NSMutableArray array];
		while (params->value)
		{
            [arr addObject:[NSString stringWithUTF8String:params->value]];
			++params;
		}
		[GameClass login:[NSString stringWithUTF8String:ad] with:arr];
	}
	
	void logout(const char *ad)
	{
		[GameClass logout:[NSString stringWithUTF8String:ad]];
	}
	
	void reportScore(const char *game, const char *Id, long score, int immediate)
	{
		[GameClass reportScore:[NSString stringWithUTF8String:game] with:[NSString stringWithUTF8String:Id] andScore:score with:immediate];
	}
	
	void showAchievements(const char *game)
	{
		[GameClass showAchievements:[NSString stringWithUTF8String:game]];
	}
	
	void showLeaderboard(const char *game, const char *Id)
	{
		[GameClass showLeaderboard:[NSString stringWithUTF8String:game] with:[NSString stringWithUTF8String:Id]];
	}
	
	void reportAchievement(const char *game, const char *Id, int steps, int immediate)
	{
		[GameClass reportAchievement:[NSString stringWithUTF8String:game] with:[NSString stringWithUTF8String:Id] andSteps:steps with:immediate];
	}
	
	void loadAchievements(const char *game)
	{
		[GameClass loadAchievements:[NSString stringWithUTF8String:game]];
	}
	
	void loadScores(const char *game, const char *Id, int span, int collection, int maxResults)
	{
		[GameClass loadScores:[NSString stringWithUTF8String:game] with:[NSString stringWithUTF8String:Id] andSpan:span forCollection:collection withResults:maxResults];
	}
	
	void loadState(const char *game, int key)
	{
		[GameClass loadState:[NSString stringWithUTF8String:game] with:key];
	}

	void updateState(const char *game, int key, const void* data, size_t size, int immediate)
	{
		[GameClass updateState:[NSString stringWithUTF8String:game] with:[NSData dataWithBytes:data length:size] forKey:key withImmediate:immediate];
	}

	void resolveState(const char *game, int key, const char* ver, const void* data, size_t size)
	{
		[GameClass resolveState:[NSString stringWithUTF8String:game] with:[NSData dataWithBytes:data length:size] forKey:key andVersion:[NSString stringWithUTF8String:ver]];
	}
	
	void deleteState(const char *game, int key)
	{
		[GameClass deleteState:[NSString stringWithUTF8String:game] with:key];
	}
	
	void map2achievement(Achievement* ach)
	{
		achievements.clear();
        
        if(ach)
		{
            while (!ach->id.empty()){
                Achievement gach = {ach->id, ach->name, ach->description, ach->status, ach->lastUpdate, ach->currentSteps, ach->totalSteps};
                achievements.push_back(gach);
                ++ach;
            }
        }
	}
	
	void map2score(Score* score)
	{
		scores.clear();
        
		if(score)
		{
            while (!score->rank.empty()){
                Score gscores = {score->rank, score->score, score->name, score->playerId, score->timestamp};
                scores.push_back(gscores);
                ++score;
            }
        }
	}
	
	void onLoginComplete(const char *caller){
		
		game_SimpleEvent *event = (game_SimpleEvent*)gevent_CreateEventStruct1(
			sizeof(game_SimpleEvent),
			offsetof(game_SimpleEvent, id), caller);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOGIN_COMPLETE_EVENT, event, 1, this);
	}
	
	void onLoginError(const char *caller, const char *value){
		game_Report *event = (game_Report*)gevent_CreateEventStruct2(
			sizeof(game_Report),
			offsetof(game_Report, caller), caller,
			offsetof(game_Report, value), value);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOGIN_ERROR_EVENT, event, 1, this);
	}
	
	void onReportAchievementComplete(const char *caller, const char *value){
		
		game_Report *event = (game_Report*)gevent_CreateEventStruct2(
			sizeof(game_Report),
			offsetof(game_Report, caller), caller,
			offsetof(game_Report, value), value);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_REPORT_ACHIEVEMENT_COMPLETE_EVENT, event, 1, this);
	}
	
	void onReportAchievementError(const char *caller, const char *value, const char *error){
		
		game_LoadError *event = (game_LoadError*)gevent_CreateEventStruct3(
			sizeof(game_LoadError),
			offsetof(game_LoadError, caller), caller,
			offsetof(game_LoadError, value), value,
			offsetof(game_LoadError, error), error);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_REPORT_ACHIEVEMENT_ERROR_EVENT, event, 1, this);
	}
	
	void onReportScoreComplete(const char *caller, const char *value, long score){
		
		game_ReportScore *event = (game_ReportScore*)gevent_CreateEventStruct2(
			sizeof(game_ReportScore),
			offsetof(game_ReportScore, caller), caller,
			offsetof(game_ReportScore, value), value);
			
		event->score = (long)score;
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_REPORT_SCORE_COMPLETE_EVENT, event, 1, this);
	}
	
	void onReportScoreError(const char *caller, const char *value, const char *error, long score){
		
		game_ReportScoreError *event = (game_ReportScoreError*)gevent_CreateEventStruct3(
			sizeof(game_ReportScoreError),
			offsetof(game_ReportScoreError, caller), caller,
			offsetof(game_ReportScoreError, value), value,
			offsetof(game_ReportScoreError, error), error);
			
			event->score = score;
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_REPORT_SCORE_ERROR_EVENT, event, 1, this);
	}
	
	void onLoadAchievementsComplete(const char* call, Achievement* ach){
		this->map2achievement(ach);
		
		size_t size = sizeof(game_Achievements);
		int count = (int)achievements.size();

		for (std::size_t i = 0; i < count; ++i)
		{
			size += sizeof(game_Achievement);
			size += achievements[i].id.size() + 1;
			size += achievements[i].name.size() + 1;
			size += achievements[i].description.size() + 1;
		}
		
		std::string caller(call);
		
		size += caller.size() + 1;
		
		// allocate it
		game_Achievements *event = (game_Achievements*)malloc(size);
		
		// and copy the data into it
		char *ptr = (char*)event + sizeof(game_Achievements);
		
		event->caller = ptr;
		strcpy(ptr, caller.c_str());
		ptr += caller.size() + 1;
		
		event->count = count;
		event->achievements = (game_Achievement*)ptr;
		
		ptr += achievements.size() * sizeof(game_Achievement);
		 
		for (std::size_t i = 0; i < count; ++i)
		{	
			event->achievements[i].id = ptr;
			strcpy(ptr, achievements[i].id.c_str());
			ptr += achievements[i].id.size() + 1;
		
			event->achievements[i].name = ptr;
			strcpy(ptr, achievements[i].name.c_str());
			ptr += achievements[i].name.size() + 1;
		
			event->achievements[i].description = ptr;
			strcpy(ptr, achievements[i].description.c_str());
			ptr += achievements[i].description.size() + 1;
		
			event->achievements[i].status = achievements[i].status;
			event->achievements[i].lastUpdate = achievements[i].lastUpdate;
			event->achievements[i].currentSteps = achievements[i].currentSteps;
			event->achievements[i].totalSteps = achievements[i].totalSteps;
		}
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOAD_ACHIEVEMENTS_COMPLETE_EVENT, event, 1, this);
	}
	
	void onLoadAchievementsError(const char *caller, const char *value){
		
		game_Report *event = (game_Report*)gevent_CreateEventStruct2(
			sizeof(game_Report),
			offsetof(game_Report, caller), caller,
			offsetof(game_Report, value), value);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOAD_ACHIEVEMENTS_ERROR_EVENT, event, 1, this);
	}
	
	void onLoadScoresComplete(const char* call, const char* jid, const char* jname, Score* score){
		this->map2score(score);
		
		size_t size = sizeof(game_Leaderboard);
		int count = (int)scores.size();
		
		for (std::size_t i = 0; i < count; ++i)
		{
			size += sizeof(game_Score);
			size += scores[i].rank.size() + 1;
			size += scores[i].score.size() + 1;
			size += scores[i].name.size() + 1;
			size += scores[i].playerId.size() + 1;
		}
		
		std::string Id(jid);
		std::string name(jname);
		std::string caller(call);
		
		size += Id.size() + 1;
		size += name.size() + 1;
		size += caller.size() + 1;
		
		// allocate it
		game_Leaderboard *event = (game_Leaderboard*)malloc(size);
		
		// and copy the data into it
		char *ptr = (char*)event + sizeof(game_Leaderboard);
		
		event->caller = ptr;
		strcpy(ptr, caller.c_str());
		ptr += caller.size() + 1;
		
		event->id = ptr;
		strcpy(ptr, Id.c_str());
		ptr += Id.size() + 1;
		
		event->name = ptr;
		strcpy(ptr, name.c_str());
		ptr += name.size() + 1;
		
		event->count = count;
		event->scores = (game_Score*)ptr;
		
		ptr += scores.size() * sizeof(game_Score);
		
		for (std::size_t i = 0; i < count; ++i)
		{	
			event->scores[i].rank = ptr;
			strcpy(ptr, scores[i].rank.c_str());
			ptr += scores[i].rank.size() + 1;
			
			event->scores[i].score = ptr;
			strcpy(ptr, scores[i].score.c_str());
			ptr += scores[i].score.size() + 1;
			
			event->scores[i].name = ptr;
			strcpy(ptr, scores[i].name.c_str());
			ptr += scores[i].name.size() + 1;
			
			event->scores[i].playerId = ptr;
			strcpy(ptr, scores[i].playerId.c_str());
			ptr += scores[i].playerId.size() + 1;
		
			event->scores[i].timestamp = scores[i].timestamp;
		}
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOAD_SCORES_COMPLETE_EVENT, event, 1, this);
	}
	
	void onLoadScoresError(const char *caller, const char *value, const char *error){
		
		game_LoadError *event = (game_LoadError*)gevent_CreateEventStruct3(
			sizeof(game_LoadError),
			offsetof(game_LoadError, caller), caller,
			offsetof(game_LoadError, value), value,
			offsetof(game_LoadError, error), error);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOAD_SCORES_ERROR_EVENT, event, 1, this);
	}
	
	void onStateLoaded(const char* type, int key, const void* data, size_t dataSize, int fresh)
	{
		size_t structSize = sizeof(game_StateLoaded);
		
		game_StateLoaded *event = (game_StateLoaded*)gevent_CreateEventStruct1(
			structSize + dataSize,
			offsetof(game_StateLoaded, type), type);
	
		event->data = (char*)event + structSize;
		memcpy((void*)event->data, data, dataSize);
		event->size = dataSize;
		
		event->key = key;
		event->fresh = fresh;
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_STATE_LOADED_EVENT, event, 1, this);
	}
	
	void onStateError(const char *type, int key, const char *error)
	{
		
		game_StateError *event = (game_StateError*)gevent_CreateEventStruct2(
			sizeof(game_StateError),
			offsetof(game_StateError, type), type,
			offsetof(game_StateError, error), error);
		event->key = key;
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_STATE_ERROR_EVENT, event, 1, this);
	}
	
	void onStateConflict(const char *type, int key, const char *ver, const void* localState, size_t localSize, const void* serverState, size_t serverSize)
	{
		size_t structSize = sizeof(game_StateConflict);
		
		game_StateConflict *event = (game_StateConflict*)gevent_CreateEventStruct2(
			structSize + localSize + serverSize,
			offsetof(game_StateConflict, type), type,
			offsetof(game_StateConflict, ver), ver);
		
		char *ptr = (char*)event + structSize;
		
		event->localData = ptr;
		memcpy((void*)event->localData, localState, localSize);
		event->localSize = localSize;
		ptr += localSize;
		
		event->serverData = ptr;
		memcpy((void*)event->serverData, serverState, serverSize);
		event->serverSize = serverSize;
		ptr += serverSize;
		
		event->key = key;
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_STATE_CONFLICT_EVENT, event, 1, this);
	}
	
	void onStateDeleted(const char *type, int key)
	{
		
		game_StateDeleted *event = (game_StateDeleted*)gevent_CreateEventStruct1(
			sizeof(game_StateDeleted),
			offsetof(game_StateDeleted, type), type);
		event->key = key;
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_STATE_DELETED_EVENT, event, 1, this);
	}
	
	g_id addCallback(gevent_Callback callback, void *udata)
	{
		return callbackList_.addCallback(callback, udata);
	}
	void removeCallback(gevent_Callback callback, void *udata)
	{
		callbackList_.removeCallback(callback, udata);
	}
	void removeCallbackWithGid(g_id gid)
	{
		callbackList_.removeCallbackWithGid(gid);
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		((GGame*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	std::vector<Achievement> achievements;
	std::vector<Score> scores;
	g_id gid_;
};

static GGame *s_game = NULL;

extern "C" {

int game_isAvailable()
{
	return 1;
}

void game_init()
{
	s_game = new GGame;
}

void game_cleanup()
{
	if(s_game)
	{
		delete s_game;
		s_game = NULL;
	}
}

void game_initialize(const char *ad)
{
	if(s_game)
	{
		s_game->init(ad);
	}
}

void game_destroy(const char *ad)
{
	if(s_game != NULL)
	{
		s_game->destroy(ad);
	}
}

void game_login(const char *ad, game_Parameter *params)
{
	if(s_game)
	{
		s_game->login(ad, params);
	}
}

void game_logout(const char *ad)
{
	if(s_game)
	{
		s_game->logout(ad);
	}
}

void game_reportScore(const char *game, const char *id, long score, int immediate)
{
	if(s_game)
	{
		s_game->reportScore(game, id, score, immediate);
	}
}

void game_showAchievements(const char *game)
{
	if(s_game)
	{
		s_game->showAchievements(game);
	}
}

void game_showLeaderboard(const char *game, const char *id)
{
	if(s_game)
	{
		s_game->showLeaderboard(game, id);
	}
}

void game_reportAchievement(const char *game, const char *id, int steps, int immediate)
{
	if(s_game)
	{
		s_game->reportAchievement(game, id, steps, immediate);
	}
}

void game_loadAchievements(const char *game)
{
	if(s_game)
	{
		s_game->loadAchievements(game);
	}
}

void game_loadScores(const char *game, const char *id, int span, int collection, int maxResults)
{
	if(s_game)
	{
		s_game->loadScores(game, id, span, collection, maxResults);
	}
}

void game_loadState(const char *game, int key)
{
	if(s_game)
		s_game->loadState(game, key);
}

void game_updateState(const char *game, int key, const void* data, size_t size, int immediate)
{
	if(s_game)
		s_game->updateState(game, key, data, size, immediate);
}

void game_resolveState(const char *game, int key, const char* ver, const void* data, size_t size)
{
	if(s_game)
		s_game->resolveState(game, key, ver, data, size);
}

void game_deleteState(const char *game, int key)
{
	if(s_game)
		s_game->deleteState(game, key);
}
    
void game_onLoginComplete(const char *caller){
    if(s_game)
        s_game->onLoginComplete(caller);
}
void game_onLoginError(const char *caller, const char *value){
    if(s_game)
        s_game->onLoginError(caller, value);
}
void game_onReportAchievementComplete(const char *caller, const char *value){
    if(s_game)
        s_game->onReportAchievementComplete(caller, value);
}
void game_onReportAchievementError(const char *caller, const char *value, const char *error){
    if(s_game)
        s_game->onReportAchievementError(caller, value, error);
}
void game_onReportScoreComplete(const char *caller, const char *value, long score){
    if(s_game)
        s_game->onReportScoreComplete(caller, value, score);
}
void game_onReportScoreError(const char *caller, const char *value, const char *error, long score){
    if(s_game)
        s_game->onReportScoreError(caller, value, error, score);
}
void game_onLoadAchievementsComplete(const char* call, Achievement* ach){
    if(s_game)
        s_game->onLoadAchievementsComplete(call, ach);
}
void game_onLoadAchievementsError(const char *caller, const char *value){
    if(s_game)
        s_game->onLoadAchievementsError(caller, value);
}
void game_onLoadScoresComplete(const char* call, const char* jid, const char* jname, Score* score){
    if(s_game)
        s_game->onLoadScoresComplete(call, jid, jname, score);
}
void game_onLoadScoresError(const char *caller, const char *value, const char *error){
    if(s_game)
        s_game->onLoadScoresError(caller, value, error);
}
void game_onStateLoaded(const char* type, int key, const void* data, size_t dataSize, int fresh){
    if(s_game)
        s_game->onStateLoaded(type, key, data, dataSize, fresh);
}
void game_onStateError(const char *type, int key, const char *error){
    if(s_game)
        s_game->onStateError(type, key, error);
}
void game_onStateConflict(const char *type, int key, const char *ver, const void* localState, size_t localSize, const void* serverState, size_t serverSize){
    if(s_game)
        s_game->onStateConflict(type, key, ver, localState, localSize, serverState, serverSize);
}
void game_onStateDeleted(const char *type, int key){
    if(s_game)
        s_game->onStateDeleted(type, key);
}

g_id game_addCallback(gevent_Callback callback, void *udata)
{
	return s_game->addCallback(callback, udata);
}

void game_removeCallback(gevent_Callback callback, void *udata)
{
	if(s_game)
	{
		s_game->removeCallback(callback, udata);
	}
}

void game_removeCallbackWithGid(g_id gid)
{
	if(s_game)
	{
		s_game->removeCallbackWithGid(gid);
	}
}

}
