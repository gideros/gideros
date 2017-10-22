/*
 * lqParticles.cpp
 *
 *  Created on: 22 oct. 2017
 *      Author: Nicolas
 */
#include "liquidfunbinder.h"
#include "lfstatus.h"
#include "lqSprites.h"
#include "lqWorld.h"

static void tableToParticleSystemDef(lua_State* L, int index, b2ParticleSystemDef* particleDef, float physicsScale)
{
    Binder binder(L);

    const struct {
    	const char *name;
    	float *ptr;
    } fValues[16]=
    {
    		{"density", &particleDef->density},
    		{"gravityScale", &particleDef->gravityScale},
    		{"pressureStrength", &particleDef->pressureStrength},
    		{"dampingStrength", &particleDef->dampingStrength},
    		{"elasticStrength", &particleDef->elasticStrength},
    		{"springStrength", &particleDef->springStrength},
    		{"viscousStrength", &particleDef->viscousStrength},
    		{"surfaceTensionPressureStrength", &particleDef->surfaceTensionPressureStrength},
    		{"surfaceTensionNormalStrength", &particleDef->surfaceTensionNormalStrength},
    		{"repulsiveStrength", &particleDef->repulsiveStrength},
    		{"powderStrength", &particleDef->powderStrength},
    		{"ejectionStrength", &particleDef->ejectionStrength},
    		{"staticPressureStrength", &particleDef->staticPressureStrength},
    		{"staticPressureRelaxation", &particleDef->staticPressureRelaxation},
    		{"colorMixingStrength", &particleDef->colorMixingStrength},
    		{"lifetimeGranularity", &particleDef->lifetimeGranularity},
    };

    for (int v=0;v<14;v++)
    {
        lua_getfield(L, index, fValues[v].name);
        if (!lua_isnil(L, -1))
            *(fValues[v].ptr)=luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    lua_getfield(L, index, "strictContactCheck");
    if (!lua_isnil(L, -1))
        particleDef->strictContactCheck = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "radius");
    if (!lua_isnil(L, -1))
        particleDef->radius=luaL_checknumber(L, -1)/physicsScale;
    lua_pop(L, 1);

    lua_getfield(L, index, "maxCount");
    if (!lua_isnil(L, -1))
        particleDef->maxCount = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "staticPressureIterations");
    if (!lua_isnil(L, -1))
        particleDef->staticPressureIterations = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "destroyByAge");
    if (!lua_isnil(L, -1))
        particleDef->destroyByAge = lua_toboolean(L, -1);
    lua_pop(L, 1);
}

int Box2DBinder2::b2World_createParticleSystem(lua_State* L)
{
    //StackChecker checker(L, "b2World_createParticleSystem", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);
    b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

    if (world->IsLocked())
        return luaL_error(L, LFStatus(5004).errorString());	// Error #5004: World is locked.

    b2ParticleSystemDef particleSystemDef;
    tableToParticleSystemDef(L, 2, &particleSystemDef, application->getPhysicsScale());

    b2ParticleSystem* particleSystem = world->CreateParticleSystem(&particleSystemDef);
    b2ParticleSystemSprite *ps=new b2ParticleSystemSprite(application,particleSystem,world);

    binder.pushInstance("b2ParticleSystem", ps->proxy_);

    return 1;
}

static void tableToParticleDef(lua_State* L, int index, b2ParticleDef* particleDef, float physicsScale)
{
    // TODO: index'tekinin table oldugunu test et

    Binder binder(L);

    lua_getfield(L, index, "flags");
    if (!lua_isnil(L, -1))
        particleDef->flags = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "position");
    if (!lua_isnil(L, -1))
    {
        particleDef->position = tableToVec2(L, -1);
        particleDef->position.x /= physicsScale;
        particleDef->position.y /= physicsScale;
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "velocity");
    if (!lua_isnil(L, -1))
        particleDef->velocity = tableToVec2(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "color");
    if (!lua_isnil(L, -1))
    {
        unsigned int color = luaL_checkinteger(L, -1);
        particleDef->color.r = (color >> 16) & 0xff;
        particleDef->color.g = (color >> 8) & 0xff;
        particleDef->color.b = color & 0xff;
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "alpha");
    if (!lua_isnil(L, -1))
    {
        int alpha = (int)(luaL_checknumber(L, -1) * 255);
        particleDef->color.a = std::min(std::max(alpha, 0), 255);
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "lifetime");
    if (!lua_isnil(L, -1))
        particleDef->lifetime = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "group");
    if (!lua_isnil(L, -1))
    	particleDef->group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", -1));
    lua_pop(L, 1);
}

int Box2DBinder2::b2ParticleSystem_createParticle(lua_State* L)
{
    //StackChecker checker(L, "b2ParticleSystem_createParticle", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());


    b2ParticleDef particleDef;
    tableToParticleDef(L, 2, &particleDef, application->getPhysicsScale());

    if (ps->GetWorld()->IsLocked())
    	return luaL_error(L, LFStatus(5004).errorString());	// Error #5004: World is locked.

    int32 p=ps->GetSystem()->CreateParticle(particleDef);
    lua_pushinteger(L, p);

    return 1;
}

