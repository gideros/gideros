/*

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2012 Gideros Mobile

Scores additions by Bowerhaus LLP
*/

#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"

#import <GameKit/GameKit.h> 

static const char KEY_OBJECTS = ' ';

static const char* AUTHENTICATE_COMPLETE = "authenticateComplete";
static const char* LOAD_FRIENDS_COMPLETE = "loadFriendsComplete";
static const char* LOAD_PLAYERS_COMPLETE = "loadPlayersComplete";
static const char* LOAD_SCORES_COMPLETE = "loadScoresComplete";
static const char* REPORT_SCORE_COMPLETE = "reportScoreComplete";
static const char* LOAD_ACHIEVEMENTS_COMPLETE = "loadAchievementsComplete";
static const char* REPORT_ACHIEVEMENT_COMPLETE = "reportAchievementComplete";
static const char* RESET_ACHIEVEMENTS_COMPLETE = "resetAchievementsComplete";
static const char* LOAD_ACHIEVEMENT_DESCRIPTIONS_COMPLETE = "loadAchievementDescriptionsComplete";

static const char* TODAY = "today";
static const char* WEEK = "week";
static const char* ALL_TIME = "allTime";

static const char* FRIENDS = "friends";
static const char* ALL_PLAYERS = "allPlayers";


@interface GGameKitHelper : NSObject <GKLeaderboardViewControllerDelegate, GKAchievementViewControllerDelegate>
@end

@implementation GGameKitHelper

-(void) presentViewController:(UIViewController*)vc
{
	UIViewController* rootVC = g_getRootViewController();
	[rootVC presentModalViewController:vc animated:YES];
}

-(void) dismissModalViewController
{
	UIViewController* rootVC = g_getRootViewController();
	[rootVC dismissModalViewControllerAnimated:YES];
}

- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController
{
	[self dismissModalViewController];
}

-(void) achievementViewControllerDidFinish:(GKAchievementViewController*)viewController
{
	[self dismissModalViewController];
}

@end

class GGameKit;

struct GGameKitEvent
{
    GGameKitEvent()
    {
    }

    GGameKitEvent(GGameKit *gameKit,
                  char const *type,
                  NSError *error,
                  NSArray *friends,
                  NSArray *players,
                  NSArray *achievements,
                  NSArray *achievementDescriptions,
                  GKLocalPlayer *localPlayer,
                  GKScore *localPlayerScore,
                  NSArray *scores) :
        gameKit(gameKit),
        type(type),
        error(error),
        friends(friends),
        players(players),
        achievements(achievements),
        achievementDescriptions(achievementDescriptions),
        localPlayer(localPlayer),
        localPlayerScore(localPlayerScore),
        scores(scores)
    {
    }

    GGameKit *gameKit;
    char const *type;
    NSError *error;
    NSArray *friends;
    NSArray *players;
    NSArray *achievements;
    NSArray *achievementDescriptions;
    GKLocalPlayer *localPlayer;
    GKScore *localPlayerScore;
    NSArray *scores;
};

@interface GGameKitEventPerformer : NSObject
{
    GGameKitEvent event;
}

- (id)init:(GGameKitEvent)event;

@end

class GGameKit : public GEventDispatcherProxy
{
public:
	GGameKit(lua_State* L) : L(L)
	{
		helper = [[GGameKitHelper alloc] init];
	}
	
	GGameKit()
	{
		[helper release];
	}
	
	BOOL isAvailable()
	{
		Class gcClass = (NSClassFromString(@"GKLocalPlayer"));
		
		NSString *reqSysVer = @"4.1";
		NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
		
		BOOL osVersionSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
		
		return (gcClass && osVersionSupported);		
	}
	
	void authenticate()
	{
		if (isAvailable() == NO)
			return;
		
		[[GKLocalPlayer localPlayer] authenticateWithCompletionHandler:^(NSError* error)
		{
            [[[GGameKitEventPerformer alloc] init:GGameKitEvent(this, AUTHENTICATE_COMPLETE, error, NULL, NULL, NULL, NULL, [GKLocalPlayer localPlayer], NULL, NULL)] autorelease];
		}];
	}
	
