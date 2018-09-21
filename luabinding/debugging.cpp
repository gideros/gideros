/*
 * debugging.cpp
 *
 *  Created on: 14 sept. 2018
 *      Author: Nico
 */

#include "debugging.h"

#define abs_index(L, i)		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
					lua_gettop(L) + (i) + 1)

Server *LuaDebugging::studio = NULL;
lua_State *LuaDebugging::L=NULL;
lua_State *LuaDebugging::breakedL=NULL;
int LuaDebugging::yieldHookMask=0;
lua_Hook LuaDebugging::hook=NULL;
int LuaDebugging::subCount=0;
int LuaDebugging::debuggerMode=0;
int LuaDebugging::lastLine=0;
std::string LuaDebugging::lastFile;

std::map<int,std::set<std::string>> LuaDebugging::breakpoints;

void LuaDebugging::studioLink(Server *server) {
	studio = server;
}

void LuaDebugging::serializeValue(ByteBuffer &buffer,lua_State *L,int n)  {
	n=abs_index(L,n);
	int type=lua_type(L,n);
	buffer << (char) type;
	switch (type) {
	case LUA_TNIL:
	case LUA_TNONE:
		break;
	case LUA_TBOOLEAN:
		buffer << (char) lua_toboolean(L,-1);
		break;
	case LUA_TTABLE:
	     lua_pushnil(L);  /* first key */
	     while (lua_next(L, n) != 0) {
	       serializeValue(buffer,L,-2);
	       serializeValue(buffer,L,-1);
	       lua_pop(L, 1);
	     }
	    serializeValue(buffer,L,-1); //Add a nil to mark end of table
        lua_pop(L, 1);
		break;
	default:
	    std::string sval;
	    size_t ssz;
	    lua_pushvalue(L,n); //tolstring change var type, so make a copy before
	    const char *val=lua_tolstring(L,-1,&ssz);
	    lua_pop(L,1);
	    if (val) sval=std::string(val,ssz);
	    buffer << sval;
	}
}

void LuaDebugging::setupBreakMode(int m) {
	debuggerMode=m;
	LuaApplication::debuggerBreak=m;
	if (debuggerMode&DBG_MASKSUB)
	{
		LuaApplication::debuggerBreak|=LUA_MASKRET|LUA_MASKCALL;
		subCount=(m&LUA_MASKRET)?1:0;
        if (subCount)
            LuaApplication::debuggerBreak&=~LUA_MASKLINE;
	}
    lua_sethook(L, LuaDebugging::hook, LuaDebugging::yieldHookMask | (LuaApplication::debuggerBreak&DBG_MASKLUA), lua_gethookcount(L));
}

void LuaDebugging::studioCommandInternal(const std::vector<char> &data,lua_State *L, lua_Debug *ar) {
	switch (data[0]) {
	case gptSetProperties:
	case gptStop: {
		LuaApplication::debuggerBreak=0;
		debuggerMode=0;
		LuaApplication::debuggerHook=NULL;
        lastLine=-1;
		break;
	}
	case gptLookupSymbol: {
		std::string sym = &data[1];
		//Lookup the variable
		//1. Locals
		int n=1;
		bool found=false;
		const char *name;
		while ((name=lua_getlocal(L,ar,n))!=NULL)
		{
			bool nfound=(!strcmp(name,sym.c_str()));
			if (found&&nfound)
				lua_remove(L,-2);
			found=nfound;
			if (!found)
				lua_pop(L,1);
			n++;
		}
		if (!found) {
			//2. upvalues
			n=1;
			while ((name=lua_getupvalue(L,1,n))!=NULL)
			{
				found=(!strcmp(name,sym.c_str()));
				if (!found)
					lua_pop(L,1);
				n++;
			}
		}
		if (!found) {
			//3. globals
			lua_getglobal(L,sym.c_str());
		}
		ByteBuffer buffer;
		buffer << (char) gptSymbolValue;
		serializeValue(buffer,L,-1);
		lua_pop(L,1);
		LuaDebugging::studio->sendData(buffer.data(),buffer.size());
		break;
	}

	case gptSetBreakpoints: {
        ByteBuffer buffer(&data[2], data.size()-2);
        setupBreakMode(data[1]);
		LuaApplication::debuggerHook=LuaDebugging::debuggerHook;
		int numBreakpoints;
		buffer >> numBreakpoints;
		LuaApplication::breakpoints.clear();
		LuaDebugging::breakpoints.clear();
		while (numBreakpoints--)
		{
			int line;
			std::string source;
			buffer >> line;
			buffer >> source;
			LuaDebugging::breakpoints[line].insert(source);
			LuaApplication::breakpoints[line]=true;
		}
         break;
	}
	case gptResume: {
        setupBreakMode(data[1]);
		LuaDebugging::breakedL=NULL;
		break;
	}
	}
}


