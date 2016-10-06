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
        {"setParticleSpeed", setParticleSpeed},
        {"setParticleColor", setParticleColor},
        {"setParticlePosition", setParticlePosition},
        {"setParticleSize", setParticleSize},
        {"setParticleAngle", setParticleAngle},
        {"setParticleDecay", setParticleDecay},
        {"setParticleTtl", setParticleTtl},
        {"getParticleDecay", getParticleDecay},
        {"getParticleSpeed", getParticleSpeed},
        {"getParticleColor", getParticleColor},
        {"getParticlePosition", getParticlePosition},
        {"getParticleSize", getParticleSize},
        {"getParticleAngle", getParticleAngle},
        {"getParticleTtl", getParticleTtl},
		{"setParticleTag",setParticleTag},
		{"getParticleTag",getParticleTag},
		{"setPaused",setPaused},
		{"isPaused",isPaused},

		{"getParticles",getParticles},
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
    if (lua_isnoneornil(L,2))
    	mesh->clearParticles();
    else if (lua_type(L, 2) == LUA_TTABLE)
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


int ParticlesBinder::setParticleSpeed(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    float ivc,ivs;
    mesh->getDecay(i,NULL,&ivc,&ivs,NULL);

    float vx=luaL_optnumber(L,3,0);
    float vy=luaL_optnumber(L,4,0);
    float va=luaL_optnumber(L,5,0);
    float decay=luaL_optnumber(L,6,1.0);
    float vs=luaL_optnumber(L,6,0.0);
    float dva=luaL_optnumber(L,6,decay);
    float dvs=luaL_optnumber(L,6,ivs);

    mesh->setSpeed(i, vx,vy,vs,va);
    mesh->setDecay(i,decay,ivc,dvs,dva);

    return 0;
}

int ParticlesBinder::setParticleDecay(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    float ivp,ivc,ivs,iva;
    mesh->getDecay(i,&ivp,&ivc,&ivs,&iva);

    float vp=luaL_optnumber(L,3,ivp);
    float vc=luaL_optnumber(L,4,ivc);
    float vs=luaL_optnumber(L,5,ivs);
    float va=luaL_optnumber(L,6,iva);

    mesh->setDecay(i, vp,vc,vs,va);

    return 0;
}

int ParticlesBinder::setParticlePosition(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    float x=luaL_optnumber(L,3,0);
    float y=luaL_optnumber(L,4,0);

    mesh->setPosition(i, x,y);

    return 0;
}

int ParticlesBinder::setParticleSize(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    float size=luaL_optnumber(L,3,0);

    mesh->setSize(i, size);

    return 0;
}

int ParticlesBinder::setParticleAngle(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    float angle=luaL_optnumber(L,3,0);

    mesh->setAngle(i, angle);

    return 0;
}

int ParticlesBinder::setParticleTtl(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    int ttl=luaL_optinteger(L,3,0);

    mesh->setTtl(i, ttl);

    return 0;
}

int ParticlesBinder::setParticleColor(lua_State *L)
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

    if (lua_type(L, 2) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 2);
        lua_newtable(L);
        for (int k = 0; k < n; ++k)
        {
        	lua_rawgeti(L, 2, k + 1);
        	if (lua_type(L,-1) != LUA_TTABLE)
        		return luaL_error(L,"Particle definition must be a table.");
        	lua_getfield(L,-1,"x");
            float x = luaL_checknumber(L, -1) ;
            lua_pop(L, 1);

        	lua_getfield(L,-1,"y");
            float y = luaL_checknumber(L, -1) ;
            lua_pop(L, 1);

        	lua_getfield(L,-1,"size");
            float size = luaL_checknumber(L, -1) ;
            lua_pop(L, 1);

        	lua_getfield(L,-1,"angle");
            float angle = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);

        	lua_getfield(L,-1,"ttl");
            int ttl = luaL_optinteger(L, -1,0) ;
            lua_pop(L, 1);

            int pnum= mesh->addParticle(x,y,size,angle,ttl);
            lua_pushinteger(L,pnum+1);
            lua_rawseti(L,-3,k+1);

        	lua_getfield(L,-1,"speedX");
            float vx = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
        	lua_getfield(L,-1,"speedY");
            float vy = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
        	lua_getfield(L,-1,"speedAngular");
            float va = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
        	lua_getfield(L,-1,"speedGrowth");
            float vs = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
        	lua_getfield(L,-1,"decay");
            float decay = luaL_optnumber(L, -1,1.0) ;
            lua_pop(L, 1);
        	lua_getfield(L,-1,"decayAngular");
            float decayA = luaL_optnumber(L, -1,decay) ;
            lua_pop(L, 1);
        	lua_getfield(L,-1,"decayGrowth");
            float decayS = luaL_optnumber(L, -1,1.0) ;
            lua_pop(L, 1);

            mesh->setSpeed(pnum,vx,vy,vs,va);

        	lua_getfield(L,-1,"color");
            unsigned int color = luaL_optinteger(L, -1,0xFFFFFF);
            lua_pop(L, 1);
        	lua_getfield(L,-1,"alpha");
            float alpha = luaL_optnumber(L, -1, 1.0);
            lua_pop(L, 1);
        	lua_getfield(L,-1,"decayAlpha");
            float decayC = luaL_optnumber(L, -1,1.0) ;
            lua_pop(L, 1);

            mesh->setColor(pnum, color, alpha);
            mesh->setDecay(pnum, decay,decayC,decayS,decayA);

        	lua_getfield(L,-1,"tag");
            const char *tag=luaL_optstring(L,-1,NULL);
            lua_pop(L, 1);
           	mesh->setTag(pnum,tag);

            lua_pop(L, 1);
        }
    }
    else
    {
        float x = luaL_checknumber(L, 2) ;
        float y = luaL_checknumber(L, 3) ;
        float size = luaL_checknumber(L, 4) ;
        float angle = luaL_optnumber(L, 5,0) ;
        int ttl = luaL_optinteger(L, 6,0) ;
        int pnum= mesh->addParticle(x,y,size,angle,ttl);
        lua_pushinteger(L,pnum+1);
    }

    return 1;
}