	void loadFriends()
	{
		if (isAvailable() == NO)
			return;

		[[GKLocalPlayer localPlayer] loadFriendsWithCompletionHandler:^(NSArray* friends, NSError* error)
		{
            [[[GGameKitEventPerformer alloc] init:GGameKitEvent(this, LOAD_FRIENDS_COMPLETE, error, friends, NULL, NULL, NULL, NULL, NULL, NULL)] autorelease];
		}];
	}
	
	void loadPlayers(NSArray* identifiers)
	{
		if (isAvailable() == NO)
			return;

		[GKPlayer loadPlayersForIdentifiers:identifiers withCompletionHandler:^(NSArray* players, NSError* error)
		{
            [[[GGameKitEventPerformer alloc] init:GGameKitEvent(this, LOAD_PLAYERS_COMPLETE, error, NULL, players, NULL, NULL, NULL, NULL, NULL)] autorelease];
		}];
	}
	
	void reportScore(int64_t value, NSString* category)
	{
		if (isAvailable() == NO)
			return;

		GKScore* score = [[[GKScore alloc] initWithCategory:category] autorelease];
		score.value = value;	
		[score reportScoreWithCompletionHandler:^(NSError* error)
		{
            [[[GGameKitEventPerformer alloc] init:GGameKitEvent(this, REPORT_SCORE_COMPLETE, error, NULL, NULL, NULL, NULL, NULL, NULL, NULL)] autorelease];
		}];
	}
	
	void showLeaderboard(NSString* category, GKLeaderboardTimeScope timeScope)
	{
		if (isAvailable() == NO)
			return;

		GKLeaderboardViewController* leaderboardVC = [[[GKLeaderboardViewController alloc] init] autorelease];
		if (leaderboardVC != nil)
		{
			if (category != nil)
				leaderboardVC.category = category;
			leaderboardVC.timeScope = timeScope;
			leaderboardVC.leaderboardDelegate = helper;
			[helper presentViewController:leaderboardVC];
		}
	}

    void loadScores(NSString *category, GKLeaderboardTimeScope timeScope, GKLeaderboardPlayerScope playerScope, int startEntry, int maxEntries)
    {
        if (isAvailable() == NO)
			return;

        GKLeaderboard *lb = [[[GKLeaderboard alloc] init] autorelease];
        lb.category = category;
        lb.timeScope = timeScope;
        lb.playerScope = playerScope;
        
        NSRange range;
        range.location = startEntry;
        range.length = maxEntries;
        lb.range = range;
		
        [lb loadScoresWithCompletionHandler:^(NSArray* scores, NSError* error)
		{
            [[[GGameKitEventPerformer alloc] init:GGameKitEvent(this, LOAD_SCORES_COMPLETE, error, NULL, NULL, NULL, NULL, NULL, lb.localPlayerScore, scores)] autorelease];
		}];
    }
	
	void showAchievements()
	{
		if (isAvailable() == NO)
			return;

		GKAchievementViewController* achievementsVC = [[[GKAchievementViewController alloc] init] autorelease];
		if (achievementsVC != nil)
		{
			achievementsVC.achievementDelegate = helper;
			[helper presentViewController:achievementsVC];
		}	
	}

	void loadAchievements()
	{
		if (isAvailable() == NO)
			return;

		[GKAchievement loadAchievementsWithCompletionHandler:^(NSArray* achievements, NSError* error)
		{
            [[[GGameKitEventPerformer alloc] init:GGameKitEvent(this, LOAD_ACHIEVEMENTS_COMPLETE, error, NULL, NULL, achievements, NULL, NULL, NULL, NULL)] autorelease];
		}];
	}
	
	void reportAchievement(NSString* identifier, double percentComplete, BOOL showsCompletionBanner)
	{
		if (isAvailable() == NO)
			return;

		GKAchievement* achievement = [[[GKAchievement alloc] initWithIdentifier:identifier] autorelease];
		achievement.percentComplete = percentComplete;
        if([achievement respondsToSelector:@selector(setShowsCompletionBanner:)])
            achievement.showsCompletionBanner = showsCompletionBanner;

        [achievement reportAchievementWithCompletionHandler:^(NSError* error)
		{
            [[[GGameKitEventPerformer alloc] init:GGameKitEvent(this, REPORT_ACHIEVEMENT_COMPLETE, error, NULL, NULL, NULL, NULL, NULL, NULL, NULL)] autorelease];
		}];	
	}
	
