/* windows.hkey module
require "windows.hkey"
local k1 = windows.hkey.HKEY_LOCAL_MACHINE:open("Software")
local k2 = k1:create("Sample")
k2:setvalue("Cheese", "Cheddar")
k2:setvalue("Fruit", 42)
local valuedata, valuetype = k2:queryvalue("Fruit")
print(valuedata, valuetype)
for keyname in k1:enumkeys() do print(keyname) end
for valuename in k2:enumvalues(false) do print(valuename) end
k2:deletevalue("Cheese")
for valuename, valuedata, valuetype in k2:enumvalues(true) do print(valuename, valuedata, valuetype) end
k2:close()
k1:delete("Sample")
*/

#include <assert.h>
#include <lua.hpp>
#include <lauxlib.h>
#include <stdlib.h>
#include <string>
#if LUA_VERSION_NUM < 501
#include "compat-5.1.h"
#define luaL_register(L,name,libs) luaL_module(L,name,libs,0)
#endif

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>

#define DBUG_ENTER(a)
#define DBUG_T_RETURN(type,var) return var
#define DBUG_PRINT(x,...)

//#include <dbug.h>

//#include "pusherror.h"
int windows_pusherror(lua_State *L, int err, int nret) {
	if (err!=ERROR_SUCCESS)
	{
		lua_pushfstring(L,"WinERR: %d",err);
		lua_error(L);
	}
	return nret;
}

#define EXTERN
#include "hkey.h"

#define nelemof(a) (sizeof(a)/sizeof*(a))

static void *
smalloc(size_t n)
{
	void *p = malloc(n);
	assert(p != 0);
	return p;
}

#define HKEYHANDLE "HKEY*"

std::wstring ws(const char *str)
{
    if (!str) return std::wstring();
    int sl=strlen(str);
    int sz = MultiByteToWideChar(CP_UTF8, 0, str, sl, 0, 0);
    std::wstring res(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, str, sl, &res[0], sz);
    return res;
}

std::string us(const wchar_t *str)
{
    if (!str) return std::string();
    int sl=wcslen(str);
    int sz = WideCharToMultiByte(CP_UTF8, 0, str, sl, 0, 0,NULL,NULL);
    std::string res(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, str, sl, &res[0], sz,NULL,NULL);
    return res;
}

/* create a new HKEYHANDLE userdatum */
static HKEY *
newkey(lua_State *L)
{
	HKEY *pk;
	DBUG_ENTER("newkey");
	pk = (HKEY *) lua_newuserdata(L, sizeof *pk);
	*pk = 0;
	luaL_getmetatable(L, HKEYHANDLE);
	lua_setmetatable(L, -2);
	DBUG_T_RETURN(HKEY *, pk);
}

/* verify that the given arg is an HKEYHANDLE userdatum */
static HKEY *
checkkey(lua_State *L, int idx)
{
	DBUG_ENTER("checkkey");
	HKEY *pk;
	pk = (HKEY *) luaL_checkudata(L, idx, HKEYHANDLE);
#if LUA_VERSION_NUM < 501
	if (pk == 0) luaL_typerror(L, idx, HKEYHANDLE);
#endif
	DBUG_T_RETURN(HKEY *, pk);
}

/* k:open(subkeyname)
 * k:open(subkeyname, SAM) */
static int
hkey_open(lua_State *L)
{
	HKEY k, *pk;
	std::wstring subkey;
	REGSAM sam = KEY_ALL_ACCESS;
	LONG ret;
	DBUG_ENTER("hkey_open");
	k = *checkkey(L, 1);
	subkey = ws(luaL_checkstring(L, 2));
	if (!lua_isnoneornil(L, 3))
		sam = luaL_checknumber(L, 3);
	pk = newkey(L);
	DBUG_PRINT("W", ("RegOpenKeyEx(%p,\"%s\",...)", k, subkey));
	ret = RegOpenKeyEx(k, subkey.c_str(), 0, sam, pk);
	DBUG_T_RETURN(int, windows_pusherror(L, ret, 1));
}

/* k:create(subkeyname)
 * k:create(subkeyname, SAM) */
static int
hkey_create(lua_State *L)
{
	HKEY k, *pk;
	std::wstring subkey;
	REGSAM sam = KEY_ALL_ACCESS;
	LONG ret;
	DBUG_ENTER("hkey_create");
	k = *checkkey(L, 1);
	subkey = ws(luaL_checkstring(L, 2));
	if (!lua_isnoneornil(L, 3))
		sam = luaL_checknumber(L, 3);
	pk = newkey(L);
	DBUG_PRINT("W", ("RegCreateKeyEx(%p,\"%s\",...)", k, subkey));
	ret = RegCreateKeyEx(k, subkey.c_str(), 0, 0, 0, sam, 0, pk, 0);
	DBUG_T_RETURN(int, windows_pusherror(L, ret, 1));
}

