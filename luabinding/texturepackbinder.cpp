#include "texturepackbinder.h"
#include "texturepack.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include "giderosexception.h"
#include <string.h>
#include <luautil.h>
#include <utf8.h>

TexturePackBinder::TexturePackBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
        {"getLocation", TexturePackBinder::getLocation},
        {"loadAsync", TexturePackBinder::loadAsync},
        {"getRegionsNames", TexturePackBinder::getRegionsNames},
        {"allocateRegion", TexturePackBinder::allocateRegion},
        {NULL, NULL},
	};

	binder.createClass("TexturePack", "TextureBase", create, destruct, functionList);
}

int TexturePackBinder::createCommon(lua_State* L,bool async)
{
    LuaApplication* luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
    Application* application = luaapplication->getApplication();

    int argn=1;
    if (async) {
        luaL_checktype(L,1,LUA_TFUNCTION);
        argn++;
    }

    if (lua_type(L, argn) == LUA_TTABLE)
    {
        // collect the filenames to vector<string>
        std::vector<std::string> fileNames;
        int n = lua_objlen(L, argn);
        for (int i = 1; i <= n; ++i)
        {
            lua_rawgeti(L, argn, i);
            fileNames.push_back(luaL_checkstring(L, -1));
            lua_pop(L, 1);
        }

        int padding = luaL_optint(L, argn+1, 2);

        bool smoothing = lua_toboolean(L, argn+2);

        bool maketransparent = false;
        unsigned int transparentcolor = 0x00000000;
        Format format = eRGBA8888;
        bool mipmap = false;
        bool rawalpha=false;
        if (!lua_isnoneornil(L, argn+3))
        {
            if (lua_type(L, argn+3) != LUA_TTABLE)
                luaL_typerror(L, argn+3, "table");

            lua_getfield(L, argn+3, "transparentColor");
            if (!lua_isnil(L, -1))
            {
                maketransparent = true;
                transparentcolor = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);

            lua_getfield(L, argn+3, "format");
            if (!lua_isnil(L, -1))
            {
                const char *formatstr = luaL_checkstring(L, -1);
                if (strcmp(formatstr, "rgba8888") == 0)
                    format = eRGBA8888;
                else if (strcmp(formatstr, "rgb888") == 0)
                    format = eRGB888;
                else if (strcmp(formatstr, "rgb565") == 0)
                    format = eRGB565;
                else if (strcmp(formatstr, "rgba4444") == 0)
                    format = eRGBA4444;
                else if (strcmp(formatstr, "rgba5551") == 0)
                    format = eRGBA5551;
                else if (strcmp(formatstr, "y8") == 0)
                    format = eY8;
                else if (strcmp(formatstr, "a8") == 0)
                    format = eA8;
                else if (strcmp(formatstr, "ya8") == 0)
                    format = eYA8;
                else
                {
                    GStatus status(2008, "format");		// Error #2008: Parameter %s must be one of the accepted values.
                    luaL_error(L, "%s", status.errorString());
                }
            }
            lua_pop(L, 1);
            lua_getfield(L, argn+3, "rawalpha");
            if (!lua_isnil(L, -1))
              rawalpha=lua_toboolean(L,-1);
            lua_pop(L, 1);
            lua_getfield(L, argn+3, "mipmap");
            if (!lua_isnil(L, -1))
              mipmap=lua_toboolean(L,-1);
            lua_pop(L, 1);
        }


        // collect the pointers to filenames into vector<const char*>
        std::vector<const char*> fileNamePointers;
        for (std::size_t i = 0; i < fileNames.size(); ++i)
            fileNamePointers.push_back(fileNames[i].c_str());
        fileNamePointers.push_back(0);

        Binder binder(L);
        TextureParameters parameters;
        parameters.filter = smoothing ? (mipmap? eLinearMipmap:eLinear) : eNearest;
        parameters.wrap=eClamp;
        parameters.format=format;
        parameters.maketransparent=maketransparent;
        parameters.transparentcolor=transparentcolor;
        parameters.rawalpha = rawalpha;
        if (async) {
            lua_State *LL=luaapplication->getLuaState();
            lua_pushvalue(L,1);
            int func=luaL_ref(L, LUA_REGISTRYINDEX);
            TexturePack::loadAsync(application,
                                   &fileNamePointers[0],
                                   padding,
                                   parameters,
                                          [=](TexturePack *texturePack,std::exception_ptr e) {
                Binder binder(LL);
                lua_rawgeti(LL, LUA_REGISTRYINDEX, func);
                luaL_unref(LL, LUA_REGISTRYINDEX, func);
                if (texturePack)
                    binder.pushInstance("TexturePack", texturePack);
                else
                    lua_pushnil(L);
                if (e) {
                    try { std::rethrow_exception(e); }
                    catch (const std::exception &e) { lua_pushstring(L,e.what()); }
                    catch (const std::string    &e) { lua_pushstring(L,e.c_str()); }
                    catch (const char           *e) { lua_pushstring(L,e); }
                    catch (...)                     { lua_pushstring(L,"Unspecified error"); }
                }
                else
                    lua_pushnil(L);
                lua_call(LL,2,0);
                });
            return 0;
        }
        else {
            TexturePack* texturePack = 0;
            try
            {
                texturePack = new TexturePack(application,
                                              &fileNamePointers[0],
                                              padding,
                                              parameters);
            }
            catch (const GiderosException& e)
            {
                luaL_error(L, "%s", e.what());				// TODO: burada luaL_error dedigimiz icin longjmp yuzunden vectorlerin destructor'lari cagirilmiyor
                return 0;
            }

            binder.pushInstance("TexturePack", texturePack);
        }
    }
    else if (lua_type(L, argn) == LUA_TSTRING && lua_type(L, argn+1) == LUA_TSTRING)
    {
        const char* texturelistfile = lua_tostring(L, argn);
        const char* imagefile = lua_tostring(L, argn+1);
        bool smoothing = lua_toboolean(L, argn+2);

        bool maketransparent = false;
        unsigned int transparentcolor = 0x00000000;
        Format format = eRGBA8888;
        bool mipmap = false;
        bool rawalpha=false;
        if (!lua_isnoneornil(L, argn+3))
        {
            if (lua_type(L, argn+3) != LUA_TTABLE)
                luaL_typerror(L, argn+3, "table");

            lua_getfield(L, argn+3, "transparentColor");
            if (!lua_isnil(L, -1))
            {
                maketransparent = true;
                transparentcolor = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);

            lua_getfield(L, argn+3, "format");
            if (!lua_isnil(L, -1))
            {
                const char *formatstr = luaL_checkstring(L, -1);
                if (strcmp(formatstr, "rgba8888") == 0)
                    format = eRGBA8888;
                else if (strcmp(formatstr, "rgb888") == 0)
                    format = eRGB888;
                else if (strcmp(formatstr, "rgb565") == 0)
                    format = eRGB565;
                else if (strcmp(formatstr, "rgba4444") == 0)
                    format = eRGBA4444;
                else if (strcmp(formatstr, "rgba5551") == 0)
                    format = eRGBA5551;
                else if (strcmp(formatstr, "y8") == 0)
                    format = eY8;
                else if (strcmp(formatstr, "a8") == 0)
                    format = eA8;
                else if (strcmp(formatstr, "ya8") == 0)
                    format = eYA8;
                else
                {
                    GStatus status(2008, "format");		// Error #2008: Parameter %s must be one of the accepted values.
                    luaL_error(L, "%s", status.errorString());
                }
            }
            lua_pop(L, 1);
            lua_getfield(L, argn+3, "rawalpha");
            if (!lua_isnil(L, -1))
              rawalpha=lua_toboolean(L,-1);
            lua_pop(L, 1);
            lua_getfield(L, argn+3, "mipmap");
            if (!lua_isnil(L, -1))
              mipmap=lua_toboolean(L,-1);
            lua_pop(L, 1);
        }

        Binder binder(L);
        TextureParameters parameters;
        parameters.filter = smoothing ? (mipmap? eLinearMipmap:eLinear) : eNearest;
        parameters.wrap=eClamp;
        parameters.format=format;
        parameters.maketransparent=maketransparent;
        parameters.transparentcolor=transparentcolor;
        parameters.rawalpha = rawalpha;

        if (async) {
            lua_State *LL=luaapplication->getLuaState();
            lua_pushvalue(L,1);
            int func=luaL_ref(L, LUA_REGISTRYINDEX);
            TexturePack::loadAsync(application,
                                   texturelistfile,
                                   imagefile,
                                   parameters,
                                          [=](TexturePack *texturePack,std::exception_ptr e) {
                Binder binder(LL);
                lua_rawgeti(LL, LUA_REGISTRYINDEX, func);
                luaL_unref(LL, LUA_REGISTRYINDEX, func);
                if (texturePack)
                    binder.pushInstance("TexturePack", texturePack);
                else
                    lua_pushnil(L);
                if (e) {
                    try { std::rethrow_exception(e); }
                    catch (const std::exception &e) { lua_pushstring(L,e.what()); }
                    catch (const std::string    &e) { lua_pushstring(L,e.c_str()); }
                    catch (const char           *e) { lua_pushstring(L,e); }
                    catch (...)                     { lua_pushstring(L,"Unspecified error"); }
                }
                else
                    lua_pushnil(L);
                lua_call(LL,2,0);
                });
            return 0;
        }
        else {
            TexturePack* texturePack = 0;
            try
            {
                texturePack = new TexturePack(application,
                                              texturelistfile,
                                              imagefile,
                                              parameters);
            }
            catch (const GiderosException& e)
            {
                luaL_error(L, "%s", e.what());
                return 0;
            }

            binder.pushInstance("TexturePack", texturePack);
        }
    }
    else
    {
        luaL_error(L, "Bad argument to 'TexturePack.new'. Candidates are TexturePack.new(table) and TexturePack.new(string, string).");
    }

    return 1;
}

