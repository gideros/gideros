#include "box2dbinder2.h"
#include <Box2D/Box2D.h>
#include "stackchecker.h"
#include <eventdispatcher.h>
#include <gstatus.h>
#include <event.h>
#include "luaapplication.h"

#include <ogl.h>
#include <color.h>
#include <sprite.h>
#include <texturebase.h>

#include "keys.h"
#include "luautil.h"

static lua_State *L = NULL;

class b2WorldED;

class b2DebugDraw : public Sprite, public b2Draw
{
public:
    b2DebugDraw(LuaApplication* application);

	virtual ~b2DebugDraw();

private:
	virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	virtual void DrawTransform(const b2Transform& xf);
    virtual void DrawParticles(const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count) { }

private:
	friend class b2WorldED;
	b2WorldED* world_;
	LuaApplication* application_;
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
};

class b2ParticleSystemSprite : public Sprite
{
public:
    b2ParticleSystemSprite(LuaApplication* application,b2ParticleSystem* b2ps,b2WorldED *world);
	virtual ~b2ParticleSystemSprite();
	b2ParticleSystem* GetSystem() { return ps_; }
	b2WorldED* GetWorld() { return world_; }
	void SetTexture(TextureBase *texture);
private:
	LuaApplication* application_;
	b2ParticleSystem* ps_;
	b2WorldED *world_;
	TextureBase* texturebase_;
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
};

b2ParticleSystemSprite::b2ParticleSystemSprite(LuaApplication* application,b2ParticleSystem* b2ps,b2WorldED *world) : Sprite(application->getApplication())
{
	ps_=b2ps;
	world_=world;
	application_=application;
	texturebase_=NULL;
}

void b2ParticleSystemSprite::SetTexture(TextureBase *texturebase)
{
	TextureBase *originaltexturebase = texturebase_;
	texturebase_ = texturebase;
	texturebase_->ref();
	if (originaltexturebase)
		originaltexturebase->unref();
}

b2ParticleSystemSprite::~b2ParticleSystemSprite()
{
	if (texturebase_ != NULL)
		texturebase_->unref();
	//TODO Destroy system ?
}

void b2ParticleSystemSprite::doDraw(const CurrentTransform& , float sx, float sy, float ex, float ey)
{
	if (ps_)
	{
		float physicsScale = application_->getPhysicsScale();

		Matrix4 modelMat=ShaderEngine::Engine->getModel();
		Matrix4 scaledMat=modelMat;
		scaledMat.scale(physicsScale,physicsScale,1);
		ShaderEngine::Engine->setModel(scaledMat);

		ShaderProgram *p=shader_;
		if (!p)
			p=ShaderProgram::stdParticle;
		if (p)
		{
			int pc=ps_->GetParticleCount();
			p->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 2,ps_->GetPositionBuffer(), pc, true, NULL);
			p->setData(ShaderProgram::DataColor, ShaderProgram::DUBYTE, 4,ps_->GetColorBuffer(), pc, true, NULL);

			float textureInfo[4]={0,0,0,0};
			if (texturebase_)
			{
				ShaderEngine::Engine->bindTexture(0,texturebase_->data->id());
				textureInfo[0]=(float)texturebase_->data->width / (float)texturebase_->data->exwidth;
				textureInfo[1]=(float)texturebase_->data->height / (float)texturebase_->data->exheight;
				textureInfo[2]=1.0/texturebase_->data->exwidth;
				textureInfo[3]=1.0/texturebase_->data->exheight;
			}
			int sc=p->getSystemConstant(ShaderProgram::SysConst_TextureInfo);
			if (sc>=0)
				p->setConstant(sc,ShaderProgram::CFLOAT4,1,textureInfo);
			sc = p->getSystemConstant(ShaderProgram::SysConst_ParticleSize);
			if (sc>=0)
			{
				float rad=ps_->GetRadius()*2; //Point Size is width, e.q diameter
				p->setConstant(sc,ShaderProgram::CFLOAT,1,&rad);
			}
			p->drawArrays(ShaderProgram::Point, 0, pc);
		}

		ShaderEngine::Engine->setModel(modelMat);
	}
}


static void getb2(lua_State* L)
{
	StackChecker checker(L, "getb2", 0);
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_b2);
	lua_pushvalue(L, -2);
	lua_rawget(L, -2);
	lua_remove(L, -2);
	lua_remove(L, -2);
}

static void getb2(lua_State* L, void* ptr)
{
	lua_pushlightuserdata(L, ptr);
	getb2(L);
}

static void setb2(lua_State* L)
{
	StackChecker checker(L, "setb2", -2);
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_b2);
	lua_pushvalue(L, -3);
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 3);
}

class b2WorldED : public b2World, public EventDispatcher
{
public:
	b2WorldED(const b2Vec2& gravity, bool doSleep) : b2World(gravity)
	{
		SetAllowSleeping(doSleep);
		m_destructionListener = NULL;
		m_contactListener = NULL;
		m_debugDraw = NULL;
	}

	virtual ~b2WorldED()
	{
		b2World::SetDestructionListener(NULL);
		delete m_destructionListener;
		delete m_contactListener;
		if (m_debugDraw)
        {
            m_debugDraw->world_ = NULL;
			m_debugDraw->unref();
        }
	}

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

	void SetDebugDraw(b2DebugDraw* draw)
	{
		if (draw == m_debugDraw)
			return;

		if (draw)
		{
			draw->ref();
			if (draw->world_)
				draw->world_->SetDebugDraw(NULL);
			draw->world_ = this;
		}

		if (m_debugDraw)
		{
			m_debugDraw->world_ = NULL;
			m_debugDraw->unref();
		}

		m_debugDraw = draw;

		b2World::SetDebugDraw(draw);
	}

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
	void dispatchEvent(const Event::Type& type, b2Contact* contact, const b2Manifold* oldManifold, const b2ContactImpulse* impulse)
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
            dispatchEventParticle2(b2WorldED::BEGIN_CONTACT_PARTICLE2, particleSystem, contact->indexA, contact->indexB);
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
		StackChecker checker(L, "DestructionListener::SayGoodbye(b2Joint*)", 0);

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
		StackChecker checker(L, "DestructionListener::SayGoodbye(b2Fixture*)", 0);

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

Box2DBinder2::Box2DBinder2(lua_State* L)
{
    ::L = L;

	StackChecker checker(L, "Box2DBinder2::Box2DBinder2", 0);

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "box2d");

	lua_pop(L, 2);
}

