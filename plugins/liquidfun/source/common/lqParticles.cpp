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
#undef min
#undef max

//COMPLETE 1.1.0
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
        luaL_error(L, "%s", LFStatus(5004).errorString());	// Error #5004: World is locked.

    b2ParticleSystemDef particleSystemDef;
    tableToParticleSystemDef(L, 2, &particleSystemDef, application->getPhysicsScale());

    b2ParticleSystem* particleSystem = world->CreateParticleSystem(&particleSystemDef);
    b2ParticleSystemSprite *ps=new b2ParticleSystemSprite(application,particleSystem,world);

    binder.pushInstance("b2ParticleSystem", ps->proxy_);
    lua_pushlightuserdata(L, particleSystem);
    lua_pushvalue(L, -2);
    setb2(L);


    return 1;
}

//COMPLETE 1.1.0
static void tableToParticleDef(lua_State* L, int index, b2ParticleDef* particleDef, float physicsScale)
{
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
    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());


    b2ParticleDef particleDef;
    tableToParticleDef(L, 2, &particleDef, application->getPhysicsScale());

    if (ps->GetWorld()->IsLocked())
    	luaL_error(L, "%s", LFStatus(5004).errorString());	// Error #5004: World is locked.

    int32 p=ps->GetSystem()->CreateParticle(particleDef);
    lua_pushinteger(L, p);

    return 1;
}

int Box2DBinder2::b2ParticleSystem_destroyParticle(lua_State* L)
{
    //StackChecker checker(L, "b2ParticleSystem_destroyParticle", 0);

    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    bool callDestructionListener = lua_toboolean(L, 3);

    ps->GetSystem()->DestroyParticle((int)luaL_checkinteger(L, 2),callDestructionListener);

    return 0;
}

int Box2DBinder2::b2ParticleSystem_setTexture(lua_State *L)
{
    //StackChecker checker(L, "b2ParticleSystem_setTexture", 0);
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    TextureBase *textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 2));
    ps->SetTexture(textureBase,luaL_optnumber(L,3,0.0));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getParticleGroupList(lua_State *L)
{
    //StackChecker checker(L, "b2ParticleSystem_getParticleGroupList", 1);
    Binder binder(L);


    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    if (ps == NULL)
    {
        luaL_error(L, "ParticleSystem required in argument #1");
    }

    if (ps->GetWorld()->IsLocked())
        luaL_error(L, "%s", LFStatus(5004).errorString());	// Error #5004: World is locked.

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

int Box2DBinder2::b2ParticleGroup_applyLinearImpulse(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    group->ApplyLinearImpulse(b2Vec2(luaL_checknumber(L,2),luaL_checknumber(L,3)));

    return 0;
}
int Box2DBinder2::b2ParticleGroup_applyForce(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    group->ApplyForce(b2Vec2(luaL_checknumber(L,2),luaL_checknumber(L,3)));

    return 0;
}
int Box2DBinder2::b2ParticleGroup_getAllParticleFlags(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    lua_pushinteger(L,group->GetAllParticleFlags());

    return 1;
}
int Box2DBinder2::b2ParticleGroup_getGroupFlags(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    lua_pushinteger(L,group->GetGroupFlags());

    return 1;
}
int Box2DBinder2::b2ParticleGroup_setGroupFlags(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    group->SetGroupFlags(luaL_checkinteger(L,2));

    return 0;
}
int Box2DBinder2::b2ParticleGroup_getMass(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    lua_pushnumber(L,group->GetMass());

    return 1;
}
int Box2DBinder2::b2ParticleGroup_getInertia(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    lua_pushnumber(L,group->GetInertia());

    return 1;
}
int Box2DBinder2::b2ParticleGroup_getAngularVelocity(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    lua_pushnumber(L,group->GetAngularVelocity());

    return 1;
}
int Box2DBinder2::b2ParticleGroup_getAngle(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    lua_pushnumber(L,group->GetAngle());

    return 1;
}
int Box2DBinder2::b2ParticleGroup_getCenter(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    b2Vec2 p=group->GetCenter();
    lua_pushnumber(L,p.x);
    lua_pushnumber(L,p.y);

    return 2;
}
int Box2DBinder2::b2ParticleGroup_getPosition(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    b2Vec2 p=group->GetPosition();
    lua_pushnumber(L,p.x);
    lua_pushnumber(L,p.y);

    return 2;
}
int Box2DBinder2::b2ParticleGroup_getLinearVelocity(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    b2Vec2 p=group->GetLinearVelocity();
    lua_pushnumber(L,p.x);
    lua_pushnumber(L,p.y);

    return 2;
}
int Box2DBinder2::b2ParticleGroup_getLinearVelocityFromWorldPoint(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    b2Vec2 p=group->GetLinearVelocityFromWorldPoint(b2Vec2(luaL_checknumber(L,2),luaL_checknumber(L,3)));
    lua_pushnumber(L,p.x);
    lua_pushnumber(L,p.y);

    return 2;
}
int Box2DBinder2::b2ParticleGroup_getTransform(lua_State* L)
{
    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);
    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    const b2Transform &transform = group->GetTransform();

    lua_pushnumber(L, transform.p.x * physicsScale);
    lua_pushnumber(L, transform.p.y * physicsScale);
    lua_pushnumber(L, transform.q.GetAngle());

    return 3;
}
int Box2DBinder2::b2ParticleGroup_getParticleSystem(lua_State *L)
{
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    getb2(L, group->GetParticleSystem());

    return 1;
}
/*
No bindings:
int32 	GetBufferIndex () const Get the offset of this group in the global particle buffer.
void * 	GetUserData () const 	Get the user data pointer that was provided in the group definition.
 void 	SetUserData (void *data) 	Set the user data. Use this to store your application specific data.
*/

