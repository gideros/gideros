#include "particlesbinder.h"
#include <particles.h>
#include "luaapplication.h"
#include <luautil.h>
#include <cfloat>

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
        {"setParticleAcceleration", setParticleAcceleration},
        {"setParticleTtl", setParticleTtl},
        {"getParticleDecay", getParticleDecay},
        {"getParticleSpeed", getParticleSpeed},
        {"getParticleSpeed", getParticleAcceleration},
        {"getParticleColor", getParticleColor},
        {"getParticlePosition", getParticlePosition},
        {"getParticleSize", getParticleSize},
        {"getParticleAngle", getParticleAngle},
        {"getParticleTtl", getParticleTtl},
        {"setParticleTag",setParticleTag},
        {"getParticleTag",getParticleTag},
        {"setParticleExtra",setParticleExtra},
        {"getParticleExtra",getParticleExtra},
        {"setPaused",setPaused},
		{"isPaused",isPaused},
		{"scaleParticles",scaleParticles},
        {"getParticles",getParticles},
        {"getDeadParticles",getDeadParticles},
        {"getNearestParticle",getNearestParticle},
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

    binder.pushInstance("Particles", new Particles(application->getApplication(),lua_toboolean(L,1),lua_toboolean(L,2)));

    return 1;
}

int ParticlesBinder::destruct(void *p)
{
    void* ptr = GIDEROS_DTOR_UDATA(p);
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
        luaL_error(L, "The supplied index is out of bounds.");

    float vx=luaL_optnumber(L,3,0);
    float vy=luaL_optnumber(L,4,0);
    int ra=5;
    float vz=0;
    if (mesh->is3d) {
        vz=luaL_optnumber(L,5,0);
        ra++;
    }
    float va=luaL_optnumber(L,ra,0);
    float vs=luaL_optnumber(L,ra+1,0.0);

    mesh->setSpeed(i, vx,vy,vz,vs,va);

    return 0;
}

int ParticlesBinder::setParticleDecay(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    float ivx,ivy,ivz,ivc,ivs,iva;
    mesh->getDecay(i,&ivx,&ivy,&ivz,&ivc,&ivs,&iva);

    const float *vvec=lua_tovector(L,3);
    if (vvec) {
        ivx=vvec[0];
        ivy=vvec[1];
        ivz=vvec[2];
    }
    else if (lua_isnumber(L,3)) {
        ivx=luaL_optnumber(L,3,ivx);
        ivy=ivx;
        ivz=ivx;
    }

    float vc=luaL_optnumber(L,4,ivc);
    float vs=luaL_optnumber(L,5,ivs);
    float va=luaL_optnumber(L,6,iva);

    mesh->setDecay(i, ivx,ivy,ivz,vc,vs,va);

    return 0;
}

int ParticlesBinder::setParticleAcceleration(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    float ivx,ivy,ivz,ivc,ivs,iva;
    mesh->getAcceleration(i,&ivx,&ivy,&ivz,&ivc,&ivs,&iva);

    const float *vvec=lua_tovector(L,3);
    if (vvec) {
        ivx=vvec[0];
        ivy=vvec[1];
        ivz=vvec[2];
    }
    else if (lua_isnumber(L,3)) {
        ivx=luaL_optnumber(L,3,ivx);
        ivy=ivx;
        ivz=ivx;
    }

    float vc=luaL_optnumber(L,4,ivc);
    float vs=luaL_optnumber(L,5,ivs);
    float va=luaL_optnumber(L,6,iva);

    mesh->setAcceleration(i, ivx,ivy,ivz,vc,vs,va);

    return 0;
}

int ParticlesBinder::setParticlePosition(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    float x=luaL_optnumber(L,3,0);
    float y=luaL_optnumber(L,4,0);
    float z=luaL_optnumber(L,5,0);

    mesh->setPosition(i, x,y,z);

    return 0;
}

int ParticlesBinder::setParticleSize(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    float size=luaL_optnumber(L,3,0);

    mesh->setSize(i, size);

    return 0;
}

