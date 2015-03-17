#include <gamification.h>
#include <jni.h>
#include <stdlib.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static void *gevent_CreateEventStruct4(size_t structSize,
                                size_t offset1, const char *value1,
                                size_t offset2, const char *value2,
                                size_t offset3, const char *value3,
                                size_t offset4, const char *value4)
{
    size_t size1 = value1 ? (strlen(value1) + 1) : 0;
    size_t size2 = value2 ? (strlen(value2) + 1) : 0;
    size_t size3 = value3 ? (strlen(value3) + 1) : 0;
    size_t size4 = value4 ? (strlen(value4) + 1) : 0;

    void *result = malloc(structSize + size1 + size2 + size3 + size4);

    char **field1 = (char**)((char*)result + offset1);
    char **field2 = (char**)((char*)result + offset2);
    char **field3 = (char**)((char*)result + offset3);
    char **field4 = (char**)((char*)result + offset4);

    *field1 = value1 ? strcpy((char*)result + structSize,                 			value1) : NULL;
    *field2 = value2 ? strcpy((char*)result + structSize + size1,         			value2) : NULL;
    *field3 = value3 ? strcpy((char*)result + structSize + size1 + size2, 			value3) : NULL;
    *field4 = value3 ? strcpy((char*)result + structSize + size1 + size2 + size3, 	value4) : NULL;

    return result;
}