int Box2DBinder2::loader(lua_State *L)
{
	StackChecker checker(L, "Box2DBinder2::loader", 1);

	Binder binder(L);

	lua_getglobal(L, "Event");	// get "Event"

	lua_pushstring(L, b2WorldED::BEGIN_CONTACT.type());
	lua_setfield(L, -2, "BEGIN_CONTACT");

	lua_pushstring(L, b2WorldED::END_CONTACT.type());
	lua_setfield(L, -2, "END_CONTACT");

    lua_pushstring(L, b2WorldED::BEGIN_CONTACT_PARTICLE.type());
    lua_setfield(L, -2, "BEGIN_CONTACT_PARTICLE");

    lua_pushstring(L, b2WorldED::END_CONTACT_PARTICLE.type());
    lua_setfield(L, -2, "END_CONTACT_PARTICLE");

    lua_pushstring(L, b2WorldED::BEGIN_CONTACT_PARTICLE2.type());
    lua_setfield(L, -2, "BEGIN_CONTACT_PARTICLE2");

    lua_pushstring(L, b2WorldED::END_CONTACT_PARTICLE2.type());
    lua_setfield(L, -2, "END_CONTACT_PARTICLE2");

	lua_pushstring(L, b2WorldED::PRE_SOLVE.type());
	lua_setfield(L, -2, "PRE_SOLVE");

	lua_pushstring(L, b2WorldED::POST_SOLVE.type());
	lua_setfield(L, -2, "POST_SOLVE");

	lua_pop(L, 1);	// pop "Event"

	const luaL_Reg b2World_functionList[] = {
		{"createBody", b2World_CreateBody},
		{"destroyBody", b2World_DestroyBody},
		{"step", b2World_Step},
		{"clearForces", b2World_ClearForces},
		{"queryAABB", b2World_QueryAABB},
		{"rayCast", b2World_rayCast},
		{"createJoint", b2World_createJoint},
		{"destroyJoint", b2World_destroyJoint},
		{"getGravity", b2World_getGravity},
		{"setGravity", b2World_setGravity},
		{"setDebugDraw", b2World_setDebugDraw},
#if BIND_LIQUIDFUN
		{"createParticleSystem", b2World_createParticleSystem},
#endif
		{NULL, NULL},
	};
	binder.createClass("b2World", "EventDispatcher", b2World_create, b2World_destruct, b2World_functionList);

	const luaL_Reg b2Body_functionList[] = {
		//{"SetMassFromShapes", b2Body_SetMassFromShapes},
		{"getPosition", b2Body_GetPosition},
		{"setPosition", b2Body_SetPosition},
		{"getAngle", b2Body_GetAngle},
		{"setAngle", b2Body_SetAngle},
		{"getLinearVelocity", b2Body_GetLinearVelocity},
		{"setLinearVelocity", b2Body_SetLinearVelocity},
		//{"SetMass", b2Body_SetMass},
		//{"SetXForm", b2Body_SetXForm},
		//{"GetXForm", b2Body_GetXForm},
		//{"getWorldCenter", b2Body_GetWorldCenter},
		//{"getLocalCenter", b2Body_GetLocalCenter},
		{"setAngularVelocity", b2Body_SetAngularVelocity},
		{"getAngularVelocity", b2Body_GetAngularVelocity},
		{"createFixture", b2Body_CreateFixture},
		{"destroyFixture", b2Body_DestroyFixture},
		{"applyForce", b2Body_ApplyForce},
		{"applyTorque", b2Body_ApplyTorque},
		{"applyLinearImpulse", b2Body_ApplyLinearImpulse},
		{"applyAngularImpulse", b2Body_ApplyAngularImpulse},
        {"isAwake", b2Body_isAwake},
        {"setAwake", b2Body_setAwake},
        {"setActive", b2Body_setActive},
		{"isActive", b2Body_isActive},
		{"setType", b2Body_setType},
		{"getType", b2Body_getType},
		{"getLinearDamping", b2Body_getLinearDamping},
		{"setLinearDamping", b2Body_setLinearDamping},
		{"getAngularDamping", b2Body_getAngularDamping},
		{"setAngularDamping", b2Body_setAngularDamping},
        {"getWorldCenter", b2Body_getWorldCenter},
        {"getLocalCenter", b2Body_getLocalCenter},
        {"getMass", b2Body_getMass},
        {"getInertia", b2Body_getInertia},
        {"setGravityScale", b2Body_setGravityScale},
        {"getGravityScale", b2Body_getGravityScale},
        {"getWorldPoint", b2Body_getWorldPoint},
        {"getWorldVector", b2Body_getWorldVector},
        {"getLocalPoint", b2Body_getLocalPoint},
        {"getLocalVector", b2Body_getLocalVector},
        {"setFixedRotation", b2Body_setFixedRotation},
        {"isFixedRotation", b2Body_isFixedRotation},
        {"setBullet", b2Body_setBullet},
        {"isBullet", b2Body_isBullet},
        {"setSleepingAllowed", b2Body_setSleepingAllowed},
        {"isSleepingAllowed", b2Body_isSleepingAllowed},
        {"setTransform", b2Body_setTransform},
        {"getTransform", b2Body_getTransform},
        {NULL, NULL},
	};
	binder.createClass("b2Body", NULL, 0, b2Body_destruct, b2Body_functionList);

	const luaL_Reg b2Fixture_functionList[] = {
		{"getBody", b2Fixture_GetBody},
		{"setSensor", b2Fixture_SetSensor},
		{"isSensor", b2Fixture_IsSensor},
		{"setFilterData", b2Fixture_SetFilterData},
		{"getFilterData", b2Fixture_GetFilterData},
		{NULL, NULL},
	};
	binder.createClass("b2Fixture", NULL, 0, b2Fixture_destruct, b2Fixture_functionList);

	const luaL_Reg b2Shape_functionList[] = {
		{NULL, NULL},
	};
	binder.createClass("b2Shape", NULL, 0, b2Shape_destruct, b2Shape_functionList);

	const luaL_Reg b2CircleShape_functionList[] = {
		{"set", b2CircleShape_set},
		{NULL, NULL},
	};
	binder.createClass("b2CircleShape", "b2Shape", b2CircleShape_create, b2CircleShape_destruct, b2CircleShape_functionList);

	const luaL_Reg b2PolygonShape_functionList[] = {
		{"setAsBox", b2PolygonShape_SetAsBox},
//		{"setAsEdge", b2PolygonShape_SetAsEdge},
		{"set", b2PolygonShape_Set},
		{NULL, NULL},
	};
	binder.createClass("b2PolygonShape", "b2Shape", b2PolygonShape_create, b2PolygonShape_destruct, b2PolygonShape_functionList);

	const luaL_Reg b2EdgeShape_functionList[] = {
		{"set", b2EdgeShape_set},
		{NULL, NULL},
	};
	binder.createClass("b2EdgeShape", "b2Shape", b2EdgeShape_create, b2EdgeShape_destruct, b2EdgeShape_functionList);

    const luaL_Reg b2ChainShape_functionList[] = {
        {"createLoop", b2ChainShape_createLoop},
        {"createChain", b2ChainShape_createChain},
        {NULL, NULL},
    };
    binder.createClass("b2ChainShape", "b2Shape", b2ChainShape_create, b2ChainShape_destruct, b2ChainShape_functionList);

	/*
	const luaL_Reg b2AABB_functionList[] = {
		{"getLowerBound", b2AABB_GetLowerBound},
		{"getUpperBound", b2AABB_GetUpperBound},
		{"setLowerBound", b2AABB_SetLowerBound},
		{"setUpperBound", b2AABB_SetUpperBound},
		{NULL, NULL},
	};
	binder.createClass("b2AABB", NULL, b2AABB_create, b2AABB_destruct, b2AABB_functionList);
*/

	const luaL_Reg b2Joint_functionList[] = {
		{"getType", b2Joint_getType},
		{"getBodyA", b2Joint_getBodyA},
		{"getBodyB", b2Joint_getBodyB},
		{"getAnchorA", b2Joint_getAnchorA},
		{"getAnchorB", b2Joint_getAnchorB},
		{"isActive", b2Joint_isActive},
        {"getReactionForce", b2Joint_getReactionForce},
        {"getReactionTorque", b2Joint_getReactionTorque},
		{NULL, NULL},
	};
	binder.createClass("b2Joint", NULL, NULL, NULL, b2Joint_functionList);

	const luaL_Reg b2RevoluteJoint_functionList[] = {
		{"getJointAngle", b2RevoluteJoint_getJointAngle},
		{"getJointSpeed", b2RevoluteJoint_getJointSpeed},
		{"isLimitEnabled", b2RevoluteJoint_isLimitEnabled},
		{"enableLimit", b2RevoluteJoint_enableLimit},
		{"getLimits", b2RevoluteJoint_getLimits},
		{"setLimits", b2RevoluteJoint_setLimits},
		{"isMotorEnabled", b2RevoluteJoint_isMotorEnabled},
		{"enableMotor", b2RevoluteJoint_enableMotor},
		{"setMotorSpeed", b2RevoluteJoint_setMotorSpeed},
		{"getMotorSpeed", b2RevoluteJoint_getMotorSpeed},
		{"setMaxMotorTorque", b2RevoluteJoint_setMaxMotorTorque},
        {"getMotorTorque", b2RevoluteJoint_getMotorTorque},
		{NULL, NULL},
	};
	binder.createClass("b2RevoluteJoint", "b2Joint", NULL, b2Joint_destruct, b2RevoluteJoint_functionList);

	const luaL_Reg b2PrismaticJoint_functionList[] = {
		{"getJointTranslation", b2PrismaticJoint_getJointTranslation},
		{"getJointSpeed", b2PrismaticJoint_getJointSpeed},
		{"isLimitEnabled", b2PrismaticJoint_isLimitEnabled},
		{"enableLimit", b2PrismaticJoint_enableLimit},
		{"getLimits", b2PrismaticJoint_getLimits},
		{"setLimits", b2PrismaticJoint_setLimits},
		{"isMotorEnabled", b2PrismaticJoint_isMotorEnabled},
		{"enableMotor", b2PrismaticJoint_enableMotor},
		{"setMotorSpeed", b2PrismaticJoint_setMotorSpeed},
		{"getMotorSpeed", b2PrismaticJoint_getMotorSpeed},
		{"setMaxMotorForce", b2PrismaticJoint_setMaxMotorForce},
        {"getMotorForce", b2PrismaticJoint_getMotorForce},
		{NULL, NULL},
	};
	binder.createClass("b2PrismaticJoint", "b2Joint", NULL, b2Joint_destruct, b2PrismaticJoint_functionList);

	const luaL_Reg b2DistanceJoint_functionList[] = {
		{"setLength", b2DistanceJoint_setLength},
		{"getLength", b2DistanceJoint_getLength},
		{"setFrequency", b2DistanceJoint_setFrequency},
		{"getFrequency", b2DistanceJoint_getFrequency},
		{"setDampingRatio", b2DistanceJoint_setDampingRatio},
		{"getDampingRatio", b2DistanceJoint_getDampingRatio},
		{NULL, NULL},
	};
	binder.createClass("b2DistanceJoint", "b2Joint", NULL, b2Joint_destruct, b2DistanceJoint_functionList);

	const luaL_Reg b2PulleyJoint_functionList[] = {
		{"getGroundAnchorA", b2PulleyJoint_getGroundAnchorA},
		{"getGroundAnchorB", b2PulleyJoint_getGroundAnchorB},
		{"getLengthA", b2PulleyJoint_getLengthA},
		{"getLengthB", b2PulleyJoint_getLengthB},
		{"getRatio", b2PulleyJoint_getRatio},
		{NULL, NULL},
	};
	binder.createClass("b2PulleyJoint", "b2Joint", NULL, b2Joint_destruct, b2PulleyJoint_functionList);

	const luaL_Reg b2MouseJoint_functionList[] = {
		{"setTarget", b2MouseJoint_setTarget},
		{"getTarget", b2MouseJoint_getTarget},
		{"setMaxForce", b2MouseJoint_setMaxForce},
		{"getMaxForce", b2MouseJoint_getMaxForce},
		{"setFrequency", b2MouseJoint_setFrequency},
		{"getFrequency", b2MouseJoint_getFrequency},
		{"setDampingRatio", b2MouseJoint_setDampingRatio},
		{"getDampingRatio", b2MouseJoint_getDampingRatio},
		{NULL, NULL},
	};
	binder.createClass("b2MouseJoint", "b2Joint", NULL, b2Joint_destruct, b2MouseJoint_functionList);

	const luaL_Reg b2GearJoint_functionList[] = {
		{"setRatio", b2GearJoint_setRatio},
		{"getRatio", b2GearJoint_getRatio},
		{NULL, NULL},
	};
	binder.createClass("b2GearJoint", "b2Joint", NULL, b2Joint_destruct, b2GearJoint_functionList);

	const luaL_Reg b2WheelJoint_functionList[] = {
		{"getJointTranslation",   b2WheelJoint_getJointTranslation},
		{"getJointSpeed",         b2WheelJoint_getJointSpeed},
		{"isMotorEnabled",        b2WheelJoint_isMotorEnabled},
		{"enableMotor",           b2WheelJoint_enableMotor},
		{"setMotorSpeed",         b2WheelJoint_setMotorSpeed},
		{"getMotorSpeed",         b2WheelJoint_getMotorSpeed},
		{"setMaxMotorTorque",     b2WheelJoint_setMaxMotorTorque},
		{"getMaxMotorTorque",     b2WheelJoint_getMaxMotorTorque},
		{"setSpringFrequencyHz",  b2WheelJoint_setSpringFrequencyHz},
		{"getSpringFrequencyHz",  b2WheelJoint_getSpringFrequencyHz},
		{"setSpringDampingRatio", b2WheelJoint_setSpringDampingRatio},
		{"getSpringDampingRatio", b2WheelJoint_getSpringDampingRatio},
        {"getMotorTorque",        b2WheelJoint_getMotorTorque},
		{NULL, NULL},
	};
	binder.createClass("b2WheelJoint", "b2Joint", NULL, b2Joint_destruct, b2WheelJoint_functionList);

	const luaL_Reg b2WeldJoint_functionList[] = {
        {"setFrequency", b2WeldJoint_setFrequency},
        {"getFrequency", b2WeldJoint_getFrequency},
        {"setDampingRatio", b2WeldJoint_setDampingRatio},
        {"getDampingRatio", b2WeldJoint_getDampingRatio},
		{NULL, NULL},
	};
	binder.createClass("b2WeldJoint", "b2Joint", NULL, b2Joint_destruct, b2WeldJoint_functionList);

	const luaL_Reg b2FrictionJoint_functionList[] = {
		{"setMaxForce", b2FrictionJoint_setMaxForce},
		{"getMaxForce", b2FrictionJoint_getMaxForce},
		{"setMaxTorque", b2FrictionJoint_setMaxTorque},
		{"getMaxTorque", b2FrictionJoint_getMaxTorque},
		{NULL, NULL},
	};
	binder.createClass("b2FrictionJoint", "b2Joint", NULL, b2Joint_destruct, b2FrictionJoint_functionList);

    const luaL_Reg b2RopeJoint_functionList[] = {
        {"setMaxLength", b2RopeJoint_setMaxLength},
        {"getMaxLength", b2RopeJoint_getMaxLength},
        {NULL, NULL},
    };
    binder.createClass("b2RopeJoint", "b2Joint", NULL, b2Joint_destruct, b2RopeJoint_functionList);

	const luaL_Reg b2DebugDraw_functionList[] = {
		{"setFlags", b2DebugDraw_setFlags},
		{"getFlags", b2DebugDraw_getFlags},
		{"appendFlags", b2DebugDraw_appendFlags},
		{"clearFlags", b2DebugDraw_clearFlags},
		{NULL, NULL},
	};
	binder.createClass("b2DebugDraw", "Sprite", b2DebugDraw_create, b2DebugDraw_destruct, b2DebugDraw_functionList);

	lua_getglobal(L, "b2DebugDraw");

	lua_pushinteger(L, b2Draw::e_shapeBit);
	lua_setfield(L, -2, "SHAPE_BIT");

	lua_pushinteger(L, b2Draw::e_jointBit);
	lua_setfield(L, -2, "JOINT_BIT");

	lua_pushinteger(L, b2Draw::e_aabbBit);
	lua_setfield(L, -2, "AABB_BIT");

	lua_pushinteger(L, b2Draw::e_pairBit);
	lua_setfield(L, -2, "PAIR_BIT");

	lua_pushinteger(L, b2Draw::e_centerOfMassBit);
	lua_setfield(L, -2, "CENTER_OF_MASS_BIT");

	lua_pop(L, 1);	// b2DebugDraw

    const luaL_Reg b2Contact_functionList[] = {
        {"getManifold", b2Contact_getManifold},
        {"getWorldManifold", b2Contact_getWorldManifold},
        {"isTouching", b2Contact_isTouching},
        {"setEnabled", b2Contact_setEnabled},
        {"isEnabled", b2Contact_isEnabled},
        {"getFixtureA", b2Contact_getFixtureA},
        {"getChildIndexA", b2Contact_getChildIndexA},
        {"getFixtureB", b2Contact_getFixtureB},
        {"getChildIndexB", b2Contact_getChildIndexB},
        {"setFriction", b2Contact_setFriction},
        {"getFriction", b2Contact_getFriction},
        {"resetFriction", b2Contact_resetFriction},
        {"setRestitution", b2Contact_setRestitution},
        {"getRestitution", b2Contact_getRestitution},
        {"resetRestitution", b2Contact_resetRestitution},
        {NULL, NULL},
    };
    binder.createClass("b2Contact", NULL, NULL, NULL, b2Contact_functionList);

    const luaL_Reg b2Manifold_functionList[] = {
        {NULL, NULL},
    };
    binder.createClass("b2Manifold", NULL, NULL, NULL, b2Manifold_functionList);

    lua_getglobal(L, "b2Manifold");

    lua_pushinteger(L, b2Manifold::e_circles);
    lua_setfield(L, -2, "CIRCLES");

    lua_pushinteger(L, b2Manifold::e_faceA);
    lua_setfield(L, -2, "FACE_A");

    lua_pushinteger(L, b2Manifold::e_faceB);
    lua_setfield(L, -2, "FACE_B");

    lua_pop(L, 1);  // b2Manifold

    const luaL_Reg b2WorldManifold_functionList[] = {
        {NULL, NULL},
    };
    binder.createClass("b2WorldManifold", NULL, NULL, NULL, b2WorldManifold_functionList);

#if BIND_LIQUIDFUN
    const luaL_Reg b2ParticleGroup_functionList[] = {
        {"destroyParticles", b2ParticleGroup_destroyParticles},
        {"getParticleCount", b2ParticleGroup_getParticleCount},
        {"containsParticle", b2ParticleGroup_containsParticle},
        {NULL, NULL},
    };
    binder.createClass("b2ParticleGroup", NULL, NULL, NULL, b2ParticleGroup_functionList);

    const luaL_Reg b2ParticleSystem_functionList[] = {
    	{"createParticle",b2ParticleSystem_createParticle},
    	{"destroyParticle",b2ParticleSystem_destroyParticle},
    	{"createParticleGroup",b2ParticleSystem_createParticleGroup},
        {"setTexture", b2ParticleSystem_setTexture},
        {"getParticleGroupList", b2ParticleSystem_getParticleGroupList},
        {NULL, NULL},
    };
    binder.createClass("b2ParticleSystem", "Sprite", NULL, NULL, b2ParticleSystem_functionList);
    lua_getglobal(L, "b2ParticleSystem");

    lua_pushinteger(L, b2_waterParticle);
    lua_setfield(L, -2, "FLAG_WATER");
    lua_pushinteger(L, b2_zombieParticle);
    lua_setfield(L, -2, "FLAG_ZOMBIE");
    lua_pushinteger(L, b2_wallParticle);
    lua_setfield(L, -2, "FLAG_WALL");
    lua_pushinteger(L, b2_springParticle);
    lua_setfield(L, -2, "FLAG_SPRING");
    lua_pushinteger(L, b2_elasticParticle);
    lua_setfield(L, -2, "FLAG_ELASTIC");
    lua_pushinteger(L, b2_viscousParticle);
    lua_setfield(L, -2, "FLAG_VISCOUS");
    lua_pushinteger(L, b2_powderParticle);
    lua_setfield(L, -2, "FLAG_POWDER");
    lua_pushinteger(L, b2_tensileParticle);
    lua_setfield(L, -2, "FLAG_TENSILE");
    lua_pushinteger(L, b2_colorMixingParticle);
    lua_setfield(L, -2, "FLAG_COLOR_MIXING");
    lua_pushinteger(L, b2_destructionListenerParticle);
    lua_setfield(L, -2, "FLAG_DESTRUCTION_LISTENER");
    lua_pushinteger(L, b2_barrierParticle);
    lua_setfield(L, -2, "FLAG_BARRIER");
    lua_pushinteger(L, b2_staticPressureParticle);
    lua_setfield(L, -2, "FLAG_STATIC_PRESSURE");
    lua_pushinteger(L, b2_reactiveParticle);
    lua_setfield(L, -2, "FLAG_REACTIVE");
    lua_pushinteger(L, b2_repulsiveParticle);
    lua_setfield(L, -2, "FLAG_REPULSIVE");
    lua_pushinteger(L, b2_fixtureContactListenerParticle);
    lua_setfield(L, -2, "FLAG_FIXTURE_CONTACT_LISTENER");
    lua_pushinteger(L, b2_particleContactListenerParticle);
    lua_setfield(L, -2, "FLAG_PARTICLE_CONTACT_LISTENER");
    lua_pushinteger(L, b2_fixtureContactFilterParticle);
    lua_setfield(L, -2, "FLAG_FIXTURE_CONTACT_FILTER");
    lua_pushinteger(L, b2_particleContactFilterParticle);
    lua_setfield(L, -2, "FLAG_PARTICLE_CONTACT_FILTER");

    lua_pop(L, 1);  // b2ParticleSystem

#endif

	lua_newtable(L);

	lua_getglobal(L, "b2World");
	lua_setfield(L, -2, "World");
	lua_pushnil(L);
	lua_setglobal(L, "b2World");

	lua_getglobal(L, "b2Body");
	lua_setfield(L, -2, "Body");
	lua_pushnil(L);
	lua_setglobal(L, "b2Body");

	lua_getglobal(L, "b2Fixture");
	lua_setfield(L, -2, "Fixture");
	lua_pushnil(L);
	lua_setglobal(L, "b2Fixture");

	lua_getglobal(L, "b2Shape");
	lua_setfield(L, -2, "Shape");
	lua_pushnil(L);
	lua_setglobal(L, "b2Shape");

	lua_getglobal(L, "b2CircleShape");
	lua_setfield(L, -2, "CircleShape");
	lua_pushnil(L);
	lua_setglobal(L, "b2CircleShape");

	lua_getglobal(L, "b2PolygonShape");
	lua_setfield(L, -2, "PolygonShape");
	lua_pushnil(L);
	lua_setglobal(L, "b2PolygonShape");

	lua_getglobal(L, "b2EdgeShape");
	lua_setfield(L, -2, "EdgeShape");
	lua_pushnil(L);
	lua_setglobal(L, "b2EdgeShape");

    lua_getglobal(L, "b2ChainShape");
    lua_setfield(L, -2, "ChainShape");
    lua_pushnil(L);
    lua_setglobal(L, "b2ChainShape");

 /*
	lua_getglobal(L, "b2AABB");
	lua_setfield(L, -2, "AABB");
	lua_pushnil(L);
	lua_setglobal(L, "b2AABB");
*/


	lua_getglobal(L, "b2Joint");
	lua_setfield(L, -2, "Joint");
	lua_pushnil(L);
	lua_setglobal(L, "b2Joint");

	lua_getglobal(L, "b2RevoluteJoint");
	lua_setfield(L, -2, "RevoluteJoint");
	lua_pushnil(L);
	lua_setglobal(L, "b2RevoluteJoint");

	lua_getglobal(L, "b2PrismaticJoint");
	lua_setfield(L, -2, "PrismaticJoint");
	lua_pushnil(L);
	lua_setglobal(L, "b2PrismaticJoint");

	lua_getglobal(L, "b2DistanceJoint");
	lua_setfield(L, -2, "DistanceJoint");
	lua_pushnil(L);
	lua_setglobal(L, "b2DistanceJoint");

	lua_getglobal(L, "b2PulleyJoint");
	lua_setfield(L, -2, "PulleyJoint");
	lua_pushnil(L);
	lua_setglobal(L, "b2PulleyJoint");

	lua_getglobal(L, "b2MouseJoint");
	lua_setfield(L, -2, "MouseJoint");
	lua_pushnil(L);
	lua_setglobal(L, "b2MouseJoint");

	lua_getglobal(L, "b2GearJoint");
	lua_setfield(L, -2, "GearJoint");
	lua_pushnil(L);
	lua_setglobal(L, "b2GearJoint");

	lua_getglobal(L, "b2WheelJoint");
	lua_setfield(L, -2, "WheelJoint");
	lua_pushnil(L);
	lua_setglobal(L, "b2WheelJoint");

	lua_getglobal(L, "b2WeldJoint");
	lua_setfield(L, -2, "WeldJoint");
	lua_pushnil(L);
	lua_setglobal(L, "b2WeldJoint");

	lua_getglobal(L, "b2FrictionJoint");
	lua_setfield(L, -2, "FrictionJoint");
	lua_pushnil(L);
	lua_setglobal(L, "b2FrictionJoint");

    lua_getglobal(L, "b2RopeJoint");
    lua_setfield(L, -2, "RopeJoint");
    lua_pushnil(L);
    lua_setglobal(L, "b2RopeJoint");

	lua_getglobal(L, "b2DebugDraw");
	lua_setfield(L, -2, "DebugDraw");
	lua_pushnil(L);
	lua_setglobal(L, "b2DebugDraw");

    lua_getglobal(L, "b2Contact");
    lua_setfield(L, -2, "Contact");
    lua_pushnil(L);
    lua_setglobal(L, "b2Contact");

    lua_getglobal(L, "b2Manifold");
    lua_setfield(L, -2, "Manifold");
    lua_pushnil(L);
    lua_setglobal(L, "b2Manifold");

    lua_getglobal(L, "b2WorldManifold");
    lua_setfield(L, -2, "WorldManifold");
    lua_pushnil(L);
    lua_setglobal(L, "b2WorldManifold");

#if BIND_LIQUIDFUN
    lua_getglobal(L, "b2ParticleGroup");
    lua_setfield(L, -2, "ParticleGroup");
    lua_pushnil(L);
    lua_setglobal(L, "b2ParticleGroup");

    lua_getglobal(L, "b2ParticleSystem");
    lua_setfield(L, -2, "ParticleSystem");
    lua_pushnil(L);
    lua_setglobal(L, "b2ParticleSystem");
#endif

	lua_pushinteger(L, b2_staticBody);
	lua_setfield(L, -2, "STATIC_BODY");

	lua_pushinteger(L, b2_kinematicBody);
	lua_setfield(L, -2, "KINEMATIC_BODY");

	lua_pushinteger(L, b2_dynamicBody);
	lua_setfield(L, -2, "DYNAMIC_BODY");


	lua_pushinteger(L, e_revoluteJoint);
	lua_setfield(L, -2, "REVOLUTE_JOINT");

	lua_pushinteger(L, e_prismaticJoint);
	lua_setfield(L, -2, "PRISMATIC_JOINT");

	lua_pushinteger(L, e_distanceJoint);
	lua_setfield(L, -2, "DISTANCE_JOINT");

	lua_pushinteger(L, e_pulleyJoint);
	lua_setfield(L, -2, "PULLEY_JOINT");

	lua_pushinteger(L, e_mouseJoint);
	lua_setfield(L, -2, "MOUSE_JOINT");

	lua_pushinteger(L, e_gearJoint);
	lua_setfield(L, -2, "GEAR_JOINT");

	lua_pushinteger(L, e_wheelJoint);
	lua_setfield(L, -2, "WHEEL_JOINT");

	lua_pushinteger(L, e_weldJoint);
	lua_setfield(L, -2, "WELD_JOINT");

	lua_pushinteger(L, e_frictionJoint);
	lua_setfield(L, -2, "FRICTION_JOINT");

    lua_pushinteger(L, e_ropeJoint);
    lua_setfield(L, -2, "ROPE_JOINT");

	lua_pushcfunction(L, b2GetScale);
	lua_setfield(L, -2, "getScale");

	lua_pushcfunction(L, b2SetScale);
	lua_setfield(L, -2, "setScale");

	lua_pushcfunction(L, getRevoluteJointDef);
	lua_setfield(L, -2, "createRevoluteJointDef");

	lua_pushcfunction(L, getPrismaticJointDef);
	lua_setfield(L, -2, "createPrismaticJointDef");

	lua_pushcfunction(L, getDistanceJointDef);
	lua_setfield(L, -2, "createDistanceJointDef");

	lua_pushcfunction(L, getPulleyJointDef);
	lua_setfield(L, -2, "createPulleyJointDef");

	lua_pushcfunction(L, getMouseJointDef);
	lua_setfield(L, -2, "createMouseJointDef");

	lua_pushcfunction(L, getGearJointDef);
	lua_setfield(L, -2, "createGearJointDef");

	lua_pushcfunction(L, getWheelJointDef);
	lua_setfield(L, -2, "createWheelJointDef");

	lua_pushcfunction(L, getWeldJointDef);
	lua_setfield(L, -2, "createWeldJointDef");

	lua_pushcfunction(L, getFrictionJointDef);
	lua_setfield(L, -2, "createFrictionJointDef");

    lua_pushcfunction(L, getRopeJointDef);
    lua_setfield(L, -2, "createRopeJointDef");

    lua_pushcfunction(L, testOverlap);
    lua_setfield(L, -2, "testOverlap");

	lua_pushvalue(L, -1);

	lua_setglobal(L, "b2");

	return 1;
}