/* k:close() */
static int
hkey_close(lua_State *L)
{
	HKEY *pk;
	LONG ret=ERROR_SUCCESS;
	DBUG_ENTER("hkey_close");
	pk = checkkey(L, 1);
	if (*pk) {
		DBUG_PRINT("W", ("RegCloseKey(%p)", *pk));
		ret = RegCloseKey(*pk);
		*pk = 0;
	}
	lua_pushboolean(L, 1);
	DBUG_T_RETURN(int, windows_pusherror(L, ret, 1));
}

/* k:delete(subkeyname) */
static int
hkey_delete(lua_State *L)
{
	HKEY k;
	std::wstring subkey;
	LONG ret;
	DBUG_ENTER("hkey_delete");
	k = *checkkey(L, 1);
	subkey = ws(luaL_checkstring(L, 2));
	DBUG_PRINT("W", ("RegDeleteKey(%p,\"%s\")", k, subkey));
	ret = RegDeleteKey(k, subkey.c_str());
	lua_pushboolean(L, 1);
	DBUG_T_RETURN(int, windows_pusherror(L, ret, 1));
}

/* k:queryvalue(valuename) */
static int
hkey_queryvalue(lua_State *L)
{
	HKEY k;
	std::wstring vnam;
	DWORD type, datalen;
	char autobuf[1024];
	void *data;
	LONG ret;
	DBUG_ENTER("hkey_queryvalue");
	k = *checkkey(L, 1);
	vnam = ws(luaL_checkstring(L, 2));
	data=NULL;
	if (lua_isnoneornil(L, 3)) {
		data=autobuf;
		datalen=1020; //Leave out four bytes for String terminator
	}
	do {
		DBUG_PRINT("W", ("RegQueryValueEx(%p,\"%s\",...)", k, vnam));
		ret = RegQueryValueEx(k, vnam.c_str(), 0, &type, (LPBYTE) data, &datalen);
	} while (ret == ERROR_MORE_DATA && data && (data = smalloc(datalen+4)));
	if (data) {
		if (ret == ERROR_SUCCESS) {
			switch (type) {
			case REG_DWORD: {
				const DWORD *num = (const DWORD *) data;
				lua_pushnumber(L, *num);
				break; }
			case REG_MULTI_SZ: /* return each string? */
			case REG_SZ:
			case REG_EXPAND_SZ: {
				*((wchar_t *)(data+datalen))=0;
				std::string u=us((wchar_t *)data);
				data=(void *) u.c_str();
				datalen=u.size();
				/*FALLTHRU*/ }
			default:
				lua_pushlstring(L, (const char *) data, datalen);
				break;
			}
		}
		if (data != autobuf)
			free(data);
	}
	lua_pushnumber(L, type);
	DBUG_T_RETURN(int, windows_pusherror(L, ret, 2));
}

/* k:setvalue(valuename,valuedata)
 * k:setvalue(valuename,valuedata,valuetype) */
static int
hkey_setvalue(lua_State *L)
{
	HKEY k;
	std::wstring wd;
	std::wstring vnam;
	DWORD type=0;
	size_t datalen;
	const void *data=NULL;
	DWORD num;
	LONG ret;
	DBUG_ENTER("hkey_setvalue");
	k = *checkkey(L, 1);
	vnam = ws(luaL_checkstring(L, 2));
	switch (lua_type(L, 3)) {
		case LUA_TNONE:
		case LUA_TNIL:
			luaL_argerror(L, 3, "value expected");
			break;
		case LUA_TBOOLEAN:
			num = lua_toboolean(L, 3);
			data = &num;
			datalen = sizeof num;
			type = REG_DWORD;
			break;
		case LUA_TNUMBER:
			num = lua_tonumber(L, 3);
			data = &num;
			datalen = sizeof num;
			type = REG_DWORD;
			break;
		case LUA_TSTRING:
			wd=ws(luaL_checklstring(L, 3, &datalen));
			data=wd.c_str();
			datalen=(wd.size()+1)*sizeof(wchar_t);
			type = REG_SZ;
			break;
		default:
			luaL_typerror(L, 3, "string, boolean, or number");
			break;
	}
	if (!lua_isnoneornil(L, 4))
		type = luaL_checknumber(L, 4);
	DBUG_PRINT("W", ("RegSetValueEx(%p,\"%s\",0,%lu,%p,%lu)", k, vnam, type, data, datalen));
	ret = RegSetValueEx(k, vnam.c_str(), 0, type,(const BYTE *) data, datalen);
	lua_pushboolean(L, 1);
	DBUG_T_RETURN(int, windows_pusherror(L, ret, 1));
}

