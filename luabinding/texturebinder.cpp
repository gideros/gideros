#include "texturebinder.h"
#include "texture.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include "giderosexception.h"
#include <string.h>
#include <luautil.h>

TextureBinder::TextureBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
        {"loadAsync",TextureBinder::loadAsync},
        {NULL, NULL},
	};

	binder.createClass("Texture", "TextureBase", create, destruct, functionList);
}

int TextureBinder::create(lua_State* L)
{
	StackChecker checker(L, "TextureBinder::create", 1);

	LuaApplication* luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
	Application* application = luaapplication->getApplication();

	bool isFromPixels=lua_isnumber(L,3);
	size_t filenamesz=0;
	const char* filename = isFromPixels?luaL_optlstring(L,1,NULL,&filenamesz):luaL_checkstring(L, 1);

	unsigned int width, height;
	if (isFromPixels)
	{
		width=luaL_checkinteger(L,2);
		height=luaL_checkinteger(L,3);
		if (filename&&(filenamesz!=(width*height*4)))
			filename=NULL;
	}

	bool smoothing = lua_toboolean(L, isFromPixels?4:2);

	bool maketransparent = false;
	unsigned int transparentcolor = 0x00000000;
    Wrap wrap = eClamp;
    Format format = eRGBA8888;
    int paramsIndex=isFromPixels?5:3;
    bool pow2=true;
    bool rawalpha=false;
    float scale=1.0;
	if (!lua_isnoneornil(L, paramsIndex))
	{
		if (lua_type(L, paramsIndex) != LUA_TTABLE)
            luaL_typerror(L, paramsIndex, "table");

		lua_getfield(L, paramsIndex, "transparentColor");
		if (!lua_isnil(L, -1))
		{
			maketransparent = true;
			transparentcolor = luaL_checkinteger(L, -1);
		}
		lua_pop(L, 1);

        lua_getfield(L, paramsIndex, "wrap");
        if (!lua_isnil(L, -1))
        {
            const char *wrapstr = luaL_checkstring(L, -1);
            if (strcmp(wrapstr, "clamp") == 0)
                wrap = eClamp;
            else if (strcmp(wrapstr, "repeat") == 0)
                wrap = eRepeat;
            else
            {
                GStatus status(2008, "wrap");		// Error #2008: Parameter %s must be one of the accepted values.
                luaL_error(L, "%s", status.errorString());
            }
        }
        lua_pop(L, 1);

        lua_getfield(L, paramsIndex, "format");
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
        lua_getfield(L, paramsIndex, "extend");
        if (!lua_isnil(L, -1))
          pow2=lua_toboolean(L,-1);
        lua_pop(L, 1);
        lua_getfield(L, paramsIndex, "scale");
        scale=luaL_optnumber(L,-1,1.0);
        lua_pop(L, 1);
        lua_getfield(L, paramsIndex, "rawalpha");
        if (!lua_isnil(L, -1))
          rawalpha=lua_toboolean(L,-1);
        lua_pop(L, 1);
    }

	
	Binder binder(L);

	TextureParameters parameters;
	parameters.filter = smoothing ? eLinear : eNearest;
	parameters.wrap = wrap;
    parameters.format = format;
	parameters.maketransparent = maketransparent;
	parameters.transparentcolor = transparentcolor;
	parameters.rawalpha = rawalpha;
    parameters.pow2 = pow2;

	Texture* texture = 0;
	try
	{
		if (isFromPixels)
            texture = new Texture(application, (unsigned char *) filename, width, height, parameters, scale);
		else
            texture = new Texture(application, filename, parameters);
	}
	catch (const GiderosException& e)
	{
        luaL_error(L, "%s", e.what());
	}

	binder.pushInstance("Texture", texture);
	return 1;
}

int TextureBinder::destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
	Texture* texture = static_cast<Texture*>(ptr);
	texture->unref();

	return 0;
}

int TextureBinder::loadAsync(lua_State* L)
{
    StackChecker checker(L, "TextureBinder::loadAsync", 0);

    LuaApplication* luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
    Application* application = luaapplication->getApplication();

    luaL_checktype(L,1,LUA_TFUNCTION);
    const char* filename = luaL_checkstring(L, 2);
    bool smoothing = lua_toboolean(L, 3);
    bool maketransparent = false;
    unsigned int transparentcolor = 0x00000000;
    Wrap wrap = eClamp;
    Format format = eRGBA8888;
    bool pow2=true;
    bool rawalpha=false;
    if (!lua_isnoneornil(L, 4))
    {
        if (lua_type(L, 4) != LUA_TTABLE)
            luaL_typerror(L, 4, "table");

        lua_getfield(L, 4, "transparentColor");
        if (!lua_isnil(L, -1))
        {
            maketransparent = true;
            transparentcolor = luaL_checkinteger(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, 4, "wrap");
        if (!lua_isnil(L, -1))
        {
            const char *wrapstr = luaL_checkstring(L, -1);
            if (strcmp(wrapstr, "clamp") == 0)
                wrap = eClamp;
            else if (strcmp(wrapstr, "repeat") == 0)
                wrap = eRepeat;
            else
            {
                GStatus status(2008, "wrap");		// Error #2008: Parameter %s must be one of the accepted values.
                luaL_error(L, "%s", status.errorString());
            }
        }
        lua_pop(L, 1);

        lua_getfield(L, 4, "format");
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
        lua_getfield(L, 4, "extend");
        if (!lua_isnil(L, -1))
          pow2=lua_toboolean(L,-1);
        lua_pop(L, 1);
        lua_getfield(L, 4, "rawalpha");
        if (!lua_isnil(L, -1))
          rawalpha=lua_toboolean(L,-1);
        lua_pop(L, 1);
    }


    lua_State *LL=luaapplication->getLuaState();
    lua_pushvalue(L,1);
    int func=luaL_ref(L, LUA_REGISTRYINDEX);

	TextureParameters parameters;
	parameters.filter = smoothing ? eLinear : eNearest;
	parameters.wrap = wrap;
    parameters.format = format;
	parameters.maketransparent = maketransparent;
	parameters.transparentcolor = transparentcolor;
	parameters.rawalpha=rawalpha;
    parameters.pow2 = pow2;

    Texture::loadAsync(application, filename, parameters,
                                  [=](Texture *texture,std::exception_ptr e) {
        Binder binder(LL);
        lua_rawgeti(LL, LUA_REGISTRYINDEX, func);
        luaL_unref(LL, LUA_REGISTRYINDEX, func);
/*        try
        {
            if (e) std::rethrow_exception(e);
        }
        catch (const GiderosException& e)
        {
            luaL_error(LL, "%s", e.what());
        }*/

        if (texture)
        	binder.pushInstance("Texture", texture);
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