int Box2DBinder2::b2GetScale(lua_State* L)
{
	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	lua_pushnumber(L, application->getPhysicsScale());

	return 1;
}

int Box2DBinder2::b2SetScale(lua_State* L)
{
	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	lua_Number scale = luaL_checknumber(L, 1);
	application->setPhysicsScale(scale);

	return 0;
}



static b2Vec2 tableToVec2(lua_State* L, int index)
{
//	lua_rawgeti(L, index, 1);
	lua_getfield(L, index, "x");
	lua_Number x = luaL_checknumber(L, -1);
	lua_pop(L, 1);

//	lua_rawgeti(L, index, 2);
	lua_getfield(L, index, "y");
	lua_Number y = luaL_checknumber(L, -1);
	lua_pop(L, 1);

	return b2Vec2(x, y);
}

static void tableToBodyDef(lua_State* L, int index, b2BodyDef* bodyDef, float physicsScale)
{
	// TODO: index'tekinin table oldugunu test et

	Binder binder(L);

	lua_getfield(L, index, "type");
	if (!lua_isnil(L, -1))
		bodyDef->type = static_cast<b2BodyType>(luaL_checkinteger(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, index, "position");
	if (!lua_isnil(L, -1))
	{
		bodyDef->position = tableToVec2(L, -1);
		bodyDef->position.x /= physicsScale;
		bodyDef->position.y /= physicsScale;
	}
	lua_pop(L, 1);

	lua_getfield(L, index, "angle");
	if (!lua_isnil(L, -1))
		bodyDef->angle = luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "linearVelocity");
	if (!lua_isnil(L, -1))
		bodyDef->linearVelocity = tableToVec2(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "angularVelocity");
	if (!lua_isnil(L, -1))
		bodyDef->angularVelocity = luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "linearDamping");
	if (!lua_isnil(L, -1))
		bodyDef->linearDamping = luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "angularDamping");
	if (!lua_isnil(L, -1))
		bodyDef->angularDamping = luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "allowSleep");
	if (!lua_isnil(L, -1))
		bodyDef->allowSleep = lua_toboolean(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "awake");
	if (!lua_isnil(L, -1))
		bodyDef->awake = lua_toboolean(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "fixedRotation");
	if (!lua_isnil(L, -1))
		bodyDef->fixedRotation = lua_toboolean(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "bullet");
	if (!lua_isnil(L, -1))
		bodyDef->bullet = lua_toboolean(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "active");
	if (!lua_isnil(L, -1))
		bodyDef->active = lua_toboolean(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "gravityScale");
	if (!lua_isnil(L, -1))
		bodyDef->gravityScale = luaL_checknumber(L, -1);
	lua_pop(L, 1);
}

int Box2DBinder2::b2World_CreateBody(lua_State* L)
{
	StackChecker checker(L, "b2World_CreateBody", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	Binder binder(L);
	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));
	//	b2BodyDef* bodyDef = static_cast<b2BodyDef*>(binder.getInstance("b2BodyDef", 2));

	if (world->IsLocked())
		return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

	b2BodyDef bodyDef;
	tableToBodyDef(L, 2, &bodyDef, application->getPhysicsScale());

	b2Body* body = world->CreateBody(&bodyDef);
	binder.pushInstance("b2Body", body);

	lua_newtable(L);
	lua_setfield(L, -2, "__fixtures");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "__world");

	lua_getfield(L, 1, "__bodies");
	lua_pushvalue(L, -2);
	lua_pushlightuserdata(L, body);
	lua_settable(L, -3);
	lua_pop(L, 1);

	return 1;
}

int Box2DBinder2::b2World_Step(lua_State* L)
{
	StackChecker checker(L, "b2World_Step", 0);

	Binder binder(L);
	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

	double timeStep = luaL_checknumber(L, 2);
	int velocityIterations = luaL_checkinteger(L, 3);
	int positionIterations = luaL_checkinteger(L, 4);

	world->error.clear();
	world->Step(timeStep, velocityIterations, positionIterations);
	if (!world->error.empty())
	{
		lua_pushstring(L, world->error.c_str());
		return lua_error(L);
	}

	return 0;
}

int Box2DBinder2::b2World_getGravity(lua_State* L)
{
	StackChecker checker(L, "b2World_getGravity", 2);

	Binder binder(L);
	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

	b2Vec2 v = world->GetGravity();

	lua_pushnumber(L, v.x);
	lua_pushnumber(L, v.y);

	return 2;
}

int Box2DBinder2::b2World_setGravity(lua_State* L)
{
	StackChecker checker(L, "b2World_setGravity", 0);

	Binder binder(L);
	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);

	world->SetGravity(b2Vec2(x, y));

	return 0;
}


static b2Body* toBody(const Binder& binder, int index)
{
	b2Body* body = static_cast<b2Body*>(binder.getInstance("b2Body", index));

	if (body == 0)
	{
		GStatus status(5001);	// Body is already destroyed.
		luaL_error(binder.L, status.errorString());
	}

	return body;
}

int Box2DBinder2::b2World_DestroyBody(lua_State* L)
{
	StackChecker checker(L, "b2World_DestroyBody", 0);

	Binder binder(L);
	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));
	b2Body* body = toBody(binder, 2);

	if (world->IsLocked())
		return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

#if 0
	// artik asagidaki islemi DestructionListener yapiyor
	// set __userdata = 0 for all fixtures of this body
	lua_getfield(L, 2, "__fixtures");
	int t = abs_index(L, -1); /* table is in the stack at index 't' */
	lua_pushnil(L);			  /* first key */
	while (lua_next(L, t) != 0)
	{
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		binder.setInstance(-2, 0);
		lua_pop(L, 1);		/* removes 'value'; keeps 'key' for next iteration */
	}
	lua_pop(L, 1);

	lua_newtable(L);
	lua_setfield(L, 2, "__fixtures");
#endif

	world->DestroyBody(body);

    binder.setInstance(2, NULL);	// set __userdata = 0

    lua_pushnil(L);
    lua_setfield(L, 2, "__world");

	lua_getfield(L, 1, "__bodies");
	lua_pushvalue(L, 2);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);

	return 0;
}

int Box2DBinder2::b2World_ClearForces(lua_State* L)
{
	StackChecker checker(L, "b2World_ClearForces", 0);

	Binder binder(L);
	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

	world->ClearForces();

	return 0;
}

class MyQueryCallback : public b2QueryCallback
{
public:
	bool ReportFixture(b2Fixture* fixture)
	{
		fixtures.push_back(fixture);
		return true;
	}

	std::vector<b2Fixture*> fixtures;
};

int Box2DBinder2::b2World_QueryAABB(lua_State* L)
{
	StackChecker checker(L, "b2World_Query", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);
	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

	lua_Number lx = luaL_checknumber(L, 2) / physicsScale;
	lua_Number ly = luaL_checknumber(L, 3) / physicsScale;
	lua_Number ux = luaL_checknumber(L, 4) / physicsScale;
	lua_Number uy = luaL_checknumber(L, 5) / physicsScale;

	b2AABB aabb;
	aabb.lowerBound.Set(lx, ly);
	aabb.upperBound.Set(ux, uy);

	MyQueryCallback callback;
	world->QueryAABB(&callback, aabb);

	lua_newtable(L);
#if 0
	lua_getglobal(L, "table");
	lua_getfield(L, -1, "insert"); // table.insert function
	for (std::size_t i = 0; i < callback.fixtures.size(); ++i)
	{
		b2Fixture* fixture = callback.fixtures[i];

		lua_pushvalue(L, -1);		// table.insert
		lua_pushvalue(L, -4);		// result
		getb2(L, fixture);			// fixture
		lua_call(L, 2, 0);			// table.insert(result, fixture)
	}
	lua_pop(L, 2);	// pop table and table.insert
#else
	for (std::size_t i = 0; i < callback.fixtures.size(); ++i)
	{
		getb2(L, callback.fixtures[i]);
		lua_rawseti(L, -2, i + 1);
	}
#endif

	return 1;
}

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

int Box2DBinder2::b2World_rayCast(lua_State* L)
{
    StackChecker checker(L, "b2World_rayCast", 0);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);
    b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

    lua_Number x1 = luaL_checknumber(L, 2) / physicsScale;
    lua_Number y1 = luaL_checknumber(L, 3) / physicsScale;
    lua_Number x2 = luaL_checknumber(L, 4) / physicsScale;
    lua_Number y2 = luaL_checknumber(L, 5) / physicsScale;
    luaL_checktype(L, 6, LUA_TFUNCTION);

    MyRayCastCallback callback(L);
    world->RayCast(&callback, b2Vec2(x1, y1), b2Vec2(x2, y2));

    return 0;
}

int Box2DBinder2::b2Body_destruct(lua_State* L)
{
#if 0
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2Body* body = static_cast<b2Body*>(ptr);
	if (body != 0)
		body->GetWorld()->DestroyBody(body);
#endif
	return 0;
}


static void tableToFilter(lua_State* L, int index, b2Filter* filter)
{
	// TODO: index'tekinin table oldugunu test et

	Binder binder(L);

	lua_getfield(L, index, "categoryBits");
	if (!lua_isnil(L, -1))
		filter->categoryBits = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "maskBits");
	if (!lua_isnil(L, -1))
		filter->maskBits = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "groupIndex");
	if (!lua_isnil(L, -1))
		filter->groupIndex = luaL_checkinteger(L, -1);
	lua_pop(L, 1);
}

static b2Shape* toShape(const Binder& binder, int index)
{
	return static_cast<b2Shape*>(binder.getInstance("b2Shape", index));
}

static void tableToFixtureDef(lua_State* L, int index, b2FixtureDef* fixtureDef)
{
	// TODO: index'tekinin table oldugunu test et

	Binder binder(L);

	lua_getfield(L, index, "shape");
	if (lua_isnil(L, -1))
		luaL_error(L, "shape must exist in fixture definition table");
	fixtureDef->shape = toShape(binder, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "friction");
	if (!lua_isnil(L, -1))
		fixtureDef->friction = luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "restitution");
	if (!lua_isnil(L, -1))
		fixtureDef->restitution = luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "density");
	if (!lua_isnil(L, -1))
		fixtureDef->density = luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "isSensor");
	if (!lua_isnil(L, -1))
		fixtureDef->isSensor = lua_toboolean(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "filter");
	if (!lua_isnil(L, -1))
		tableToFilter(L, -1, &fixtureDef->filter);
	lua_pop(L, 1);
}

int Box2DBinder2::b2Body_CreateFixture(lua_State* L)
{
	StackChecker checker(L, "b2Body_CreateFixture", 1);

	Binder binder(L);
	b2Body* body = toBody(binder, 1);
	//b2FixtureDef* fixtureDef = static_cast<b2FixtureDef*>(binder.getInstance("b2FixtureDef", 2));

	if (body->GetWorld()->IsLocked())
		return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

	b2FixtureDef fixtureDef;
	tableToFixtureDef(L, 2, &fixtureDef);

	b2Fixture* fixture = body->CreateFixture(&fixtureDef);
	binder.pushInstance("b2Fixture", fixture);

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "__body");

	lua_getfield(L, 1, "__fixtures");
	lua_pushvalue(L, -2);
	lua_pushlightuserdata(L, fixture);
	lua_settable(L, -3);
	lua_pop(L, 1);

	lua_pushlightuserdata(L, fixture);
	lua_pushvalue(L, -2);
	setb2(L);

	return 1;
}

static b2Fixture* toFixture(const Binder& binder, int index)
{
	b2Fixture* fixture = static_cast<b2Fixture*>(binder.getInstance("b2Fixture", index));

	if (fixture == 0)
	{
		GStatus status(5002);	// Fixture is already destroyed.
		luaL_error(binder.L, status.errorString());
	}

	return fixture;
}

static b2Joint* toJoint(const Binder& binder, int index, const char* type = "b2Joint")
{
	b2Joint* joint = static_cast<b2Joint*>(binder.getInstance(type, index));

	if (joint == 0)
	{
		GStatus status(5003);	// Joint is already destroyed.
		luaL_error(binder.L, status.errorString());
	}

	return joint;
}

int Box2DBinder2::b2Body_DestroyFixture(lua_State* L)
{
	StackChecker checker(L, "b2Body_DestroyFixture", 0);

	Binder binder(L);
	b2Body* body = toBody(binder, 1);
	b2Fixture* fixture = toFixture(binder, 2);

	if (body->GetWorld()->IsLocked())
		return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

	body->DestroyFixture(fixture);

    binder.setInstance(2, NULL);	// set fixture.__userdata = 0

    lua_pushnil(L);
    lua_setfield(L, 2, "__body");

	lua_getfield(L, 1, "__fixtures");
	lua_pushvalue(L, 2);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);

	lua_pushlightuserdata(L, fixture);
	lua_pushnil(L);
	setb2(L);

	return 0;
}

int Box2DBinder2::b2Body_GetPosition(lua_State* L)
{
	StackChecker checker(L, "b2Body_GetPosition", 2);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);
	b2Body* body = toBody(binder, 1);

	const b2Vec2& position = body->GetPosition();

/*	lua_getglobal(L, "b2Vec2");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);
	lua_pushnumber(L, position.x);
	lua_pushnumber(L, position.y);
	lua_call(L, 2, 1);
	return 1; */

	lua_pushnumber(L, position.x * physicsScale);
	lua_pushnumber(L, position.y * physicsScale);
	return 2;
}


int Box2DBinder2::b2Body_SetPosition(lua_State* L)
{
	StackChecker checker(L, "b2Body_SetPosition", 0);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);
	b2Body* body = toBody(binder, 1);

	if (body->GetWorld()->IsLocked())
		return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	body->SetTransform(b2Vec2(x / physicsScale, y / physicsScale), body->GetAngle());

	return 0;
}


int Box2DBinder2::b2Body_SetAngle(lua_State* L)
{
	StackChecker checker(L, "b2Body_SetAngle", 0);

	Binder binder(L);
	b2Body* body = toBody(binder, 1);

	if (body->GetWorld()->IsLocked())
		return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

	body->SetTransform(body->GetPosition(), luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2Body_GetAngle(lua_State* L)
{
	StackChecker checker(L, "b2Body_GetAngle", 1);

	Binder binder(L);
	b2Body* body = toBody(binder, 1);

	lua_pushnumber(L, body->GetAngle());

	return 1;
}

int Box2DBinder2::b2Body_GetLinearVelocity(lua_State* L)
{
	StackChecker checker(L, "b2Body_GetLinearVelocity", 2);

	Binder binder(L);
	b2Body* body = toBody(binder, 1);

	b2Vec2 linearVelocity = body->GetLinearVelocity();

/*	lua_getglobal(L, "b2Vec2");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);
	lua_pushnumber(L, linearVelocity.x);
	lua_pushnumber(L, linearVelocity.y);
	lua_call(L, 2, 1); */

	lua_pushnumber(L, linearVelocity.x);
	lua_pushnumber(L, linearVelocity.y);

	return 2;
}

int Box2DBinder2::b2Body_SetLinearVelocity(lua_State* L)
{
	StackChecker checker(L, "b2Body_SetLinearVelocity", 0);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);

	body->SetLinearVelocity(b2Vec2(x, y));

	return 0;
}

int Box2DBinder2::b2Body_SetAngularVelocity(lua_State* L)
{
	StackChecker checker(L, "b2Body_SetAngularVelocity", 0);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	lua_Number angularVelocity = luaL_checknumber(L, 2);

	body->SetAngularVelocity(angularVelocity);

	return 0;
}


int Box2DBinder2::b2Body_GetAngularVelocity(lua_State* L)
{
	StackChecker checker(L, "b2Body_GetAngularVelocity", 1);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	lua_pushnumber(L, body->GetAngularVelocity());

	return 1;
}


int Box2DBinder2::b2Body_ApplyForce(lua_State* L)
{
	StackChecker checker(L, "b2Body_ApplyForce", 0);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);

	b2Body* body = toBody(binder, 1);

	lua_Number forcex = luaL_checknumber(L, 2);
	lua_Number forcey = luaL_checknumber(L, 3);
	lua_Number pointx = luaL_checknumber(L, 4) / physicsScale;
	lua_Number pointy = luaL_checknumber(L, 5) / physicsScale;

    body->ApplyForce(b2Vec2(forcex, forcey), b2Vec2(pointx, pointy), true);

	return 0;
}

int Box2DBinder2::b2Body_ApplyTorque(lua_State* L)
{
	StackChecker checker(L, "b2Body_ApplyTorque", 0);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	lua_Number torque = luaL_checknumber(L, 2);

    body->ApplyTorque(torque, true);

	return 0;
}