int ParticlesBinder::scaleParticles(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    float size=luaL_checknumber(L,2);
    bool absolute=lua_toboolean(L,3);

    mesh->scaleParticles(size,absolute);

    return 0;
}

int ParticlesBinder::setParticleAngle(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

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
        luaL_error(L, "The supplied index is out of bounds.");

    int ttl=luaL_optinteger(L,3,0);

    mesh->setTtl(i, ttl);

    return 0;
}

int ParticlesBinder::setParticleExtra(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    float extra=luaL_optnumber(L,3,0);

    mesh->setExtra(i, extra);

    return 0;
}

int ParticlesBinder::setParticleColor(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    const float *cvec=lua_tovector(L,3);
    if (cvec) {
        unsigned int col = ((((int)(cvec[0]*255))&0xFF)<<16)|((((int)(cvec[1]*255))&0xFF)<<8)|((((int)(cvec[2]*255))&0xFF)<<0);
        mesh->setColor(i, col, cvec[3]);
    }
    else {
        unsigned int color = luaL_checkinteger(L, 3);
        float alpha = luaL_optnumber(L, 4, 1.0);

        mesh->setColor(i, color, alpha);
    }

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
                luaL_error(L,"Particle definition must be a table.");
        	lua_getfield(L,-1,"x");
            float x = luaL_checknumber(L, -1) ;
            lua_pop(L, 1);

        	lua_getfield(L,-1,"y");
            float y = luaL_checknumber(L, -1) ;
            lua_pop(L, 1);

            lua_getfield(L,-1,"z");
            float z = luaL_optnumber(L, -1, 0) ;
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

            lua_getfield(L,-1,"extra");
            float extra = luaL_optnumber(L, -1, 0) ;
            lua_pop(L, 1);

            int pnum= mesh->addParticle(x,y,z,size,angle,ttl,extra);
            lua_pushinteger(L,pnum+1);
            lua_rawseti(L,-3,k+1);

        	lua_getfield(L,-1,"speedX");
            float vx = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
            lua_getfield(L,-1,"speedY");
            float vy = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
            lua_getfield(L,-1,"speedZ");
            float vz = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
            lua_getfield(L,-1,"speedAngular");
            float va = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
        	lua_getfield(L,-1,"speedGrowth");
            float vs = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
        	lua_getfield(L,-1,"decay");
            const float *vvec=lua_tovector(L,-1);
            float decayX=1;
            float decayY=1;
            float decayZ=1;
            if (vvec) {
                decayX=vvec[0];
                decayY=vvec[1];
                decayZ=vvec[2];
            }
            else if (lua_isnumber(L,-1)) {
                decayX=luaL_optnumber(L,-1,decayX);
                decayY=decayX;
                decayZ=decayX;
            }
            lua_pop(L, 1);
        	lua_getfield(L,-1,"decayAngular");
            float decayA = luaL_optnumber(L, -1,decayX) ;
            lua_pop(L, 1);
        	lua_getfield(L,-1,"decayGrowth");
            float decayS = luaL_optnumber(L, -1,1.0) ;
            lua_pop(L, 1);

            lua_getfield(L,-1,"acceleration");
            vvec=lua_tovector(L,-1);
            float accX=0;
            float accY=0;
            float accZ=0;
            if (vvec) {
                accX=vvec[0];
                accY=vvec[1];
                accZ=vvec[2];
            }
            lua_pop(L, 1);
            lua_getfield(L,-1,"accelerationAngular");
            float accA = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
            lua_getfield(L,-1,"accelerationGrowth");
            float accS = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);
            lua_getfield(L,-1,"accelerationAlpha");
            float accC = luaL_optnumber(L, -1,0) ;
            lua_pop(L, 1);

            mesh->setSpeed(pnum,vx,vy,vz,vs,va);

        	lua_getfield(L,-1,"color");
            const float *cvec=lua_tovector(L,-1);
            unsigned int color;
            float alpha;
            if (cvec) {
                color = ((((int)(cvec[0]*255))&0xFF)<<16)|((((int)(cvec[1]*255))&0xFF)<<8)|((((int)(cvec[2]*255))&0xFF)<<0);
                alpha=cvec[3];
            }
            else {
                color = luaL_optinteger(L, -1,0xFFFFFF);
                lua_pop(L, 1);
            	lua_getfield(L,-1,"alpha");
                alpha = luaL_optnumber(L, -1, 1.0);
            }
            lua_pop(L, 1);
        	lua_getfield(L,-1,"decayAlpha");
            float decayC = luaL_optnumber(L, -1,1.0) ;
            lua_pop(L, 1);

            mesh->setColor(pnum, color, alpha);
            mesh->setDecay(pnum, decayX,decayY,decayZ,decayC,decayS,decayA);
            mesh->setAcceleration(pnum, accX,accY,accZ,accC,accS,accA);

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
        float z=0;
        int idx=4;
        if (mesh->is3d) {
            z = luaL_checknumber(L, 4) ;
            idx++;
        }
        float size = luaL_checknumber(L, idx) ;
        float angle = luaL_optnumber(L, idx+1,0) ;
        int ttl = luaL_optinteger(L, idx+2,0) ;
        float extra = luaL_optnumber(L, idx+3,0) ;
        int pnum= mesh->addParticle(x,y,z,size,angle,ttl,extra);
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
        luaL_error(L, "The supplied index is out of bounds.");

    float vx,vy,vz,va,vs;
    mesh->getSpeed(i, &vx,&vy,&vz,&vs,&va);
    lua_pushnumber(L, vx);
    lua_pushnumber(L, vy);
    if (mesh->is3d)
        lua_pushnumber(L, vz);
    lua_pushnumber(L, va);
    lua_pushnumber(L, vs);

    return mesh->is3d?5:4;
}