	void resetAchievements()
	{
		if (isAvailable() == NO)
			return;

		[GKAchievement resetAchievementsWithCompletionHandler:^(NSError* error)
		{
            [[[GGameKitEventPerformer alloc] init:GGameKitEvent(this, RESET_ACHIEVEMENTS_COMPLETE, error, NULL, NULL, NULL, NULL, NULL, NULL, NULL)] autorelease];
		}];
	}
	
	void loadAchievementDescriptions()
	{
		if (isAvailable() == NO)
			return;

		[GKAchievementDescription loadAchievementDescriptionsWithCompletionHandler:^(NSArray* achievementDescriptions, NSError* error)
		{
            [[[GGameKitEventPerformer alloc] init:GGameKitEvent(this, LOAD_ACHIEVEMENT_DESCRIPTIONS_COMPLETE, error, NULL, NULL, NULL, achievementDescriptions, NULL, NULL, NULL)] autorelease];
		}];
	}
	
    static void pushScoreAsTable(lua_State *L, GKScore *score) {
        // Helper method that takes a GKScore and pushes the contents as a table onto the Lua stack.
        lua_newtable(L);
        lua_pushstring(L, [score.playerID UTF8String]);
        lua_setfield(L, -2, "playerID");
        lua_pushstring(L, [score.category UTF8String]);
        lua_setfield(L, -2, "category");
        lua_pushnumber(L, score.value);
        lua_setfield(L, -2, "value");
        lua_pushstring(L, [score.formattedValue UTF8String]);
        lua_setfield(L, -2, "formattedValue");

        // Don't include context yet as this is only available wth iOS 5.0 and later.
        // Ideally we should guard this with a version check.
        // lua_pushnumber(L, score.context);
        // lua_setfield(L, -2, "context");

        lua_pushnumber(L, score.rank);
        lua_setfield(L, -2, "rank");
        NSDateFormatter *dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
        [dateFormatter setDateFormat: @"yyyy-MM-dd HH:mm:ss"];
        lua_pushstring(L, [[dateFormatter stringFromDate:score.date] UTF8String]);
        lua_setfield(L, -2, "date");
    }