int Box2DBinder2::b2Body_ApplyLinearImpulse(lua_State* L)
{
	StackChecker checker(L, "b2Body_ApplyLinearImpulse", 0);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);

	b2Body* body = toBody(binder, 1);

	lua_Number impulsex = luaL_checknumber(L, 2);
	lua_Number impulsey = luaL_checknumber(L, 3);
	lua_Number pointx = luaL_checknumber(L, 4) / physicsScale;
	lua_Number pointy = luaL_checknumber(L, 5) / physicsScale;

    body->ApplyLinearImpulse(b2Vec2(impulsex, impulsey), b2Vec2(pointx, pointy), true);

	return 0;
}

int Box2DBinder2::b2Body_ApplyAngularImpulse(lua_State* L)
{
	StackChecker checker(L, "b2Body_ApplyAngularImpulse", 0);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	lua_Number impulse = luaL_checknumber(L, 2);

    body->ApplyAngularImpulse(impulse, true);

	return 0;
}

int Box2DBinder2::b2Body_isAwake(lua_State* L)
{
	StackChecker checker(L, "b2Body_isAwake", 1);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	lua_pushboolean(L, body->IsAwake() ? 1 : 0);

	return 1;
}

int Box2DBinder2::b2Body_setAwake(lua_State* L)
{
    StackChecker checker(L, "b2Body_setAwake", 0);

    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    body->SetAwake(lua_toboolean(L, 2) != 0);

    return 0;
}

int Box2DBinder2::b2Body_setActive(lua_State* L)
{
	StackChecker checker(L, "b2Body_setActive", 0);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);

	if (body->GetWorld()->IsLocked())
		return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

	body->SetActive(lua_toboolean(L, 2) != 0);

	return 0;
}

int Box2DBinder2::b2Body_isActive(lua_State* L)
{
	StackChecker checker(L, "b2Body_isActive", 1);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	lua_pushboolean(L, body->IsActive() ? 1 : 0);

	return 1;
}

int Box2DBinder2::b2Body_setType(lua_State* L)
{
	StackChecker checker(L, "b2Body_setType", 0);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);

	if (body->GetWorld()->IsLocked())
		return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

	body->SetType((b2BodyType)luaL_checkinteger(L, 2));

	return 0;
}

int Box2DBinder2::b2Body_getType(lua_State* L)
{
	StackChecker checker(L, "b2Body_getType", 1);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	lua_pushinteger(L, body->GetType());

	return 1;
}

int Box2DBinder2::b2Body_getLinearDamping(lua_State* L)
{
	StackChecker checker(L, "b2Body_getLinearDamping", 1);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	lua_pushnumber(L, body->GetLinearDamping());

	return 1;
}

int Box2DBinder2::b2Body_setLinearDamping(lua_State* L)
{
	StackChecker checker(L, "b2Body_setLinearDamping", 0);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	body->SetLinearDamping(luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2Body_getAngularDamping(lua_State* L)
{
	StackChecker checker(L, "b2Body_getAngularDamping", 1);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	lua_pushnumber(L, body->GetAngularDamping());

	return 1;
}

int Box2DBinder2::b2Body_setAngularDamping(lua_State* L)
{
	StackChecker checker(L, "b2Body_setAngularDamping", 0);

	Binder binder(L);

	b2Body* body = toBody(binder, 1);
	body->SetAngularDamping(luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2Body_getWorldCenter(lua_State* L)
{
    StackChecker checker(L, "b2Body_getWorldCenter", 2);

    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    b2Vec2 center = body->GetWorldCenter();
    lua_pushnumber(L, center.x * physicsScale);
    lua_pushnumber(L, center.y * physicsScale);

    return 2;
}

int Box2DBinder2::b2Body_getLocalCenter(lua_State* L)
{
    StackChecker checker(L, "b2Body_getLocalCenter", 2);

    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    b2Vec2 center = body->GetLocalCenter();
    lua_pushnumber(L, center.x * physicsScale);
    lua_pushnumber(L, center.y * physicsScale);

    return 2;
}

int Box2DBinder2::b2Body_getMass(lua_State* L)
{
    StackChecker checker(L, "b2Body_getMass", 1);

    Binder binder(L);

    b2Body* body = toBody(binder, 1);
    lua_pushnumber(L, body->GetMass());

    return 1;
}

int Box2DBinder2::b2Body_getInertia(lua_State* L)
{
    StackChecker checker(L, "b2Body_getInertia", 1);

    Binder binder(L);

    b2Body* body = toBody(binder, 1);
    lua_pushnumber(L, body->GetInertia());

    return 1;
}

int Box2DBinder2::b2Body_setGravityScale(lua_State* L)
{
    StackChecker checker(L, "b2Body_setGravityScale", 0);

    Binder binder(L);

    b2Body* body = toBody(binder, 1);
    body->SetGravityScale(luaL_checknumber(L, 2));

    return 0;
}

int Box2DBinder2::b2Body_getGravityScale(lua_State* L)
{
    StackChecker checker(L, "b2Body_getGravityScale", 1);

    Binder binder(L);

    b2Body* body = toBody(binder, 1);
    lua_pushnumber(L, body->GetGravityScale());

    return 1;
}

int Box2DBinder2::b2Body_getWorldPoint(lua_State* L)
{
    StackChecker checker(L, "b2Body_getWorldPoint", 2);

    LuaApplication *application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    b2Vec2 localPoint(luaL_checknumber(L, 2) / physicsScale, luaL_checknumber(L, 3) / physicsScale);

    b2Vec2 worldPoint = body->GetWorldPoint(localPoint);

    lua_pushnumber(L, worldPoint.x * physicsScale);
    lua_pushnumber(L, worldPoint.y * physicsScale);

    return 2;
}

int Box2DBinder2::b2Body_getWorldVector(lua_State* L)
{
    StackChecker checker(L, "b2Body_getWorldVector", 2);

    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    b2Vec2 localVector(luaL_checknumber(L, 2), luaL_checknumber(L, 3));

    b2Vec2 worldVector = body->GetWorldVector(localVector);

    lua_pushnumber(L, worldVector.x);
    lua_pushnumber(L, worldVector.y);

    return 2;
}

int Box2DBinder2::b2Body_getLocalPoint(lua_State* L)
{
    StackChecker checker(L, "b2Body_getLocalPoint", 2);

    LuaApplication *application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    b2Vec2 worldPoint(luaL_checknumber(L, 2) / physicsScale, luaL_checknumber(L, 3) / physicsScale);

    b2Vec2 localPoint = body->GetLocalPoint(worldPoint);

    lua_pushnumber(L, localPoint.x * physicsScale);
    lua_pushnumber(L, localPoint.y * physicsScale);

    return 2;
}

int Box2DBinder2::b2Body_getLocalVector(lua_State* L)
{
    StackChecker checker(L, "b2Body_getLocalVector", 2);

    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    b2Vec2 worldVector(luaL_checknumber(L, 2), luaL_checknumber(L, 3));

    b2Vec2 localVector = body->GetLocalVector(worldVector);

    lua_pushnumber(L, localVector.x);
    lua_pushnumber(L, localVector.y);

    return 2;
}

int Box2DBinder2::b2Body_setFixedRotation(lua_State* L)
{
    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    body->SetFixedRotation(lua_toboolean2(L, 2));

    return 0;
}

int Box2DBinder2::b2Body_isFixedRotation(lua_State* L)
{
    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    lua_pushboolean(L, body->IsFixedRotation());

    return 1;
}

int Box2DBinder2::b2Body_setBullet(lua_State* L)
{
    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    body->SetBullet(lua_toboolean2(L, 2));

    return 0;
}

int Box2DBinder2::b2Body_isBullet(lua_State* L)
{
    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    lua_pushboolean(L, body->IsBullet());

    return 1;
}

int Box2DBinder2::b2Body_setSleepingAllowed(lua_State* L)
{
    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    body->SetSleepingAllowed(lua_toboolean2(L, 2));

    return 0;
}

int Box2DBinder2::b2Body_isSleepingAllowed(lua_State* L)
{
    Binder binder(L);

    b2Body* body = toBody(binder, 1);

    lua_pushboolean(L, body->IsSleepingAllowed());

    return 1;
}

int Box2DBinder2::b2Body_setTransform(lua_State* L)
{
    StackChecker checker(L, "b2Body_setTransform", 0);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);
    b2Body* body = toBody(binder, 1);

    if (body->GetWorld()->IsLocked())
        return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

    lua_Number x = luaL_checknumber(L, 2);
    lua_Number y = luaL_checknumber(L, 3);
    lua_Number angle = luaL_checknumber(L, 3);
    body->SetTransform(b2Vec2(x / physicsScale, y / physicsScale), angle);

    return 0;
}

int Box2DBinder2::b2Body_getTransform(lua_State* L)
{
    StackChecker checker(L, "b2Body_getTransform", 3);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);
    b2Body* body = toBody(binder, 1);

    const b2Transform &transform = body->GetTransform();

    lua_pushnumber(L, transform.p.x * physicsScale);
    lua_pushnumber(L, transform.p.y * physicsScale);
    lua_pushnumber(L, transform.q.GetAngle());

    return 3;
}


int Box2DBinder2::b2Fixture_destruct(lua_State* L)
{
#if 0
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2Fixture* fixture = static_cast<b2Fixture*>(ptr);
	if (fixture != 0)
		fixture->GetBody()->DestroyFixture(fixture);
#endif
	return 0;
}

int Box2DBinder2::b2Fixture_GetBody(lua_State* L)
{
	StackChecker checker(L, "b2Fixture_GetBody", 1);

	Binder binder(L);

	toFixture(binder, 1);

	lua_getfield(L, 1, "__body");

	return 1;
}

int Box2DBinder2::b2Fixture_SetSensor(lua_State* L)
{
	StackChecker checker(L, "b2Fixture_SetSensor", 0);

	Binder binder(L);

	b2Fixture* fixture = toFixture(binder, 1);
	bool sensor = lua_toboolean(L, 2);

	fixture->SetSensor(sensor);

	return 0;
}

int Box2DBinder2::b2Fixture_IsSensor(lua_State* L)
{
	StackChecker checker(L, "b2Fixture_IsSensor", 1);

	Binder binder(L);

	b2Fixture* fixture = toFixture(binder, 1);

	lua_pushboolean(L, fixture->IsSensor());

	return 1;
}




int Box2DBinder2::b2Fixture_SetFilterData(lua_State* L)
{
	StackChecker checker(L, "b2Fixture_SetFilterData", 0);

	Binder binder(L);

	b2Fixture* fixture = toFixture(binder, 1);
	b2Filter filter;
	tableToFilter(L, 2, &filter);

	fixture->SetFilterData(filter);

	return 0;
}

int Box2DBinder2::b2Fixture_GetFilterData(lua_State* L)
{
	StackChecker checker(L, "b2Fixture_GetFilterData", 1);

	Binder binder(L);

	b2Fixture* fixture = toFixture(binder, 1);

	const b2Filter& filter = fixture->GetFilterData();

	lua_newtable(L);

	lua_pushinteger(L, filter.categoryBits);
	lua_setfield(L, -2, "categoryBits");

	lua_pushinteger(L, filter.maskBits);
	lua_setfield(L, -2, "maskBits");

	lua_pushinteger(L, filter.groupIndex);
	lua_setfield(L, -2, "groupIndex");

	return 1;
}

int Box2DBinder2::b2Shape_destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2Shape* shape = static_cast<b2Shape*>(ptr);
	delete shape;

	return 0;
}


int Box2DBinder2::b2CircleShape_create(lua_State* L)
{
	StackChecker checker(L, "b2CircleShape_create", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);

	b2CircleShape* circleShape = new b2CircleShape;

	if (lua_gettop(L) >= 3)
	{
		lua_Number positionx = luaL_checknumber(L, 1) / physicsScale;
		lua_Number positiony = luaL_checknumber(L, 2) / physicsScale;
		lua_Number radius = luaL_checknumber(L, 3) / physicsScale;

		circleShape->m_p = b2Vec2(positionx, positiony);
		circleShape->m_radius = radius;
	}

	binder.pushInstance("b2CircleShape", circleShape);

	return 1;
}

int Box2DBinder2::b2CircleShape_destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2CircleShape* circleShape = static_cast<b2CircleShape*>(ptr);
	delete circleShape;

	return 0;
}

int Box2DBinder2::b2CircleShape_set(lua_State* L)
{
	StackChecker checker(L, "b2CircleShape_set", 0);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);

	b2CircleShape* circleShape = static_cast<b2CircleShape*>(binder.getInstance("b2CircleShape", 1));

	lua_Number positionx = luaL_checknumber(L, 2) / physicsScale;
	lua_Number positiony = luaL_checknumber(L, 3) / physicsScale;
	lua_Number radius = luaL_checknumber(L, 4) / physicsScale;

	circleShape->m_p = b2Vec2(positionx, positiony);
	circleShape->m_radius = radius;

	return 0;
}


int Box2DBinder2::b2PolygonShape_create(lua_State* L)
{
	StackChecker checker(L, "b2PolygonShape_create", 1);

	Binder binder(L);

	b2PolygonShape* polygonShape = new b2PolygonShape;

	binder.pushInstance("b2PolygonShape", polygonShape);

	return 1;
}

int Box2DBinder2::b2PolygonShape_destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2PolygonShape* polygonShape = static_cast<b2PolygonShape*>(ptr);
	delete polygonShape;

	return 0;
}


int Box2DBinder2::b2PolygonShape_SetAsBox(lua_State* L)
{
	StackChecker checker(L, "b2PolygonShape_SetAsBox", 0);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);
	b2PolygonShape* polygonShape = static_cast<b2PolygonShape*>(binder.getInstance("b2PolygonShape", 1));


	if (lua_gettop(L) >= 5)
	{
		lua_Number hx = luaL_checknumber(L, 2) / physicsScale;
		lua_Number hy = luaL_checknumber(L, 3) / physicsScale;
		lua_Number centerx = luaL_checknumber(L, 4) / physicsScale;
		lua_Number centery = luaL_checknumber(L, 5) / physicsScale;
		lua_Number angle = luaL_checknumber(L, 6);

		polygonShape->SetAsBox(hx, hy, b2Vec2(centerx, centery), angle);
	}
	else
	{
		lua_Number hx = luaL_checknumber(L, 2) / physicsScale;
		lua_Number hy = luaL_checknumber(L, 3) / physicsScale;

		polygonShape->SetAsBox(hx, hy);
	}

	return 0;
}

/*
int Box2DBinder2::b2PolygonShape_SetAsEdge(lua_State* L)
{
	StackChecker checker(L, "b2PolygonShape_SetAsEdge", 0);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);
	b2PolygonShape* polygonShape = static_cast<b2PolygonShape*>(binder.getInstance("b2PolygonShape", 1));

	lua_Number v1x = luaL_checknumber(L, 2) / physicsScale;
	lua_Number v1y = luaL_checknumber(L, 3) / physicsScale;
	lua_Number v2x = luaL_checknumber(L, 4) / physicsScale;
	lua_Number v2y = luaL_checknumber(L, 5) / physicsScale;

	polygonShape->SetAsEdge(b2Vec2(v1x, v1y), b2Vec2(v2x, v2y));

	return 0;
}
*/

int Box2DBinder2::b2PolygonShape_Set(lua_State* L)
{
	StackChecker checker(L, "b2PolygonShape_Set", 0);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);
	b2PolygonShape* polygonShape = static_cast<b2PolygonShape*>(binder.getInstance("b2PolygonShape", 1));

	std::vector<b2Vec2> vertices;

	int n = lua_gettop(L);

	b2Vec2 v;
	for (int i = 2; i <= n; i++)
	{
		lua_Number d = luaL_checknumber(L, i) / physicsScale;

		if (i % 2 == 0)
			v.x = d;
		else
		{
			v.y = d;
			vertices.push_back(v);
		}
	}

	// check polygon vertices
	{
		int32 count = vertices.size();
		int32 m_vertexCount = count;
		std::vector<b2Vec2>& m_vertices = vertices;

		if (!(3 <= count && count <= b2_maxPolygonVertices))
			return luaL_error(L, "Number of polygon vertices should be between 3 and 8.");

		// Compute normals. Ensure the edges have non-zero length.
		for (int32 i = 0; i < m_vertexCount; ++i)
		{
			int32 i1 = i;
			int32 i2 = i + 1 < m_vertexCount ? i + 1 : 0;
			b2Vec2 edge = m_vertices[i2] - m_vertices[i1];
			if (!(edge.LengthSquared() > b2_epsilon * b2_epsilon))
				return luaL_error(L, "Polygon edges should have non-zero length.");
		}

		// Ensure the polygon is convex and the interior
		// is to the left of each edge.
		for (int32 i = 0; i < m_vertexCount; ++i)
		{
			int32 i1 = i;
			int32 i2 = i + 1 < m_vertexCount ? i + 1 : 0;
			b2Vec2 edge = m_vertices[i2] - m_vertices[i1];

			for (int32 j = 0; j < m_vertexCount; ++j)
			{
				// Don't check vertices on the current edge.
				if (j == i1 || j == i2)
				{
					continue;
				}

				b2Vec2 r = m_vertices[j] - m_vertices[i1];

				// If this crashes, your polygon is non-convex, has colinear edges,
				// or the winding order is wrong.
				float32 s = b2Cross(edge, r);
				if (!(s > 0.0f))
					return luaL_error(L, "Polygon should be convex and should have a CCW winding order.");
			}
		}
	}

	polygonShape->Set(&vertices[0], vertices.size());

	return 0;
}





