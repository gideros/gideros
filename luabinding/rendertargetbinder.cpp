#include "rendertargetbinder.h"
#include <grendertarget.h>
#include "luaapplication.h"
#include <luautil.h>

RenderTargetBinder::RenderTargetBinder(lua_State *L)
{
    Binder binder(L);

    static const luaL_Reg functionList[] = {
        {"clear", clear},
        {"draw", draw},
		{"getPixel", getPixel},
		{"getPixels", getPixels},
        {NULL, NULL},
    };

    binder.createClass("RenderTarget", "TextureBase", create, destruct, functionList);
}


int RenderTargetBinder::create(lua_State *L)
{
    LuaApplication *application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);
    bool smoothing = lua_toboolean(L, 3);

    binder.pushInstance("RenderTarget", new GRenderTarget(application->getApplication(), width, height, smoothing ? eLinear : eNearest));

    return 1;
}

int RenderTargetBinder::destruct(lua_State *L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    GReferenced* refptr = static_cast<GReferenced*>(ptr);
    refptr->unref();

    return 0;
}

int RenderTargetBinder::clear(lua_State *L)
{
    Binder binder(L);

    GRenderTarget *renderTarget = static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", 1));
    unsigned int color = luaL_checkinteger(L, 2);
    float alpha = luaL_optnumber(L, 3, 1.0);
    int x=luaL_optinteger(L,4,0);
    int y=luaL_optinteger(L,5,0);
    int w=luaL_optinteger(L,6,-1);
    int h=luaL_optinteger(L,7,-1);

    renderTarget->clear(color, alpha, x, y, w, h);

    return 0;
}

int RenderTargetBinder::draw(lua_State *L)
{
    Binder binder(L);

    GRenderTarget *renderTarget = static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", 1));
    Sprite *sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 2));

    renderTarget->draw(sprite);

    return 0;
}

int RenderTargetBinder::getPixels(lua_State *L)
{
    Binder binder(L);

    GRenderTarget *renderTarget = static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", 1));
    unsigned int x = luaL_checkinteger(L, 2);
    unsigned int y = luaL_checkinteger(L, 3);
    unsigned int w = luaL_checkinteger(L, 4);
    unsigned int h = luaL_checkinteger(L, 5);

    void *buffer=malloc(w*h*4);
    renderTarget->getPixels(x,y,w,h,buffer);
    lua_pushlstring(L,(char *)buffer,w*h*4);
    free(buffer);

    return 1;
}

int RenderTargetBinder::getPixel(lua_State *L)
{
    Binder binder(L);

    GRenderTarget *renderTarget = static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", 1));
    unsigned int x = luaL_checkinteger(L, 2);
    unsigned int y = luaL_checkinteger(L, 3);

    unsigned char pixel[4];

    renderTarget->getPixels(x,y,1,1,pixel);

    lua_pushnumber(L,(pixel[0]<<16)|(pixel[1]<<8)|(pixel[2]<<0));
    lua_pushnumber(L,((float)pixel[3])/255.0);

    return 2;
}
