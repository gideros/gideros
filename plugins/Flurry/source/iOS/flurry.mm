#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"

#import "FlurryAnalytics.h"

static BOOL isSessionStarted = NO;

static int isAvailable(lua_State* L)
{
	lua_pushboolean(L, 1);
	return 1;
}

static int startSession(lua_State* L)
{
	if (isSessionStarted == YES)
		return 0;
		
	NSString* apiKey = [NSString stringWithUTF8String:luaL_checkstring(L, 1)];
	[FlurryAnalytics startSession:apiKey];

	isSessionStarted = YES;

	return 0;
}

static NSMutableDictionary* table2dictionary(lua_State*L, int index)
{
	NSMutableDictionary* dictionary = [NSMutableDictionary dictionaryWithCapacity:10];
	
	int t = index;
	
	/* table is in the stack at index 't' */
	lua_pushnil(L);  /* first key */
	while (lua_next(L, t) != 0)
	{
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		lua_pushvalue(L, -2);
		NSString* key = [NSString stringWithUTF8String:luaL_checkstring(L, -1)];
		lua_pop(L, 1);
		
		NSString* value = [NSString stringWithUTF8String:luaL_checkstring(L, -1)];
		
		[dictionary setObject:value forKey:key];
		
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);
	}	
	
	return dictionary;
}

static int logEvent(lua_State* L)
{
	NSString* eventName = [NSString stringWithUTF8String:luaL_checkstring(L, 1)];

	if (!lua_isnoneornil(L, 2))
	{
		NSMutableDictionary* dictionary = table2dictionary(L, 2);
		
		if (!lua_toboolean(L, 3))
			[FlurryAnalytics logEvent:eventName withParameters:dictionary];
		else
			[FlurryAnalytics logEvent:eventName withParameters:dictionary timed:YES];
	}
	else
	{
		if (!lua_toboolean(L, 3))
			[FlurryAnalytics logEvent:eventName];
		else
			[FlurryAnalytics logEvent:eventName timed:YES];
	}
	
	return 0;
}


static int endTimedEvent(lua_State* L)
{
	NSString* eventName = [NSString stringWithUTF8String:luaL_checkstring(L, 1)];

	if (!lua_isnoneornil(L, 2))
	{
		NSMutableDictionary* dictionary = table2dictionary(L, 2);		
		[FlurryAnalytics endTimedEvent:eventName withParameters:dictionary];
	}
	else
	{
		[FlurryAnalytics endTimedEvent:eventName withParameters:nil];
	}
	
	return 0;
}

static int loader(lua_State* L)
{
	lua_newtable(L);

	lua_pushcfunction(L, isAvailable);
	lua_setfield(L, -2, "isAvailable");
	lua_pushcfunction(L, startSession);
	lua_setfield(L, -2, "startSession");
	lua_pushcfunction(L, logEvent);
	lua_setfield(L, -2, "logEvent");
	lua_pushcfunction(L, endTimedEvent);
	lua_setfield(L, -2, "endTimedEvent");

	lua_pushvalue(L, -1);
	lua_setglobal(L, "flurry");

	return 1;
}

static void g_initializePlugin(lua_State *L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "flurry");

	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
}

REGISTER_PLUGIN("Flurry", "1.0")