int TexturePackBinder::create(lua_State* L)
{
	StackChecker checker(L, "TexturePackBinder::create", 1);
    Binder binder(L);
    if (binder.isInstanceOf("RenderTarget",1))
    {
        LuaApplication* luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
        Application* application = luaapplication->getApplication();
        TexturePack *texturePack = new TexturePack(application, (GRenderTarget *)binder.getInstance("RenderTarget",1));
        binder.pushInstance("TexturePack", texturePack);
        return 1;
    }
    return createCommon(L,false);
}

int TexturePackBinder::loadAsync(lua_State* L)
{
    StackChecker checker(L, "TexturePackBinder::loadAsync", 0);
    return createCommon(L,true);
}


int TexturePackBinder::destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
	TexturePack* texturePack = static_cast<TexturePack*>(ptr);
	texturePack->unref();

	return 0;
}

int TexturePackBinder::getRegionsNames(lua_State* L)
{
	StackChecker checker(L, "TexturePackBinder::getRegionsNames", 1);

	Binder binder(L);
	TexturePack* texturePack = static_cast<TexturePack*>(binder.getInstance("TexturePack", 1));

	std::vector<std::string> names=texturePack->getRegionsNames();
	size_t nnames=names.size();
	lua_createtable(L,nnames,0);
	for (size_t i=0;i<nnames;i++) {
		lua_pushstring(L,names[i].c_str());
		lua_rawseti(L,-2,i+1);
	}
	return 1;
}