	void dispatchEvent(char const *type, NSError *error, NSArray *friends, NSArray *players, NSArray *achievements, NSArray *achievementDescriptions, GKLocalPlayer *localPlayer, GKScore *localPlayerScore, NSArray* scores)
	{
		lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
		lua_rawget(L, LUA_REGISTRYINDEX);
		
		lua_rawgeti(L, -1, 1);

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
		lua_pushstring(L, type);
		lua_call(L, 1, 1);
		
		if (error)
		{
			lua_pushinteger(L, error.code);
			lua_setfield(L, -2, "errorCode");
			
			lua_pushstring(L, [error.localizedDescription UTF8String]);
			lua_setfield(L, -2, "errorDescription");			
		}
		
		if (friends)
		{
			lua_newtable(L);
			for(int i = 0; i < [friends count]; ++i)
			{
				NSString* playerId = [friends objectAtIndex:i];
				lua_pushstring(L, [playerId UTF8String]);
				lua_rawseti(L, -2, i + 1);
			}
			lua_setfield(L, -2, "friends");
		}
		
		if (players)
		{
			lua_newtable(L);
			for(int i = 0; i < [players count]; ++i)
			{
				GKPlayer* player = [players objectAtIndex:i];

				lua_newtable(L);
				lua_pushstring(L, [player.playerID UTF8String]);
				lua_setfield(L, -2, "playerID");
				lua_pushstring(L, [player.alias UTF8String]);
				lua_setfield(L, -2, "alias");
				lua_pushboolean(L, player.isFriend);
				lua_setfield(L, -2, "isFriend");

				lua_rawseti(L, -2, i + 1);				
			}
			lua_setfield(L, -2, "players");
		}
		
		if (achievements)
		{
			lua_newtable(L);
			for(int i = 0; i < [achievements count]; ++i)
			{
				GKAchievement* achievement = [achievements objectAtIndex:i];
				
				lua_newtable(L);
				lua_pushstring(L, [achievement.identifier UTF8String]);
				lua_setfield(L, -2, "identifier");
				lua_pushnumber(L, achievement.percentComplete);
				lua_setfield(L, -2, "percentComplete");
				lua_pushboolean(L, achievement.completed);
				lua_setfield(L, -2, "completed");
				lua_pushboolean(L, achievement.hidden);
				lua_setfield(L, -2, "hidden");
				
				lua_rawseti(L, -2, i + 1);				
			}
			lua_setfield(L, -2, "achievements");
		}
		
		if (achievementDescriptions)
		{
			lua_newtable(L);
			for(int i = 0; i < [achievementDescriptions count]; ++i)
			{
				GKAchievementDescription* achievementDescription = [achievementDescriptions objectAtIndex:i];
				
				lua_newtable(L);
				lua_pushstring(L, [achievementDescription.identifier UTF8String]);
				lua_setfield(L, -2, "identifier");
				lua_pushstring(L, [achievementDescription.title UTF8String]);
				lua_setfield(L, -2, "title");
				lua_pushstring(L, [achievementDescription.achievedDescription UTF8String]);
				lua_setfield(L, -2, "achievedDescription");
				lua_pushstring(L, [achievementDescription.unachievedDescription UTF8String]);
				lua_setfield(L, -2, "unachievedDescription");
				lua_pushinteger(L, achievementDescription.maximumPoints);
				lua_setfield(L, -2, "maximumPoints");
				lua_pushboolean(L, achievementDescription.hidden);
				lua_setfield(L, -2, "hidden");
				
				lua_rawseti(L, -2, i + 1);				
			}
			lua_setfield(L, -2, "achievementDescriptions");
		}

        
        if (localPlayer)
        {
            lua_newtable(L);
            lua_pushstring(L, [localPlayer.playerID UTF8String]);
            lua_setfield(L, -2, "playerID");
            lua_pushstring(L, [localPlayer.alias UTF8String]);
            lua_setfield(L, -2, "alias");
            lua_pushboolean(L, localPlayer.isFriend);
            lua_setfield(L, -2, "isFriend");
            lua_pushboolean(L, localPlayer.underage);
            lua_setfield(L, -2, "underage");
            
            lua_setfield(L, -2, "localPlayer");            
        }
        
        if (localPlayerScore)
        {
            pushScoreAsTable(L, localPlayerScore);
            lua_setfield(L, -2, "localPlayerScore");
        }

        if (scores)
        {
            lua_newtable(L);
            for (int i = 0; i < [scores count]; ++i)
            {
                GKScore* score = [scores objectAtIndex:i];
                pushScoreAsTable(L, score);
                lua_rawseti(L, -2, i + 1);
            }
            lua_setfield(L, -2, "scores");
        }
        
		if (lua_pcall(L, 2, 0, 0) != 0)
			g_error(L, lua_tostring(L, -1));
		
		lua_pop(L, 2);
	}

private:
	lua_State* L;
	GGameKitHelper* helper;

};

@implementation GGameKitEventPerformer

- (id)init:(GGameKitEvent)theEvent
{
    if (self = [super init])
    {
        self->event = theEvent;
        [self performSelectorOnMainThread:@selector(performEvent) withObject:nil waitUntilDone:YES];
    }
    
    return self;
}

- (void)performEvent
{
    event.gameKit->dispatchEvent(event.type,
                                 event.error,
                                 event.friends,
                                 event.players,
                                 event.achievements,
                                 event.achievementDescriptions,
                                 event.localPlayer,
                                 event.localPlayerScore,
                                 event.scores);
}

@end


static int destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	GGameKit* gamekit = static_cast<GGameKit*>(object->proxy());
	
	gamekit->unref();
	
	return 0;
}

static GGameKit* getInstance(lua_State* L, int index)
{
	GReferenced* object = static_cast<GReferenced*>(g_getInstance(L, "GameKit", index));
	GGameKit* gamekit = static_cast<GGameKit*>(object->proxy());

	return gamekit;
}


static int isAvailable(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);
	lua_pushboolean(L, gamekit->isAvailable());
	
	return 1;	
}

static int authenticate(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);
	gamekit->authenticate();
	
	return 0;
}

static int loadFriends(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);
	gamekit->loadFriends();
	
	return 0;
}