int Box2DBinder2::b2EdgeShape_create(lua_State* L)
{
	StackChecker checker(L, "b2EdgeShape_create", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);

	b2EdgeShape* edgeShape = new b2EdgeShape;

	if (lua_gettop(L) >= 4)
	{
		lua_Number v1x = luaL_checknumber(L, 1) / physicsScale;
		lua_Number v1y = luaL_checknumber(L, 2) / physicsScale;
		lua_Number v2x = luaL_checknumber(L, 3) / physicsScale;
		lua_Number v2y = luaL_checknumber(L, 4) / physicsScale;

		edgeShape->Set(b2Vec2(v1x, v1y), b2Vec2(v2x, v2y));
	}

	binder.pushInstance("b2EdgeShape", edgeShape);

	return 1;
}

int Box2DBinder2::b2EdgeShape_destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2EdgeShape* edgeShape = static_cast<b2EdgeShape*>(ptr);
	delete edgeShape;

	return 0;
}


int Box2DBinder2::b2EdgeShape_set(lua_State* L)
{
	StackChecker checker(L, "b2EdgeShape_set", 0);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);

	b2EdgeShape* edgeShape = static_cast<b2EdgeShape*>(binder.getInstance("b2EdgeShape", 1));
	lua_Number v1x = luaL_checknumber(L, 2) / physicsScale;
	lua_Number v1y = luaL_checknumber(L, 3) / physicsScale;
	lua_Number v2x = luaL_checknumber(L, 4) / physicsScale;
	lua_Number v2y = luaL_checknumber(L, 5) / physicsScale;

	edgeShape->Set(b2Vec2(v1x, v1y), b2Vec2(v2x, v2y));

	return 0;
}


int Box2DBinder2::b2ChainShape_create(lua_State* L)
{
    StackChecker checker(L, "b2ChainShape_create", 1);

    Binder binder(L);

    b2ChainShape* chainShape = new b2ChainShape;

    binder.pushInstance("b2ChainShape", chainShape);

    return 1;
}

int Box2DBinder2::b2ChainShape_destruct(lua_State* L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    b2ChainShape* chainShape = static_cast<b2ChainShape*>(ptr);
    delete chainShape;

    return 0;
}

int Box2DBinder2::b2ChainShape_createLoop(lua_State* L)
{
    StackChecker checker(L, "b2ChainShape_createLoop", 0);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);

    b2ChainShape* chainShape = static_cast<b2ChainShape*>(binder.getInstance("b2ChainShape", 1));

    std::vector<b2Vec2> vertices;

    int n = lua_gettop(L);

    b2Vec2 v;
    for (int i = 2; i <= n; i++)
    {
        lua_Number d = luaL_checknumber(L, i) / physicsScale;

        if (i % 2 == 0)
            v.x = d;
        else
        {
            v.y = d;
            vertices.push_back(v);
        }
    }

    if (!(vertices.size() >= 3))
        return luaL_error(L, "Number of vertices should be greater than or equal to 3.");

    if (!(chainShape->m_vertices == NULL && chainShape->m_count == 0))
        return luaL_error(L, "Vertices are set already.");

    chainShape->CreateLoop(&vertices[0], vertices.size());

    return 0;
}

int Box2DBinder2::b2ChainShape_createChain(lua_State* L)
{
    StackChecker checker(L, "b2ChainShape_createChain", 0);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);

    b2ChainShape* chainShape = static_cast<b2ChainShape*>(binder.getInstance("b2ChainShape", 1));

    std::vector<b2Vec2> vertices;

    int n = lua_gettop(L);

    b2Vec2 v;
    for (int i = 2; i <= n; i++)
    {
        lua_Number d = luaL_checknumber(L, i) / physicsScale;

        if (i % 2 == 0)
            v.x = d;
        else
        {
            v.y = d;
            vertices.push_back(v);
        }
    }

    if (!(vertices.size() >= 2))
        return luaL_error(L, "Number of vertices should be greater than or equal to 2.");

    if (!(chainShape->m_vertices == NULL && chainShape->m_count == 0))
        return luaL_error(L, "Vertices are set already.");

    chainShape->CreateChain(&vertices[0], vertices.size());

    return 0;
}


/*
int Box2DBinder2::b2AABB_create(lua_State* L)
{
	Binder binder(L);

	b2AABB* aabb = new b2AABB;
	aabb->lowerBound.SetZero();
	aabb->upperBound.SetZero();
	binder.pushInstance("b2AABB", aabb);

	return 1;
}

int Box2DBinder2::b2AABB_destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2AABB* aabb = static_cast<b2AABB*>(ptr);
	delete aabb;

	return 0;
}

int Box2DBinder2::b2AABB_GetLowerBound(lua_State* L)
{
	StackChecker checker(L, "b2AABB_GetLowerBound", 2);

	Binder binder(L);

	b2AABB* aabb = static_cast<b2AABB*>(binder.getInstance("b2AABB", 1));

	lua_pushnumber(L, aabb->lowerBound.x);
	lua_pushnumber(L, aabb->lowerBound.y);

	return 2;
}

int Box2DBinder2::b2AABB_GetUpperBound(lua_State* L)
{
	StackChecker checker(L, "b2AABB_GetUpperBound", 2);

	Binder binder(L);

	b2AABB* aabb = static_cast<b2AABB*>(binder.getInstance("b2AABB", 1));

	lua_pushnumber(L, aabb->upperBound.x);
	lua_pushnumber(L, aabb->upperBound.y);

	return 2;
}

int Box2DBinder2::b2AABB_SetLowerBound(lua_State* L)
{
	StackChecker checker(L, "b2AABB_SetLowerBound", 0);

	Binder binder(L);

	b2AABB* aabb = static_cast<b2AABB*>(binder.getInstance("b2AABB", 1));

	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	aabb->lowerBound.Set(x, y);

	return 0;
}

int Box2DBinder2::b2AABB_SetUpperBound(lua_State* L)
{
	StackChecker checker(L, "b2AABB_SetUpperBound", 0);

	Binder binder(L);

	b2AABB* aabb = static_cast<b2AABB*>(binder.getInstance("b2AABB", 1));

	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	aabb->upperBound.Set(x, y);

	return 0;
}

*/


int Box2DBinder2::b2World_createJoint(lua_State* L)
{
	StackChecker checker(L, "b2World_createJoint", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);

	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

	if (world->IsLocked())
		return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

	// table to jointdef
	int index = 2;

	lua_getfield(L, index, "type");
	b2JointType type = (b2JointType)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	if (type != e_revoluteJoint &&
		type != e_prismaticJoint &&
		type != e_distanceJoint &&
		type != e_pulleyJoint &&
		type != e_mouseJoint &&
		type != e_gearJoint &&
		type != e_wheelJoint &&
		type != e_weldJoint &&
		type != e_frictionJoint &&
        type != e_ropeJoint)
	{
		GStatus status(2008, "joint type");	// Parameter %s must be one of the accepted values.
		luaL_error(binder.L, status.errorString());
	}

	b2JointDef* jointDef = NULL;

	b2RevoluteJointDef revoluteJointDef;
	b2PrismaticJointDef prismaticJointDef;
	b2DistanceJointDef distanceJointDef;
	b2PulleyJointDef pulleyJointDef;
	b2MouseJointDef mouseJointDef;
	b2GearJointDef gearJointDef;
	b2WheelJointDef wheelJointDef;
	b2WeldJointDef weldJointDef;
	b2FrictionJointDef frictionJointDef;
    b2RopeJointDef ropeJointDef;

	switch (type)
	{
		case e_revoluteJoint:
			lua_getfield(L, index, "localAnchorA");
			if (!lua_isnil(L, -1))
			{
				revoluteJointDef.localAnchorA = tableToVec2(L, -1);
				revoluteJointDef.localAnchorA.x /= physicsScale;
				revoluteJointDef.localAnchorA.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "localAnchorB");
			if (!lua_isnil(L, -1))
			{
				revoluteJointDef.localAnchorB = tableToVec2(L, -1);
				revoluteJointDef.localAnchorB.x /= physicsScale;
				revoluteJointDef.localAnchorB.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "referenceAngle");
			if (!lua_isnil(L, -1))
				revoluteJointDef.referenceAngle = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "enableLimit");
			if (!lua_isnil(L, -1))
				revoluteJointDef.enableLimit = lua_toboolean(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "lowerAngle");
			if (!lua_isnil(L, -1))
				revoluteJointDef.lowerAngle = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "upperAngle");
			if (!lua_isnil(L, -1))
				revoluteJointDef.upperAngle = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "enableMotor");
			if (!lua_isnil(L, -1))
				revoluteJointDef.enableMotor = lua_toboolean(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "motorSpeed");
			if (!lua_isnil(L, -1))
				revoluteJointDef.motorSpeed = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "maxMotorTorque");
			if (!lua_isnil(L, -1))
				revoluteJointDef.maxMotorTorque = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			jointDef = &revoluteJointDef;

			break;
		case e_prismaticJoint:
			lua_getfield(L, index, "localAnchorA");
			if (!lua_isnil(L, -1))
			{
				prismaticJointDef.localAnchorA = tableToVec2(L, -1);
				prismaticJointDef.localAnchorA.x /= physicsScale;
				prismaticJointDef.localAnchorA.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "localAnchorB");
			if (!lua_isnil(L, -1))
			{
				prismaticJointDef.localAnchorB = tableToVec2(L, -1);
				prismaticJointDef.localAnchorB.x /= physicsScale;
				prismaticJointDef.localAnchorB.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "localAxisA");
			if (!lua_isnil(L, -1))
				prismaticJointDef.localAxisA = tableToVec2(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "referenceAngle");
			if (!lua_isnil(L, -1))
				prismaticJointDef.referenceAngle = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "enableLimit");
			if (!lua_isnil(L, -1))
				prismaticJointDef.enableLimit = lua_toboolean(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "lowerTranslation");
			if (!lua_isnil(L, -1))
				prismaticJointDef.lowerTranslation = luaL_checknumber(L, -1) / physicsScale;
			lua_pop(L, 1);

			lua_getfield(L, index, "upperTranslation");
			if (!lua_isnil(L, -1))
				prismaticJointDef.upperTranslation = luaL_checknumber(L, -1) / physicsScale;
			lua_pop(L, 1);

			lua_getfield(L, index, "enableMotor");
			if (!lua_isnil(L, -1))
				prismaticJointDef.enableMotor = lua_toboolean(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "maxMotorForce");
			if (!lua_isnil(L, -1))
				prismaticJointDef.maxMotorForce = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "motorSpeed");
			if (!lua_isnil(L, -1))
				prismaticJointDef.motorSpeed = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			jointDef = &prismaticJointDef;
			break;
		case e_distanceJoint:
			lua_getfield(L, index, "localAnchorA");
			if (!lua_isnil(L, -1))
			{
				distanceJointDef.localAnchorA = tableToVec2(L, -1);
				distanceJointDef.localAnchorA.x /= physicsScale;
				distanceJointDef.localAnchorA.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "localAnchorB");
			if (!lua_isnil(L, -1))
			{
				distanceJointDef.localAnchorB = tableToVec2(L, -1);
				distanceJointDef.localAnchorB.x /= physicsScale;
				distanceJointDef.localAnchorB.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "length");
			if (!lua_isnil(L, -1))
				distanceJointDef.length = luaL_checknumber(L, -1) / physicsScale;
			lua_pop(L, 1);

			lua_getfield(L, index, "frequencyHz");
			if (!lua_isnil(L, -1))
				distanceJointDef.frequencyHz = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "dampingRatio");
			if (!lua_isnil(L, -1))
				distanceJointDef.dampingRatio = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			jointDef = &distanceJointDef;
			break;
		case e_pulleyJoint:
			lua_getfield(L, index, "groundAnchorA");
			if (!lua_isnil(L, -1))
			{
				pulleyJointDef.groundAnchorA = tableToVec2(L, -1);
				pulleyJointDef.groundAnchorA.x /= physicsScale;
				pulleyJointDef.groundAnchorA.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "groundAnchorB");
			if (!lua_isnil(L, -1))
			{
				pulleyJointDef.groundAnchorB = tableToVec2(L, -1);
				pulleyJointDef.groundAnchorB.x /= physicsScale;
				pulleyJointDef.groundAnchorB.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "localAnchorA");
			if (!lua_isnil(L, -1))
			{
				pulleyJointDef.localAnchorA = tableToVec2(L, -1);
				pulleyJointDef.localAnchorA.x /= physicsScale;
				pulleyJointDef.localAnchorA.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "localAnchorB");
			if (!lua_isnil(L, -1))
			{
				pulleyJointDef.localAnchorB = tableToVec2(L, -1);
				pulleyJointDef.localAnchorB.x /= physicsScale;
				pulleyJointDef.localAnchorB.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "lengthA");
			if (!lua_isnil(L, -1))
				pulleyJointDef.lengthA = luaL_checknumber(L, -1) / physicsScale;
			lua_pop(L, 1);

			lua_getfield(L, index, "lengthB");
			if (!lua_isnil(L, -1))
				pulleyJointDef.lengthB = luaL_checknumber(L, -1) / physicsScale;
			lua_pop(L, 1);

			lua_getfield(L, index, "ratio");
			if (!lua_isnil(L, -1))
				pulleyJointDef.ratio = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			jointDef = &pulleyJointDef;
			break;
		case e_mouseJoint:
			lua_getfield(L, index, "target");
			if (!lua_isnil(L, -1))
			{
				mouseJointDef.target = tableToVec2(L, -1);
				mouseJointDef.target.x /= physicsScale;
				mouseJointDef.target.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "maxForce");
			if (!lua_isnil(L, -1))
				mouseJointDef.maxForce = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "frequencyHz");
			if (!lua_isnil(L, -1))
				mouseJointDef.frequencyHz = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "dampingRatio");
			if (!lua_isnil(L, -1))
				mouseJointDef.dampingRatio = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			jointDef = &mouseJointDef;
			break;
		case e_gearJoint:
			lua_getfield(L, index, "joint1");
			if (!lua_isnil(L, -1))
				gearJointDef.joint1 = toJoint(binder, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "joint2");
			if (!lua_isnil(L, -1))
				gearJointDef.joint2 = toJoint(binder, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "ratio");
			if (!lua_isnil(L, -1))
				gearJointDef.ratio = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			jointDef = &gearJointDef;
			break;
		case e_wheelJoint:
			lua_getfield(L, index, "localAnchorA");
			if (!lua_isnil(L, -1))
			{
				wheelJointDef.localAnchorA = tableToVec2(L, -1);
				wheelJointDef.localAnchorA.x /= physicsScale;
				wheelJointDef.localAnchorA.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "localAnchorB");
			if (!lua_isnil(L, -1))
			{
				wheelJointDef.localAnchorB = tableToVec2(L, -1);
				wheelJointDef.localAnchorB.x /= physicsScale;
				wheelJointDef.localAnchorB.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "localAxisA");
			if (!lua_isnil(L, -1))
				wheelJointDef.localAxisA = tableToVec2(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "enableMotor");
			if (!lua_isnil(L, -1))
				wheelJointDef.enableMotor = lua_toboolean(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "maxMotorTorque");
			if (!lua_isnil(L, -1))
				wheelJointDef.maxMotorTorque = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "motorSpeed");
			if (!lua_isnil(L, -1))
				wheelJointDef.motorSpeed = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "frequencyHz");
			if (!lua_isnil(L, -1))
				wheelJointDef.frequencyHz = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "dampingRatio");
			if (!lua_isnil(L, -1))
				wheelJointDef.dampingRatio = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			jointDef = &wheelJointDef;
			break;
		case e_weldJoint:
			lua_getfield(L, index, "localAnchorA");
			if (!lua_isnil(L, -1))
			{
				weldJointDef.localAnchorA = tableToVec2(L, -1);
				weldJointDef.localAnchorA.x /= physicsScale;
				weldJointDef.localAnchorA.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "localAnchorB");
			if (!lua_isnil(L, -1))
			{
				weldJointDef.localAnchorB = tableToVec2(L, -1);
				weldJointDef.localAnchorB.x /= physicsScale;
				weldJointDef.localAnchorB.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "referenceAngle");
			if (!lua_isnil(L, -1))
				weldJointDef.referenceAngle = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			jointDef = &weldJointDef;
			break;
		case e_frictionJoint:
			lua_getfield(L, index, "localAnchorA");
			if (!lua_isnil(L, -1))
			{
				frictionJointDef.localAnchorA = tableToVec2(L, -1);
				frictionJointDef.localAnchorA.x /= physicsScale;
				frictionJointDef.localAnchorA.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "localAnchorB");
			if (!lua_isnil(L, -1))
			{
				frictionJointDef.localAnchorB = tableToVec2(L, -1);
				frictionJointDef.localAnchorB.x /= physicsScale;
				frictionJointDef.localAnchorB.y /= physicsScale;
			}
			lua_pop(L, 1);

			lua_getfield(L, index, "maxForce");
			if (!lua_isnil(L, -1))
				frictionJointDef.maxForce = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, index, "maxTorque");
			if (!lua_isnil(L, -1))
				frictionJointDef.maxTorque = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			jointDef = &frictionJointDef;
			break;
        case e_ropeJoint:
            lua_getfield(L, index, "localAnchorA");
            if (!lua_isnil(L, -1))
            {
                ropeJointDef.localAnchorA = tableToVec2(L, -1);
                ropeJointDef.localAnchorA.x /= physicsScale;
                ropeJointDef.localAnchorA.y /= physicsScale;
            }
            lua_pop(L, 1);

            lua_getfield(L, index, "localAnchorB");
            if (!lua_isnil(L, -1))
            {
                ropeJointDef.localAnchorB = tableToVec2(L, -1);
                ropeJointDef.localAnchorB.x /= physicsScale;
                ropeJointDef.localAnchorB.y /= physicsScale;
            }
            lua_pop(L, 1);

            lua_getfield(L, index, "maxLength");
            if (!lua_isnil(L, -1))
                ropeJointDef.maxLength = luaL_checknumber(L, -1) / physicsScale;
            lua_pop(L, 1);

            jointDef = &ropeJointDef;
            break;
		default:
			break;
	}

	lua_getfield(L, index, "bodyA");
	if (lua_isnil(L, -1))
		luaL_error(L, "bodyA must exist in joint definition table");
	jointDef->bodyA = toBody(binder, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "bodyB");
	if (lua_isnil(L, -1))
		luaL_error(L, "bodyB must exist in joint definition table");
	jointDef->bodyB = toBody(binder, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "collideConnected");
	if (!lua_isnil(L, -1))
		jointDef->collideConnected = lua_toboolean(L, -1);
	lua_pop(L, 1);


	b2Joint* joint = world->CreateJoint(jointDef);
	joint->SetUserData(world);

	switch (type)
	{
		case e_revoluteJoint:
			binder.pushInstance("b2RevoluteJoint", joint);
			break;
		case e_prismaticJoint:
			binder.pushInstance("b2PrismaticJoint", joint);
			break;
		case e_distanceJoint:
			binder.pushInstance("b2DistanceJoint", joint);
			break;
		case e_pulleyJoint:
			binder.pushInstance("b2PulleyJoint", joint);
			break;
		case e_mouseJoint:
			binder.pushInstance("b2MouseJoint", joint);
			break;
		case e_gearJoint:
			binder.pushInstance("b2GearJoint", joint);
			break;
		case e_wheelJoint:
			binder.pushInstance("b2WheelJoint", joint);
			break;
		case e_weldJoint:
			binder.pushInstance("b2WeldJoint", joint);
			break;
		case e_frictionJoint:
			binder.pushInstance("b2FrictionJoint", joint);
			break;
        case e_ropeJoint:
            binder.pushInstance("b2RopeJoint", joint);
            break;
		default:
			break;
	}

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "__world");


	lua_getfield(L, index, "bodyA");
	lua_setfield(L, -2, "__bodyA");

	lua_getfield(L, index, "bodyB");
	lua_setfield(L, -2, "__bodyB");

	lua_getfield(L, 1, "__joints");
	lua_pushvalue(L, -2);
	lua_pushlightuserdata(L, joint);
	lua_settable(L, -3);
	lua_pop(L, 1);

	lua_pushlightuserdata(L, joint);
	lua_pushvalue(L, -2);
	setb2(L);

	return 1;
}