class GGame
{
public:
	GGame()
	{
		gid_ = g_NextId();
		
		JNIEnv *env = g_getJNIEnv();

		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/gaming/Game");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);
		
		jclass class_bundle = env->FindClass("android/os/Bundle");
		clsBundle = static_cast<jclass>(env->NewGlobalRef(class_bundle));
		env->DeleteLocalRef(class_bundle);
		
		jclass class_sparse = env->FindClass("android/util/SparseArray");
		clsSparse = static_cast<jclass>(env->NewGlobalRef(class_sparse));
		env->DeleteLocalRef(class_sparse);

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "init", "(J)V"), (jlong)this);
	}

	~GGame()
	{
		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "cleanup", "()V"));
		
		env->DeleteGlobalRef(cls_);
		env->DeleteGlobalRef(clsSparse);
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	void init(const char *ad)
	{
		JNIEnv *env = g_getJNIEnv();
		
		jstring jAd = env->NewStringUTF(ad);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "initialize", "(Ljava/lang/String;)V"), jAd);
		env->DeleteLocalRef(jAd);
	}
	
	void destroy(const char *ad)
	{
		JNIEnv *env = g_getJNIEnv();
		
		jstring jAd = env->NewStringUTF(ad);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "destroy", "(Ljava/lang/String;)V"), jAd);
		env->DeleteLocalRef(jAd);
	}
	
	void login(const char *ad, game_Parameter *params)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		//create Java object
		jobject jparams = env->NewObject(clsSparse, env->GetMethodID(clsSparse, "<init>", "()V"));
		int i = 0;
		while (params->value)
		{
			jstring jVal = env->NewStringUTF(params->value);
			env->CallVoidMethod(jparams, env->GetMethodID(clsSparse, "put", "(ILjava/lang/Object;)V"), (jint)i, jVal);
			env->DeleteLocalRef(jVal);
			++params;
			i++;
		}
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "login", "(Ljava/lang/String;Ljava/lang/Object;)V"), jAd, jparams);
		env->DeleteLocalRef(jparams);
		env->DeleteLocalRef(jAd);
	}
	
	void logout(const char *ad)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jAd = env->NewStringUTF(ad);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "logout", "(Ljava/lang/String;)V"), jAd);
		env->DeleteLocalRef(jAd);
	}
	
	void reportScore(const char *game, const char *id, long score, int immediate)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jGame = env->NewStringUTF(game);
		jstring jId = env->NewStringUTF(id);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "reportScore", "(Ljava/lang/String;Ljava/lang/String;JI)V"), jGame, jId, (jlong)score, (jint)immediate);
		env->DeleteLocalRef(jGame);
		env->DeleteLocalRef(jId);
	}
	
	void showAchievements(const char *game)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jGame = env->NewStringUTF(game);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "showAchievements", "(Ljava/lang/String;)V"), jGame);
		env->DeleteLocalRef(jGame);
	}
	
	void showLeaderboard(const char *game, const char *id)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jGame = env->NewStringUTF(game);
		jstring jId = env->NewStringUTF(id);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "showLeaderboard", "(Ljava/lang/String;Ljava/lang/String;)V"), jGame, jId);
		env->DeleteLocalRef(jId);
		env->DeleteLocalRef(jGame);
	}
	
	void reportAchievement(const char *game, const char *id, double steps, int immediate)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jGame = env->NewStringUTF(game);
		jstring jId = env->NewStringUTF(id);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "reportAchievement", "(Ljava/lang/String;Ljava/lang/String;DI)V"), jGame, jId, (jdouble)steps, (jint)immediate);
		env->DeleteLocalRef(jId);
		env->DeleteLocalRef(jGame);
	}
	
	void loadAchievements(const char *game)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jGame = env->NewStringUTF(game);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "loadAchievements", "(Ljava/lang/String;)V"), jGame);
		env->DeleteLocalRef(jGame);
	}
	
	void loadScores(const char *game, const char *id, int span, int collection, int maxResults)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jGame = env->NewStringUTF(game);
		jstring jId = env->NewStringUTF(id);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "loadScores", "(Ljava/lang/String;Ljava/lang/String;III)V"), jGame, jId, (jint)span, (jint)collection, (jint)maxResults);
		env->DeleteLocalRef(jId);
		env->DeleteLocalRef(jGame);
	}
	
	void loadState(const char *game, int key)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jGame = env->NewStringUTF(game);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "loadState", "(Ljava/lang/String;I)V"), jGame, (jint)key);
		env->DeleteLocalRef(jGame);
	}

	void updateState(const char *game, int key, const void* data, size_t size, int immediate)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jGame = env->NewStringUTF(game);
		jbyteArray jdata = env->NewByteArray(size);
		env->SetByteArrayRegion(jdata, 0, size, (const jbyte*)data);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "updateState", "(Ljava/lang/String;I[BI)V"), jGame, (jint)key, jdata, (jint)immediate);
		env->DeleteLocalRef(jGame);
		env->DeleteLocalRef(jdata);
	}

	void resolveState(const char *game, int key, const char* ver, const void* data, size_t size)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jGame = env->NewStringUTF(game);
		jstring jVer = env->NewStringUTF(ver);
		jbyteArray jdata = env->NewByteArray(size);
		env->SetByteArrayRegion(jdata, 0, size, (const jbyte*)data);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "resolveState", "(Ljava/lang/String;ILjava/lang/String;[B)V"), jGame, (jint)key, jVer, jdata);
		env->DeleteLocalRef(jGame);
		env->DeleteLocalRef(jVer);
		env->DeleteLocalRef(jdata);
	}
	
	void deleteState(const char *game, int key)
	{
		JNIEnv *env = g_getJNIEnv();
		jstring jGame = env->NewStringUTF(game);
		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "deleteState", "(Ljava/lang/String;I)V"), jGame, (jint)key);
		env->DeleteLocalRef(jGame);
	}
	
	std::string mapGetStr(const char *str, jobject jsubobj)
	{
		JNIEnv *env = g_getJNIEnv();
		//get value
		jstring jStr = env->NewStringUTF(str);
		jstring jretStr = (jstring)env->CallObjectMethod(jsubobj, env->GetMethodID(clsBundle, "getString", "(Ljava/lang/String;)Ljava/lang/String;"), jStr);
		env->DeleteLocalRef(jStr);
	
		const char *retVal = env->GetStringUTFChars(jretStr, NULL);
		std::string result = retVal;
		env->ReleaseStringUTFChars(jretStr, retVal);

		return result;
	}
	
	int mapGetInt(const char *str, jobject jsubobj)
	{
		JNIEnv *env = g_getJNIEnv();
		//get value
		jstring jStr = env->NewStringUTF(str);
		int ret = (int)env->CallIntMethod(jsubobj, env->GetMethodID(clsBundle, "getInt", "(Ljava/lang/String;)I"), jStr);
		env->DeleteLocalRef(jStr);
		
		return ret;
	}
	
	void map2achievement(jobject jmapobj)
	{
		JNIEnv *env = g_getJNIEnv();
		int size = (int)env->CallIntMethod(jmapobj, env->GetMethodID(clsSparse, "size", "()I"));
		if(size == 0)
		{
			return;
		}
		
		achievements.clear();
		
		for (int i = 0; i < size; i++) {
			jobject jsubobj = env->CallObjectMethod(jmapobj, env->GetMethodID(clsSparse, "valueAt", "(I)Ljava/lang/Object;"), (jint)i);
			
			Achievement gach = {this->mapGetStr("id", jsubobj), this->mapGetStr("name", jsubobj), this->mapGetStr("description", jsubobj), this->mapGetInt("status", jsubobj), this->mapGetInt("lastUpdate", jsubobj), this->mapGetInt("currentSteps", jsubobj), this->mapGetInt("totalSteps", jsubobj)};
			
			achievements.push_back(gach);
			
			env->DeleteLocalRef(jsubobj);
		}
	}
	
	void map2score(jobject jmapobj)
	{
		JNIEnv *env = g_getJNIEnv();
		int size = (int)env->CallIntMethod(jmapobj, env->GetMethodID(clsSparse, "size", "()I"));
		if(size == 0)
		{
			return;
		}
		
		scores.clear();
		
		for (int i = 0; i < size; i++) {
			jobject jsubobj = env->CallObjectMethod(jmapobj, env->GetMethodID(clsSparse, "valueAt", "(I)Ljava/lang/Object;"), (jint)i);
			
			Score gscores = {this->mapGetStr("rank", jsubobj), this->mapGetStr("score", jsubobj), this->mapGetStr("name", jsubobj), this->mapGetStr("playerId", jsubobj), this->mapGetStr("pic", jsubobj) , this->mapGetInt("timestamp", jsubobj)};
			
			scores.push_back(gscores);
			
			env->DeleteLocalRef(jsubobj);
		}
	}
	
	std::string MyGetStringUTFChars(JNIEnv *env, jstring jstr)
	{
		const char *str = env->GetStringUTFChars(jstr, NULL);
		std::string result = str;
		env->ReleaseStringUTFChars(jstr, str);
		return result;
	}
	
	void onLoginComplete(jstring jCaller){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		
		game_SimpleEvent *event = (game_SimpleEvent*)gevent_CreateEventStruct1(
			sizeof(game_SimpleEvent),
			offsetof(game_SimpleEvent, id), caller);
	
		env->ReleaseStringUTFChars(jCaller, caller);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOGIN_COMPLETE_EVENT, event, 1, this);
	}
	
	void onLoginError(jstring jCaller, jstring jError){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *value = env->GetStringUTFChars(jError, NULL);
		
		game_Report *event = (game_Report*)gevent_CreateEventStruct2(
			sizeof(game_Report),
			offsetof(game_Report, caller), caller,
			offsetof(game_Report, value), value);
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jError, value);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOGIN_ERROR_EVENT, event, 1, this);
	}
	
	void onPlayerInfoComplete(jstring jCaller, jstring jId, jstring jName, jstring jPic){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *id = env->GetStringUTFChars(jId, NULL);
		const char *name = env->GetStringUTFChars(jName, NULL);
		const char *pic = env->GetStringUTFChars(jPic, NULL);
		
		game_Player *event = (game_Player*)gevent_CreateEventStruct4(
			sizeof(game_Player),
			offsetof(game_Player, caller), caller,
			offsetof(game_Player, playerId), id,
			offsetof(game_Player, name), name,
			offsetof(game_Player, pic), pic);
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jId, id);
		env->ReleaseStringUTFChars(jName, name);
		env->ReleaseStringUTFChars(jPic, pic);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_PLAYER_INFORMATION_COMPLETE_EVENT, event, 1, this);
	}
	
	void onPlayerInfoError(jstring jCaller, jstring jError){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *value = env->GetStringUTFChars(jError, NULL);
		
		game_Report *event = (game_Report*)gevent_CreateEventStruct2(
			sizeof(game_Report),
			offsetof(game_Report, caller), caller,
			offsetof(game_Report, value), value);
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jError, value);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_PLAYER_INFORMATION_ERROR_EVENT, event, 1, this);
	}
	
	void onPlayerScoreComplete(jstring jCaller, jstring jId, jint rank, jlong score, jint timestamp){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *value = env->GetStringUTFChars(jId, NULL);
		
		game_PlayerScore *event = (game_PlayerScore*)gevent_CreateEventStruct2(
			sizeof(game_PlayerScore),
			offsetof(game_PlayerScore, caller), caller,
			offsetof(game_PlayerScore, id), value);
		
		event->rank = (int)rank;
		event->score = (long)score;
		event->timestamp = (int)timestamp;
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jId, value);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_PLAYER_SCORE_COMPLETE_EVENT, event, 1, this);
	}
	
	void onPlayerScoreError(jstring jCaller, jstring jId, jstring jError){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *value = env->GetStringUTFChars(jId, NULL);
		const char *error = env->GetStringUTFChars(jError, NULL);
		
		game_LoadError *event = (game_LoadError*)gevent_CreateEventStruct3(
			sizeof(game_LoadError),
			offsetof(game_LoadError, caller), caller,
			offsetof(game_LoadError, value), value,
			offsetof(game_LoadError, error), error);
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jId, value);
		env->ReleaseStringUTFChars(jError, error);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_PLAYER_SCORE_ERROR_EVENT, event, 1, this);
	}
	
	void onReportAchievementComplete(jstring jCaller, jstring jId){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *value = env->GetStringUTFChars(jId, NULL);
		
		game_Report *event = (game_Report*)gevent_CreateEventStruct2(
			sizeof(game_Report),
			offsetof(game_Report, caller), caller,
			offsetof(game_Report, value), value);
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jId, value);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_REPORT_ACHIEVEMENT_COMPLETE_EVENT, event, 1, this);
	}
	
	void onReportAchievementError(jstring jCaller, jstring jId, jstring jError){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *value = env->GetStringUTFChars(jId, NULL);
		const char *error = env->GetStringUTFChars(jError, NULL);
		
		game_LoadError *event = (game_LoadError*)gevent_CreateEventStruct3(
			sizeof(game_LoadError),
			offsetof(game_LoadError, caller), caller,
			offsetof(game_LoadError, value), value,
			offsetof(game_LoadError, error), error);
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jId, value);
		env->ReleaseStringUTFChars(jError, error);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_REPORT_ACHIEVEMENT_ERROR_EVENT, event, 1, this);
	}
	
	void onReportScoreComplete(jstring jCaller, jstring jId, jlong score){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *value = env->GetStringUTFChars(jId, NULL);
		
		game_ReportScore *event = (game_ReportScore*)gevent_CreateEventStruct2(
			sizeof(game_ReportScore),
			offsetof(game_ReportScore, caller), caller,
			offsetof(game_ReportScore, value), value);
			
		event->score = (long)score;
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jId, value);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_REPORT_SCORE_COMPLETE_EVENT, event, 1, this);
	}
	
	void onReportScoreError(jstring jCaller, jstring jId, jstring jError, jlong score){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *value = env->GetStringUTFChars(jId, NULL);
		const char *error = env->GetStringUTFChars(jError, NULL);
		
		game_ReportScoreError *event = (game_ReportScoreError*)gevent_CreateEventStruct3(
			sizeof(game_ReportScoreError),
			offsetof(game_ReportScoreError, caller), caller,
			offsetof(game_ReportScoreError, value), value,
			offsetof(game_ReportScoreError, error), error);
			
			event->score = (long)score;
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jId, value);
		env->ReleaseStringUTFChars(jError, error);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_REPORT_SCORE_ERROR_EVENT, event, 1, this);
	}
	
	void onLoadAchievementsComplete(jstring jCaller, jobject jAch){
		JNIEnv *env = g_getJNIEnv();
		this->map2achievement(jAch);
		
		size_t size = sizeof(game_Achievements);
		int count = (int)achievements.size();
		
		for (std::size_t i = 0; i < count; ++i)
		{
			size += sizeof(game_Achievement);
			size += achievements[i].id.size() + 1;
			size += achievements[i].name.size() + 1;
			size += achievements[i].description.size() + 1;
		}
		
		std::string caller = MyGetStringUTFChars(env, jCaller);
		
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
	
	void onLoadAchievementsError(jstring jCaller, jstring jError){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *value = env->GetStringUTFChars(jError, NULL);
		
		game_Report *event = (game_Report*)gevent_CreateEventStruct2(
			sizeof(game_Report),
			offsetof(game_Report, caller), caller,
			offsetof(game_Report, value), value);
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jError, value);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOAD_ACHIEVEMENTS_ERROR_EVENT, event, 1, this);
	}
	
	void onLoadScoresComplete(jstring jCaller, jstring jId, jstring jName, jobject jScores){
		JNIEnv *env = g_getJNIEnv();
		this->map2score(jScores);
		
		size_t size = sizeof(game_Leaderboard);
		int count = (int)scores.size();
		
		for (std::size_t i = 0; i < count; ++i)
		{
			size += sizeof(game_Score);
			size += scores[i].rank.size() + 1;
			size += scores[i].score.size() + 1;
			size += scores[i].name.size() + 1;
			size += scores[i].playerId.size() + 1;
			size += scores[i].pic.size() + 1;
		}
		
		std::string id = MyGetStringUTFChars(env, jId);
		std::string name = MyGetStringUTFChars(env, jName);
		std::string caller = MyGetStringUTFChars(env, jCaller);
		
		size += id.size() + 1;
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
		strcpy(ptr, id.c_str());
		ptr += id.size() + 1;
		
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
			
			event->scores[i].pic = ptr;
			strcpy(ptr, scores[i].pic.c_str());
			ptr += scores[i].pic.size() + 1;
		
			event->scores[i].timestamp = scores[i].timestamp;
		}
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOAD_SCORES_COMPLETE_EVENT, event, 1, this);
	}
	
	void onLoadScoresError(jstring jCaller, jstring jId, jstring jError){
		JNIEnv *env = g_getJNIEnv();

		const char *caller = env->GetStringUTFChars(jCaller, NULL);
		const char *value = env->GetStringUTFChars(jId, NULL);
		const char *error = env->GetStringUTFChars(jError, NULL);
		
		game_LoadError *event = (game_LoadError*)gevent_CreateEventStruct3(
			sizeof(game_LoadError),
			offsetof(game_LoadError, caller), caller,
			offsetof(game_LoadError, value), value,
			offsetof(game_LoadError, error), error);
	
		env->ReleaseStringUTFChars(jCaller, caller);
		env->ReleaseStringUTFChars(jId, value);
		env->ReleaseStringUTFChars(jError, error);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_LOAD_SCORES_ERROR_EVENT, event, 1, this);
	}
	
	void onStateLoaded(jstring jCaller, jint key, jbyteArray jState, jint fresh)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *type = env->GetStringUTFChars(jCaller, NULL);
		size_t structSize = sizeof(game_StateLoaded);
		size_t dataSize = env->GetArrayLength(jState);
		
		game_StateLoaded *event = (game_StateLoaded*)gevent_CreateEventStruct1(
			structSize + dataSize,
			offsetof(game_StateLoaded, type), type);
	
		event->data = (char*)event + structSize;
		env->GetByteArrayRegion(jState, 0, dataSize, (jbyte*)event->data);
		event->size = dataSize;
		
		event->key = key;
		event->fresh = fresh;
		
		env->ReleaseStringUTFChars(jCaller, type);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_STATE_LOADED_EVENT, event, 1, this);
	}
	
	void onStateError(jstring jCaller, jint key, jstring jErr)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *type = env->GetStringUTFChars(jCaller, NULL);
		const char *error = env->GetStringUTFChars(jErr, NULL);
		
		game_StateError *event = (game_StateError*)gevent_CreateEventStruct2(
			sizeof(game_StateError),
			offsetof(game_StateError, type), type,
			offsetof(game_StateError, error), error);
		event->key = key;
		
		env->ReleaseStringUTFChars(jCaller, type);
		env->ReleaseStringUTFChars(jErr, error);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_STATE_ERROR_EVENT, event, 1, this);
	}
	
	void onStateConflict(jstring jCaller, jint key, jstring jVer, jbyteArray jlocalState, jbyteArray jserverState)
	{
		JNIEnv *env = g_getJNIEnv();
		const char *type = env->GetStringUTFChars(jCaller, NULL);
		const char *ver = env->GetStringUTFChars(jVer, NULL);
		
		size_t structSize = sizeof(game_StateConflict);
		size_t localSize = env->GetArrayLength(jlocalState);
		size_t serverSize = env->GetArrayLength(jserverState);
		
		game_StateConflict *event = (game_StateConflict*)gevent_CreateEventStruct2(
			structSize + localSize + serverSize,
			offsetof(game_StateConflict, type), type,
			offsetof(game_StateConflict, ver), ver);
		
		char *ptr = (char*)event + structSize;
		
		event->localData = ptr;
		env->GetByteArrayRegion(jlocalState, 0, localSize, (jbyte*)event->localData);
		event->localSize = localSize;
		ptr += localSize;
		
		event->serverData = ptr;
		env->GetByteArrayRegion(jserverState, 0, serverSize, (jbyte*)event->serverData);
		event->serverSize = serverSize;
		ptr += serverSize;
		
		event->key = key;
		
		env->ReleaseStringUTFChars(jCaller, type);
		env->ReleaseStringUTFChars(jVer, ver);
		
		gevent_EnqueueEvent(gid_, callback_s, GAME_STATE_CONFLICT_EVENT, event, 1, this);
	}
	
	void onStateDeleted(jstring jCaller, jint key)
	{
		JNIEnv *env = g_getJNIEnv();
		
		const char *type = env->GetStringUTFChars(jCaller, NULL);
		
		game_StateDeleted *event = (game_StateDeleted*)gevent_CreateEventStruct1(
			sizeof(game_StateDeleted),
			offsetof(game_StateDeleted, type), type);
		event->key = key;
		
		env->ReleaseStringUTFChars(jCaller, type);
		
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
	jclass cls_;
	jclass clsSparse;
	jclass clsBundle;
	std::vector<Achievement> achievements;
	std::vector<Score> scores;
	g_id gid_;
};