int ParticlesBinder::getParticleDecay(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    float vx,vy,vz,vc,vs,va;
    mesh->getDecay(i, &vx,&vy,&vz,&vc,&vs,&va);
    lua_pushnumber(L, vx);
    lua_pushnumber(L, vc);
    lua_pushnumber(L, vs);
    lua_pushnumber(L, va);
    lua_pushvector(L, vx,vy,vz,0);

    return 5;
}

int ParticlesBinder::getParticleAcceleration(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    float vx,vy,vz,vc,vs,va;
    mesh->getAcceleration(i, &vx,&vy,&vz,&vc,&vs,&va);
    lua_pushvector(L, vx,vy,vz,0);
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
        luaL_error(L, "The supplied index is out of bounds.");

    float x,y,z;
    mesh->getPosition(i, &x,&y,&z);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);

    return 3;
}

int ParticlesBinder::getParticleSize(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    lua_pushnumber(L, mesh->getSize(i));

    return 1;
}

int ParticlesBinder::getParticleAngle(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    lua_pushnumber(L, mesh->getAngle(i));

    return 1;
}

int ParticlesBinder::getParticleTtl(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    lua_pushinteger(L, mesh->getTtl(i));

    return 1;
}

int ParticlesBinder::getParticleExtra(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

    lua_pushnumber(L, mesh->getExtra(i));

    return 1;
}

int ParticlesBinder::getParticleColor(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

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
        luaL_error(L, "The supplied index is out of bounds.");

    mesh->setTag(i,luaL_optstring(L,3,NULL));

    return 0;
}

