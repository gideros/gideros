/*
 * debugging.cpp
 *
 *  Created on: 14 sept. 2018
 *      Author: Nico
 */

#include "debugging.h"
#include <string.h>
#include <stdint.h>
#define abs_index(L, i)		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
					lua_gettop(L) + (i) + 1)

NetworkLink *LuaDebugging::studio = NULL;
lua_State *LuaDebugging::L=NULL;
lua_State *LuaDebugging::breakedL=NULL;
int LuaDebugging::yieldHookMask=0;
lua_Hook LuaDebugging::hook=NULL;
int LuaDebugging::subCount=0;
int LuaDebugging::debuggerMode=0;
int LuaDebugging::lastLine=0;
std::string LuaDebugging::lastFile;
bool LuaDebugging::profiling=false;

std::map<int,std::set<std::string>> LuaDebugging::breakpoints;

void LuaDebugging::studioLink(NetworkLink *server) {
	studio = server;
}

#define LUA_TREF    20 //Not a lua type, but indicates a ref to an already outputted table

void LuaDebugging::serializeValue(ByteBuffer &buffer,lua_State *L,int n,int nrefs)  {
	n=abs_index(L,n);
    nrefs=abs_index(L,nrefs);
	int type=lua_type(L,n);
    if (type==LUA_TTABLE) {
        lua_pushvalue(L,n);
        lua_rawget(L,nrefs);
        if (lua_toboolean(L,-1))
            type=LUA_TREF;
        lua_pop(L,1);
    }
    std::string sval;
    size_t ssz;
    const char *val;

    buffer << (char) type;
	switch (type) {
	case LUA_TNIL:
	case LUA_TNONE:
		break;
    case LUA_TREF:
        lua_pushvalue(L,n);
        lua_rawget(L,nrefs);
        val=lua_tolstring(L,-1,&ssz);
        lua_pop(L,1);
        if (val) sval=std::string(val,ssz);
        buffer << sval;
        break;
	case LUA_TBOOLEAN:
        buffer << (char) lua_toboolean(L,n);
		break;
	case LUA_TTABLE:
         //Mark the table as seen
         lua_pushvalue(L,n);
         lua_pushfstring(L,"%p",lua_topointer(L, n));
         val=lua_tolstring(L,-1,&ssz);
         lua_rawset(L,nrefs);
         if (val) sval=std::string(val,ssz);
         buffer << sval;
         //Serialize table
	     lua_pushnil(L);  /* first key */
	     while (lua_next(L, n) != 0) {
           serializeValue(buffer,L,-2,nrefs);
           serializeValue(buffer,L,-1,nrefs);
	       lua_pop(L, 1);
	     }
        lua_pushnil(L);
        serializeValue(buffer,L,-1,nrefs); //Add a nil to mark end of table
        lua_pop(L, 1);
		break;
	default:
	    lua_pushvalue(L,n); //tolstring change var type, so make a copy before
        val=lua_tolstring(L,-1,&ssz);
	    lua_pop(L,1);
	    if (val) sval=std::string(val,ssz);
	    buffer << sval;
	}
}

#ifdef LUA_IS_LUAU
#include "lstate.h"
#endif

void LuaDebugging::setupBreakMode(int m) {
    //Modes used by the studio:
    //0x00: stop debugger, i.e resume without any breakpoints
    //0x04: step into, i.e break at next source line
    //0x44: step over, i.e break at next source line excluding sub calls
    //0x46: step return, i.e break at return exclusing sub calls (plus line, needed for internal processing)
    //0x84: resume, i.e break at next line matching a breakpoint
	debuggerMode=m;
	LuaApplication::debuggerBreak=m;
#ifndef LUA_IS_LUAU
	if (debuggerMode&DBG_MASKSUB)
	{
		LuaApplication::debuggerBreak|=LUA_MASKRET|LUA_MASKCALL;
		subCount=(m&LUA_MASKRET)?1:0;
        if (subCount)
            LuaApplication::debuggerBreak&=~LUA_MASKLINE;
	}
    lua_sethook(L, LuaDebugging::hook, LuaDebugging::yieldHookMask | (LuaApplication::debuggerBreak&DBG_MASKLUA), lua_gethookcount(L));
#else
	if (debuggerMode&DBG_MASKSUB)
	{
		L->profilerHook=profilerHook;
        subCount=(m&0x02)?1:0;
        lua_singlestep(L,1);
	}
	else {
		L->profilerHook=NULL;
        subCount=0;
        lua_singlestep(L,m?1:0);
	}
#endif
}

