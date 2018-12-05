extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}
#include "RtMidi.h"

#include <map>
#include <iostream>

using namespace std;

#define LUAMIDI "LuaMIDI"	// the name of the library
#define MIDIOUT "MidiOut"	// the registered name of the MidiOut datatype
#define MIDIIN  "MidiIn"	// the registered name of the MidiIn datatype

static bool base0 = true;	// the channels comming in are base0 or 1?

static RtMidiOut* OpenOut(lua_State *L, int port);

static void debugLua(lua_State *L)
{
	for(int j = 0; j <= lua_gettop(L); ++j)
	{
		int i = j - lua_gettop(L);
		cout << i << ": ";
		if(lua_isboolean(L, i))
			cout << "boolean " << lua_toboolean(L, i);
		else if(lua_iscfunction(L, i))
			cout << "cfunction " << lua_tocfunction(L, i);
		else if(lua_isfunction(L, i))
			cout << "function";
		else if(lua_islightuserdata(L, i))
			cout << "lightuserdata " << lua_touserdata(L, i);	
		else if(lua_isnil(L, i))
			cout << "nil";
		else if(lua_isnumber(L, i))
			cout << "number " << lua_tonumber(L, i);
		else if(lua_isstring(L, i))
			cout << "string " << lua_tostring(L, i);
		else if(lua_istable(L, i))
			cout << "table";
		else if(lua_isuserdata(L, i))
			cout << "userdata " << lua_touserdata(L, i);
		else
			cout << "wtf?";
		cout << endl;
	}
	cout << endl;
}

// nextout is used in the usual case where no port has yet been specified
// yet an RtMidiOut object still needs to be used for querying port counts.
// This way, we can create an object and have it ready for when the port is
// known
map<int, RtMidiOut*> outs;		// outputs that have been opened
map<int, RtMidiIn*> ins;		// inputs that have been opened
RtMidiOut* nextout = NULL;		// an output that hasn't been opened
RtMidiIn* nextin = NULL;

// some basic lua helper methods:
// assume 
// 0that table is at the top
void setfield(lua_State *L, const char *index, const char *value)
{
	lua_pushstring(L, index);
	lua_pushstring(L, value);
	lua_settable(L, -3);
}

void setfield(lua_State *L, int index, const char *value)
{
	lua_pushnumber(L, index);
	lua_pushstring(L, value);
	lua_settable(L, -3);
}

// some helper methods for accessing RtMidiOut:
static RtMidiOut* toMidiOut(lua_State *L, int index)
{
	RtMidiOut **midiout = (RtMidiOut**)lua_touserdata(L, index);
	if(midiout == NULL)
		luaL_typerror(L, index, MIDIOUT);
	return *midiout;
}

static RtMidiOut* checkMidiOut(lua_State* L, int index)
{
	RtMidiOut** midiout;
	RtMidiOut* ret;
	luaL_checktype(L, index, LUA_TUSERDATA);
	midiout = (RtMidiOut**)luaL_checkudata(L, index, MIDIOUT);
	if(midiout == NULL)
		luaL_typerror(L, index, MIDIOUT);
	ret = *midiout;
	if(!ret)
		luaL_error(L, "null RtMidiOut*");
	return ret;
}

static RtMidiOut** pushMidiOut(lua_State* L, RtMidiOut* mo)
{
	RtMidiOut** midiout = (RtMidiOut**)lua_newuserdata(L, sizeof(RtMidiOut*));
	*midiout = mo;
	luaL_getmetatable(L, MIDIOUT);
	lua_setmetatable(L, -2);
	return midiout;
}

// Some wrappers of the RtMidiOut class

static RtMidiOut* getGenericOutput()
{
	// if a portless RtMidiOut has already been created, use it
	if(nextout)
	{
		return nextout;
	}
	// if there aren't any MidiOuts yet, create a nextout
	else if(outs.begin() == outs.end())
	{
		nextout = new RtMidiOut();
		return nextout;
	}
	// we've got some ports opened already
	else
	{
		return outs.begin()->second;
	}
}

static RtMidiOut* OpenOut(lua_State *L, int port)
{
	int portcount = getGenericOutput()->getPortCount();
	if(port >= portcount)
		luaL_error(L, "Invalid Port Number");
	// if that port is already open
	if(outs.find(port) != outs.end())
		return outs[port];
	else if(!nextout)
		nextout = new RtMidiOut();

	// if there is not a nextout make one.
	if(port < 0)
		nextout->openVirtualPort("mm looper");
	else
		nextout->openPort(port);
	outs[port] = nextout;
	nextout = NULL;
	return outs[port];
}