//COMPLETE 1.1.0
static void tableToParticleGroupDef(lua_State* L, int index, b2ParticleGroupDef* particleGroupDef, float physicsScale)
{
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
    if (!lua_isnil(L, -1))
    	particleGroupDef->shape = toShape(binder, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "shapes");
    if (!lua_isnil(L, -1))
    {
    	if (!lua_istable(L,-1))
           luaL_error(L, "shapes must be table");
    	int sc=lua_objlen(L,-1);
    	b2Shape **sh=new b2Shape *[sc];
    	particleGroupDef->shapes=sh;
    	particleGroupDef->shapeCount=sc;
    	for (int i=1;i<=sc;i++)
    	{
    		lua_rawgeti(L,-1,i);
    		sh[i-1]=toShape(binder,-1);
    		lua_pop(L,1);
    	}
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "stride");
    if (!lua_isnil(L, -1))
        particleGroupDef->stride = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "particles");
    if (!lua_isnil(L, -1))
    {
    	if (!lua_istable(L,-1))
           luaL_error(L, "particles must be table");
    	int sc=lua_objlen(L,-1);
    	b2Vec2 *sh=new b2Vec2 [sc];
    	particleGroupDef->positionData=sh;
    	particleGroupDef->particleCount=sc;
    	for (int i=1;i<=sc;i++)
    	{
    		lua_rawgeti(L,-1,i);
    		sh[i-1]=tableToVec2(L,-1);
            sh[i-1].x /= physicsScale;
            sh[i-1].y /= physicsScale;
    		lua_pop(L,1);
    	}
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "lifetime");
    if (!lua_isnil(L, -1))
        particleGroupDef->lifetime = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "group");
    if (!lua_isnil(L, -1))
    	particleGroupDef->group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", -1));
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
    	luaL_error(L, "%s", LFStatus(5004).errorString());	// Error #5004: World is locked.

    binder.pushInstance("b2ParticleGroup", particleGroup);
	lua_pushlightuserdata(L, particleGroup);
	lua_pushvalue(L, -2);
	setb2(L);

    return 1;
}