static int loadPlayers(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);

	int len = lua_objlen(L, 2);

	NSMutableArray* identifiers = [NSMutableArray arrayWithCapacity:len];
	
	for (int i = 1; i <= len; i++)
    {
		lua_rawgeti(L, 2, i);
        [identifiers addObject:[NSString stringWithUTF8String:luaL_checkstring(L, -1)]];
		lua_pop(L, 1);
    }
	
	gamekit->loadPlayers(identifiers);
	
	return 0;
}

static int reportScore(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);

	int value = luaL_checkinteger(L, 2);
	const char* category = luaL_checkstring(L, 3);
	
	gamekit->reportScore(value, [NSString stringWithUTF8String:category]);
	
	return 0;
}

static int luaL_checktimescope(lua_State* L, int index)
{
    GKLeaderboardTimeScope timeScope = GKLeaderboardTimeScopeAllTime;
    const char* timeScopeStr = luaL_checkstring(L, index);

    if (strcmp(timeScopeStr, TODAY) == 0)
    {
        timeScope = GKLeaderboardTimeScopeToday;
    }
    else if (strcmp(timeScopeStr, WEEK) == 0)
    {
        timeScope = GKLeaderboardTimeScopeWeek;
    }
    else if (strcmp(timeScopeStr, ALL_TIME) == 0)
    {
        timeScope = GKLeaderboardTimeScopeAllTime;
    }
    else
    {
        luaL_error(L, "Parameter 'timeScope' must be one of the accepted values.");
    }
    return timeScope;
}

static int luaL_checkplayerscope(lua_State* L, int index)
{
    GKLeaderboardPlayerScope playerScope = GKLeaderboardPlayerScopeGlobal;
    const char* playScopeStr = luaL_checkstring(L, index);

    if (strcmp(playScopeStr, FRIENDS) == 0)
    {
        playerScope = GKLeaderboardPlayerScopeFriendsOnly;
    }
    else if (strcmp(playScopeStr, ALL_PLAYERS) == 0)
    {
        playerScope = GKLeaderboardPlayerScopeGlobal;
    }
    else
    {
        luaL_error(L, "Parameter 'playerScope' must be one of the accepted values.");
    }
    return playerScope;
}

static int showLeaderboard(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);
	
	NSString* category = nil;
	GKLeaderboardTimeScope timeScope = GKLeaderboardTimeScopeAllTime;
	
	if (!lua_isnoneornil(L, 2))
		category = [NSString stringWithUTF8String:luaL_checkstring(L, 2)];
		
	if (!lua_isnoneornil(L, 3))
		timeScope= (GKLeaderboardTimeScope)luaL_checktimescope(L, 3);

	gamekit->showLeaderboard(category, timeScope);
	
	return 0;
}

static int loadScores(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);

	NSString* category = nil;
	GKLeaderboardTimeScope timeScope = GKLeaderboardTimeScopeAllTime;
    GKLeaderboardPlayerScope playerScope = GKLeaderboardPlayerScopeGlobal;
    int startEntry=1;
    int maxEntries=25; // Default

	if (!lua_isnoneornil(L, 2))
		category = [NSString stringWithUTF8String:luaL_checkstring(L, 2)];

	if (!lua_isnoneornil(L, 3))
		timeScope= (GKLeaderboardTimeScope)luaL_checktimescope(L, 3);

    if (!lua_isnoneornil(L, 4))
   		playerScope= (GKLeaderboardPlayerScope)luaL_checkplayerscope(L, 4);

    if (!lua_isnoneornil(L, 5))
   		startEntry= luaL_checknumber(L, 5);

    if (!lua_isnoneornil(L, 6))
   		maxEntries= luaL_checknumber(L, 6);

    gamekit->loadScores(category, timeScope, playerScope, startEntry, maxEntries);

	return 0;
}

static int showAchievements(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);
	gamekit->showAchievements();
	
	return 0;
}

static int loadAchievements(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);
	gamekit->loadAchievements();

	return 0;
}

static int reportAchievement(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);

	const char* identifier = luaL_checkstring(L, 2);
	double percentComplete = luaL_checknumber(L, 3);
    int showsCompletionBanner = lua_toboolean(L, 4);
	
	gamekit->reportAchievement([NSString stringWithUTF8String:identifier], percentComplete, showsCompletionBanner);
	
	return 0;
}