int Box2DBinder2::b2Joint_getType(lua_State* L)
{
	StackChecker checker(L, "b2Joint_getType", 1);

	Binder binder(L);

	b2Joint* joint = toJoint(binder, 1);

	switch (joint->GetType())
	{
		case e_unknownJoint:
			lua_pushnil(L);
			break;
		case e_revoluteJoint:
			lua_pushinteger(L, e_revoluteJoint);
			break;
		case e_prismaticJoint:
			lua_pushinteger(L, e_prismaticJoint);
			break;
		case e_distanceJoint:
			lua_pushinteger(L, e_distanceJoint);
			break;
		case e_pulleyJoint:
			lua_pushinteger(L, e_pulleyJoint);
			break;
		case e_mouseJoint:
			lua_pushinteger(L, e_mouseJoint);
			break;
		case e_gearJoint:
			lua_pushinteger(L, e_gearJoint);
			break;
		case e_wheelJoint:
			lua_pushinteger(L, e_wheelJoint);
			break;
		case e_weldJoint:
			lua_pushinteger(L, e_weldJoint);
			break;
		case e_frictionJoint:
			lua_pushinteger(L, e_frictionJoint);
			break;
        case e_ropeJoint:
            lua_pushinteger(L, e_ropeJoint);
            break;
		default:
			lua_pushnil(L);
			break;
	};

	return 1;
}

int Box2DBinder2::b2Joint_getBodyA(lua_State* L)
{
	StackChecker checker(L, "b2Joint_getBodyA", 1);

	Binder binder(L);

	toJoint(binder, 1);

	lua_getfield(L, 1, "__bodyA");

	return 1;
}
int Box2DBinder2::b2Joint_getBodyB(lua_State* L)
{
	StackChecker checker(L, "b2Joint_getBodyB", 1);

	Binder binder(L);

	toJoint(binder, 1);

	lua_getfield(L, 1, "__bodyB");

	return 1;
}
int Box2DBinder2::b2Joint_getAnchorA(lua_State* L)
{
	StackChecker checker(L, "b2Joint_getAnchorA", 2);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);

	b2Joint* joint = toJoint(binder, 1);

	b2Vec2 v = joint->GetAnchorA();

	lua_pushnumber(L, v.x * physicsScale);
	lua_pushnumber(L, v.y * physicsScale);

	return 2;
}
int Box2DBinder2::b2Joint_getAnchorB(lua_State* L)
{
	StackChecker checker(L, "b2Joint_getAnchorB", 2);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	Binder binder(L);

	b2Joint* joint = toJoint(binder, 1);

	b2Vec2 v = joint->GetAnchorB();

	lua_pushnumber(L, v.x * physicsScale);
	lua_pushnumber(L, v.y * physicsScale);

	return 2;
}
int Box2DBinder2::b2Joint_isActive(lua_State* L)
{
	StackChecker checker(L, "b2Joint_isActive", 1);

	Binder binder(L);

	b2Joint* joint = toJoint(binder, 1);

	lua_pushboolean(L, joint->IsActive());

	return 1;
}

int Box2DBinder2::b2Joint_getReactionForce(lua_State* L)
{
    Binder binder(L);
    b2Joint* joint = static_cast<b2Joint*>(toJoint(binder, 1, "b2Joint"));

    b2Vec2 force = joint->GetReactionForce(luaL_checknumber(L, 2));
    lua_pushnumber(L, force.x);
    lua_pushnumber(L, force.y);

    return 2;
}

int Box2DBinder2::b2Joint_getReactionTorque(lua_State* L)
{
    Binder binder(L);
    b2Joint* joint = static_cast<b2Joint*>(toJoint(binder, 1, "b2Joint"));

    lua_pushnumber(L, joint->GetReactionTorque(luaL_checknumber(L, 2)));

    return 1;
}

int Box2DBinder2::b2Joint_destruct(lua_State* L)
{
#if 0
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2Joint* joint = static_cast<b2Joint*>(ptr);
	if (joint != 0)
		static_cast<b2WorldED*>(joint->GetUserData())->DestroyJoint(joint);
#endif
	return 0;
}

int Box2DBinder2::b2World_destroyJoint(lua_State* L)
{
	StackChecker checker(L, "b2World_destroyJoint", 0);

	Binder binder(L);
	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));
	b2Joint* joint = toJoint(binder, 2);

	if (world->IsLocked())
		return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

	world->DestroyJoint(joint);

    binder.setInstance(2, NULL);	// set joint.__userdata = 0

    lua_pushnil(L);
    lua_setfield(L, 2, "__world");

    lua_pushnil(L);
    lua_setfield(L, 2, "__bodyA");

    lua_pushnil(L);
    lua_setfield(L, 2, "__bodyB");

	lua_getfield(L, 1, "__joints");
	lua_pushvalue(L, 2);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);

	lua_pushlightuserdata(L, joint);
	lua_pushnil(L);
	setb2(L);

	return 0;
}

int Box2DBinder2::getRevoluteJointDef(lua_State* L)
{
	StackChecker checker(L, "getRevoluteJointDef", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	b2RevoluteJointDef jd;

	Binder binder(L);

	b2Body* bodyA = toBody(binder, 1);
	b2Body* bodyB = toBody(binder, 2);
	b2Vec2 anchor(luaL_checknumber(L, 3) / physicsScale, luaL_checknumber(L, 4) / physicsScale);

	jd.Initialize(bodyA, bodyB, anchor);

	lua_newtable(L);

	lua_pushinteger(L, jd.type);
	lua_setfield(L, -2, "type");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "bodyA");

	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "bodyB");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorA.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorA.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorA");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorB.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorB.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorB");

	lua_pushnumber(L, jd.referenceAngle);
	lua_setfield(L, -2, "referenceAngle");

	return 1;
}

int Box2DBinder2::getPrismaticJointDef(lua_State* L)
{
	StackChecker checker(L, "getPrismaticJointDef", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	b2PrismaticJointDef jd;

	Binder binder(L);

	b2Body* bodyA = toBody(binder, 1);
	b2Body* bodyB = toBody(binder, 2);
	b2Vec2 anchor(luaL_checknumber(L, 3) / physicsScale, luaL_checknumber(L, 4) / physicsScale);
	b2Vec2 axis(luaL_checknumber(L, 5), luaL_checknumber(L, 6));

	jd.Initialize(bodyA, bodyB, anchor, axis);

	lua_newtable(L);

	lua_pushinteger(L, jd.type);
	lua_setfield(L, -2, "type");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "bodyA");

	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "bodyB");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorA.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorA.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorA");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorB.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorB.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorB");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAxisA.x);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAxisA.y);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAxisA");

	lua_pushnumber(L, jd.referenceAngle);
	lua_setfield(L, -2, "referenceAngle");

	return 1;
}

int Box2DBinder2::getDistanceJointDef(lua_State* L)
{
	StackChecker checker(L, "getDistanceJointDef", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	b2DistanceJointDef jd;

	Binder binder(L);

	b2Body* bodyA = toBody(binder, 1);
	b2Body* bodyB = toBody(binder, 2);
	b2Vec2 anchorA(luaL_checknumber(L, 3) / physicsScale, luaL_checknumber(L, 4) / physicsScale);
	b2Vec2 anchorB(luaL_checknumber(L, 5) / physicsScale, luaL_checknumber(L, 6) / physicsScale);

	jd.Initialize(bodyA, bodyB, anchorA, anchorB);

	lua_newtable(L);

	lua_pushinteger(L, jd.type);
	lua_setfield(L, -2, "type");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "bodyA");

	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "bodyB");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorA.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorA.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorA");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorB.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorB.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorB");

	lua_pushnumber(L, jd.length * physicsScale);
	lua_setfield(L, -2, "length");

	return 1;
}

int Box2DBinder2::getPulleyJointDef(lua_State* L)
{
	StackChecker checker(L, "getPulleyJointDef", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	b2PulleyJointDef jd;

	Binder binder(L);

	// TODO: box2d'de b2Assert(ratio > b2_epsilon); testi var. o test burada yapilmali

	b2Body* bodyA = toBody(binder, 1);
	b2Body* bodyB = toBody(binder, 2);

	b2Vec2 groundAnchorA(luaL_checknumber(L, 3) / physicsScale, luaL_checknumber(L, 4) / physicsScale);
	b2Vec2 groundAnchorB(luaL_checknumber(L, 5) / physicsScale, luaL_checknumber(L, 6) / physicsScale);

	b2Vec2 anchorA(luaL_checknumber(L, 7) / physicsScale, luaL_checknumber(L, 8) / physicsScale);
	b2Vec2 anchorB(luaL_checknumber(L, 9) / physicsScale, luaL_checknumber(L, 10) / physicsScale);

	lua_Number ratio = luaL_checknumber(L, 11);

	jd.Initialize(bodyA, bodyB, groundAnchorA, groundAnchorB, anchorA, anchorB, ratio);

	lua_newtable(L);

	lua_pushinteger(L, jd.type);
	lua_setfield(L, -2, "type");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "bodyA");

	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "bodyB");

	lua_newtable(L);
	lua_pushnumber(L, jd.groundAnchorA.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.groundAnchorA.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "groundAnchorA");

	lua_newtable(L);
	lua_pushnumber(L, jd.groundAnchorB.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.groundAnchorB.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "groundAnchorB");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorA.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorA.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorA");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorB.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorB.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorB");

	lua_pushnumber(L, jd.lengthA * physicsScale);
	lua_setfield(L, -2, "lengthA");

	lua_pushnumber(L, jd.lengthB * physicsScale);
	lua_setfield(L, -2, "lengthB");

	lua_pushnumber(L, jd.ratio);
	lua_setfield(L, -2, "ratio");

	return 1;
}

int Box2DBinder2::getMouseJointDef(lua_State* L)
{
	int nnil = std::max(0, 7 - lua_gettop(L));
	for (int i = 0; i < nnil; ++i)
		lua_pushnil(L);

	StackChecker checker(L, "getMouseJointDef", 1);

	Binder binder(L);

	/*b2Body* bodyA = */toBody(binder, 1);
	/*b2Body* bodyB = */toBody(binder, 2);

	lua_newtable(L);

	lua_pushinteger(L, e_mouseJoint);
	lua_setfield(L, -2, "type");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "bodyA");

	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "bodyB");

	lua_newtable(L);
	lua_pushnumber(L, luaL_checknumber(L, 3));
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, luaL_checknumber(L, 4));
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "target");

	lua_pushnumber(L, luaL_checknumber(L, 5));
	lua_setfield(L, -2, "maxForce");

	if (!lua_isnoneornil(L, 6))
	{
		lua_pushnumber(L, luaL_checknumber(L, 6));
		lua_setfield(L, -2, "frequencyHz");
	}

	if (!lua_isnoneornil(L, 7))
	{
		lua_pushnumber(L, luaL_checknumber(L, 7));
		lua_setfield(L, -2, "dampingRatio");
	}

	return 1;
}

int Box2DBinder2::getGearJointDef(lua_State* L)
{
	int nnil = std::max(0, 5 - lua_gettop(L));
	for (int i = 0; i < nnil; ++i)
		lua_pushnil(L);

	StackChecker checker(L, "getGearJointDef", 1);

	Binder binder(L);

	/*b2Body* bodyA = */toBody(binder, 1);
	/*b2Body* bodyB = */toBody(binder, 2);

	lua_newtable(L);

	lua_pushinteger(L, e_gearJoint);
	lua_setfield(L, -2, "type");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "bodyA");

	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "bodyB");

	/*b2Joint* joint1 = */toJoint(binder, 3);
	/*b2Joint* joint2 = */toJoint(binder, 4);

	lua_pushvalue(L, 3);
	lua_setfield(L, -2, "joint1");

	lua_pushvalue(L, 4);
	lua_setfield(L, -2, "joint2");

	if (!lua_isnoneornil(L, 5))
	{
		lua_pushnumber(L, luaL_checknumber(L, 5));
		lua_setfield(L, -2, "ratio");
	}

	return 1;
}


int Box2DBinder2::getWheelJointDef(lua_State* L)
{
	StackChecker checker(L, "getWheelJointDef", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	b2WheelJointDef jd;

	Binder binder(L);

	b2Body* bodyA = toBody(binder, 1);
	b2Body* bodyB = toBody(binder, 2);
	b2Vec2 anchor(luaL_checknumber(L, 3) / physicsScale, luaL_checknumber(L, 4) / physicsScale);
	b2Vec2 axis(luaL_checknumber(L, 5), luaL_checknumber(L, 6));

	jd.Initialize(bodyA, bodyB, anchor, axis);

	lua_newtable(L);

	lua_pushinteger(L, jd.type);
	lua_setfield(L, -2, "type");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "bodyA");

	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "bodyB");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorA.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorA.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorA");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorB.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorB.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorB");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAxisA.x);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAxisA.y);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAxisA");

	return 1;
}

int Box2DBinder2::getWeldJointDef(lua_State* L)
{
	StackChecker checker(L, "getWeldJointDef", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	b2WeldJointDef jd;

	Binder binder(L);

	b2Body* bodyA = toBody(binder, 1);
	b2Body* bodyB = toBody(binder, 2);
	b2Vec2 anchor(luaL_checknumber(L, 3) / physicsScale, luaL_checknumber(L, 4) / physicsScale);

	jd.Initialize(bodyA, bodyB, anchor);

	lua_newtable(L);

	lua_pushinteger(L, jd.type);
	lua_setfield(L, -2, "type");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "bodyA");

	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "bodyB");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorA.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorA.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorA");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorB.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorB.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorB");

	lua_pushnumber(L, jd.referenceAngle);
	lua_setfield(L, -2, "referenceAngle");

	return 1;
}

int Box2DBinder2::getFrictionJointDef(lua_State* L)
{
	StackChecker checker(L, "getFrictionJointDef", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	b2FrictionJointDef jd;

	Binder binder(L);

	b2Body* bodyA = toBody(binder, 1);
	b2Body* bodyB = toBody(binder, 2);
	b2Vec2 anchor(luaL_checknumber(L, 3) / physicsScale, luaL_checknumber(L, 4) / physicsScale);

	jd.Initialize(bodyA, bodyB, anchor);

	lua_newtable(L);

	lua_pushinteger(L, jd.type);
	lua_setfield(L, -2, "type");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "bodyA");

	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "bodyB");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorA.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorA.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorA");

	lua_newtable(L);
	lua_pushnumber(L, jd.localAnchorB.x * physicsScale);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, jd.localAnchorB.y * physicsScale);
	lua_setfield(L, -2, "y");
	lua_setfield(L, -2, "localAnchorB");

	return 1;
}

