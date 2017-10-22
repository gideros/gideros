/*
 * lqWorld.cpp
 *
 *  Created on: 22 oct. 2017
 *      Author: Nicolas
 */
#include "lqWorld.h"
#include "lqSprites.h"

	b2WorldED::b2WorldED(const b2Vec2& gravity, bool doSleep) : b2World(gravity)
	{
		SetAllowSleeping(doSleep);
		m_destructionListener = NULL;
		m_contactListener = NULL;
		m_debugDraw = NULL;
	}

	b2WorldED::~b2WorldED()
	{
		b2World::SetDestructionListener(NULL);
		delete m_destructionListener;
		delete m_contactListener;
		if (m_debugDraw)
        {
            m_debugDraw->world_ = NULL;
			m_debugDraw->proxy_->unref();
        }
	}

	void b2WorldED::SetDebugDraw(b2DebugDraw* draw)
	{
		if (draw == m_debugDraw)
			return;

		if (draw)
		{
			draw->proxy_->ref();
			if (draw->world_)
				draw->world_->SetDebugDraw(NULL);
			draw->world_ = this;
		}

		if (m_debugDraw)
		{
			m_debugDraw->world_ = NULL;
			m_debugDraw->proxy_->unref();
		}

		m_debugDraw = draw;

		b2World::SetDebugDraw(draw);
	}

Event::Type b2WorldED::BEGIN_CONTACT("beginContact");
Event::Type b2WorldED::END_CONTACT("endContact");
Event::Type b2WorldED::BEGIN_CONTACT_PARTICLE("beginContactParticle");
Event::Type b2WorldED::END_CONTACT_PARTICLE("endContactParticle");
Event::Type b2WorldED::BEGIN_CONTACT_PARTICLE2("beginContactParticle2");
Event::Type b2WorldED::END_CONTACT_PARTICLE2("endContactParticle2");
Event::Type b2WorldED::PRE_SOLVE("preSolve");
Event::Type b2WorldED::POST_SOLVE("postSolve");

class EventContactListener : public b2ContactListener
{
public:
    EventContactListener(b2WorldED* world) : world(world) {}

private:
	void dispatchEvent(const Event::Type& type, b2Contact* contact, const b2Manifold* _U(oldManifold), const b2ContactImpulse* impulse)
	{
        Binder binder(L);

		getb2(L, world);

		if (!lua_isnil(L, -1))
		{
			lua_getfield(L, -1, "dispatchEvent");

			lua_pushvalue(L, -2); // create copy of world

/*
			lua_getglobal(L, "b2ContactEvent");
			lua_getfield(L, -1, "new");
			lua_remove(L, -2);

			b2ContactEvent event(type, contact, oldManifold, impulse);
			lua_pushlightuserdata(L, &event);
			lua_call(L, 1, 1); // call b2ContactEvent.new
*/
/*
			lua_getglobal(L, "Event");
			lua_getfield(L, -1, "new");
			lua_remove(L, -2);				// remove global "Event"

			lua_pushstring(L, type.type());
			lua_call(L, 1, 1); // call Event.new
			*/

			if (type.id() == b2WorldED::BEGIN_CONTACT.id())
				lua_getfield(L, -1, "__beginContactEvent");
			else if (type.id() == b2WorldED::END_CONTACT.id())
				lua_getfield(L, -1, "__endContactEvent");
			else if (type.id() == b2WorldED::PRE_SOLVE.id())
				lua_getfield(L, -1, "__preSolveEvent");
			else if (type.id() == b2WorldED::POST_SOLVE.id())
				lua_getfield(L, -1, "__postSolveEvent");

			getb2(L, contact->GetFixtureA());
			lua_setfield(L, -2, "fixtureA");

			getb2(L, contact->GetFixtureB());
			lua_setfield(L, -2, "fixtureB");


			if (impulse)
			{
				float32 maxImpulse = 0;
				for (int i = 0; i < impulse->count; ++i)
					maxImpulse = b2Max(maxImpulse, impulse->normalImpulses[i]);

				lua_pushnumber(L, maxImpulse);
				lua_setfield(L, -2, "maxImpulse");
			}

            if (contact)
            {
                lua_getfield(L, -2, "__contact");
                binder.setInstance(-1, contact);
                lua_setfield(L, -2, "contact");
            }

			//lua_call(L, 2, 0); // call world:dispatchEvent(event)
			if (lua_pcall(L, 2, 0, 0) != 0)
			{
				world->error = lua_tostring(L, -1);
				lua_pop(L, 1);
			}

            if (contact)
            {
                lua_getfield(L, -1, "__contact");
                binder.setInstance(-1, NULL);
                lua_pop(L, 1);
            }

			lua_pop(L, 1);
		}
		else
			lua_pop(L, 1);
	}

