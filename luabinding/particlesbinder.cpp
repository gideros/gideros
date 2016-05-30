#include "particlesbinder.h"
#include <particles.h>
#include "luaapplication.h"
#include <luautil.h>

ParticlesBinder::ParticlesBinder(lua_State *L)
{
    Binder binder(L);

    static const luaL_Reg functionList[] = {
            {"addParticles", addParticles},
            {"removeParticles", removeParticles},
        {"setSpeed", setSpeed},
        {"setColor", setColor},
        {"getSpeed", getSpeed},
        {"getColor", getColor},

        {"setTexture", setTexture},
        {"clearTexture", clearTexture},

        {NULL, NULL},
    };

    binder.createClass("Particles", "Sprite", create, destruct, functionList);
}

int ParticlesBinder::create(lua_State *L)
{
    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

    binder.pushInstance("Particles", new Particles(application->getApplication()));

    return 1;
}

int ParticlesBinder::destruct(lua_State *L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    Particles* mesh = static_cast<Particles*>(ptr);
    mesh->unref();

    return 0;
}

int ParticlesBinder::removeParticles(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    if (lua_type(L, 2) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 2);
        for (int k = 0; k < n; ++k)
        {
            lua_rawgeti(L, 2, k + 1);
            int i = luaL_checkinteger(L, -1) - 1;
            lua_pop(L, 1);
            mesh->removeParticle(i);
        }
    }
    else
    {
        int n = lua_gettop(L) - 1;
        for (int k = 0; k < n; ++k)
        {
            int i = luaL_checkinteger(L, k + 2) - 1;
            mesh->removeParticle(i);
        }
    }

    return 0;
}

int ParticlesBinder::setSpeed(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    float vx=luaL_optnumber(L,3,0);
    float vy=luaL_optnumber(L,4,0);
    float va=luaL_optnumber(L,5,0);
    float decay=luaL_optnumber(L,6,0);

    mesh->setSpeed(i, vx,vy,va,decay);

    return 0;
}

int ParticlesBinder::setColor(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");
    unsigned int color = luaL_checkinteger(L, 3);
    float alpha = luaL_optnumber(L, 4, 1.0);

    mesh->setColor(i, color, alpha);

    return 0;
}

int ParticlesBinder::addParticles(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    if (lua_type(L, 2) == LUA_TTABLE) //TODO
    {
        int n = lua_objlen(L, 2);
        for (int k = 0; k < n/4; ++k)
        {
            lua_rawgeti(L, 2, k * 3 + 1);
            int i = luaL_checkinteger(L, -1) - 1;
            lua_pop(L, 1);

            lua_rawgeti(L, 2, k * 3 + 2);
            unsigned int color = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_rawgeti(L, 2, k * 3 + 3);
            float alpha = luaL_checknumber(L, -1);
            lua_pop(L, 1);

            mesh->setColor(i, color, alpha);
        }
    }
    else
    {
        int n = lua_gettop(L) - 1;
        for (int k = 0; k < n/4; ++k)
        {
            int i = luaL_checkinteger(L, k * 3 + 2) - 1;
            unsigned int color = luaL_checkinteger(L, k * 3 + 3);
            float alpha = luaL_checknumber(L, k * 3 + 4);

            mesh->setColor(i, color, alpha);
        }
    }

    return 0;
}

int ParticlesBinder::getSpeed(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    float vx,vy,va,decay;
    mesh->getSpeed(i, &vx,&vy,&va,&decay);
    lua_pushnumber(L, vx);
    lua_pushnumber(L, vy);
    lua_pushnumber(L, va);
    lua_pushnumber(L, decay);

    return 4;
}

int ParticlesBinder::getColor(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    unsigned int color;
    float alpha;
    mesh->getColor(i, &color, &alpha);
    lua_pushinteger(L, color);
    lua_pushnumber(L, alpha);

    return 2;
}

int ParticlesBinder::setTexture(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 2));

    mesh->setTexture(textureBase);

    return 0;
}

int ParticlesBinder::clearTexture(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    mesh->clearTexture();

    return 0;
}
