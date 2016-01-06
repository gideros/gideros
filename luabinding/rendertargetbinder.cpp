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