    void dispatchEventParticle(const Event::Type& type, b2ParticleSystem* particleSystem, b2Fixture* fixture, int particleIndex)
    {
        Binder binder(L);

        getb2(L, world);

        if (!lua_isnil(L, -1))
        {
            lua_getfield(L, -1, "dispatchEvent");

            lua_pushvalue(L, -2); // create copy of world

            if (type.id() == b2WorldED::BEGIN_CONTACT_PARTICLE.id())
                lua_getfield(L, -1, "__beginContactEventParticle");
            else if (type.id() == b2WorldED::END_CONTACT_PARTICLE.id())
                lua_getfield(L, -1, "__endContactEventParticle");

            getb2(L, fixture);
            lua_setfield(L, -2, "fixture");

            lua_pushinteger(L, particleIndex);
            lua_setfield(L, -2, "index");

            getb2(L, particleSystem);
            lua_setfield(L, -2, "system");

            //lua_call(L, 2, 0); // call world:dispatchEvent(event)
            if (lua_pcall(L, 2, 0, 0) != 0)
            {
                world->error = lua_tostring(L, -1);
                lua_pop(L, 1);
            }

            lua_pop(L, 1);
        }
        else
            lua_pop(L, 1);
    }

    void dispatchEventParticle2(const Event::Type& type, b2ParticleSystem* particleSystem, int particleIndex1, int particleIndex2)
    {
        Binder binder(L);

        getb2(L, world);

        if (!lua_isnil(L, -1))
        {
            lua_getfield(L, -1, "dispatchEvent");

            lua_pushvalue(L, -2); // create copy of world

            if (type.id() == b2WorldED::BEGIN_CONTACT_PARTICLE2.id())
                lua_getfield(L, -1, "__beginContactEventParticle2");
            else if (type.id() == b2WorldED::END_CONTACT_PARTICLE2.id())
                lua_getfield(L, -1, "__endContactEventParticle2");

            lua_pushinteger(L, particleIndex1);
            lua_setfield(L, -2, "indexA");

            lua_pushinteger(L, particleIndex2);
            lua_setfield(L, -2, "indexB");

            getb2(L, particleSystem);
            lua_setfield(L, -2, "system");

            //lua_call(L, 2, 0); // call world:dispatchEvent(event)
            if (lua_pcall(L, 2, 0, 0) != 0)
            {
                world->error = lua_tostring(L, -1);
                lua_pop(L, 1);
            }

            lua_pop(L, 1);
        }
        else
            lua_pop(L, 1);
    }

public:
	virtual void BeginContact(b2Contact* contact)
	{
		if (world->hasEventListener(b2WorldED::BEGIN_CONTACT))
			dispatchEvent(b2WorldED::BEGIN_CONTACT, contact, 0, 0);
	}

	virtual void EndContact(b2Contact* contact)
	{
		if (world->hasEventListener(b2WorldED::END_CONTACT))
			dispatchEvent(b2WorldED::END_CONTACT, contact, 0, 0);
	}

    virtual void BeginContact(b2ParticleSystem* particleSystem,
                              b2ParticleBodyContact* contact)
    {
        if (world->hasEventListener(b2WorldED::BEGIN_CONTACT_PARTICLE))
            dispatchEventParticle(b2WorldED::BEGIN_CONTACT_PARTICLE, particleSystem, contact->fixture, contact->index);
    }