int ParticlesBinder::getParticleSpeed(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    float vx,vy,va,vs;
    mesh->getSpeed(i, &vx,&vy,&vs,&va);
    lua_pushnumber(L, vx);
    lua_pushnumber(L, vy);
    lua_pushnumber(L, va);
    lua_pushnumber(L, vs);

    return 4;
}

int ParticlesBinder::getParticleDecay(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    float vp,vc,vs,va;
    mesh->getDecay(i, &vp,&vc,&vs,&va);
    lua_pushnumber(L, vp);
    lua_pushnumber(L, vc);
    lua_pushnumber(L, vs);
    lua_pushnumber(L, va);

    return 4;
}

int ParticlesBinder::getParticlePosition(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    float x,y;
    mesh->getPosition(i, &x,&y);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);

    return 2;
}

int ParticlesBinder::getParticleSize(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    lua_pushnumber(L, mesh->getSize(i));

    return 1;
}

int ParticlesBinder::getParticleAngle(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    lua_pushnumber(L, mesh->getAngle(i));

    return 1;
}

int ParticlesBinder::getParticleTtl(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    lua_pushinteger(L, mesh->getTtl(i));

    return 1;
}



int ParticlesBinder::getParticleColor(lua_State *L)
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

int ParticlesBinder::setParticleTag(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    mesh->setTag(i,luaL_optstring(L,3,NULL));

    return 0;
}

int ParticlesBinder::getParticleTag(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        return luaL_error(L, "The supplied index is out of bounds.");

    const char *tag=mesh->getTag(i);
    if (tag)
    	lua_pushstring(L,tag);
    else
    	lua_pushnil(L);

    return 1;
}

int ParticlesBinder::setPaused(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    mesh->setPaused(lua_toboolean(L,2));
    return 0;
}

int ParticlesBinder::isPaused(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    lua_pushboolean(L,mesh->isPaused());
    return 1;
}

static void buildParticleTable(lua_State *L,Particles *mesh,int i)
{
	lua_newtable(L);
	float x,y;
	mesh->getPosition(i,&x,&y);
	float size=mesh->getSize(i);
	float angle=mesh->getAngle(i);
	int ttl=mesh->getTtl(i);
	float vx,vy,va,vs;
	mesh->getSpeed(i,&vx,&vy,&vs,&va);
	float dp,dc,da,ds;
	mesh->getDecay(i,&dp,&dc,&ds,&da);
	unsigned int col;
	float alpha;
	mesh->getColor(i,&col,&alpha);
	const char *tag=mesh->getTag(i);

	lua_pushnumber(L,x);
	lua_setfield(L,-2,"x");
	lua_pushnumber(L,y);
	lua_setfield(L,-2,"y");
	lua_pushnumber(L,size);
	lua_setfield(L,-2,"size");
	lua_pushnumber(L,angle);
	lua_setfield(L,-2,"angle");
	lua_pushinteger(L,ttl);
	lua_setfield(L,-2,"ttl");

	lua_pushnumber(L,vx);
	lua_setfield(L,-2,"speedX");
	lua_pushnumber(L,vy);
	lua_setfield(L,-2,"speedY");
	lua_pushnumber(L,va);
	lua_setfield(L,-2,"speedAngular");
	lua_pushnumber(L,vs);
	lua_setfield(L,-2,"speedGrowth");

	lua_pushnumber(L,dp);
	lua_setfield(L,-2,"decay");
	lua_pushnumber(L,dc);
	lua_setfield(L,-2,"decayAlpha");
	lua_pushnumber(L,da);
	lua_setfield(L,-2,"decayAngular");
	lua_pushnumber(L,ds);
	lua_setfield(L,-2,"decayGrowth");

	lua_pushinteger(L,col);
	lua_setfield(L,-2,"color");
	lua_pushnumber(L,alpha);
	lua_setfield(L,-2,"alpha");
	if (tag)
	{
		lua_pushstring(L,tag);
		lua_setfield(L,-2,"tag");
	}
}

int ParticlesBinder::getParticles(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    const char *filter=luaL_optstring(L,3,NULL);
    int psize=mesh->getParticleCount();
    bool hasTable=lua_istable(L, 2); //Check before pushing another arg
	lua_newtable(L);
    if (hasTable)
    {
        int n = lua_objlen(L, 2);
        for (int k = 0; k < n; ++k)
        {
        	lua_rawgeti(L, 2, k + 1);
        	int i=luaL_checkinteger(L,-1)-1;
        	lua_pop(L,1);
            if (i < 0 || i >= psize)
            	continue;
            if (mesh->getSize(i)==0) //Dead particle
            	continue;
            if ((!filter)||(!strcmp(mesh->getTag(i),filter)))
            {
            	buildParticleTable(L,mesh,i);
            	lua_rawseti(L,-2,i+1);
            }
        }
    }
    else
    {
    	for (int i=0;i<psize;i++)
    	{
            if (mesh->getSize(i)==0) //Dead particle
            	continue;
            if ((!filter)||(!strcmp(mesh->getTag(i),filter)))
            {
            	buildParticleTable(L,mesh,i);
            	lua_rawseti(L,-2,i+1);
            }
    	}
    }

    return 1;
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