/* k:deletevalue(valuename) */
static int
hkey_deletevalue(lua_State *L)
{
	DBUG_ENTER("hkey_deletevalue");
	HKEY k;
	std::wstring vnam;
	LONG ret;
	k = *checkkey(L, 1);
	vnam = ws(luaL_checkstring(L, 2));
	DBUG_PRINT("W", ("RegDeleteValue(%p,\"%s\")", k, vnam));
	ret = RegDeleteValue(k, vnam.c_str());
	lua_pushboolean(L, 1);
	DBUG_T_RETURN(int, windows_pusherror(L, ret, 1));
}

/* for keyname in k:enumkeys() do ; end */
struct enumkeys_iter_s {
	HKEY k;
	DWORD index;
};
static int
enumkeys_iter(lua_State *L)
{
	struct enumkeys_iter_s *state;
	wchar_t name[256];
	DWORD namelen;
	LONG ret;
	DBUG_ENTER("enumkeys_iter");
	state = (struct enumkeys_iter_s *) lua_touserdata(L, 1);
	namelen = sizeof name / sizeof *name;
	DBUG_PRINT("W", ("RegEnumKeyEx(%p,%lu,...)", state->k, state->index, name));
	ret = RegEnumKeyEx(state->k, state->index++, name, &namelen, 0, 0, 0, 0);
	if (ret == ERROR_SUCCESS)
		lua_pushstring(L, us(name).c_str());
	DBUG_T_RETURN(int, windows_pusherror(L, ret, 1));
}
static int
hkey_enumkeys(lua_State *L)
{
	HKEY k;
	struct enumkeys_iter_s *state;
	DBUG_ENTER("hkey_enumkeys");
	k = *checkkey(L, 1);
	lua_pushcfunction(L, enumkeys_iter);
	state = (struct enumkeys_iter_s *) lua_newuserdata(L, sizeof *state);
	state->k = k;
	state->index = 0;
	DBUG_T_RETURN(int, 2);
}

/* for valuename, valuedata, valuetype in k:enumvalues() do ; end */
struct enumvalues_iter_s {
	HKEY k;
	DWORD index;
	size_t vnamlen;
	wchar_t *vnam;
	size_t datalen;
	void *data;
};
static int
enumvalues_iter(lua_State *L)
{
	struct enumvalues_iter_s *state;
	DWORD vnamlen, type, datalen;
	LONG ret;
	DBUG_ENTER("enumvalues_iter");
	state = (struct enumvalues_iter_s *)lua_touserdata(L, 1);
	if (state == 0) {
		char buf[40];
		sprintf(buf, "expected enumvalues iteration state not %s\n", lua_typename(L,lua_type(L, 1)));
		luaL_argerror(L,1,buf);
	}
	vnamlen = state->vnamlen;
	datalen = state->datalen;
	DBUG_PRINT("W", ("RegEnumValue(%p,%lu,\"%s\",...)",
		state->k, state->index, state->vnam));
	ret = RegEnumValue(state->k, state->index++,
		state->vnam, &vnamlen, 0,
		&type, (LPBYTE) state->data, &datalen);
	if (ret == ERROR_SUCCESS) {
		lua_pushstring(L, us(state->vnam).c_str());
		lua_pushlstring(L, (const char *) state->data, datalen);
		lua_pushnumber(L, type);
	}
	DBUG_T_RETURN(int, windows_pusherror(L, ret, state->data ? 3 : 1));
}
static int
hkey_enumvalues(lua_State *L)
{
	HKEY k;
	struct enumvalues_iter_s *state;
	DWORD vnamlen, datalen;
	LONG ret;
	DBUG_ENTER("hkey_enumvalues");
	k = *checkkey(L, 1);
	lua_pushcfunction(L, enumvalues_iter);
	DBUG_PRINT("W", ("RegQueryInfoKey(%p,...)", k));
	ret = RegQueryInfoKey(k, 0, 0, 0, 0, 0, 0, 0, &vnamlen, &datalen, 0, 0);
	if (ret == ERROR_SUCCESS) {
		vnamlen++; /* One more for the Gipper. */
		state = (struct enumvalues_iter_s *)lua_newuserdata(L, sizeof *state + vnamlen*sizeof(wchar_t) + datalen);
		state->k = k;
		state->index = 0;
		state->vnamlen = vnamlen;
		state->vnam = (wchar_t *)&state[1];
		state->datalen = datalen;
		state->data = state->vnam + vnamlen;
	}
	DBUG_T_RETURN(int, windows_pusherror(L, ret, 2));
}

