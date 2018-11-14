/*
 * debugging.h
 *
 *  Created on: 14 sept. 2018
 *      Author: Nico
 */

#ifndef LUABINDING_DEBUGGING_H_
#define LUABINDING_DEBUGGING_H_

#include "libnetwork.h"
#include "luaapplication.h"
#include "bytebuffer.h"


class LuaDebugging {
	static NetworkLink *studio;
	static lua_State *breakedL;
	static std::map<int,std::set<std::string>> breakpoints;
	static int subCount;
	static int debuggerMode;
    static std::string lastFile;
	static void studioCommandInternal(const std::vector<char> &data,lua_State *L,lua_Debug *ar);
    static void serializeValue(ByteBuffer &buffer,lua_State *L,int n,int nref);
	static void setupBreakMode(int m);
public:
    static bool profiling;
    static int lastLine;
    static lua_State *L;
    static int yieldHookMask;
    static lua_Hook hook;
    static void studioLink(NetworkLink *server);
	static void studioCommand(const std::vector<char> &data);
	static void debuggerHook(void *context,lua_State *L,lua_Debug *ar);
};

#endif /* LUABINDING_DEBUGGING_H_ */
