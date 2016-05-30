#ifndef PARTICLESBINDER_H
#define PARTICLESBINDER_H

#include "binder.h"

class ParticlesBinder
{
public:
    ParticlesBinder(lua_State *L);

private:
    static int create(lua_State *L);
    static int destruct(lua_State *L);

    static int addParticles(lua_State *L);
    static int removeParticles(lua_State *L);
    static int setSpeed(lua_State *L);
    static int setColor(lua_State *L);
    static int getSpeed(lua_State *L);
    static int getColor(lua_State *L);

    static int setTexture(lua_State *L);
    static int clearTexture(lua_State *L);
};


#endif