    virtual void EndContact(b2Fixture* fixture, b2ParticleSystem* particleSystem, int32 index)
    {
        if (world->hasEventListener(b2WorldED::END_CONTACT_PARTICLE))
            dispatchEventParticle(b2WorldED::END_CONTACT_PARTICLE, particleSystem, fixture, index);
    }

    virtual void BeginContact(b2ParticleSystem* particleSystem,
                              b2ParticleContact* contact)
    {
        if (world->hasEventListener(b2WorldED::BEGIN_CONTACT_PARTICLE2))
            dispatchEventParticle2(b2WorldED::BEGIN_CONTACT_PARTICLE2, particleSystem, contact->GetIndexA(), contact->GetIndexB());
    }

    /// Called when two particles start touching if
    /// b2_particleContactFilterParticle flag is set on either particle.
    virtual void EndContact(b2ParticleSystem* particleSystem,
                            int32 indexA, int32 indexB)
    {
        if (world->hasEventListener(b2WorldED::END_CONTACT_PARTICLE2))
            dispatchEventParticle2(b2WorldED::END_CONTACT_PARTICLE2, particleSystem, indexA, indexB);
    }

	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		if (world->hasEventListener(b2WorldED::PRE_SOLVE))
			dispatchEvent(b2WorldED::PRE_SOLVE, contact, oldManifold, 0);
	}

	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		if (world->hasEventListener(b2WorldED::POST_SOLVE))
			dispatchEvent(b2WorldED::POST_SOLVE, contact, 0, impulse);
	}

private:
	b2WorldED* world;
};

class DestructionListener : public b2DestructionListener
{
public:
    DestructionListener(b2WorldED* world) : world(world)
	{

	}

	virtual void SayGoodbye(b2Joint* joint)
	{
		//StackChecker checker(L, "DestructionListener::SayGoodbye(b2Joint*)", 0);

		getb2(L, joint);

		if (!lua_isnil(L, -1))
		{
			lua_getfield(L, -1, "__world");
			lua_getfield(L, -1, "__joints");

			lua_pushvalue(L, -3);
			lua_pushnil(L);
			lua_settable(L, -3);

			Binder binder(L);
            binder.setInstance(-3, NULL);

			lua_pushlightuserdata(L, joint);
			lua_pushnil(L);
			setb2(L);

            lua_pop(L, 2);

            lua_pushnil(L);
            lua_setfield(L, -2, "__world");

            lua_pushnil(L);
            lua_setfield(L, -2, "__bodyA");

            lua_pushnil(L);
            lua_setfield(L, -2, "__bodyB");

            lua_pop(L, 1);
        }
		else
		{
			lua_pop(L, 1);
		}
	}

	virtual void SayGoodbye(b2Fixture* fixture)
	{
		//StackChecker checker(L, "DestructionListener::SayGoodbye(b2Fixture*)", 0);

		getb2(L, fixture);

		if (!lua_isnil(L, -1))
		{
			lua_getfield(L, -1, "__body");
			lua_getfield(L, -1, "__fixtures");

			lua_pushvalue(L, -3);
			lua_pushnil(L);
			lua_settable(L, -3);

			Binder binder(L);
            binder.setInstance(-3, NULL);

			lua_pushlightuserdata(L, fixture);
			lua_pushnil(L);
			setb2(L);

            lua_pop(L, 2);

            lua_pushnil(L);
            lua_setfield(L, -2, "__body");

            lua_pop(L, 1);
        }
		else
		{
			lua_pop(L, 1);
		}
	}

private:
	b2WorldED* world;
};