int Box2DBinder2::b2ParticleSystem_destroyParticle(lua_State* L)
{
    //StackChecker checker(L, "b2ParticleSystem_destroyParticle", 0);

    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());

    ps->GetSystem()->DestroyParticle((int)luaL_checkinteger(L, 2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_setTexture(lua_State *L)
{
    //StackChecker checker(L, "b2ParticleSystem_setTexture", 0);
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    TextureBase *textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 2));
    ps->SetTexture(textureBase);

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getParticleGroupList(lua_State *L)
{
    //StackChecker checker(L, "b2ParticleSystem_getParticleGroupList", 1);
    Binder binder(L);


    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    if (ps == NULL)
    {
        return luaL_error(L, "ParticleSystem required in argument #1");
    }

    if (ps->GetWorld()->IsLocked())
        return luaL_error(L, LFStatus(5004).errorString());	// Error #5004: World is locked.

    int index = 0;
    lua_newtable(L);
    for (b2ParticleGroup* group = ps->GetSystem()->GetParticleGroupList(); group; group = group->GetNext(), index++)
    {
        binder.pushInstance("b2ParticleGroup", group);
        lua_rawseti(L, -2, index + 1);
    }

    return 1;
}

int Box2DBinder2::b2ParticleGroup_destroyParticles(lua_State *L)
{
    //StackChecker checker(L, "b2ParticleGroup_destroyParticles", 0);
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    bool callDestructionListener = lua_toboolean(L, 2);

    group->DestroyParticles(callDestructionListener);

    return 0;
}

int Box2DBinder2::b2ParticleGroup_getParticleCount(lua_State* L)
{
    //StackChecker checker(L, "b2ParticleGroup_getParticleCount", 1);
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    lua_pushinteger(L, group->GetParticleCount());

    return 1;
}

int Box2DBinder2::b2ParticleGroup_containsParticle(lua_State* L)
{
    //StackChecker checker(L, "b2ParticleGroup_containsParticle", 1);
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    int index = lua_tointeger(L, 2);

    lua_pushboolean(L, group->ContainsParticle(index));

    return 1;
}


static void tableToParticleGroupDef(lua_State* L, int index, b2ParticleGroupDef* particleGroupDef, float physicsScale)
{
    // TODO: index'tekinin table oldugunu test et

    Binder binder(L);

    lua_getfield(L, index, "flags");
    if (!lua_isnil(L, -1))
        particleGroupDef->flags = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "groupFlags");
    if (!lua_isnil(L, -1))
        particleGroupDef->groupFlags = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "position");
    if (!lua_isnil(L, -1))
    {
        particleGroupDef->position = tableToVec2(L, -1);
        particleGroupDef->position.x /= physicsScale;
        particleGroupDef->position.y /= physicsScale;
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "angle");
    if (!lua_isnil(L, -1))
        particleGroupDef->angle = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "linearVelocity");
    if (!lua_isnil(L, -1))
        particleGroupDef->linearVelocity = tableToVec2(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "angularVelocity");
    if (!lua_isnil(L, -1))
        particleGroupDef->angularVelocity = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "color");
    if (!lua_isnil(L, -1))
    {
        unsigned int color = luaL_checkinteger(L, -1);
        particleGroupDef->color.r = (color >> 16) & 0xff;
        particleGroupDef->color.g = (color >> 8) & 0xff;
        particleGroupDef->color.b = color & 0xff;
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "alpha");
    if (!lua_isnil(L, -1))
    {
        int alpha = (int)(luaL_checknumber(L, -1) * 255);
        particleGroupDef->color.a = std::min(std::max(alpha, 0), 255);
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "strength");
    if (!lua_isnil(L, -1))
        particleGroupDef->strength = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "shape");
    if (lua_isnil(L, -1))
        luaL_error(L, "shape must exist in particle group definition table");
    particleGroupDef->shape = toShape(binder, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "lifetime");
    if (!lua_isnil(L, -1))
        particleGroupDef->lifetime = luaL_checknumber(L, -1);
    lua_pop(L, 1);
}


int Box2DBinder2::b2ParticleSystem_createParticleGroup(lua_State* L)
{
    //StackChecker checker(L, "b2ParticleSystem_createParticleGroup", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());

    b2ParticleGroupDef particleGroupDef;
    tableToParticleGroupDef(L, 2, &particleGroupDef, application->getPhysicsScale());

    b2ParticleGroup* particleGroup =ps->GetSystem()->CreateParticleGroup(particleGroupDef);
    if (!particleGroup)
    	return luaL_error(L, LFStatus(5004).errorString());	// Error #5004: World is locked.

    binder.pushInstance("b2ParticleGroup", particleGroup);

    return 1;
}