int TexturePackBinder::allocateRegion(lua_State* L)
{
    Binder binder(L);
    TexturePack* texturePack = static_cast<TexturePack*>(binder.getInstance("TexturePack", 1));
    const char *name=luaL_checkstring(L,2);
    int w=luaL_checkinteger(L,3);
    int h=luaL_checkinteger(L,4);
    int x,y;
    if (!texturePack->allocate(name,w,h,x,y))
        return 0;
    lua_pushnumber(L,x);
    lua_pushnumber(L,y);
    return 2;
}

int TexturePackBinder::getLocation(lua_State* L)
{
	StackChecker checker(L, "TexturePackBinder::getLocation", 8);
	
	Binder binder(L);
	TexturePack* texturePack = static_cast<TexturePack*>(binder.getInstance("TexturePack", 1));

	if (lua_type(L, 2) != LUA_TNUMBER && lua_type(L, 2) != LUA_TSTRING)
		luaL_typerror(L, 2, "number or string");

	int x = 0, y = 0, width = 0, height = 0, dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
	bool success;

	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		int index = lua_tointeger(L, 2);
		success = texturePack->location(index - 1, &x, &y, &width, &height, &dx1, &dy1, &dx2, &dy2); // indices are started from 1 in lua
	}
	else
	{
		const char* name = lua_tostring(L, 2);
		success = texturePack->location(name, &x, &y, &width, &height, &dx1, &dy1, &dx2, &dy2);
	}

	if (success == true)
	{
		lua_pushinteger(L, x);
		lua_pushinteger(L, y);
		lua_pushinteger(L, width);
		lua_pushinteger(L, height);
		lua_pushinteger(L, dx1);
		lua_pushinteger(L, dy1);
		lua_pushinteger(L, dx2);
		lua_pushinteger(L, dy2);
	}
	else
	{
		lua_pushnil(L);			// in fact, this is unnecassary, we should just return 0. But stackchecker requires this sillyness :)
		lua_pushnil(L);			// note 2: I think there is difference :)
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
	}

	return 8;
}