/* tostring(k) */
static int
hkey_tostring(lua_State *L)
{
	HKEY k;
	char buf[20];
	DBUG_ENTER("hkey_tostring");
	k = *checkkey(L, 1);
	lua_pushlstring(L, buf, sprintf(buf, "HKEY:%p", (void *)k));
	DBUG_T_RETURN(int, 1);
}


extern "C" int
luaopen_windows_hkey(lua_State *L)
{
	const luaL_reg hkey_lib[] = { {0,0} };
	const luaL_reg hkey_methods[] = {
		{ "open",           hkey_open },
		{ "create",         hkey_create },
		{ "close",          hkey_close },
		{ "delete",         hkey_delete },
		{ "queryvalue",     hkey_queryvalue },
		{ "setvalue",       hkey_setvalue },
		{ "deletevalue",    hkey_deletevalue },
		{ "enumkeys",       hkey_enumkeys },
		{ "enumvalues",     hkey_enumvalues },
		{ "__tostring",     hkey_tostring },
		{ "__gc",           hkey_close },
		{ 0, 0 }
	};
	const struct constant {
		const char *name;
		DWORD value;
	} *pconst, hkey_consts[] = {

	/* Registry Value Types */
		{ "REG_SZ",                  REG_SZ },
		{ "REG_EXPAND_SZ",           REG_EXPAND_SZ },
		{ "REG_DWORD",               REG_DWORD },
		{ "REG_QWORD",               REG_QWORD },
		{ "REG_BINARY",              REG_BINARY },
		{ "REG_NONE",                REG_NONE },
		{ "REG_DWORD_LITTLE_ENDIAN", REG_DWORD_LITTLE_ENDIAN },
		{ "REG_DWORD_BIG_ENDIAN",    REG_DWORD_BIG_ENDIAN },
		{ "REG_LINK",                REG_LINK },
		{ "REG_MULTI_SZ",            REG_MULTI_SZ },
		{ "REG_QWORD_LITTLE_ENDIAN", REG_QWORD_LITTLE_ENDIAN },

	/* Registry access rights  for open() and create() methods */
		{ "KEY_ALL_ACCESS",          KEY_ALL_ACCESS },
		{ "KEY_CREATE_LINK",         KEY_CREATE_LINK },
		{ "KEY_CREATE_SUB_KEY",      KEY_CREATE_SUB_KEY },
		{ "KEY_ENUMERATE_SUB_KEYS",  KEY_ENUMERATE_SUB_KEYS },
		{ "KEY_EXECUTE",             KEY_EXECUTE },
		{ "KEY_NOTIFY",              KEY_NOTIFY },
		{ "KEY_QUERY_VALUE",         KEY_QUERY_VALUE },
		{ "KEY_READ",                KEY_READ },
		{ "KEY_SET_VALUE",           KEY_SET_VALUE },
		{ "KEY_WRITE",               KEY_WRITE },

		{ "KEY_WOW64_64KEY",         KEY_WOW64_64KEY },
		{ "KEY_WOW64_32KEY",         KEY_WOW64_32KEY },

		{ 0, 0 }
	};
	const struct toplevel {
		const char *name;
		HKEY key;
	} *ptop, hkey_top[] = {
		{ "HKEY_CLASSES_ROOT",   HKEY_CLASSES_ROOT },
		{ "HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG },
		{ "HKEY_CURRENT_USER",   HKEY_CURRENT_USER },
		{ "HKEY_LOCAL_MACHINE",  HKEY_LOCAL_MACHINE },
		{ "HKEY_USERS",          HKEY_USERS },
		{ 0, 0 }
	};
	DBUG_ENTER("luaopen_windows_hkey");

	DBUG_PRINT("init", ("metatable for '%s'", HKEYHANDLE));
	luaL_newmetatable(L, HKEYHANDLE);           /* mt */
	lua_pushliteral(L, "__index");              /* mt, "__index" */
	lua_pushvalue(L, -2);                       /* mt, "__index", mt */
	lua_settable(L, -3);                        /* mt */
	luaL_openlib(L, 0, hkey_methods, 0);        /* mt */

	DBUG_PRINT("init", ("openlib"));
	luaL_register(L, "windows.hkey", hkey_lib);

	DBUG_PRINT("init", ("hkey constants"));
	for (pconst = hkey_consts; pconst->name; pconst++) {
		lua_pushstring(L, pconst->name);        /* hkey, name */
		lua_pushnumber(L, pconst->value);       /* hkey, name, value */
		lua_settable(L, -3);                    /* hkey */
	}

	DBUG_PRINT("init", ("toplevel keys"));
	for (ptop = hkey_top; ptop->name; ptop++) {
		lua_pushstring(L, ptop->name);          /* hkey, name */
		*newkey(L) = ptop->key;                 /* hkey, name, key */
		lua_settable(L, -3);                    /* hkey */
	}

	DBUG_T_RETURN(int, 1);
}

