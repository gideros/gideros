#include "gms.h"
#import "GooglePlayService.h"
#include <stdlib.h>
#include <glog.h>
#include <gapplication.h>

class GMS
{
public:
	GMS()
	{
		gid_ = g_NextId();
		[GooglePlayService initialize];
        gapplication_addCallback(openUrl_s, this);
	}

	~GMS()
	{
        gapplication_removeCallback(openUrl_s, this);
		[GooglePlayService deinitialize];
		gevent_RemoveEventsWithGid(gid_);
	}
	
	bool isAvailable()
	{
		return [GooglePlayService isAvailable];
	}
		
	void login()
	{
		[GooglePlayService login];
	}
	
	void logout()
	{
		[GooglePlayService logout];
	}
	
	void showSettings()
	{
		[GooglePlayService showSettings];
	}
	
	void showLeaderboard(const char *Id)
	{
		[GooglePlayService showLeaderboard:[NSString stringWithUTF8String:Id]];
	}
	
	void reportScore(const char *Id, long score, int immediate)
	{
		[GooglePlayService reportScore:[NSString stringWithUTF8String:Id] andScore: score with:immediate];
	}
	
	void showAchievements()
	{
		[GooglePlayService showAchievements];
	}
	
	void reportAchievement(const char *Id, int steps, int immediate)
	{
        if(steps == 0)
		{
            [GooglePlayService reportAchievement:[NSString stringWithUTF8String:Id] andSteps: steps with:immediate];
        }
        else{
            [GooglePlayService reportAchievement:[NSString stringWithUTF8String:Id] with:immediate];
        }
	}
	
	void loadAchievements()
	{
		[GooglePlayService loadAchievements];
	}
	
	void loadScores(const char *Id, int span, int collection, int maxResults )
	{
		[GooglePlayService loadScores:[NSString stringWithUTF8String:Id] andSpan:span forCollection:collection withResults:maxResults];
	}
	
	void loadPlayerScores(const char *Id, int span, int collection, int maxResults )
	{
		[GooglePlayService loadPlayerScores:[NSString stringWithUTF8String:Id] andSpan:span forCollection:collection withResults:maxResults];
	}
	
	const char* getCurrentPlayer()
	{
		return [[GooglePlayService getCurrentPlayer] UTF8String];
	}
	
	const char* getCurrentPlayerId()
	{
		return [[GooglePlayService getCurrentPlayerId] UTF8String];
	}
	
	void loadState(int key)
	{
		[GooglePlayService loadState: key];
	}

	void updateState(int key, const void* data, size_t size, int immediate)
	{
		[GooglePlayService updateState:[NSData dataWithBytes:data length:size] forKey: key withImmediate:immediate];
	}

	void resolveState(int key, const char* ver, const void* data, size_t size)
	{
		[GooglePlayService resolveState:[NSData dataWithBytes:data length:size] forKey:key andVersion:[NSString stringWithUTF8String:ver]];
	}
	
