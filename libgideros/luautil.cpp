#include "luautil.h"

extern "C" {

#ifndef abs_index

/* convert a stack index to positive */
#define abs_index(L, i)		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
	lua_gettop(L) + (i) + 1)

#endif

int lua_toboolean2(lua_State *L, int idx)
{
    if (lua_isnone(L, idx))
        luaL_typerror(L, idx, "boolean");

    return lua_toboolean(L, idx);
}

void luaL_newweaktable(lua_State* L)
{
	// TODO: create newtable as metatable. don't reuse itself as metatable. (i think about performance issues)
	lua_newtable(L);			// create table for instance list
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");	  // set as weak-value table
	lua_pushvalue(L, -1);             // duplicate table
	lua_setmetatable(L, -2);          // set itself as metatable
}

void luaL_nullifytable(lua_State*L, int index)
{
	int t = abs_index(L, index);

	/* table is in the stack at index 't' */
	lua_pushnil(L);  /* first key */
	while (lua_next(L, t) != 0)
	{
		/* uses 'key' (at index -2) and 'value' (at index -1) */
#if 0
		printf("%s - %s\n",
			lua_typename(L, lua_type(L, -2)),
			lua_typename(L, lua_type(L, -1)));
#endif

		lua_pushvalue(L, -2);
		lua_pushnil(L);
		lua_settable(L, t);		// table[key] = nil

		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);
	}
}

static int db_errorfb(lua_State *L);

int luaC_traceback(lua_State *L)
{
	if (!lua_isstring(L, 1))  /* 'message' not a string? */
		return 1;  /* keep it intact */

/*	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 1;
	}*/
	lua_pushcnfunction(L, db_errorfb, "db_errorfb");
	lua_pushvalue(L, 1);  /* pass error message */
	lua_pushinteger(L, 2);  /* skip this function and traceback */
	lua_call(L, 2, 1);  /* call debug.traceback */
	return 1;
}

static lua_State *getthread (lua_State *L, int *arg) {
  if (lua_isthread(L, 1)) {
    *arg = 1;
    return lua_tothread(L, 1);
  }
  else {
    *arg = 0;
    return L;
  }
}


#define LEVELS1	12	/* size of the first part of the stack */
#define LEVELS2	10	/* size of the second part of the stack */

static int db_errorfb (lua_State *L) {
  int level;
  int firstpart = 1;  /* still before eventual `...' */
  int arg;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  if (lua_isnumber(L, arg+2)) {
    level = (int)lua_tointeger(L, arg+2);
    lua_pop(L, 1);
  }
  else
    level = (L == L1) ? 1 : 0;  /* level 0 may be this own function */
  if (lua_gettop(L) == arg)
    lua_pushliteral(L, "");
  else if (!lua_isstring(L, arg+1)) return 1;  /* message is not a string */
  else lua_pushliteral(L, "\n");
  lua_pushliteral(L, "stack traceback:");
#ifdef LUA_IS_LUAU
  while (lua_getinfo(L1, level++, "sSnl", &ar)) {
    if (level > LEVELS1 && firstpart) {
      /* no more than `LEVELS2' more levels? */
      if (!lua_getinfo(L1, level+LEVELS2, "sSnl", &ar))
        level--;  /* keep going */
      else {
        lua_pushliteral(L, "\n\t...");  /* too many levels */
        while (lua_getinfo(L1, level+LEVELS2, "sSnl", &ar))  /* find last levels */
          level++;
      }
      firstpart = 0;
      continue;
    }
    if (*ar.what == 'C')
        continue;
    lua_pushliteral(L, "\n\t");
    lua_pushfstring(L, "%s:", ar.short_src);
    if (ar.currentline > 0)
      lua_pushfstring(L, "%d:", ar.currentline);
    if (ar.name)  /* is there a name? */
        lua_pushfstring(L, " in function %s", ar.name);
    else {
      if (*ar.what == 'm')  /* main? */
        lua_pushfstring(L, " in main chunk");
      else if (*ar.what == 'C' || *ar.what == 't')
        lua_pushliteral(L, " ?");  /* C function or tail call */
      else
        lua_pushfstring(L, " in function <%s:%d>",
                           ar.short_src, ar.linedefined);
    }
#else
  while (lua_getstack(L1, level++, &ar)) {
    if (level > LEVELS1 && firstpart) {
      /* no more than `LEVELS2' more levels? */
      if (!lua_getstack(L1, level+LEVELS2, &ar))
        level--;  /* keep going */
      else {
        lua_pushliteral(L, "\n\t...");  /* too many levels */
        while (lua_getstack(L1, level+LEVELS2, &ar))  /* find last levels */
          level++;
      }
      firstpart = 0;
      continue;
    }
    lua_getinfo(L1, "Snl", &ar);
	if (*ar.what == 'C')
        continue;
	lua_pushliteral(L, "\n\t");
    lua_pushfstring(L, "%s:", ar.short_src);
    if (ar.currentline > 0)
      lua_pushfstring(L, "%d:", ar.currentline);
    if (*ar.namewhat != '\0')  /* is there a name? */
        lua_pushfstring(L, " in function " LUA_QS, ar.name);
    else {
      if (*ar.what == 'm')  /* main? */
        lua_pushfstring(L, " in main chunk");
      else if (*ar.what == 'C' || *ar.what == 't')
        lua_pushliteral(L, " ?");  /* C function or tail call */
      else
        lua_pushfstring(L, " in function <%s:%d>",
                           ar.short_src, ar.linedefined);
    }
#endif
    lua_concat(L, lua_gettop(L) - arg);
  }
  lua_concat(L, lua_gettop(L) - arg);
  return 1;
}
static char key_tracebackFunction = ' ';

int lua_pcall_traceback(lua_State* L, int nargs, int nresults, int unused)
{
//	return lua_pcall(L, nargs, nresults, 0);

	int base = lua_gettop(L) - nargs;  /* function index */

	lua_pushlightuserdata(L, &key_tracebackFunction);
	lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_pushcnfunction(L, ::luaC_traceback, "luaC_traceback");
        lua_pushlightuserdata(L, &key_tracebackFunction);
        lua_pushvalue(L, -2);
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
//	lua_pushcfunction(L, luaC_traceback);  /* push traceback function */

	lua_insert(L, base);  /* put it under chunk and args */

	int status = lua_pcall(L, nargs, nresults, base);

	lua_remove(L, base);  /* remove traceback function */
	
	return status;
}

void lua_traceback(lua_State* L)
{
	if (!lua_isstring(L, -1))  /* 'message' not a string? */
		return;  /* keep it intact */
    lua_pushcnfunction(L, db_errorfb, "db_errorfb");
	lua_pushvalue(L, -2);  /* pass error message */
	lua_pushinteger(L, 1);  /* skip this function */
	lua_call(L, 2, 1);  /* call debug.traceback */
}


void luaL_rawgetptr(lua_State *L, int idx, void* ptr)
{
	idx = abs_index(L, idx);
	lua_pushlightuserdata(L, ptr);
	lua_rawget(L, idx);
}

void luaL_rawsetptr(lua_State *L, int idx, void* ptr)
{
	idx = abs_index(L, idx);
	lua_pushlightuserdata(L, ptr);
	lua_insert(L, -2);
	lua_rawset(L, idx);
}

static char key_data = ' ';

void luaL_setdata(lua_State *L, void *data)
{
    lua_pushlightuserdata(L, data);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_data);
}

void *luaL_getdata(lua_State *L)
{
    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_data);
    void *result = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return result;

}

}