int Box2DBinder2::getRopeJointDef(lua_State* L)
{
    StackChecker checker(L, "getRopeJointDef", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);

    b2Body* bodyA = toBody(binder, 1);
    b2Body* bodyB = toBody(binder, 2);
    b2Vec2 anchorA(luaL_checknumber(L, 3) / physicsScale, luaL_checknumber(L, 4) / physicsScale);
    b2Vec2 anchorB(luaL_checknumber(L, 5) / physicsScale, luaL_checknumber(L, 6) / physicsScale);
    b2Vec2 localAnchorA = bodyA->GetLocalPoint(anchorA);
    b2Vec2 localAnchorB = bodyB->GetLocalPoint(anchorB);

    lua_newtable(L);

    lua_pushinteger(L, e_ropeJoint);
    lua_setfield(L, -2, "type");

    lua_pushvalue(L, 1);
    lua_setfield(L, -2, "bodyA");

    lua_pushvalue(L, 2);
    lua_setfield(L, -2, "bodyB");

    lua_newtable(L);
    lua_pushnumber(L, localAnchorA.x * physicsScale);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, localAnchorA.y * physicsScale);
    lua_setfield(L, -2, "y");
    lua_setfield(L, -2, "localAnchorA");

    lua_newtable(L);
    lua_pushnumber(L, localAnchorB.x * physicsScale);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, localAnchorB.y * physicsScale);
    lua_setfield(L, -2, "y");
    lua_setfield(L, -2, "localAnchorB");

    lua_pushnumber(L, luaL_checknumber(L, 7));
    lua_setfield(L, -2, "maxLength");

    return 1;
}