// lua wrappers for methods that require a port and a RtMidiOut*

static int MidiOut_gc(lua_State *L)
{
	//TODO: delete
	return 0;
}

// some helper methods for accessing RtMidiIn:
static RtMidiIn* toMidiIn(lua_State *L, int index)
{
	RtMidiIn **midiin = (RtMidiIn**)lua_touserdata(L, index);
	if(midiin == NULL)
		luaL_typerror(L, index, MIDIIN);
	return *midiin;
}

static RtMidiIn* checkMidiIn(lua_State* L, int index)
{
	RtMidiIn** midiin;
	RtMidiIn* ret;
	luaL_checktype(L, index, LUA_TUSERDATA);
	midiin = (RtMidiIn**)luaL_checkudata(L, index, MIDIIN);
	if(midiin == NULL)
		luaL_typerror(L, index, MIDIIN);
	ret = *midiin;
	if(!ret)
		luaL_error(L, "null RtMidiIn*");
	return ret;
}

static RtMidiIn** pushMidiIn(lua_State* L, RtMidiIn* mi)
{
	RtMidiIn** midiin = (RtMidiIn**)lua_newuserdata(L, sizeof(RtMidiIn*));
	*midiin = mi;
	luaL_getmetatable(L, MIDIIN);
	lua_setmetatable(L, -2);
	return midiin;
}

// Some wrappers of the RtMidiIn class

static RtMidiIn* getGenericInput()
{
	// if a portless RtMidiIn has already been created, use it
	if(nextin)
	{
		return nextin;
	}
	// if there aren't any MidiIns yet, create a nextout
	else if(ins.begin() == ins.end())
	{
		nextin = new RtMidiIn();
		return nextin;
	}
	// we've got some ports opened already
	else
	{
		return ins.begin()->second;
	}
}

static RtMidiIn* OpenIn(lua_State *L, int port)
{
	int portcount = getGenericInput()->getPortCount();
	if(port >= portcount)
		luaL_error(L, "Invalid Port Number");
	// if that port is already open
	if(ins.find(port) != ins.end())
		return ins[port];
	else if(!nextin)
		nextin = new RtMidiIn();

	// if there is not a nextin make one.
	if(port < 0)
		nextin->openVirtualPort("in to MM");
	else
		nextin->openPort(port);
	ins[port] = nextin;
	nextin = NULL;
	return ins[port];
}

// lua wrappers for methods that require a port and a RtMidiIn*

static int MidiIn_gc(lua_State *L)
{
	//TODO: delete
	return 0;
}

static int MidiOut_noteOn(lua_State *L)
{
	std::vector<unsigned char> message;
	message.resize(3);
	RtMidiOut* midiout = checkMidiOut(L, 1);
	if(lua_isnumber(L, 4))
	message[0] = 144;
	message[1] = luaL_checkint(L, 2);
	message[2] = 127;
	if(lua_isnumber(L, 3))
		message[2] = (unsigned char)lua_tonumber(L, 3);
	if(lua_isnumber(L, 4))
	{
		int channel = luaL_checkint(L, 4);
		if(!base0)
			channel--;
		if(channel < 0)
			channel = 0;
		else if(channel > 15)
			channel = 15;
		message[0] += channel;
	}
	midiout->sendMessage(&message);
	return 0;
}

static int MidiOut_noteOff(lua_State *L)
{
	std::vector<unsigned char> message;
	message.resize(3);
	RtMidiOut* midiout = checkMidiOut(L, 1);
	message[0] = 144;
	message[1] = luaL_checkint(L, 2);
	message[2] = 0;
	if(lua_isnumber(L, 3))
	{
		int channel = luaL_checkint(L, 3);
		if(!base0)
			channel--;
		if(channel < 0)
			channel = 0;
		else if(channel > 15)
			channel = 15;
		message[0] += channel;
	}

	midiout->sendMessage(&message);
	return 0;
}

static int MidiOut_sendMessage(lua_State *L)
{
	std::vector<unsigned char> message;
	message.resize(3);
	RtMidiOut* midiout = checkMidiOut(L, 1);
	message[0] = luaL_checkint(L, 2);
	message[1] = luaL_checkint(L, 3);
	message[2] = luaL_checkint(L, 4);

	midiout->sendMessage(&message);
	return 0;
}

// lua wrappers for the operations on RtMidiOut that do not require
// a port to be open.

static int luamidi_openout(lua_State* L)
{
	int port = luaL_checkint(L, 1);
	pushMidiOut(L, OpenOut(L, port));
	return 1;
}