static void lookupVariable(const char *sym,lua_State *L, lua_Debug *ar) {
	//1. Locals
	int n=1;
	bool found=false;
	const char *name;
#ifdef LUA_IS_LUAU
    while ((name=lua_getlocal(L,0,n))!=NULL)
#else
    while ((name=lua_getlocal(L,ar,n))!=NULL)
#endif
	{
		bool nfound=(!strcmp(name,sym));
		if (found&&nfound)
			lua_remove(L,-2);
        if (nfound) found=true;
        if (!nfound)
			lua_pop(L,1);
		n++;
	}
	if (!found) {
		//2. upvalues
		n=1;
#ifdef LUA_IS_LUAU
        lua_getinfo(L, 0, "f", ar);
#else
        lua_getinfo(L, "f", ar);
#endif
        while ((name=lua_getupvalue(L,-1,n))!=NULL)
		{
			found=(!strcmp(name,sym));
			if (!found)
				lua_pop(L,1);
			else
				break;
			n++;
		}
        if (found)
            lua_remove(L,-2);
        else
            lua_pop(L,1);
	}
	if (!found) {
		//3. globals
		lua_getglobal(L,sym);
	}
}

#ifdef LUA_IS_LUAU
void LuaDebugging::profilerHook(lua_State *L,int enter)
{
    G_UNUSED(L);
    if (enter)
		subCount++;
    else if (subCount>0)
		subCount--;
}
#endif

void LuaDebugging::studioCommandInternal(const std::vector<char> &data,lua_State *L, lua_Debug *ar) {
	switch (data[0]) {
	case gptSetProperties:
	case gptStop: {
		LuaApplication::debuggerBreak=0;
		debuggerMode=0;
		LuaApplication::debuggerHook=NULL;
        lastLine=-1;
        if (L&&profiling&&(data[0]==gptStop)) {
        	LuaApplication::Core_profilerReport(L);
    		ByteBuffer buffer;
    		buffer << (char) gptProfilingResult;
            lua_newtable(L);
            serializeValue(buffer,L,-2,-1);
            lua_pop(L,2);
    		LuaDebugging::studio->sendData(buffer.data(),buffer.size());
        }
		LuaDebugging::profiling=false;
		break;
	}
	case gptProfilingOn:
		LuaDebugging::profiling=true;
		if (L) {
			LuaApplication::Core_profilerReset(L);
			LuaApplication::Core_profilerStart(L);
		}
		break;
	case gptLookupSymbol: {
		std::string sym = &data[1];
		//Lookup the variable
		size_t spos=0;
		size_t dot=sym.find_first_of('.');
		lookupVariable(sym.substr(0,dot).c_str(),L,ar);
		while (dot!=std::string::npos) {
			sym=sym.substr(dot+1);
			dot=sym.find_first_of('.');
			std::string sub=sym.substr(0,dot);
			if ((sub.size()==0)||!lua_istable(L,-1)) //Empty sub element=bad symbol
			{
				lua_pop(L,1);
				lua_pushnil(L);
				break;
			}
			lua_pushstring(L,sub.c_str());
			lua_rawget(L,-2);
			lua_remove(L,-2);
		}
		ByteBuffer buffer;
		buffer << (char) gptSymbolValue;
        lua_newtable(L);
        serializeValue(buffer,L,-2,-1);
        lua_pop(L,2);
		LuaDebugging::studio->sendData(buffer.data(),buffer.size());
		break;
	}

    case gptResume:
    case gptSetBreakpoints: {
        ByteBuffer buffer(&data[2], data.size()-2);
        setupBreakMode(data[1]);
#ifndef LUA_IS_LUAU
		LuaApplication::debuggerHook=LuaDebugging::debuggerHook;
#else
        L->global->cb.debugstep=LuaDebugging::debuggerHook;
#endif
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
        if (data[0]==gptResume)
            LuaDebugging::breakedL=NULL;
         break;
	}
	}
}


void LuaDebugging::studioCommand(const std::vector<char> &data) {
	studioCommandInternal(data,L,NULL);
}

#ifndef LUA_IS_LUAU
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
#else
void LuaDebugging::debuggerHook(lua_State *L, lua_Debug *ar) {
    if (subCount) return;
    lua_getinfo(L,0,"s",ar);
    std::string src=ar->source;
	if ((lastFile==src)&&(lastLine==ar->currentline)) return;
	lastFile=src;
	lastLine=ar->currentline;
    if (LuaApplication::debuggerBreak&DBG_MASKBREAK)
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
                     lua_singlestep(L,0);
		     		 L->profilerHook=NULL;
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
                 lua_singlestep(L,0);
                 L->profilerHook=NULL;
	        	 breakedL=NULL;
	         }
		}

	}
}
#endif
