#include "rendertargetbinder.h"
#include <grendertarget.h>
#include "luaapplication.h"

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
    LuaApplication *application = static_cast<LuaApplication*>(lua_getdata(L));

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

    renderTarget->clear(color, alpha);

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