void LuaDebugging::studioCommand(const std::vector<char> &data) {
	studioCommandInternal(data,L,NULL);
}

void LuaDebugging::debuggerHook(void *context, lua_State *L, lua_Debug *ar) {
	if ((ar->event==LUA_HOOKCALL)&&(LuaApplication::debuggerBreak&DBG_MASKSUB)) {
		subCount++;
		LuaApplication::debuggerBreak&=~LUA_MASKLINE;
        lua_sethook(L, LuaDebugging::hook, LuaDebugging::yieldHookMask | (LuaApplication::debuggerBreak&DBG_MASKLUA), lua_gethookcount(L));
        return;
	}
	if (((ar->event==LUA_HOOKRET)||(ar->event==LUA_HOOKTAILRET))&&(LuaApplication::debuggerBreak&DBG_MASKSUB)) {
		subCount--;
		if (subCount==0)
		{
			LuaApplication::debuggerBreak|=(debuggerMode&LUA_MASKLINE);
			lua_sethook(L, LuaDebugging::hook, LuaDebugging::yieldHookMask | (LuaApplication::debuggerBreak&DBG_MASKLUA), lua_gethookcount(L));
		}
        return;
	}
    if (ar->event==LUA_HOOKLINE) {
        std::string src=ar->source;
        if ((lastFile==src)&&(lastLine==ar->currentline)) return;
        lastFile=src;
        lastLine=ar->currentline;
    }
    if ((ar->event==LUA_HOOKLINE)&&(LuaApplication::debuggerBreak&DBG_MASKBREAK))
	{
		//Breakpoint, check source file match
		std::string src=ar->source;
		if (breakpoints[ar->currentline].find(src)==breakpoints[ar->currentline].end())
			return; // Not a matching source, resume
	}
    if (ar->currentline<0) //Line can't be determined, continue
        return;
	if (studio)
	{
		breakedL=L;
		ByteBuffer buffer;
		buffer << (char) gptBreaked;
		buffer << (int) ar->currentline;
		std::string src=ar->source;
		buffer << src;
		studio->sendData(buffer.data(),buffer.size());
		while (breakedL) {
	        NetworkEvent event;
            studio->tick(&event);
	         if (event.eventCode == eDataReceived){
	        	 if (event.data[0]==gptResume||event.data[0]==gptLookupSymbol||event.data[0]==gptSetBreakpoints)
	        		 studioCommandInternal(event.data,L,ar);
	        	 else { //Unexpected command, disable debugger
		        	 LuaApplication::debuggerBreak=0;
		     		 LuaApplication::debuggerHook=NULL;
		        	 breakedL=NULL;
	        	 }
	         }
	         else if (event.eventCode == eNone)
	         {
	 			//Sleep XXX Need to make a cross platform/platform specific way to sleep a few ms
	        	 continue;
	         }
	         else { //Unexpected event, disable debugger
	        	 LuaApplication::debuggerBreak=0;
	     		 LuaApplication::debuggerHook=NULL;
	        	 breakedL=NULL;
	         }
		}

	}
}