int Box2DBinder2::b2RevoluteJoint_getJointAngle(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	lua_pushnumber(L, joint->GetJointAngle());

	return 1;
}
int Box2DBinder2::b2RevoluteJoint_getJointSpeed(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	lua_pushnumber(L, joint->GetJointSpeed());

	return 1;
}
int Box2DBinder2::b2RevoluteJoint_isLimitEnabled(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	lua_pushboolean(L, joint->IsLimitEnabled());

	return 1;
}
int Box2DBinder2::b2RevoluteJoint_enableLimit(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	joint->EnableLimit(lua_toboolean(L, 2));

	return 0;
}
int Box2DBinder2::b2RevoluteJoint_getLimits(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	lua_pushnumber(L, joint->GetLowerLimit());
	lua_pushnumber(L, joint->GetUpperLimit());

	return 2;
}
int Box2DBinder2::b2RevoluteJoint_setLimits(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	joint->SetLimits(luaL_checknumber(L, 2), luaL_checknumber(L, 3));

	return 0;
}
int Box2DBinder2::b2RevoluteJoint_isMotorEnabled(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	lua_pushboolean(L, joint->IsMotorEnabled());

	return 1;
}
int Box2DBinder2::b2RevoluteJoint_enableMotor(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	joint->EnableMotor(lua_toboolean(L, 2));

	return 0;
}
int Box2DBinder2::b2RevoluteJoint_setMotorSpeed(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	joint->SetMotorSpeed(luaL_checknumber(L, 2));

	return 0;
}
int Box2DBinder2::b2RevoluteJoint_getMotorSpeed(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	lua_pushnumber(L, joint->GetMotorSpeed());

	return 1;
}
int Box2DBinder2::b2RevoluteJoint_setMaxMotorTorque(lua_State* L)
{
	Binder binder(L);
	b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

	joint->SetMaxMotorTorque(luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2RevoluteJoint_getMotorTorque(lua_State* L)
{
    Binder binder(L);
    b2RevoluteJoint* joint = static_cast<b2RevoluteJoint*>(toJoint(binder, 1, "b2RevoluteJoint"));

    lua_pushnumber(L, joint->GetMotorTorque(luaL_checknumber(L, 2)));

    return 1;
}


int Box2DBinder2::b2PrismaticJoint_getJointTranslation(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	lua_pushnumber(L, joint->GetJointTranslation() * physicsScale);

	return 1;
}
int Box2DBinder2::b2PrismaticJoint_getJointSpeed(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	lua_pushnumber(L, joint->GetJointSpeed());

	return 1;
}
int Box2DBinder2::b2PrismaticJoint_isLimitEnabled(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	lua_pushboolean(L, joint->IsLimitEnabled());

	return 0;
}
int Box2DBinder2::b2PrismaticJoint_enableLimit(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	joint->EnableLimit(lua_toboolean(L, 2));

	return 0;
}
int Box2DBinder2::b2PrismaticJoint_getLimits(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	lua_pushnumber(L, joint->GetLowerLimit() * physicsScale);
	lua_pushnumber(L, joint->GetUpperLimit() * physicsScale);

	return 2;
}
int Box2DBinder2::b2PrismaticJoint_setLimits(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	joint->SetLimits(luaL_checknumber(L, 2) / physicsScale, luaL_checknumber(L, 3) / physicsScale);

	return 0;
}
int Box2DBinder2::b2PrismaticJoint_isMotorEnabled(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	lua_pushboolean(L, joint->IsMotorEnabled());

	return 1;
}
int Box2DBinder2::b2PrismaticJoint_enableMotor(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	joint->EnableMotor(lua_toboolean(L, 2));

	return 0;
}
int Box2DBinder2::b2PrismaticJoint_setMotorSpeed(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	joint->SetMotorSpeed(luaL_checknumber(L, 2));

	return 0;
}
int Box2DBinder2::b2PrismaticJoint_getMotorSpeed(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	lua_pushnumber(L, joint->GetMotorSpeed());

	return 1;
}
int Box2DBinder2::b2PrismaticJoint_setMaxMotorForce(lua_State* L)
{
	Binder binder(L);
	b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

	joint->SetMaxMotorForce(luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2PrismaticJoint_getMotorForce(lua_State* L)
{
    Binder binder(L);
    b2PrismaticJoint* joint = static_cast<b2PrismaticJoint*>(toJoint(binder, 1, "b2PrismaticJoint"));

    lua_pushnumber(L, joint->GetMotorForce(luaL_checknumber(L, 2)));

    return 1;
}


int Box2DBinder2::b2DistanceJoint_setLength(lua_State* L)
{
	Binder binder(L);
	b2DistanceJoint* joint = static_cast<b2DistanceJoint*>(toJoint(binder, 1, "b2DistanceJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	joint->SetLength(luaL_checknumber(L, 2) / physicsScale);

	return 0;
}
int Box2DBinder2::b2DistanceJoint_getLength(lua_State* L)
{
	Binder binder(L);
	b2DistanceJoint* joint = static_cast<b2DistanceJoint*>(toJoint(binder, 1, "b2DistanceJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	lua_pushnumber(L, joint->GetLength() * physicsScale);

	return 1;
}
int Box2DBinder2::b2DistanceJoint_setFrequency(lua_State* L)
{
	Binder binder(L);
	b2DistanceJoint* joint = static_cast<b2DistanceJoint*>(toJoint(binder, 1, "b2DistanceJoint"));

	joint->SetFrequency(luaL_checknumber(L, 2));

	return 0;
}
int Box2DBinder2::b2DistanceJoint_getFrequency(lua_State* L)
{
	Binder binder(L);
	b2DistanceJoint* joint = static_cast<b2DistanceJoint*>(toJoint(binder, 1, "b2DistanceJoint"));

	lua_pushnumber(L, joint->GetFrequency());

	return 1;
}
int Box2DBinder2::b2DistanceJoint_setDampingRatio(lua_State* L)
{
	Binder binder(L);
	b2DistanceJoint* joint = static_cast<b2DistanceJoint*>(toJoint(binder, 1, "b2DistanceJoint"));

	joint->SetDampingRatio(luaL_checknumber(L, 2));

	return 0;
}
int Box2DBinder2::b2DistanceJoint_getDampingRatio(lua_State* L)
{
	Binder binder(L);
	b2DistanceJoint* joint = static_cast<b2DistanceJoint*>(toJoint(binder, 1, "b2DistanceJoint"));

	lua_pushnumber(L, joint->GetDampingRatio());

	return 1;
}

int Box2DBinder2::b2PulleyJoint_getGroundAnchorA(lua_State* L)
{
	Binder binder(L);
	b2PulleyJoint* joint = static_cast<b2PulleyJoint*>(toJoint(binder, 1, "b2PulleyJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	b2Vec2 v = joint->GetGroundAnchorA();

	lua_pushnumber(L, v.x * physicsScale);
	lua_pushnumber(L, v.y * physicsScale);

	return 2;
}

int Box2DBinder2::b2PulleyJoint_getGroundAnchorB(lua_State* L)
{
	Binder binder(L);
	b2PulleyJoint* joint = static_cast<b2PulleyJoint*>(toJoint(binder, 1, "b2PulleyJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	b2Vec2 v = joint->GetGroundAnchorB();

	lua_pushnumber(L, v.x * physicsScale);
	lua_pushnumber(L, v.y * physicsScale);

	return 2;
}


int Box2DBinder2::b2PulleyJoint_getLengthA(lua_State* L)
{
	Binder binder(L);
	b2PulleyJoint* joint = static_cast<b2PulleyJoint*>(toJoint(binder, 1, "b2PulleyJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	lua_pushnumber(L, joint->GetLengthA() * physicsScale);

	return 1;
}


int Box2DBinder2::b2PulleyJoint_getLengthB(lua_State* L)
{
	Binder binder(L);
	b2PulleyJoint* joint = static_cast<b2PulleyJoint*>(toJoint(binder, 1, "b2PulleyJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	lua_pushnumber(L, joint->GetLengthB() * physicsScale);

	return 1;
}


int Box2DBinder2::b2PulleyJoint_getRatio(lua_State* L)
{
	Binder binder(L);
	b2PulleyJoint* joint = static_cast<b2PulleyJoint*>(toJoint(binder, 1, "b2PulleyJoint"));

	lua_pushnumber(L, joint->GetRatio());

	return 1;
}

int Box2DBinder2::b2MouseJoint_setTarget(lua_State* L)
{
	Binder binder(L);
	b2MouseJoint* joint = static_cast<b2MouseJoint*>(toJoint(binder, 1, "b2MouseJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	joint->SetTarget(b2Vec2(luaL_checknumber(L, 2) / physicsScale, luaL_checknumber(L, 3) / physicsScale));

	return 0;
}
int Box2DBinder2::b2MouseJoint_getTarget(lua_State* L)
{
	Binder binder(L);
	b2MouseJoint* joint = static_cast<b2MouseJoint*>(toJoint(binder, 1, "b2MouseJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	b2Vec2 v = joint->GetTarget();

	lua_pushnumber(L, v.x * physicsScale);
	lua_pushnumber(L, v.y * physicsScale);

	return 2;
}
int Box2DBinder2::b2MouseJoint_setMaxForce(lua_State* L)
{
	Binder binder(L);
	b2MouseJoint* joint = static_cast<b2MouseJoint*>(toJoint(binder, 1, "b2MouseJoint"));

	joint->SetMaxForce(luaL_checknumber(L, 2));

	return 0;
}
int Box2DBinder2::b2MouseJoint_getMaxForce(lua_State* L)
{
	Binder binder(L);
	b2MouseJoint* joint = static_cast<b2MouseJoint*>(toJoint(binder, 1, "b2MouseJoint"));

	lua_pushnumber(L, joint->GetMaxForce());

	return 1;
}
int Box2DBinder2::b2MouseJoint_setFrequency(lua_State* L)
{
	Binder binder(L);
	b2MouseJoint* joint = static_cast<b2MouseJoint*>(toJoint(binder, 1, "b2MouseJoint"));

	joint->SetFrequency(luaL_checknumber(L, 2));

	return 0;
}
int Box2DBinder2::b2MouseJoint_getFrequency(lua_State* L)
{
	Binder binder(L);
	b2MouseJoint* joint = static_cast<b2MouseJoint*>(toJoint(binder, 1, "b2MouseJoint"));

	lua_pushnumber(L, joint->GetFrequency());

	return 1;
}
int Box2DBinder2::b2MouseJoint_setDampingRatio(lua_State* L)
{
	Binder binder(L);
	b2MouseJoint* joint = static_cast<b2MouseJoint*>(toJoint(binder, 1, "b2MouseJoint"));

	joint->SetDampingRatio(luaL_checknumber(L, 2));

	return 0;
}
int Box2DBinder2::b2MouseJoint_getDampingRatio(lua_State* L)
{
	Binder binder(L);
	b2MouseJoint* joint = static_cast<b2MouseJoint*>(toJoint(binder, 1, "b2MouseJoint"));

	lua_pushnumber(L, joint->GetDampingRatio());

	return 1;
}

int Box2DBinder2::b2GearJoint_setRatio(lua_State* L)
{
	Binder binder(L);
	b2GearJoint* joint = static_cast<b2GearJoint*>(toJoint(binder, 1, "b2GearJoint"));

	joint->SetRatio(luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2GearJoint_getRatio(lua_State* L)
{
	Binder binder(L);
	b2GearJoint* joint = static_cast<b2GearJoint*>(toJoint(binder, 1, "b2GearJoint"));

	lua_pushnumber(L, joint->GetRatio());

	return 1;
}

int Box2DBinder2::b2WheelJoint_getJointTranslation(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	float physicsScale = application->getPhysicsScale();

	lua_pushnumber(L, joint->GetJointTranslation() * physicsScale);

	return 1;
}
int Box2DBinder2::b2WheelJoint_getJointSpeed(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	lua_pushnumber(L, joint->GetJointSpeed());

	return 1;
}
int Box2DBinder2::b2WheelJoint_isMotorEnabled(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	lua_pushboolean(L, joint->IsMotorEnabled());

	return 1;
}
int Box2DBinder2::b2WheelJoint_enableMotor(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	joint->EnableMotor(lua_toboolean(L, 2));

	return 0;
}
int Box2DBinder2::b2WheelJoint_setMotorSpeed(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	joint->SetMotorSpeed(luaL_checknumber(L, 2));

	return 0;
}
int Box2DBinder2::b2WheelJoint_getMotorSpeed(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	lua_pushnumber(L, joint->GetMotorSpeed());

	return 1;
}

int Box2DBinder2::b2WheelJoint_setMaxMotorTorque(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	joint->SetMaxMotorTorque(luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2WheelJoint_getMaxMotorTorque(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	lua_pushnumber(L, joint->GetMaxMotorTorque());

	return 1;
}

int Box2DBinder2::b2WheelJoint_setSpringFrequencyHz(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	joint->SetSpringFrequencyHz(luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2WheelJoint_getSpringFrequencyHz(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	lua_pushnumber(L, joint->GetSpringFrequencyHz());

	return 1;
}

int Box2DBinder2::b2WheelJoint_setSpringDampingRatio(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	joint->SetSpringDampingRatio(luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2WheelJoint_getSpringDampingRatio(lua_State* L)
{
	Binder binder(L);
	b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

	lua_pushnumber(L, joint->GetSpringDampingRatio());

	return 1;
}

int Box2DBinder2::b2WheelJoint_getMotorTorque(lua_State* L)
{
    Binder binder(L);
    b2WheelJoint* joint = static_cast<b2WheelJoint*>(toJoint(binder, 1, "b2WheelJoint"));

    lua_pushnumber(L, joint->GetMotorTorque(luaL_checknumber(L, 2)));

    return 1;
}

int Box2DBinder2::b2WeldJoint_setFrequency(lua_State* L)
{
    Binder binder(L);
    b2WeldJoint* joint = static_cast<b2WeldJoint*>(toJoint(binder, 1, "b2WeldJoint"));

    joint->SetFrequency(luaL_checknumber(L, 2));

    return 0;
}

int Box2DBinder2::b2WeldJoint_getFrequency(lua_State* L)
{
    Binder binder(L);
    b2WeldJoint* joint = static_cast<b2WeldJoint*>(toJoint(binder, 1, "b2WeldJoint"));

    lua_pushnumber(L, joint->GetFrequency());

    return 1;
}

int Box2DBinder2::b2WeldJoint_setDampingRatio(lua_State* L)
{
    Binder binder(L);
    b2WeldJoint* joint = static_cast<b2WeldJoint*>(toJoint(binder, 1, "b2WeldJoint"));

    joint->SetDampingRatio(luaL_checknumber(L, 2));

    return 0;
}

int Box2DBinder2::b2WeldJoint_getDampingRatio(lua_State* L)
{
    Binder binder(L);
    b2WeldJoint* joint = static_cast<b2WeldJoint*>(toJoint(binder, 1, "b2WeldJoint"));

    lua_pushnumber(L, joint->GetDampingRatio());

    return 1;
}


int Box2DBinder2::b2FrictionJoint_setMaxForce(lua_State* L)
{
	Binder binder(L);
	b2FrictionJoint* joint = static_cast<b2FrictionJoint*>(toJoint(binder, 1, "b2FrictionJoint"));

	joint->SetMaxForce(luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2FrictionJoint_getMaxForce(lua_State* L)
{
	Binder binder(L);
	b2FrictionJoint* joint = static_cast<b2FrictionJoint*>(toJoint(binder, 1, "b2FrictionJoint"));

	lua_pushnumber(L, joint->GetMaxForce());

	return 1;
}

int Box2DBinder2::b2FrictionJoint_setMaxTorque(lua_State* L)
{
	Binder binder(L);
	b2FrictionJoint* joint = static_cast<b2FrictionJoint*>(toJoint(binder, 1, "b2FrictionJoint"));

	joint->SetMaxTorque(luaL_checknumber(L, 2));

	return 0;
}

int Box2DBinder2::b2FrictionJoint_getMaxTorque(lua_State* L)
{
	Binder binder(L);
	b2FrictionJoint* joint = static_cast<b2FrictionJoint*>(toJoint(binder, 1, "b2FrictionJoint"));

	lua_pushnumber(L, joint->GetMaxTorque());

	return 1;
}

int Box2DBinder2::b2RopeJoint_setMaxLength(lua_State* L)
{
    Binder binder(L);
    b2RopeJoint* joint = static_cast<b2RopeJoint*>(toJoint(binder, 1, "b2RopeJoint"));

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    joint->SetMaxLength(luaL_checknumber(L, 2) / physicsScale);

    return 0;
}

int Box2DBinder2::b2RopeJoint_getMaxLength(lua_State* L)
{
    Binder binder(L);
    b2RopeJoint* joint = static_cast<b2RopeJoint*>(toJoint(binder, 1, "b2RopeJoint"));

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    lua_pushnumber(L, joint->GetMaxLength() * physicsScale);

    return 1;
}


b2DebugDraw::b2DebugDraw(LuaApplication* application) :
    Sprite(application->getApplication()),
    world_(NULL),
    application_(application)
{
	SetFlags(e_shapeBit);
}

b2DebugDraw::~b2DebugDraw()
{
}

void b2DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	glPushColor();
	glMultColor(color.r, color.g, color.b,1);
	ShaderProgram::stdBasic->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 2,vertices, vertexCount, true, NULL);
	ShaderProgram::stdBasic->drawArrays(ShaderProgram::LineLoop, 0, vertexCount);
	glPopColor();
}

void b2DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	ShaderProgram::stdBasic->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT,2, vertices, vertexCount, true, NULL);

	glPushColor();
	glMultColor(color.r, color.g, color.b,0.5f);
	ShaderProgram::stdBasic->drawArrays(ShaderProgram::TriangleFan, 0, vertexCount);
	glPopColor();

	glPushColor();
	glMultColor(color.r, color.g, color.b,1);
	ShaderProgram::stdBasic->drawArrays(ShaderProgram::LineLoop, 0, vertexCount);
	glPopColor();
}

void b2DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	const float32 k_segments = 16.0f;
    const int vertexCount=16;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
	float32 theta = 0.0f;

	float				glVertices[vertexCount*2];
	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		glVertices[i*2]=v.x;
		glVertices[i*2+1]=v.y;
		theta += k_increment;
	}

	glPushColor();
	glMultColor(color.r, color.g, color.b,1);
	ShaderProgram::stdBasic->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT,2, glVertices, vertexCount, true, NULL);
	ShaderProgram::stdBasic->drawArrays(ShaderProgram::LineLoop, 0, vertexCount);
	glPopColor();
}

void b2DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	const float32 k_segments = 16.0f;
    const int vertexCount=16;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
	float32 theta = 0.0f;

	float				glVertices[vertexCount*2];
	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		glVertices[i*2]=v.x;
		glVertices[i*2+1]=v.y;
		theta += k_increment;
	}

	glPushColor();
	glMultColor(color.r, color.g, color.b,0.5f);
	ShaderProgram::stdBasic->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2, glVertices, vertexCount, true, NULL);
	ShaderProgram::stdBasic->drawArrays(ShaderProgram::TriangleFan, 0, vertexCount);
	glPopColor();
	glPushColor();
	glMultColor(color.r, color.g, color.b,1);
	ShaderProgram::stdBasic->drawArrays(ShaderProgram::LineLoop, 0, vertexCount);
	glPopColor();

	// Draw the axis line
	DrawSegment(center,center+radius*axis,color);
}

void b2DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	glPushColor();
	glMultColor(color.r, color.g, color.b,1);
	float				glVertices[] = {
		p1.x,p1.y,p2.x,p2.y
	};
	ShaderProgram::stdBasic->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2, glVertices, 2, true, NULL);
	ShaderProgram::stdBasic->drawArrays(ShaderProgram::Lines, 0, 2);
	glPopColor();
}

void b2DebugDraw::DrawTransform(const b2Transform& xf)
{
	b2Vec2 p1 = xf.p, p2;
	const float32 k_axisScale = 0.4f;

	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	DrawSegment(p1,p2,b2Color(1,0,0));

	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	DrawSegment(p1,p2,b2Color(0,1,0));
}

void b2DebugDraw::doDraw(const CurrentTransform& , float sx, float sy, float ex, float ey)
{
	if (world_)
	{

		float physicsScale = application_->getPhysicsScale();

		Matrix4 modelMat=ShaderEngine::Engine->getModel();
		float mc[16];
		memcpy(mc,modelMat.get(),16*sizeof(float));
		mc[0]*=physicsScale;
		mc[5]*=physicsScale;
		Matrix4 scaledMat;
		scaledMat.set(mc);
		ShaderEngine::Engine->setModel(scaledMat);

		world_->DrawDebugData();

		ShaderEngine::Engine->setModel(modelMat);
	}
}

int Box2DBinder2::b2DebugDraw_create(lua_State* L)
{
	Binder binder(L);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	b2DebugDraw* debugDraw = new b2DebugDraw(application);
	binder.pushInstance("b2DebugDraw", debugDraw);

	return 1;
}

int Box2DBinder2::b2DebugDraw_destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(ptr);
	debugDraw->unref();

	return 0;
}

int Box2DBinder2::b2World_setDebugDraw(lua_State* L)
{
    StackChecker checker(L, "b2World_setDebugDraw", 0);

	Binder binder(L);
	b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

	if (lua_isnoneornil(L, 2))
	{
		world->SetDebugDraw(NULL);
	}
	else
	{
		b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(binder.getInstance("b2DebugDraw", 2));
		world->SetDebugDraw(debugDraw);
	}

	return 0;
}


int Box2DBinder2::b2DebugDraw_setFlags(lua_State* L)
{
	Binder binder(L);
	b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(binder.getInstance("b2DebugDraw", 1));

	debugDraw->SetFlags(luaL_checkinteger(L, 2));

	return 0;
}

int Box2DBinder2::b2DebugDraw_getFlags(lua_State* L)
{
	Binder binder(L);
	b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(binder.getInstance("b2DebugDraw", 1));

	lua_pushinteger(L, debugDraw->GetFlags());

	return 1;
}

int Box2DBinder2::b2DebugDraw_appendFlags(lua_State* L)
{
	Binder binder(L);
	b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(binder.getInstance("b2DebugDraw", 1));

	debugDraw->AppendFlags(luaL_checkinteger(L, 2));

	return 0;
}

int Box2DBinder2::b2DebugDraw_clearFlags(lua_State* L)
{
	Binder binder(L);
	b2DebugDraw* debugDraw = static_cast<b2DebugDraw*>(binder.getInstance("b2DebugDraw", 1));

	debugDraw->ClearFlags(luaL_checkinteger(L, 2));

	return 0;
}


static b2Contact *toContact(const Binder& binder, int index)
{
    b2Contact *contact = static_cast<b2Contact*>(binder.getInstance("b2Contact", index));

    if (contact == NULL)
        luaL_error(binder.L, "Contact is not valid.");

    return contact;
}

int Box2DBinder2::b2Contact_getManifold(lua_State *L)
{
    StackChecker checker(L, "b2Contact_getManifold", 1);

    LuaApplication *application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    b2Manifold *manifold = contact->GetManifold();

    lua_getfield(L, 1, "__manifold");

    lua_getfield(L, -1, "points");

    for (int i = b2_maxManifoldPoints; i >= 1; --i)
    {
        lua_pushnil(L);
        lua_rawseti(L, -2, i);
    }

    for (int i = 0; i < manifold->pointCount; ++i)
    {
        lua_getfield(L, 1, "__points");
        lua_rawgeti(L, -1, i + 1);
        lua_getfield(L, -1, "localPoint");
        lua_pushnumber(L, manifold->points[i].localPoint.x * physicsScale);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, manifold->points[i].localPoint.y * physicsScale);
        lua_setfield(L, -2, "y");
        lua_pop(L, 1);
        lua_pushnumber(L, manifold->points[i].normalImpulse);
        lua_setfield(L, -2, "normalImpulse");
        lua_pushnumber(L, manifold->points[i].tangentImpulse);
        lua_setfield(L, -2, "tangentImpulse");
        lua_rawseti(L, -3, i + 1);
        lua_pop(L, 1);
    }

    lua_pop(L, 1);


    lua_getfield(L, -1, "localNormal");
    lua_pushnumber(L, manifold->localNormal.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, manifold->localNormal.y);
    lua_setfield(L, -2, "y");
    lua_pop(L, 1);

    lua_getfield(L, -1, "localPoint");
    lua_pushnumber(L, manifold->localPoint.x * physicsScale);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, manifold->localPoint.y * physicsScale);
    lua_setfield(L, -2, "y");
    lua_pop(L, 1);

    lua_pushinteger(L, manifold->type);
    lua_setfield(L, -2, "type");

    return 1;
}

int Box2DBinder2::b2Contact_getWorldManifold(lua_State *L)
{
    StackChecker checker(L, "b2Contact_getWorldManifold", 1);

    LuaApplication *application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    b2Manifold *manifold = contact->GetManifold();

    b2WorldManifold worldManifold;
    contact->GetWorldManifold(&worldManifold);

    lua_getfield(L, 1, "__worldManifold");

    lua_getfield(L, -1, "points");

    for (int i = b2_maxManifoldPoints; i >= 1; --i)
    {
        lua_pushnil(L);
        lua_rawseti(L, -2, i);
    }

    for (int i = 0; i < manifold->pointCount; ++i)
    {
        lua_getfield(L, 1, "__worldPoints");
        lua_rawgeti(L, -1, i + 1);
        lua_pushnumber(L, worldManifold.points[i].x * physicsScale);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, worldManifold.points[i].y * physicsScale);
        lua_setfield(L, -2, "y");
        lua_rawseti(L, -3, i + 1);
        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    lua_getfield(L, -1, "normal");
    lua_pushnumber(L, worldManifold.normal.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, worldManifold.normal.y);
    lua_setfield(L, -2, "y");
    lua_pop(L, 1);

    return 1;
}

int Box2DBinder2::b2Contact_isTouching(lua_State *L)
{
    StackChecker checker(L, "b2Contact_isTouching", 1);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    lua_pushboolean(L, contact->IsTouching());

    return 1;
}

int Box2DBinder2::b2Contact_setEnabled(lua_State *L)
{
    StackChecker checker(L, "b2Contact_setEnabled", 0);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    contact->SetEnabled(lua_toboolean2(L, 2));

    return 0;
}

int Box2DBinder2::b2Contact_isEnabled(lua_State *L)
{
    StackChecker checker(L, "b2Contact_isEnabled", 1);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    lua_pushboolean(L, contact->IsEnabled());

    return 1;
}

int Box2DBinder2::b2Contact_getFixtureA(lua_State *L)
{
    StackChecker checker(L, "b2Contact_getFixtureA", 1);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    getb2(L, contact->GetFixtureA());

    return 1;
}

int Box2DBinder2::b2Contact_getChildIndexA(lua_State *L)
{
    StackChecker checker(L, "b2Contact_getChildIndexA", 1);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    lua_pushinteger(L, contact->GetChildIndexA());

    return 1;
}

int Box2DBinder2::b2Contact_getFixtureB(lua_State *L)
{
    StackChecker checker(L, "b2Contact_getFixtureB", 1);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    getb2(L, contact->GetFixtureB());

    return 1;
}

int Box2DBinder2::b2Contact_getChildIndexB(lua_State *L)
{
    StackChecker checker(L, "b2Contact_getChildIndexB", 1);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    lua_pushinteger(L, contact->GetChildIndexB());

    return 1;
}

int Box2DBinder2::b2Contact_setFriction(lua_State *L)
{
    StackChecker checker(L, "b2Contact_setFriction", 0);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    contact->SetFriction(luaL_checknumber(L, 2));

    return 0;
}

int Box2DBinder2::b2Contact_getFriction(lua_State *L)
{
    StackChecker checker(L, "b2Contact_getFriction", 1);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    lua_pushnumber(L, contact->GetFriction());

    return 1;
}

int Box2DBinder2::b2Contact_resetFriction(lua_State *L)
{
    StackChecker checker(L, "b2Contact_resetFriction", 0);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    contact->ResetFriction();

    return 0;
}

int Box2DBinder2::b2Contact_setRestitution(lua_State *L)
{
    StackChecker checker(L, "b2Contact_setRestitution", 0);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    contact->SetRestitution(luaL_checknumber(L, 2));

    return 0;
}

int Box2DBinder2::b2Contact_getRestitution(lua_State *L)
{
    StackChecker checker(L, "b2Contact_getRestitution", 1);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    lua_pushnumber(L, contact->GetRestitution());

    return 1;
}

int Box2DBinder2::b2Contact_resetRestitution(lua_State *L)
{
    StackChecker checker(L, "b2Contact_resetRestitution", 0);

    Binder binder(L);

    b2Contact* contact = toContact(binder, 1);

    contact->ResetRestitution();

    return 0;
}

int Box2DBinder2::testOverlap(lua_State *L)
{
    StackChecker checker(L, "testOverlap", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    float physicsScale = application->getPhysicsScale();

    Binder binder(L);

    b2Shape *shapeA = toShape(binder, 1);
    int indexA = luaL_checkinteger(L, 2) - 1;
    b2Shape *shapeB = toShape(binder, 3);
    int indexB = luaL_checkinteger(L, 4) - 1;

    lua_Number xa = luaL_checknumber(L, 5) / physicsScale;
    lua_Number ya = luaL_checknumber(L, 6) / physicsScale;
    lua_Number aa = luaL_checknumber(L, 7);

    lua_Number xb = luaL_checknumber(L, 8) / physicsScale;
    lua_Number yb = luaL_checknumber(L, 9) / physicsScale;
    lua_Number ab = luaL_checknumber(L, 10);

    b2Transform xfA(b2Vec2(xa, ya), b2Rot(aa));
    b2Transform xfB(b2Vec2(xb, yb), b2Rot(ab));

    lua_pushboolean(L, b2TestOverlap(shapeA, indexA, shapeB, indexB, xfA, xfB));

    return 1;
}

#if BIND_LIQUIDFUN

static void tableToParticleSystemDef(lua_State* L, int index, b2ParticleSystemDef* particleDef, float physicsScale)
{
    Binder binder(L);

    const struct {
    	const char *name;
    	float *ptr;
    } fValues[14]=
    {
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

    lua_getfield(L, index, "radius");
    if (!lua_isnil(L, -1))
        particleDef->radius=luaL_checknumber(L, -1)/physicsScale;
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
    StackChecker checker(L, "b2World_createParticleSystem", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);
    b2WorldED* world = static_cast<b2WorldED*>(binder.getInstance("b2World", 1));

    if (world->IsLocked())
        return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

    b2ParticleSystemDef particleSystemDef;
    tableToParticleSystemDef(L, 2, &particleSystemDef, application->getPhysicsScale());

    b2ParticleSystem* particleSystem = world->CreateParticleSystem(&particleSystemDef);
    b2ParticleSystemSprite *ps=new b2ParticleSystemSprite(application,particleSystem,world);

    binder.pushInstance("b2ParticleSystem", ps);

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
    StackChecker checker(L, "b2ParticleSystem_createParticle", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(binder.getInstance("b2ParticleSystem", 1));


    b2ParticleDef particleDef;
    tableToParticleDef(L, 2, &particleDef, application->getPhysicsScale());

    if (ps->GetWorld()->IsLocked())
    	return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

    int32 p=ps->GetSystem()->CreateParticle(particleDef);
    lua_pushinteger(L, p);

    return 1;
}

int Box2DBinder2::b2ParticleSystem_destroyParticle(lua_State* L)
{
    StackChecker checker(L, "b2ParticleSystem_destroyParticle", 0);

    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(binder.getInstance("b2ParticleSystem", 1));

    ps->GetSystem()->DestroyParticle(luaL_checkinteger(L, 2));

    return 0;
}

int Box2DBinder2::b2ParticleSystem_setTexture(lua_State *L)
{
    StackChecker checker(L, "b2ParticleSystem_setTexture", 0);
    Binder binder(L);

    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(binder.getInstance("b2ParticleSystem", 1));
    TextureBase *textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 2));
    ps->SetTexture(textureBase);

    return 0;
}

int Box2DBinder2::b2ParticleSystem_getParticleGroupList(lua_State *L)
{
    StackChecker checker(L, "b2ParticleSystem_getParticleGroupList", 1);
    Binder binder(L);


    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(binder.getInstance("b2ParticleSystem", 1));
    if (ps == NULL)
    {
        return luaL_error(L, "ParticleSystem required in argument #1");
    }

    if (ps->GetWorld()->IsLocked())
        return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

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
    StackChecker checker(L, "b2ParticleGroup_destroyParticles", 0);
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    bool callDestructionListener = lua_toboolean(L, 2);

    group->DestroyParticles(callDestructionListener);

    return 0;
}

int Box2DBinder2::b2ParticleGroup_getParticleCount(lua_State* L)
{
    StackChecker checker(L, "b2ParticleGroup_getParticleCount", 1);
    Binder binder(L);

    b2ParticleGroup* group = static_cast<b2ParticleGroup*>(binder.getInstance("b2ParticleGroup", 1));
    lua_pushinteger(L, group->GetParticleCount());

    return 1;
}

int Box2DBinder2::b2ParticleGroup_containsParticle(lua_State* L)
{
    StackChecker checker(L, "b2ParticleGroup_containsParticle", 1);
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
    StackChecker checker(L, "b2ParticleSystem_createParticleGroup", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);
    b2ParticleSystemSprite* ps = static_cast<b2ParticleSystemSprite*>(binder.getInstance("b2ParticleSystem", 1));

    b2ParticleGroupDef particleGroupDef;
    tableToParticleGroupDef(L, 2, &particleGroupDef, application->getPhysicsScale());

    b2ParticleGroup* particleGroup =ps->GetSystem()->CreateParticleGroup(particleGroupDef);
    if (!particleGroup)
    	return luaL_error(L, GStatus(5004).errorString());	// Error #5004: World is locked.

    binder.pushInstance("b2ParticleGroup", particleGroup);

    return 1;
}

#endif