int ParticlesBinder::getParticleTag(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));

    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getParticleCount())
        luaL_error(L, "The supplied index is out of bounds.");

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
    float x,y,z;
    mesh->getPosition(i,&x,&y,&z);
	float size=mesh->getSize(i);
	float angle=mesh->getAngle(i);
	int ttl=mesh->getTtl(i);
    float vx,vy,vz,va,vs,extra;
    mesh->getSpeed(i,&vx,&vy,&vz,&vs,&va);
    extra=mesh->getExtra(i);
    float dx,dy,dz,dc,da,ds;
    mesh->getDecay(i,&dx,&dy,&dz,&dc,&ds,&da);
    float ax,ay,az,ac,aa,as;
    mesh->getAcceleration(i,&ax,&ay,&az,&ac,&as,&aa);
    unsigned int col;
	float alpha;
	mesh->getColor(i,&col,&alpha);
	const char *tag=mesh->getTag(i);

	lua_pushnumber(L,x);
	lua_setfield(L,-2,"x");
    lua_pushnumber(L,y);
    lua_setfield(L,-2,"y");
    if (mesh->is3d) {
        lua_pushnumber(L,z);
        lua_setfield(L,-2,"z");
    }
    lua_pushnumber(L,size);
	lua_setfield(L,-2,"size");
	lua_pushnumber(L,angle);
	lua_setfield(L,-2,"angle");
	lua_pushinteger(L,ttl);
	lua_setfield(L,-2,"ttl");
    lua_pushnumber(L,extra);
    lua_setfield(L,-2,"extra");

	lua_pushnumber(L,vx);
	lua_setfield(L,-2,"speedX");
	lua_pushnumber(L,vy);
	lua_setfield(L,-2,"speedY");
    if (mesh->is3d) {
        lua_pushnumber(L,vz);
        lua_setfield(L,-2,"speedZ");
    }
    lua_pushnumber(L,va);
	lua_setfield(L,-2,"speedAngular");
	lua_pushnumber(L,vs);
	lua_setfield(L,-2,"speedGrowth");

    lua_pushnumber(L,dx);
    lua_setfield(L,-2,"decay");
    lua_pushvector(L,dx,dy,dz,0);
    lua_setfield(L,-2,"decayVector");
    lua_pushnumber(L,dc);
	lua_setfield(L,-2,"decayAlpha");
	lua_pushnumber(L,da);
	lua_setfield(L,-2,"decayAngular");
	lua_pushnumber(L,ds);
	lua_setfield(L,-2,"decayGrowth");

    lua_pushvector(L,ax,ay,az,0);
    lua_setfield(L,-2,"accelerationVector");
    lua_pushnumber(L,ac);
    lua_setfield(L,-2,"accelerationAlpha");
    lua_pushnumber(L,aa);
    lua_setfield(L,-2,"accelerationAngular");
    lua_pushnumber(L,as);
    lua_setfield(L,-2,"accelerationGrowth");

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

int ParticlesBinder::getNearestParticle(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    double x=luaL_checknumber(L,2);
    double y=luaL_checknumber(L,3);
    double z=luaL_optnumber(L,4,0);
    int psize=mesh->getParticleCount();
    double pd=DBL_MAX;
    int pi=0;
	for (int i=0;i<psize;i++)
	{
		if (mesh->getSize(i)==0) //Dead particle
			continue;
        float px,py,pz;
        mesh->getPosition(i,&px,&py,&pz);
        double d=(px-x)*(px-x)+(py-y)*(py-y)+(pz-z)*(pz-z);
		if (d<pd) {
			pd=d;
			pi=i+1;
		}
	}
	if (pi>0)
	{
		lua_pushinteger(L,pi);
		lua_pushnumber(L,pd);
		return 2;
	}

    return 0;
}

int ParticlesBinder::getDeadParticles(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    std::set<int> dead=mesh->getDead();
    lua_createtable(L,dead.size(),0);
    int ii=1;
    for (auto it=dead.cbegin();it!=dead.cend();it++) {
        lua_pushinteger(L,*it);
        lua_rawseti(L,-2,ii++);
    }
    return 1;
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
    int slot=luaL_optinteger(L,3,0);

    mesh->setTexture(textureBase, slot);

    return 0;
}

int ParticlesBinder::clearTexture(lua_State *L)
{
    Binder binder(L);
    Particles *mesh = static_cast<Particles*>(binder.getInstance("Particles", 1));
    int slot=luaL_optinteger(L,2,0);

    mesh->clearTexture(slot);

    return 0;
}