TexturePackFontBinder::TexturePackFontBinder(lua_State* L)
{
    Binder binder(L);

    static const luaL_Reg functionList[] = {
        {"mapCharacter", mapCharacter},
        {NULL, NULL},
    };

    binder.createClass("TexturePackFont", "FontBase", create, destruct, functionList);
}

int TexturePackFontBinder::create(lua_State* L)
{
    StackChecker checker(L, "TexturePackFontBinder::create", 1);
    LuaApplication* luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
    Application* application = luaapplication->getApplication();

    Binder binder(L);

    if (!binder.isInstanceOf("TexturePack", 1))
    {
        luaL_typerror(L, 1, "TexturePack");
        return 0;
    }
    TexturePack *tp= static_cast<TexturePack*>(binder.getInstance("TexturePack", 1));
    std::map<wchar32_t,std::string> mappings;
    if (!lua_istable(L,2))
    {
        luaL_typerror(L, 2, "table");
        return 0;
    }
    double scale=luaL_optnumber(L,3,1);
    double anchory=luaL_optnumber(L,4,1);
    lua_pushnil(L);  /* first key */
    while (lua_next(L, 2) != 0) {
       const char *key=lua_tostring(L,-2);
       wchar32_t w=0;
       utf8_to_wchar(key,strlen(key),&w,1,0);
       mappings[w]=lua_tostring(L,-1);
       lua_pop(L, 1);
    }

    TexturePackFont *tpf=new TexturePackFont(application,tp,mappings,scale,anchory);
    binder.pushInstance("TexturePackFont", tpf);

    return 1;
}


int TexturePackFontBinder::destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
    TexturePackFont* texturePack = static_cast<TexturePackFont*>(ptr);
    texturePack->unref();

    return 0;
}

int TexturePackFontBinder::mapCharacter(lua_State* L)
{
    Binder binder(L);
    TexturePackFont* tpf = static_cast<TexturePackFont*>(binder.getInstance("TexturePackFont", 1));
    size_t kl;
    const char *key=luaL_checklstring(L,2,&kl);
    const char *val=luaL_optstring(L,3,NULL);
    wchar32_t w=0;
    utf8_to_wchar(key,kl,&w,1,0);
    tpf->mapCharacter(w,val);
    return 0;
}