extern "C" {

void Java_com_giderosmobile_android_plugins_gaming_Game_onLoginComplete(JNIEnv *env, jclass clz, jstring jCaller, jlong data)
{
	((GGame*)data)->onLoginComplete(jCaller);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onLoginError(JNIEnv *env, jclass clz, jstring jCaller, jstring jError, jlong data)
{
	((GGame*)data)->onLoginError(jCaller, jError);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onPlayerInfoComplete(JNIEnv *env, jclass clz, jstring jCaller, jstring jId, jstring jName, jstring jPic, jlong data)
{
	((GGame*)data)->onPlayerInfoComplete(jCaller, jId, jName, jPic);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onPlayerInfoError(JNIEnv *env, jclass clz, jstring jCaller, jstring jError, jlong data)
{
	((GGame*)data)->onPlayerInfoError(jCaller, jError);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onPlayerScoreComplete(JNIEnv *env, jclass clz, jstring jCaller, jstring jId, jint rank, jlong score, jint timestamp, jlong data)
{
	((GGame*)data)->onPlayerScoreComplete(jCaller, jId, rank, score, timestamp);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onPlayerScoreError(JNIEnv *env, jclass clz, jstring jCaller, jstring jId, jstring jError, jlong data)
{
	((GGame*)data)->onPlayerScoreError(jCaller, jId, jError);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onReportAchievementComplete(JNIEnv *env, jclass clz, jstring jCaller, jstring jId, jlong data)
{
	((GGame*)data)->onReportAchievementComplete(jCaller, jId);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onReportAchievementError(JNIEnv *env, jclass clz, jstring jCaller, jstring jId, jstring jError, jlong data)
{
	((GGame*)data)->onReportAchievementError(jCaller, jId, jError);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onReportScoreComplete(JNIEnv *env, jclass clz, jstring jCaller, jstring jId, jlong jScore, jlong data)
{
	((GGame*)data)->onReportScoreComplete(jCaller, jId, jScore);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onReportScoreError(JNIEnv *env, jclass clz, jstring jCaller, jstring jId, jstring jError, jlong jScore, jlong data)
{
	((GGame*)data)->onReportScoreError(jCaller, jId, jError, jScore);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onLoadAchievementsComplete(JNIEnv *env, jclass clz, jstring jCaller, jobject jOb, jlong data)
{
	((GGame*)data)->onLoadAchievementsComplete(jCaller, jOb);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onLoadAchievementsError(JNIEnv *env, jclass clz, jstring jCaller, jstring jError, jlong data)
{
	((GGame*)data)->onLoadAchievementsError(jCaller, jError);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onLoadScoresComplete(JNIEnv *env, jclass clz, jstring jCaller, jstring jId, jstring jName, jobject jOb, jlong data)
{
	((GGame*)data)->onLoadScoresComplete(jCaller, jId, jName, jOb);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onLoadScoresError(JNIEnv *env, jclass clz, jstring jCaller, jstring jId, jstring jError, jlong data)
{
	((GGame*)data)->onLoadScoresError(jCaller, jId, jError);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onStateLoaded(JNIEnv *env, jclass clz, jstring jCaller, jint key, jbyteArray state, jint fresh, jlong data)
{
	((GGame*)data)->onStateLoaded(jCaller, key, state, fresh);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onStateError(JNIEnv *env, jclass clz, jstring jCaller, jint key, jstring error, jlong data)
{
	((GGame*)data)->onStateError(jCaller, key, error);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onStateConflict(JNIEnv *env, jclass clz, jstring jCaller, jint key, jstring ver, jbyteArray localState, jbyteArray serverState, jlong data)
{
	((GGame*)data)->onStateConflict(jCaller, key, ver, localState, serverState);
}

void Java_com_giderosmobile_android_plugins_gaming_Game_onStateDeleted(JNIEnv *env, jclass clz, jstring jCaller, jint key, jlong data)
{
	((GGame*)data)->onStateDeleted(jCaller, key);
}

}

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

void game_reportAchievement(const char *game, const char *id, double steps, int immediate)
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