static int resetAchievements(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);
	gamekit->resetAchievements();
	
	return 0;
}

static int loadAchievementDescriptions(lua_State* L)
{
	GGameKit* gamekit = getInstance(L, 1);
	gamekit->loadAchievementDescriptions();
	
	return 0;
}


static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{"isAvailable", isAvailable},
		{"authenticate", authenticate},
		{"loadFriends", loadFriends},
		{"loadPlayers", loadPlayers},
        {"loadScores", loadScores},
		{"reportScore", reportScore},
		{"showLeaderboard", showLeaderboard},
		{"showAchievements", showAchievements},
		{"loadAchievements", loadAchievements},
		{"reportAchievement", reportAchievement},
		{"resetAchievements", resetAchievements},
		{"loadAchievementDescriptions", loadAchievementDescriptions},
		{NULL, NULL},
	};
	
	g_createClass(L, "GameKit", "EventDispatcher", NULL, destruct, functionlist);
	
	lua_getglobal(L, "GameKit");
	lua_pushstring(L, TODAY);
	lua_setfield(L, -2, "TODAY");
	lua_pushstring(L, WEEK);
	lua_setfield(L, -2, "WEEK");
	lua_pushstring(L, ALL_TIME);
	lua_setfield(L, -2, "ALL_TIME");
    lua_pushstring(L, ALL_PLAYERS);
   	lua_setfield(L, -2, "ALL_PLAYERS");
    lua_pushstring(L, FRIENDS);
   	lua_setfield(L, -2, "FRIENDS");
	lua_pop(L, 1);
	
	lua_getglobal(L, "Event");
	lua_pushstring(L, AUTHENTICATE_COMPLETE);
	lua_setfield(L, -2, "AUTHENTICATE_COMPLETE");
	lua_pushstring(L, LOAD_FRIENDS_COMPLETE);
	lua_setfield(L, -2, "LOAD_FRIENDS_COMPLETE");
	lua_pushstring(L, LOAD_PLAYERS_COMPLETE);
	lua_setfield(L, -2, "LOAD_PLAYERS_COMPLETE");
    lua_pushstring(L, LOAD_SCORES_COMPLETE);
   	lua_setfield(L, -2, "LOAD_SCORES_COMPLETE");
	lua_pushstring(L, REPORT_SCORE_COMPLETE);
	lua_setfield(L, -2, "REPORT_SCORE_COMPLETE");
	lua_pushstring(L, LOAD_ACHIEVEMENTS_COMPLETE);
	lua_setfield(L, -2, "LOAD_ACHIEVEMENTS_COMPLETE");
	lua_pushstring(L, REPORT_ACHIEVEMENT_COMPLETE);
	lua_setfield(L, -2, "REPORT_ACHIEVEMENT_COMPLETE");
	lua_pushstring(L, RESET_ACHIEVEMENTS_COMPLETE);
	lua_setfield(L, -2, "RESET_ACHIEVEMENTS_COMPLETE");
	lua_pushstring(L, LOAD_ACHIEVEMENT_DESCRIPTIONS_COMPLETE);
	lua_setfield(L, -2, "LOAD_ACHIEVEMENT_DESCRIPTIONS_COMPLETE");
	lua_pop(L, 1);
	
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of KEY_OBJECTS
	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_newtable(L);                  // create a table
	lua_pushliteral(L, "v");
	lua_setfield(L, -2, "__mode");    // set as weak-value table
	lua_pushvalue(L, -1);             // duplicate table
	lua_setmetatable(L, -2);          // set itself as metatable
	lua_rawset(L, LUA_REGISTRYINDEX);	
	
	GGameKit* gamekit = new GGameKit(L);
	g_pushInstance(L, "GameKit", gamekit->object());

	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushvalue(L, -2);
	lua_rawseti(L, -2, 1);
	lua_pop(L, 1);	

	lua_pushvalue(L, -1);
	lua_setglobal(L, "gamekit");
	
	return 1;
}

static void g_initializePlugin(lua_State* L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "gamekit");
	
	lua_pop(L, 2);	
}

static void g_deinitializePlugin(lua_State *L)
{

}

REGISTER_PLUGIN("Game Kit", "1.0")