int Box2DBinder2::b2ParticleSystem_getPaused(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushboolean(L,ps->GetSystem()->GetPaused());

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setPaused(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetPaused(lua_toboolean(L,2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getDestructionByAge(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushboolean(L,ps->GetSystem()->GetDestructionByAge());

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setDestructionByAge(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetDestructionByAge(lua_toboolean(L,2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getStrictContactCheck(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushboolean(L,ps->GetSystem()->GetStrictContactCheck());

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setStrictContactCheck(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetStrictContactCheck(lua_toboolean(L,2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getDensity(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushnumber(L,ps->GetSystem()->GetDensity());

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setDensity(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetDensity(luaL_checknumber(L,2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getGravityScale(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushnumber(L,ps->GetSystem()->GetGravityScale());

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setGravityScale(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetGravityScale(luaL_checknumber(L,2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getDamping(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushnumber(L,ps->GetSystem()->GetDamping());

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setDamping(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetDamping(luaL_checknumber(L,2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getStaticPressureIterations(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetStaticPressureIterations());

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setStaticPressureIterations(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetStaticPressureIterations(luaL_checkinteger(L,2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getParticleLifetime(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushnumber(L,ps->GetSystem()->GetParticleLifetime(luaL_checkinteger(L,2)));

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setParticleLifetime(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetParticleLifetime(luaL_checkinteger(L,2),luaL_checknumber(L,3));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getParticleFlags(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetParticleFlags(luaL_checkinteger(L,2)));

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setParticleFlags(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetParticleFlags(luaL_checkinteger(L,2),luaL_checkinteger(L,3));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getMaxParticleCount(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetMaxParticleCount());

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setMaxParticleCount(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetMaxParticleCount(luaL_checkinteger(L,2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getRadius(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushnumber(L,ps->GetSystem()->GetRadius());

    return 1;
}

int Box2DBinder2::b2ParticleSystem_setRadius(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetRadius(luaL_checknumber(L,2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getContacts(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    int cn=ps->GetSystem()->GetContactCount();
    const b2ParticleContact *ca=ps->GetSystem()->GetContacts();
    lua_createtable(L,cn,0);
    for (int i=0;i<cn;i++)
    {
    	lua_createtable(L,0,6);
    	lua_pushinteger(L,ca[i].GetIndexA()); lua_setfield(L,-2,"indexA");
    	lua_pushinteger(L,ca[i].GetIndexB()); lua_setfield(L,-2,"indexB");
    	lua_pushnumber(L,ca[i].GetWeight()); lua_setfield(L,-2,"weight");
    	lua_pushinteger(L,ca[i].GetFlags()); lua_setfield(L,-2,"flags");
    	lua_pushnumber(L,ca[i].GetNormal().x); lua_setfield(L,-2,"normalX");
    	lua_pushnumber(L,ca[i].GetNormal().y); lua_setfield(L,-2,"normalY");
    	lua_rawseti(L,-2,i+1);
    }
    return 1;
}
int Box2DBinder2::b2ParticleSystem_getContactCount(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetContactCount());
    return 1;
}

int Box2DBinder2::b2ParticleSystem_getBodyContacts(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    int cn=ps->GetSystem()->GetBodyContactCount();
    const b2ParticleBodyContact *ca=ps->GetSystem()->GetBodyContacts();
    lua_createtable(L,cn,0);
    for (int i=0;i<cn;i++)
    {
    	lua_createtable(L,0,7);
    	lua_pushinteger(L,ca[i].index); lua_setfield(L,-2,"index");
        getb2(L,ca[i].body); lua_setfield(L,-2,"body");
        getb2(L,ca[i].fixture); lua_setfield(L,-2,"fixture");
    	lua_pushnumber(L,ca[i].weight); lua_setfield(L,-2,"weight");
    	lua_pushnumber(L,ca[i].mass); lua_setfield(L,-2,"mass");
    	lua_pushnumber(L,ca[i].normal.x); lua_setfield(L,-2,"normalX");
    	lua_pushnumber(L,ca[i].normal.y); lua_setfield(L,-2,"normalY");
    	lua_rawseti(L,-2,i+1);
    }
    return 1;
}
int Box2DBinder2::b2ParticleSystem_getBodyContactCount(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetBodyContactCount());
    return 1;
}

int Box2DBinder2::b2ParticleSystem_getPairs(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    int cn=ps->GetSystem()->GetPairCount();
    const b2ParticlePair *ca=ps->GetSystem()->GetPairs();
    lua_createtable(L,cn,0);
    for (int i=0;i<cn;i++)
    {
    	lua_createtable(L,0,4);
    	lua_pushinteger(L,ca[i].indexA); lua_setfield(L,-2,"indexA");
    	lua_pushinteger(L,ca[i].indexB); lua_setfield(L,-2,"indexB");
    	lua_pushnumber(L,ca[i].strength); lua_setfield(L,-2,"strength");
    	lua_pushnumber(L,ca[i].distance); lua_setfield(L,-2,"distance");
    	lua_pushinteger(L,ca[i].flags); lua_setfield(L,-2,"flags");
    	lua_rawseti(L,-2,i+1);
    }
    return 1;
}
int Box2DBinder2::b2ParticleSystem_getPairCount(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetPairCount());
    return 1;
}

int Box2DBinder2::b2ParticleSystem_getTriads(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    int cn=ps->GetSystem()->GetTriadCount();
    const b2ParticleTriad *ca=ps->GetSystem()->GetTriads();
    lua_createtable(L,cn,0);
    for (int i=0;i<cn;i++)
    {
    	lua_createtable(L,0,4);
    	lua_pushinteger(L,ca[i].indexA); lua_setfield(L,-2,"indexA");
    	lua_pushinteger(L,ca[i].indexB); lua_setfield(L,-2,"indexB");
    	lua_pushinteger(L,ca[i].indexC); lua_setfield(L,-2,"indexC");
    	lua_pushnumber(L,ca[i].strength); lua_setfield(L,-2,"strength");
    	lua_pushinteger(L,ca[i].flags); lua_setfield(L,-2,"flags");
    	lua_rawseti(L,-2,i+1);
    }
    return 1;
}
int Box2DBinder2::b2ParticleSystem_getTriadCount(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetTriadCount());
    return 1;
}

int Box2DBinder2::b2ParticleSystem_setStuckThreshold(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->SetStuckThreshold(luaL_checkinteger(L,2));
    return 0;
}
int Box2DBinder2::b2ParticleSystem_getStuckCandidates(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    int cn=ps->GetSystem()->GetStuckCandidateCount();
    const int32 *ca=ps->GetSystem()->GetStuckCandidates();
    lua_createtable(L,cn,0);
    for (int i=0;i<cn;i++)
    {
    	lua_pushinteger(L,ca[i]);
    	lua_rawseti(L,-2,i+1);
    }
    return 1;
}
int Box2DBinder2::b2ParticleSystem_getStuckCandidateCount(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetStuckCandidateCount());
    return 1;
}

int Box2DBinder2::b2ParticleSystem_computeCollisionEnergy(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushnumber(L,ps->GetSystem()->ComputeCollisionEnergy());
    return 1;
}

int Box2DBinder2::b2ParticleSystem_particleApplyLinearImpulse(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->ParticleApplyLinearImpulse(luaL_checkinteger(L,2),
    		b2Vec2(luaL_checknumber(L,3),luaL_checknumber(L,4)));

    return 0;
}
int Box2DBinder2::b2ParticleSystem_applyLinearImpulse(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->ApplyLinearImpulse(luaL_checkinteger(L,2),luaL_checkinteger(L,3),
    		b2Vec2(luaL_checknumber(L,4),luaL_checknumber(L,5)));

    return 0;
}
int Box2DBinder2::b2ParticleSystem_particleApplyForce(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->ParticleApplyForce(luaL_checkinteger(L,2),
    		b2Vec2(luaL_checknumber(L,3),luaL_checknumber(L,4)));

    return 0;
}
int Box2DBinder2::b2ParticleSystem_applyForce(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    ps->GetSystem()->ApplyForce(luaL_checkinteger(L,2),luaL_checkinteger(L,3),
    		b2Vec2(luaL_checknumber(L,4),luaL_checknumber(L,5)));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_destroyOldestParticle(lua_State* L)
{
    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    bool callDestructionListener = lua_toboolean(L, 3);

    ps->GetSystem()->DestroyOldestParticle((int)luaL_checkinteger(L, 2),callDestructionListener);

    return 0;
}

int Box2DBinder2::b2ParticleSystem_destroyParticlesInShape(lua_State* L)
{
    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    b2Shape *shape=toShape(binder,2);
    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    b2Transform xf=toTransform(L,3,application);
    bool callDestructionListener = lua_toboolean(L, 4);

    lua_pushinteger(L,ps->GetSystem()->DestroyParticlesInShape(*shape,xf,callDestructionListener));

    return 1;
}
static b2ParticleGroup* toParticleGroup(const Binder& binder, int index)
{
	b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", index));

	if (group == 0)
	{
		LFStatus status(5005);	// Group is already destroyed.
		luaL_error(binder.L, "%s", status.errorString());
	}

	return group;
}
int Box2DBinder2::b2ParticleSystem_joinParticleGroups(lua_State* L)
{
    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());

    ps->GetSystem()->JoinParticleGroups(toParticleGroup(binder,2),toParticleGroup(binder,3));

    return 0;
}
int Box2DBinder2::b2ParticleSystem_splitParticleGroup(lua_State* L)
{
    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());

    ps->GetSystem()->SplitParticleGroup(toParticleGroup(binder,2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_computeAABB(lua_State* L)
{
	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());

    b2AABB aabb;
    ps->GetSystem()->ComputeAABB(&aabb);

    lua_pushnumber(L,aabb.lowerBound.x*physicsScale);
    lua_pushnumber(L,aabb.lowerBound.y*physicsScale);
    lua_pushnumber(L,aabb.upperBound.x*physicsScale);
    lua_pushnumber(L,aabb.upperBound.y*physicsScale);
	return 4;
}

int Box2DBinder2::b2ParticleSystem_queryShapeAABB(lua_State* L)
{
	Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());

    b2Shape *shape=toShape(binder,2);
    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    b2Transform xf=toTransform(L,3,application);

    MyQueryCallback callback;
	ps->GetSystem()->QueryShapeAABB(&callback, *shape, xf);

	return callback.Result(L);
}

int Box2DBinder2::b2ParticleSystem_queryAABB(lua_State* L)
{
	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());

	lua_Number lx = luaL_checknumber(L, 2) / physicsScale;
	lua_Number ly = luaL_checknumber(L, 3) / physicsScale;
	lua_Number ux = luaL_checknumber(L, 4) / physicsScale;
	lua_Number uy = luaL_checknumber(L, 5) / physicsScale;

	b2AABB aabb;
	aabb.lowerBound.Set(lx, ly);
	aabb.upperBound.Set(ux, uy);

	MyQueryCallback callback;
	ps->GetSystem()->QueryAABB(&callback, aabb);

	return callback.Result(L);
}

int Box2DBinder2::b2ParticleSystem_rayCast(lua_State* L)
{
    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());

    lua_Number x1 = luaL_checknumber(L, 2) / physicsScale;
    lua_Number y1 = luaL_checknumber(L, 3) / physicsScale;
    lua_Number x2 = luaL_checknumber(L, 4) / physicsScale;
    lua_Number y2 = luaL_checknumber(L, 5) / physicsScale;
    luaL_checktype(L, 6, LUA_TFUNCTION);

    MyRayCastCallback callback(L);
    ps->GetSystem()->RayCast(&callback, b2Vec2(x1, y1), b2Vec2(x2, y2));

    return 0;
}


int Box2DBinder2::b2ParticleSystem_getParticleGroupCount(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetParticleGroupCount());

    return 1;
}
int Box2DBinder2::b2ParticleSystem_getAllParticleFlags(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetAllParticleFlags());

    return 1;
}
int Box2DBinder2::b2ParticleSystem_getAllGroupFlags(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetAllGroupFlags());

    return 1;
}
int Box2DBinder2::b2ParticleSystem_expirationTimeToLifetime(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushnumber(L,ps->GetSystem()->ExpirationTimeToLifetime(luaL_checkinteger(L,2)));

    return 1;
}

int Box2DBinder2::b2ParticleSystem_getParticleCount(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    lua_pushinteger(L,ps->GetSystem()->GetParticleCount());

    return 1;
}

int Box2DBinder2::b2ParticleSystem_getPositionBuffer(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    int n=ps->GetSystem()->GetParticleCount();
    b2Vec2 *p=ps->GetSystem()->GetPositionBuffer();
    lua_createtable(L,n,0);
    for (int k=1;k<=n;k++) {
    	lua_pushvector(L,p->x,p->y,0,0);
    	lua_rawseti(L,-2,k);
    	p++;
    }
    return 1;
}

int Box2DBinder2::b2ParticleSystem_getColorBuffer(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    int n=ps->GetSystem()->GetParticleCount();
    b2ParticleColor *p=ps->GetSystem()->GetColorBuffer();
    lua_createtable(L,n,0);
    for (int k=1;k<=n;k++) {
    	lua_pushvector(L,p->r/255.0,p->g/255.0,p->b/255.0,p->a/255.0);
    	lua_rawseti(L,-2,k);
    	p++;
    }
    return 1;
}

int Box2DBinder2::b2ParticleSystem_getVelocityBuffer(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    int n=ps->GetSystem()->GetParticleCount();
    b2Vec2 *p=ps->GetSystem()->GetVelocityBuffer();
    lua_createtable(L,n,0);
    for (int k=1;k<=n;k++) {
    	lua_pushvector(L,p->x,p->y,0,0);
    	lua_rawseti(L,-2,k);
    	p++;
    }
    return 1;
}

int Box2DBinder2::b2ParticleSystem_getWeightBuffer(lua_State *L)
{
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(static_cast<SpriteProxy *>(binder.getInstance("b2ParticleSystem", 1))->getContext());
    int n=ps->GetSystem()->GetParticleCount();
    float32 *p=ps->GetSystem()->GetWeightBuffer();
    lua_createtable(L,n,0);
    for (int k=1;k<=n;k++) {
    	lua_pushnumber(L,*p);
    	lua_rawseti(L,-2,k);
    	p++;
    }
    return 1;
}


/*
 * No bindings for:
void 	SetFlagsBuffer (uint32 *buffer, int32 capacity)
void 	SetPositionBuffer (b2Vec2 *buffer, int32 capacity)
void 	SetVelocityBuffer (b2Vec2 *buffer, int32 capacity)
void 	SetColorBuffer (b2ParticleColor *buffer, int32 capacity)
b2ParticleGroup *const * 	GetGroupBuffer ()
const uint32 * 	GetFlagsBuffer () const
const int32 * 	GetExpirationTimeBuffer ()
const int32 * 	GetIndexByExpirationTimeBuffer ()
const b2ParticleHandle * 	GetParticleHandleFromIndex (const int32 index) Retrieve a handle to the particle at the specified index. More...
b2ParticleSystem * 	GetNext () Get the next particle-system in the world's particle-system list.
const b2ParticleSystem * 	GetNext () const
*/