static int luamidi_openin(lua_State* L)
{
	int port = luaL_checkint(L, 1);
	pushMidiIn(L, OpenIn(L, port));
	return 1;
}

static int luamidi_base0(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TBOOLEAN);
	base0 = lua_toboolean(L, 1);
	return 0;
}

static int luamidi_base1(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TBOOLEAN);
	base0 = !lua_toboolean(L, 1);
	return 0;
}

static int luamidi_noteOn(lua_State *L)
{
	std::vector<unsigned char> message;
	message.resize(3);
	int port = luaL_checkint(L, 1);
	RtMidiOut* midiout = OpenOut(L, port);
	message[0] = 144;
	message[1] = luaL_checkint(L, 2);
	message[2] = 127;
	if(lua_isnumber(L, 3))
		message[2] = (unsigned char)lua_tonumber(L, 3);
	if(lua_isnumber(L, 4))
	{
		int channel = luaL_checkint(L, 4);
		if(!base0)
			channel--;
		if(channel < 0)
			channel = 0;
		else if(channel > 15)
			channel = 15;
		message[0] += channel;
		cout << "channel: " << channel << endl;
	}
	midiout->sendMessage(&message);
	return 0;
}

static int luamidi_noteOff(lua_State *L)
{
	std::vector<unsigned char> message;
	message.resize(3);
	int port = luaL_checkint(L, 1);
	RtMidiOut* midiout = OpenOut(L, port);
	message[0] = 144;
	message[1] = luaL_checkint(L, 2);
	message[2] = 0;
	if(lua_isnumber(L, 3))
	{
		int channel = luaL_checkint(L, 3);
		if(!base0)
			channel--;
		if(channel < 0)
			channel = 0;
		else if(channel > 15)
			channel = 15;
		message[0] += channel;
	}
	midiout->sendMessage(&message);
	return 0;
}

static int luamidi_sendMessage(lua_State *L)
{
	std::vector<unsigned char> message;
	message.resize(3);
	int port = luaL_checkint(L, 1);
	RtMidiOut* midiout = OpenOut(L, port);
	message[0] = luaL_checkint(L, 2);
	message[1] = luaL_checkint(L, 3);
	message[2] = luaL_checkint(L, 4);
	
	if(lua_isnumber(L, 5))
	{
		int channel = luaL_checkint(L, 5);
		if(!base0)
			channel--;
		if(channel < 0)
			channel = 0;
		else if(channel > 15)
			channel = 15;
		message[0] += channel;
	}

	midiout->sendMessage(&message);
	return 0;
}

static int luamidi_getMessage(lua_State *L)
{
//	cout << "getting..." << endl;
	std::vector<unsigned char> message;
	int port = luaL_checkint(L, 1);
	RtMidiIn* midiin = OpenIn(L, port);
	double delta = midiin->getMessage(&message);
	if(message.size() == 3)
	{
		lua_pushnumber(L, message[0]);
		lua_pushnumber(L, message[1]);
		lua_pushnumber(L, message[2]);
		lua_pushnumber(L, delta);
		return 4;
	}
	return 0;
}

static int luamidi_getoutportcount (lua_State *L)
{
	// return true just to know that everything is working
	lua_pushnumber(L, getGenericOutput()->getPortCount());
	return 1;
}

static int luamidi_getinportcount(lua_State* L)
{
	lua_pushnumber(L, getGenericInput()->getPortCount());
	return 1;
}

//
// Better name pending.
// returns a table (array) where each key is a port number
// and each value is the name of that port
//
static int luamidi_enumerateoutports (lua_State *L)
{
	// the return table
	lua_newtable(L);

	RtMidiOut* midi = getGenericOutput();
	int portcount = midi->getPortCount();
	std::string portname;
	for(int i = 0; i < portcount; ++i)
	{
		setfield(L, i, midi->getPortName(i).c_str());
	}

	return 1;
}

//
// Better name pending.
// returns a table (array) where each key is a port number
// and each value is the name of that port
//
static int luamidi_enumerateinports (lua_State *L)
{
	// the return table
	lua_newtable(L);

	RtMidiIn* midi = getGenericInput();
	int portcount = midi->getPortCount();
	std::string portname;
	for(int i = 0; i < portcount; ++i)
	{
		setfield(L, i, midi->getPortName(i).c_str());
	}

	return 1;
}