int Box2DBinder2::b2World_create(lua_State* L)
{
	Binder binder(L);

	lua_Number gravityx = luaL_checknumber(L, 1);
	lua_Number gravityy = luaL_checknumber(L, 2);

	bool doSleep = true;
	if (!lua_isnone(L, 3))
		doSleep = lua_toboolean(L, 3);

	b2WorldED* world = new b2WorldED(b2Vec2(gravityx, gravityy), doSleep);
    EventContactListener* contactListener = new EventContactListener(world);
	world->SetContactListener(contactListener);
    DestructionListener* destructionListener = new DestructionListener(world);
	world->SetDestructionListener(destructionListener);

	binder.pushInstance("b2World", world);

	lua_newtable(L);
	lua_setfield(L, -2, "__bodies");

	lua_newtable(L);
	lua_setfield(L, -2, "__joints");

	lua_pushlightuserdata(L, world);
	lua_pushvalue(L, -2);
	setb2(L);


	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);				// remove global "Event"

	lua_pushvalue(L, -1);	// duplicate Event.new
	lua_pushstring(L, b2WorldED::BEGIN_CONTACT.type());
	lua_call(L, 1, 1); // call Event.new
	lua_setfield(L, -3, "__beginContactEvent");

	lua_pushvalue(L, -1);	// duplicate Event.new
	lua_pushstring(L, b2WorldED::END_CONTACT.type());
	lua_call(L, 1, 1); // call Event.new
	lua_setfield(L, -3, "__endContactEvent");

    lua_pushvalue(L, -1);	// duplicate Event.new
    lua_pushstring(L, b2WorldED::BEGIN_CONTACT_PARTICLE.type());
    lua_call(L, 1, 1); // call Event.new
    lua_setfield(L, -3, "__beginContactEventParticle");

    lua_pushvalue(L, -1);	// duplicate Event.new
    lua_pushstring(L, b2WorldED::END_CONTACT_PARTICLE.type());
    lua_call(L, 1, 1); // call Event.new
    lua_setfield(L, -3, "__endContactEventParticle");

    lua_pushvalue(L, -1);	// duplicate Event.new
    lua_pushstring(L, b2WorldED::BEGIN_CONTACT_PARTICLE2.type());
    lua_call(L, 1, 1); // call Event.new
    lua_setfield(L, -3, "__beginContactEventParticle2");

    lua_pushvalue(L, -1);	// duplicate Event.new
    lua_pushstring(L, b2WorldED::END_CONTACT_PARTICLE2.type());
    lua_call(L, 1, 1); // call Event.new
    lua_setfield(L, -3, "__endContactEventParticle2");

	lua_pushvalue(L, -1);	// duplicate Event.new
	lua_pushstring(L, b2WorldED::PRE_SOLVE.type());
	lua_call(L, 1, 1); // call Event.new
	lua_setfield(L, -3, "__preSolveEvent");

	lua_pushvalue(L, -1);	// duplicate Event.new
	lua_pushstring(L, b2WorldED::POST_SOLVE.type());
	lua_call(L, 1, 1); // call Event.new
	lua_setfield(L, -3, "__postSolveEvent");

	lua_pop(L, 1);

    binder.pushInstance("b2Contact", NULL);         // __contact
    binder.pushInstance("b2Manifold", NULL);        // __manifold
    lua_newtable(L);
    lua_setfield(L, -2, "points");
    lua_newtable(L);
    lua_setfield(L, -2, "localNormal");
    lua_newtable(L);
    lua_setfield(L, -2, "localPoint");
    lua_setfield(L, -2, "__manifold");
    lua_newtable(L);                                // __points
    for (int i = 0; i < b2_maxManifoldPoints; ++i)
    {
        lua_newtable(L);
        lua_newtable(L);
        lua_setfield(L, -2, "localPoint");
        lua_rawseti(L, -2, i + 1);
    }
    lua_setfield(L, -2, "__points");
    binder.pushInstance("b2WorldManifold", NULL);   // __worldManifold
    lua_newtable(L);
    lua_setfield(L, -2, "points");
    lua_newtable(L);
    lua_setfield(L, -2, "normal");
    lua_setfield(L, -2, "__worldManifold");
    lua_newtable(L);                                // __worldPoints
    for (int i = 0; i < b2_maxManifoldPoints; ++i)
    {
        lua_newtable(L);
        lua_rawseti(L, -2, i + 1);
    }
    lua_setfield(L, -2, "__worldPoints");
    lua_setfield(L, -2, "__contact");

	return 1;
}

int Box2DBinder2::b2World_destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2WorldED* world = static_cast<b2WorldED*>(ptr);
	world->unref();

	return 0;
}




