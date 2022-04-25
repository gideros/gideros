#ifndef PARTICLESBINDER_H
#define PARTICLESBINDER_H

#include "binder.h"

class ParticlesBinder
{
public:
    ParticlesBinder(lua_State *L);

private:
    static int create(lua_State *L);
    static int destruct(void *p);

    static int addParticles(lua_State *L);
    static int removeParticles(lua_State *L);
    static int setParticleSpeed(lua_State *L);
    static int setParticleDecay(lua_State *L);
    static int setParticleAcceleration(lua_State *L);
    static int setParticleColor(lua_State *L);
    static int setParticlePosition(lua_State *L);
    static int setParticleSize(lua_State *L);
    static int setParticleAngle(lua_State *L);
    static int setParticleTtl(lua_State *L);
    static int getParticleSpeed(lua_State *L);
    static int getParticleDecay(lua_State *L);
    static int getParticleAcceleration(lua_State *L);
    static int getParticleColor(lua_State *L);
    static int getParticlePosition(lua_State *L);
    static int getParticleSize(lua_State *L);
    static int getParticleAngle(lua_State *L);
    static int getParticleTtl(lua_State *L);
    static int setParticleTag(lua_State *L);
    static int getParticleTag(lua_State *L);
    static int setParticleExtra(lua_State *L);
    static int getParticleExtra(lua_State *L);
    static int getDeadParticles(lua_State *L);
    static int setPaused(lua_State *L);
    static int isPaused(lua_State *L);
    static int getParticles(lua_State *L);
    static int setTexture(lua_State *L);
    static int clearTexture(lua_State *L);
    static int scaleParticles(lua_State *L);
    static int getNearestParticle(lua_State *L);
};


#endif
