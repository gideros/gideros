/*
 * lqWorld.h
 *
 *  Created on: 22 oct. 2017
 *      Author: Nicolas
 */

#ifndef LQWORLD_H_
#define LQWORLD_H_
#include "liquidfunbinder.h"
#include <eventdispatcher.h>
#include <event.h>
#include "luaapplication.h"

#include <string>
class b2DebugDraw;
class b2WorldED : public b2World, public EventDispatcher
{
public:
	b2WorldED(const b2Vec2& gravity, bool doSleep);
	virtual ~b2WorldED();

	void SetDestructionListener(b2DestructionListener* listener)
	{
		m_destructionListener = listener;
		b2World::SetDestructionListener(listener);
	}

	b2DestructionListener* GetDestructionListener() const
	{
		return m_destructionListener;
	}

	void SetContactListener(b2ContactListener* listener)
	{
		m_contactListener = listener;
		b2World::SetContactListener(listener);
	}

	b2ContactListener* GetContactListener() const
	{
		return m_contactListener;
	}

	b2DebugDraw* GetDebugDraw()
	{
		return m_debugDraw;
	}

	void SetDebugDraw(b2DebugDraw* draw);

	static Event::Type BEGIN_CONTACT;
	static Event::Type END_CONTACT;
    static Event::Type BEGIN_CONTACT_PARTICLE;
    static Event::Type END_CONTACT_PARTICLE;
    static Event::Type BEGIN_CONTACT_PARTICLE2;
    static Event::Type END_CONTACT_PARTICLE2;
	static Event::Type PRE_SOLVE;
	static Event::Type POST_SOLVE;

	std::string error;

private:
	b2DestructionListener* m_destructionListener;
	b2ContactListener* m_contactListener;
	b2DebugDraw* m_debugDraw;
};

class MyRayCastCallback : public b2RayCastCallback
{
public:
    MyRayCastCallback(lua_State* L) : L(L) {}
    virtual float32 ReportFixture(	b2Fixture* fixture, const b2Vec2& point,
                                    const b2Vec2& normal, float32 fraction)
    {
        LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
        float physicsScale = application->getPhysicsScale();

        bool data = !lua_isnone(L, 7);

        lua_pushvalue(L, 6);

        if (data)
            lua_pushvalue(L, 7);

        getb2(L, fixture);
        lua_pushnumber(L, point.x * physicsScale);
        lua_pushnumber(L, point.y * physicsScale);
        lua_pushnumber(L, normal.x);
        lua_pushnumber(L, normal.y);
        lua_pushnumber(L, fraction);

        lua_call(L, data ? 7 : 6, 1);

        lua_Number result = luaL_optnumber(L, -1, -1);

        lua_pop(L, 1);

        return result;
    }

private:
    lua_State* L;
};

class MyQueryCallback : public b2QueryCallback
{
private:
	struct _particleInfo {
		const b2ParticleSystem *system;
		int32 index;
	};
public:
	bool ReportParticle(const b2ParticleSystem* particleSystem,
									int32 index)
	{
		_particleInfo p;
		p.system=particleSystem;
		p.index=index;
		particles.push_back(p);
		return true;
	}
	bool ReportFixture(b2Fixture* fixture)
	{
		fixtures.push_back(fixture);
		return true;
	}
	int Result(lua_State *L) {
		lua_createtable(L,fixtures.size(),0);
		for (std::size_t i = 0; i < fixtures.size(); ++i)
		{
			getb2(L, fixtures[i]);
			lua_rawseti(L, -2, i + 1);
		}
		lua_createtable(L,particles.size(),0);
		for (std::size_t i = 0; i < particles.size(); ++i)
		{
			lua_createtable(L,0,2);
			getb2(L, particles[i].system);
			lua_setfield(L,-2,"system");
			lua_pushinteger(L,particles[i].index);
			lua_setfield(L,-2,"index");
			lua_rawseti(L, -2, i + 1);
		}
		return 2;
	}
private:
	std::vector<b2Fixture*> fixtures;
	std::vector<_particleInfo> particles;
};


#endif /* LQWORLD_H_ */