// takes in a port number and returns the port's name
static int luamidi_getInPortName(lua_State *L)
{
	int port = luaL_checkint(L, 1);
	RtMidiIn* midi = getGenericInput();
	if(port < midi->getPortCount())
		lua_pushstring(L, midi->getPortName(port).c_str());
	else
		lua_pushstring(L, "");
	return 1;
}

// takes in a port number and returns the port's name
static int luamidi_getOutPortName(lua_State *L)
{
	int port = luaL_checkint(L, 1);
	RtMidiOut* midi = getGenericOutput();
	if(port < midi->getPortCount())
		lua_pushstring(L, midi->getPortName(port).c_str());
	else
		lua_pushstring(L, "");
	return 1;
}

//
// Called when the module is cleaned up.
//
static int __gc(lua_State *L)
{
	cout << "clean up" << endl;
	// TODO: clean up here
	
	// should all of the outs be cleaned up here or when they are garbade
	// collected?  Here for now.
	for(map<int, RtMidiOut*>::iterator iter = outs.begin();
		iter != outs.end(); ++iter)
	{
		delete iter->second;
	}
	outs.clear();

	for(map<int, RtMidiIn*>::iterator iter = ins.begin();
		iter != ins.end(); ++iter)
	{
		delete iter->second;
	}
	ins.clear();

	if(nextout)
		delete nextout;

	if(nextin)
		delete nextin;
	
	return 0;
}

//
// Assumes the table is on top of the stack.
//
static void set_info (lua_State *L)
{
	lua_pushliteral (L, "_COPYRIGHT");
	lua_pushliteral (L, "Copyright (C) 2007 LuaMIDI");
	lua_settable (L, -3);
	lua_pushliteral (L, "_DESCRIPTION");
	lua_pushliteral (L, "Provides a simple interface for MIDI I/O");
	lua_settable (L, -3);
	lua_pushliteral (L, "_VERSION");
	lua_pushliteral (L, "LuaMIDI 0.1");
	lua_settable (L, -3);
}

// these are methods that the library provides
static const struct luaL_reg luamidi_methods [] =
{
	{"getoutportcount",	luamidi_getoutportcount},
	{"getinportcount",	luamidi_getinportcount},
	{"enumerateoutports",	luamidi_enumerateoutports},
	{"enumerateinports",	luamidi_enumerateinports},
	{"getInPortName",	luamidi_getInPortName},
	{"getOutPortName",	luamidi_getOutPortName},
	{"openout",		luamidi_openout},
	{"openin",		luamidi_openin},
	{"noteOn",		luamidi_noteOn},
	{"noteOff",		luamidi_noteOff},
	{"sendMessage",		luamidi_sendMessage},
	{"getMessage",		luamidi_getMessage},
	{"base0",		luamidi_base0},
	{"base1",		luamidi_base1},
	{"__gc",		__gc},
	{NULL, 			NULL}
};

// these are the methods that MidiOut objects have
static const luaL_reg MidiOut_meta[] = 
{
	{"__gc",	MidiOut_gc},
	{"noteOn",	MidiOut_noteOn},
	{"noteOff",	MidiOut_noteOff},
	{"sendMessage", MidiOut_sendMessage},
	{NULL, 		NULL}
};

static int MidiOut_register(lua_State* L)
{
	luaL_newmetatable(L, MIDIOUT);
	lua_pushliteral(L, "__index");
	lua_newtable(L);
	luaL_openlib(L, 0, MidiOut_meta, 0);
	lua_rawset(L, -3);
	lua_pop(L, 1);
	return 1;
}

static const luaL_reg MidiIn_meta[] = 
{
	{"__gc",	MidiIn_gc},
//	{"getMessage",	MidiIn_getMessage},
	{NULL,		NULL}
};

static int MidiIn_register(lua_State* L)
{
	luaL_newmetatable(L, MIDIIN);
	lua_pushliteral(L, "__index");
	lua_newtable(L);
	luaL_openlib(L, 0, MidiIn_meta, 0);
	lua_rawset(L, -3);
	lua_pop(L, 1);
	return 1;
}

extern "C"
{

int luaopen_luamidi (lua_State *L)
{
	MidiOut_register(L);
	MidiIn_register(L);

	luaL_openlib (L, "luamidi", luamidi_methods, 0);
	set_info (L);

	return 1;
}

}


static void g_initializePlugin(lua_State *L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

    lua_pushcnfunction(L, luaopen_luamidi,"luamidi_loader");
    lua_setfield(L, -2, "luamidi");

	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
}

#include "gideros.h"

REGISTER_PLUGIN_NAMED("LuaMidi", "1.0", luamidi)