	void deleteState(int key)
	{
		[GooglePlayService deleteState: key];
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
	
	void onSignInFailed()
	{
		gevent_EnqueueEvent(gid_, callback_s, GMS_LOGIN_ERROR_EVENT, NULL, 1, this);
	}
	
	void onSignInSucceeded()
	{
		gevent_EnqueueEvent(gid_, callback_s, GMS_LOGIN_COMPLETE_EVENT, NULL, 1, this);
	}
	
	void onAchievementUpdated(const char* id)
	{
		gms_SimpleEvent *event = (gms_SimpleEvent*)gevent_CreateEventStruct1(
			sizeof(gms_SimpleEvent),
			offsetof(gms_SimpleEvent, id), id);
	
		gevent_EnqueueEvent(gid_, callback_s, GMS_REPORT_ACHIEVEMENT_COMPLETE_EVENT, event, 1, this);
	}
	
	void onScoreSubmitted()
	{
		gevent_EnqueueEvent(gid_, callback_s, GMS_REPORT_SCORE_COMPLETE_EVENT, NULL, 1, this);
	}
	
	void onAchievementsLoaded(Achievement* ach)
	{
		map2achievement(ach);
		size_t size = sizeof(gms_Achievements);
		int count = (int)achievements.size();
		
		for (std::size_t i = 0; i < count; ++i)
		{
			size += sizeof(gms_Achievement);
			size += achievements[i].id.size() + 1;
			size += achievements[i].name.size() + 1;
			size += achievements[i].description.size() + 1;
		}
		
		// allocate it
		gms_Achievements *event = (gms_Achievements*)malloc(size);
		
		// and copy the data into it
		char *ptr = (char*)event + sizeof(gms_Achievements);
		
		event->count = count;
		event->achievements = (gms_Achievement*)ptr;
		
		ptr += achievements.size() * sizeof(gms_Achievement);
		 
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
		
		gevent_EnqueueEvent(gid_, callback_s, GMS_LOAD_ACHIEVEMENTS_COMPLETE_EVENT, event, 1, this);
	}
	
	void onLeaderboardScoresLoaded(const char* jId, const char* jName, Score* score)
	{
		
		map2score(score);
		size_t size = sizeof(gms_Leaderboard);
		int count = (int)scores.size();
		
		for (std::size_t i = 0; i < count; ++i)
		{
			size += sizeof(gms_Score);
			size += scores[i].rank.size() + 1;
			size += scores[i].score.size() + 1;
			size += scores[i].name.size() + 1;
			size += scores[i].playerId.size() + 1;
		}
		
		std::string id = jId;
		std::string name = jName;
		
		size += id.size() + 1;
		size += name.size() + 1;
		
		// allocate it
		gms_Leaderboard *event = (gms_Leaderboard*)malloc(size);
		
		// and copy the data into it
		char *ptr = (char*)event + sizeof(gms_Leaderboard);
		
		event->id = ptr;
		strcpy(ptr, id.c_str());
		ptr += id.size() + 1;
		
		event->name = ptr;
		strcpy(ptr, name.c_str());
		ptr += name.size() + 1;
		
		event->count = count;
		event->scores = (gms_Score*)ptr;
		
		ptr += scores.size() * sizeof(gms_Score);
		
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
		
		gevent_EnqueueEvent(gid_, callback_s, GMS_LOAD_SCORES_COMPLETE_EVENT, event, 1, this);
	}
	
	void onStateLoaded(int key, const void* data, size_t size, int fresh)
	{	
		size_t structSize = sizeof(gms_StateLoaded);
		
		gms_StateLoaded *event = (gms_StateLoaded*)malloc(structSize + size);

		event->data = (char*)event + structSize;
		memcpy((void*)event->data, data, size);
		event->size = size;
		event->key = key;
		event->fresh = fresh;
		
		gevent_EnqueueEvent(gid_, callback_s, GMS_STATE_LOADED_EVENT, event, 1, this);
	}
	
	void onStateError(int key, const char* error)
	{	
		gms_StateError *event = (gms_StateError*)gevent_CreateEventStruct1(
			sizeof(gms_StateError),
			offsetof(gms_StateError, error), error);
		event->key = key;
		
		gevent_EnqueueEvent(gid_, callback_s, GMS_STATE_ERROR_EVENT, event, 1, this);
	}
	
	void onStateConflict(int key, const char* ver, const void* localState, size_t localSize, const void* serverState, size_t serverSize)
	{	
		size_t structSize = sizeof(gms_StateConflict);
		
		gms_StateConflict *event = (gms_StateConflict*)gevent_CreateEventStruct1(
			structSize + localSize + serverSize,
			offsetof(gms_StateConflict, ver), ver);
		
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
		
		gevent_EnqueueEvent(gid_, callback_s, GMS_STATE_CONFLICT_EVENT, event, 1, this);
	}
	
	void onStateDeleted(int key)
	{	
		gms_StateDeleted *event = (gms_StateDeleted*)malloc(sizeof(gms_StateDeleted));
		event->key = key;
		
		gevent_EnqueueEvent(gid_, callback_s, GMS_STATE_DELETED_EVENT, event, 1, this);
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
    static void openUrl_s(int type, void *event, void *udata)
    {
        static_cast<GMS*>(udata)->openUrl(type, event);
    }
    
    void openUrl(int type, void *event)
    {
        if (type == GAPPLICATION_OPEN_URL_EVENT)
        {
            gapplication_OpenUrlEvent *event2 = (gapplication_OpenUrlEvent*)event;
            [GooglePlayService handleOpenUrl:[NSURL URLWithString:[NSString stringWithUTF8String:event2->url]]];
        }
    }
private:
	static void callback_s(int type, void *event, void *udata)
	{
		((GMS*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	std::vector<gms_Player> player;
	std::vector<Achievement> achievements;
	std::vector<Score> scores;
	g_id gid_;
};


static GMS *s_gms = NULL;

extern "C" {

void gms_init()
{
	s_gms = new GMS;
}

void gms_cleanup()
{
	delete s_gms;
	s_gms = NULL;
}

bool gms_isAvailable()
{
	return s_gms->isAvailable();
}

void gms_login()
{
	s_gms->login();
}

void gms_logout()
{
	s_gms->logout();
}

void gms_showSettings()
{
	s_gms->showSettings();
}

void gms_showLeaderboard(const char *id)
{
	s_gms->showLeaderboard(id);
}

void gms_reportScore(const char *id, long score, int immediate)
{
	s_gms->reportScore(id, score, immediate);
}

void gms_showAchievements()
{
	s_gms->showAchievements();
}

void gms_reportAchievement(const char *id, int steps, int immediate)
{
	s_gms->reportAchievement(id, steps, immediate);
}

void gms_loadAchievements()
{
	s_gms->loadAchievements();
}

void gms_loadScores(const char *id, int span, int collection, int maxResults)
{
	s_gms->loadScores(id, span, collection, maxResults);
}

void gms_loadPlayerScores(const char *id, int span, int collection, int maxResults)
{
	s_gms->loadPlayerScores(id, span, collection, maxResults);
}

const char* gms_getCurrentPlayer()
{
	return s_gms->getCurrentPlayer();
}

const char* gms_getCurrentPlayerId()
{
	return s_gms->getCurrentPlayerId();
}

void gms_loadState(int key)
{
	s_gms->loadState(key);
}

void gms_updateState(int key, const void* data, size_t size, int immediate)
{
	s_gms->updateState(key, data, size, immediate);
}

void gms_resolveState(int key, const char* ver, const void* data, size_t size)
{
	s_gms->resolveState(key, ver, data, size);
}

void gms_deleteState(int key)
{
	s_gms->deleteState(key);
}

g_id gms_addCallback(gevent_Callback callback, void *udata)
{
	return s_gms->addCallback(callback, udata);
}

void gms_removeCallback(gevent_Callback callback, void *udata)
{
	s_gms->removeCallback(callback, udata);
}

void gms_removeCallbackWithGid(g_id gid)
{
	s_gms->removeCallbackWithGid(gid);
}
    
void gms_onSignInFailed()
{
    s_gms->onSignInFailed();
}
    
void gms_onSignInSucceeded()
{
    s_gms->onSignInSucceeded();
}
    
void gms_onAchievementUpdated(const char* Id)
{
    s_gms->onAchievementUpdated(Id);
}
    
void gms_onAchievementsLoaded(Achievement* ach)
{
    s_gms->onAchievementsLoaded(ach);
}
    
void gms_onLeaderboardScoresLoaded(const char* jId, const char* jName, Score* score)
{
    s_gms->onLeaderboardScoresLoaded(jId, jName, score);
}
    
void gms_onScoreSubmitted()
{
    s_gms->onScoreSubmitted();
}
    
void gms_onStateLoaded(int key, const void* data, size_t size, int fresh)
{
    s_gms->onStateLoaded(key, data, size, fresh);
}
	
void gms_onStateError(int key, const char* error)
{
	s_gms->onStateError(key, error);
}
	
void gms_onStateConflict(int key, const char* ver, const void* localState, size_t localSize, const void* serverState, size_t serverSizes)
{
	s_gms->onStateConflict(key, ver, localState, localSize, serverState, serverSizes);
}
	
void gms_onStateDeleted(int key)
{
	s_gms->onStateDeleted(key);
}

}
