#include "gstdio.h"
#include "application.h"
#include "lua.hpp"
#include "luaapplication.h"
#include "luautil.h"
#include "gplugin.h"
#include "binder.h"
#include "gglobal.h"
#include "glog.h"
#include <sprite.h>

#include "reactphysics3d/reactphysics3d.h"
using namespace reactphysics3d;
#include <utility>
#include <set>
#include <stdexcept>

#define _UNUSED(n)

static lua_State *L;
static char key_b2 = ' ';

static void getb2(lua_State* L) {
	//StackChecker checker(L, "getb2", 0);
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_b2);
	lua_pushvalue(L, -2);
	lua_rawget(L, -2);
	lua_remove(L, -2);
	lua_remove(L, -2);
}

static void getb2(lua_State* L, const void* ptr) {
	lua_pushlightuserdata(L, (void *) ptr);
	getb2(L);
}

static void setb2(lua_State* L) {
	//StackChecker checker(L, "setb2", -2);
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_b2);
	lua_pushvalue(L, -3);
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 3);
}

static PhysicsCommon physicsCommon;

class GidEventListener: public EventListener {
public:
	lua_State *L;
	int cbn;
	virtual void onContact(
			const CollisionCallback::CallbackData& ci) {
		int nbc=ci.getNbContactPairs();
		for (int k=0;k<nbc;k++) {
			CollisionCallback::ContactPair cp=ci.getContactPair(k);
			CollisionCallback::ContactPair::EventType ce=cp.getEventType();
			if ((ce==CollisionCallback::ContactPair::EventType::ContactStart)||(ce==CollisionCallback::ContactPair::EventType::ContactExit)) {
				lua_getref(L, cbn);
				lua_pushstring(L, (ce==CollisionCallback::ContactPair::EventType::ContactStart)?"beginContact":"endContact");
				lua_newtable(L);
				getb2(L, cp.getBody1());
				lua_setfield(L, -2, "body1");
				getb2(L, cp.getBody2());
				lua_setfield(L, -2, "body2");
				getb2(L, cp.getCollider1());
				lua_setfield(L, -2, "fixture1");
				getb2(L, cp.getCollider2());
				lua_setfield(L, -2, "fixture2");
				lua_call(L, 2, 0);
			}
		}
	}
	virtual void onTrigger(const OverlapCallback::CallbackData& ci)
	{
		int nbc=ci.getNbOverlappingPairs();
		for (int k=0;k<nbc;k++) {
			OverlapCallback::OverlapPair cp=ci.getOverlappingPair(k);
			OverlapCallback::OverlapPair::EventType ce=cp.getEventType();
			if ((ce==OverlapCallback::OverlapPair::EventType::OverlapStart)||(ce==OverlapCallback::OverlapPair::EventType::OverlapExit)) {
				lua_getref(L, cbn);
				lua_pushstring(L, (ce==OverlapCallback::OverlapPair::EventType::OverlapStart)?"beginTrigger":"endTrigger");
				lua_newtable(L);
				getb2(L, cp.getBody1());
				lua_setfield(L, -2, "body1");
				getb2(L, cp.getBody2());
				lua_setfield(L, -2, "body2");
				getb2(L, cp.getCollider1());
				lua_setfield(L, -2, "fixture1");
				getb2(L, cp.getCollider2());
				lua_setfield(L, -2, "fixture2");
				lua_call(L, 2, 0);
			}
		}
	}
};
static std::map<rp3d::PhysicsWorld *, GidEventListener *> events;
class GidExtraData {
public:
	virtual ~GidExtraData() { };
};
static std::map<rp3d::CollisionShape *, GidExtraData *> shapeData;
enum ShapeType {
	SHP_BOX,
	SHP_SPHERE,
	SHP_CAPSULE,
	SHP_CONCAVE,
	SHP_CONVEX,
	SHP_HEIGHTFIELD
};
static std::map<rp3d::CollisionShape *, ShapeType> shapeType;

static int r3dWorld_create(lua_State* L) {
	Binder binder(L);

	PhysicsWorld::WorldSettings ws;
	ws.gravity.x = luaL_checknumber(L, 1);
	ws.gravity.y = luaL_checknumber(L, 2);
	ws.gravity.z = luaL_checknumber(L, 3);
	if (lua_type(L,4)==LUA_TTABLE) {
#define PINTT(n,t) lua_getfield(L,4,#n); if (!lua_isnoneornil(L,-1)) ws.n=(t) luaL_checkinteger(L,-1); lua_pop(L,1);
#define PNUM(n) lua_getfield(L,4,#n); if (!lua_isnoneornil(L,-1)) ws.n=luaL_checknumber(L,-1); lua_pop(L,1);
#define PBOOL(n) lua_getfield(L,4,#n); ws.n=lua_toboolean(L,-1); lua_pop(L,1);
#define PUINT(n) PINTT(n,uint)
		PNUM(persistentContactDistanceThreshold);
        /// Default friction coefficient for a rigid body
		PNUM(defaultFrictionCoefficient);
        /// Default bounciness factor for a rigid body
		PNUM(defaultBounciness);
        /// Velocity threshold for contact velocity restitution
		PNUM(restitutionVelocityThreshold);
        /// True if the sleeping technique is enabled
		PBOOL(isSleepingEnabled);
        /// Number of iterations when solving the velocity constraints of the Sequential Impulse technique
		PUINT(defaultVelocitySolverNbIterations);
        /// Number of iterations when solving the position constraints of the Sequential Impulse technique
		PUINT(defaultPositionSolverNbIterations);
        /// Time (in seconds) that a body must stay still to be considered sleeping
		PNUM(defaultTimeBeforeSleep);
        /// A body with a linear velocity smaller than the sleep linear velocity (in m/s)
        /// might enter sleeping mode.
        PNUM(defaultSleepLinearVelocity);
        /// A body with angular velocity smaller than the sleep angular velocity (in rad/s)
        /// might enter sleeping mode
        PNUM(defaultSleepAngularVelocity);
        /// This is used to test if two contact manifold are similar (same contact normal) in order to
        /// merge them. If the cosine of the angle between the normals of the two manifold are larger
        /// than the value bellow, the manifold are considered to be similar.
        PNUM(cosAngleSimilarContactManifold);
#undef PNUM
#undef PBOOL
#undef PUINT
#undef PINTT
	}

	rp3d::PhysicsWorld *world = physicsCommon.createPhysicsWorld(ws);

	binder.pushInstance("r3dWorld", world);
    lua_newtable(L);
    lua_setfield(L, -2, "__joints");

	return 1;
}

static int r3dWorld_destruct(void *p) {
	void* ptr = GIDEROS_DTOR_UDATA(p);
	rp3d::PhysicsWorld* world = static_cast<rp3d::PhysicsWorld*>(ptr);
	GidEventListener *e = events[world];
	if (e) {
		if (!lua_isclosing(L))
			lua_unref(L, e->cbn);
		delete e;
		events[world] = NULL;
	}
	physicsCommon.destroyPhysicsWorld(world);

	return 0;
}

static int r3dWorld_SetEventListener(lua_State* L) {
	Binder binder(L);
	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	GidEventListener *e = events[world];
	if (e)
		lua_unref(L, e->cbn);
	if (lua_isnoneornil(L,2)) {
		if (e) {
			delete e;
			events[world] = NULL;
		}
		world->setEventListener(NULL);
	} else {
		if (!e)
			e = new GidEventListener();
		luaL_checktype(L, 2, LUA_TFUNCTION);
        e->cbn = lua_ref(L,2);
		events[world] = e;
		world->setEventListener(e);
	}

	return 0;
}

#define TORAD (3.141592654/180)
#define GETFIELD(n,f,t) \
	lua_getfield(L,n,f); \
	t=luaL_optnumber(L,-1,0); \
	lua_pop(L,1);

#define PUTFIELD(n,f,t) \
	lua_pushnumber(L,t); \
	lua_setfield(L,n,f);
#define TO_VECTOR(L,n,v) v.x=luaL_checknumber(L,n); v.y=luaL_checknumber(L,n+1); v.z=luaL_checknumber(L,n+2);


#define OBJ_SETNUM(obj,robj,fct) \
static int r3d##obj##_Set##fct(lua_State* L) {\
	Binder binder(L);\
	rp3d::robj* body = static_cast<rp3d::robj*>(binder.getInstance("r3d" #obj, 1));\
	body->set##fct((float)(luaL_checknumber(L, 2)));\
	return 0;\
}

#define OBJ_GETNUM(obj,robj,fct) \
static int r3d##obj##_Get##fct(lua_State* L) {\
	Binder binder(L);\
	rp3d::robj* body = static_cast<rp3d::robj*>(binder.getInstance("r3d" #obj, 1));\
	lua_pushnumber(L,body->get##fct());\
	return 1;\
}

#define OBJ_SETGETNUM(obj,robj,fct) OBJ_SETNUM(obj,robj,fct) OBJ_GETNUM(obj,robj,fct)

#define OBJ_SETVEC3(obj,robj,fct) \
static int r3d##obj##_Set##fct(lua_State* L) {\
	Binder binder(L);\
	rp3d::robj* body = static_cast<rp3d::robj*>(binder.getInstance("r3d" #obj, 1));\
	rp3d::Vector3 v;\
	TO_VECTOR(L, 2, v);\
	body->set##fct(v);\
	return 0;\
}

#define OBJ_GETVEC3(obj,robj,fct) \
static int r3d##obj##_Get##fct(lua_State* L) {\
	Binder binder(L);\
	rp3d::robj* body = static_cast<rp3d::robj*>(binder.getInstance("r3d" #obj, 1));\
	rp3d::Vector3 v=body->get##fct();\
	lua_pushnumber(L,v.x);\
	lua_pushnumber(L,v.y);\
	lua_pushnumber(L,v.z);\
	return 3;\
}
#define OBJ_SETGETVEC3(obj,robj,fct) OBJ_SETVEC3(obj,robj,fct) OBJ_GETVEC3(obj,robj,fct)

#define WORLD_SETGETVEC3(fct) OBJ_SETGETVEC3(World,PhysicsWorld,fct)

WORLD_SETGETVEC3(Gravity);


static void toTransform(lua_State *L, int n, rp3d::Transform &transform) {
	Binder binder(L);
	lua_pushvalue(L, n);
	if (binder.isInstanceOf("Matrix", -1)) {
		float mat[16];
		lua_getfield(L, -1, "getMatrix");
		lua_pushvalue(L, -2);
		lua_call(L, 1, 16);
		for (size_t k = 0; k < 16; k++)
			mat[k] = luaL_optnumber(L, -16 + k, 0);
		lua_pop(L, 17);
		transform.setFromOpenGL(mat);
	}
	else
		lua_pop(L,1);
}

#define CHECK_BODY(body,argNum) if (!body) luaL_errorL(L,"Body at argument %d as been destroyed",argNum);
static int r3dWorld_CreateBody(lua_State* L) {
	Binder binder(L);
	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::Transform transform;
	toTransform(L, 2, transform);
	rp3d::RigidBody *body = world->createRigidBody(transform);
	binder.pushInstance("r3dBody", body);

	lua_newtable(L);
	lua_setfield(L, -2, "__fixtures");
	lua_newtable(L);
	lua_setfield(L, -2, "__joints");
    lua_pushvalue(L,1);
	lua_setfield(L, -2, "__world"); //Retain world

    body->setUserData(world);

	lua_pushlightuserdata(L, body);
	lua_pushvalue(L, -2);
    setb2(L);

	return 1;
}

static int r3dWorld_DestroyBody(lua_State* L) {
	Binder binder(L);
	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
	if (!body) return 0; //Sanity check in case we try to destroy it twice
	//Clear joints for world table
	lua_getfield(L, 1, "__joints");
	lua_getfield(L, 2, "__joints");
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		lua_pop(L, 1);
		lua_pushvalue(L, -1);
		lua_pushnil(L);
		lua_rawset(L, -4);
	}
	//Remove world reference
	lua_pushnil(L);
	lua_setfield(L, 2, "__world");

    body->setUserData(NULL);
	world->destroyRigidBody(body);

	binder.setInstance(2, NULL);

	lua_pushlightuserdata(L, body);
	lua_pushnil(L);
    setb2(L);

	return 0;
}

static int r3dWorld_Step(lua_State* L) {
	Binder binder(L);
	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	GidEventListener *e = events[world];
	if (e)
		e->L = L;
#ifndef _NO_THROW
	try {
#endif
		float step=luaL_checknumber(L, 2);
		if (step==0) step=0.001; //Step cannot be 0, use a dummy tiny step instead
		world->update(step);
#ifndef _NO_THROW
    } catch (std::exception &e) {
        throw;
    } catch (std::runtime_error &e) {
		luaL_error(L,"Failed to step world, something is not set up correctly");
	} catch (...) {
		luaL_error(L,"Failed to step world, something is not set up correctly");
	}
#endif

	return 0;
}


class GidCollisionCallback: public CollisionCallback {
	lua_State *L;
	int cbn;
public:
	GidCollisionCallback(lua_State *L,int cbn) { this->L=L; this->cbn=cbn; }
	void onContact(const CallbackData& ci) {
		int nbc=ci.getNbContactPairs();
		for (int k=0;k<nbc;k++) {
			CollisionCallback::ContactPair cp=ci.getContactPair(k);
			lua_pushvalue(L, cbn);
			lua_newtable(L);
			getb2(L, cp.getBody1());
			lua_setfield(L, -2, "body1");
			getb2(L, cp.getBody2());
			lua_setfield(L, -2, "body2");
			getb2(L, cp.getCollider1());
			lua_setfield(L, -2, "fixture1");
			getb2(L, cp.getCollider2());
			lua_setfield(L, -2, "fixture2");
			lua_call(L, 1, 0);
		}
	}
};

static int r3dWorld_TestOverlap(lua_State* L) {
	Binder binder(L);
	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
    CHECK_BODY(body1,2);
	rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
    CHECK_BODY(body2,3);
    lua_pushboolean(L,world->testOverlap(body1, body2));

	return 1;
}

static int r3dWorld_TestCollision(lua_State* L) {
	Binder binder(L);
	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	if (lua_type(L,2)==LUA_TFUNCTION) {
		GidCollisionCallback gcb(L,2);
		world->testCollision(gcb);
	}
	else {
		rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
				"r3dBody", 2));
        CHECK_BODY(body1,2);
        if (lua_type(L,3)==LUA_TFUNCTION) {
			GidCollisionCallback gcb(L,3);
			world->testCollision(body1, gcb);
		}
		else {
			rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
					"r3dBody", 3));
            CHECK_BODY(body2,3);
            luaL_checktype(L,4,LUA_TFUNCTION);
			GidCollisionCallback gcb(L,4);
			world->testCollision(body1, body2, gcb);

		}
	}
	return 0;
}

static void push_Vector(lua_State *L, const rp3d::Vector3 &v) {
	lua_newtable(L);
	lua_pushnumber(L, v.x);
	lua_rawseti(L, -2, 1);
	lua_pushnumber(L, v.y);
	lua_rawseti(L, -2, 2);
	lua_pushnumber(L, v.z);
	lua_rawseti(L, -2, 3);
}

static void push_RayCastInfo(lua_State *L, const RaycastInfo& info) {
	lua_newtable(L);
	push_Vector(L, info.worldPoint);
	lua_setfield(L, -2, "worldPoint");
	push_Vector(L, info.worldNormal);
	lua_setfield(L, -2, "worldNormal");
	lua_pushnumber(L, info.hitFraction);
	lua_setfield(L, -2, "hitFraction");
	lua_pushinteger(L, info.meshSubpart);
	lua_setfield(L, -2, "meshSubpart");
	lua_pushinteger(L, info.triangleIndex);
	lua_setfield(L, -2, "triangleIndex");
	getb2(L, info.body);
	lua_setfield(L, -2, "body");
	getb2(L, info.collider);
	lua_setfield(L, -2, "fixture");
}

class GidRayCastCallback: public rp3d::RaycastCallback {
	lua_State *L;
	int cbn;
public:
	GidRayCastCallback(lua_State *L, int cbn) {
		this->L = L;
		this->cbn = cbn;
	}
	virtual decimal notifyRaycastHit(const RaycastInfo& info) {
		lua_pushvalue(L, cbn);
		push_RayCastInfo(L, info);
		lua_call(L, 1, 1);
		decimal ret = luaL_optnumber(L, -1, 1.0);
		lua_pop(L, 1);
		return ret;
	}
};

static int r3dWorld_RayCast(lua_State* L) {
	Binder binder(L);
	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::Vector3 rs, re;
	TO_VECTOR(L, 2, rs);
	TO_VECTOR(L, 5, re);
	luaL_checktype(L, 8, LUA_TFUNCTION);
	unsigned short cat = luaL_optinteger(L, 9, 0xFFFF);

	Ray ray(rs, re);
	GidRayCastCallback cb(L, 8);
	world->raycast(ray, &cb, cat);

	return 0;
}

static int r3dBody_CreateFixture(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	rp3d::CollisionShape* shape =
			static_cast<rp3d::CollisionShape*>(binder.getInstance("r3dShape", 2));
	rp3d::Transform transform;
	toTransform(L, 3, transform);
	rp3d::Collider *fixture = body->addCollider(shape, transform);
	binder.pushInstance("r3dFixture", fixture);
	//Link shape to body
	lua_getfield(L, 1, "__fixtures");
	lua_pushvalue(L, -2);
	lua_pushvalue(L, 2);
	lua_settable(L, -3);
	lua_pop(L, 1);

	lua_pushlightuserdata(L, fixture);
	lua_pushvalue(L, -2);
	setb2(L);

	return 1;
}

static int r3dBody_DestroyFixture(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Collider* shape = static_cast<rp3d::Collider*>(binder.getInstance(
			"r3dFixture", 2));
	body->removeCollider(shape);

	//Unlink shape from body
	lua_getfield(L, 1, "__fixtures");
	lua_pushvalue(L, 2);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);

	binder.setInstance(2, NULL);

	lua_pushlightuserdata(L, shape);
	lua_pushnil(L);
	setb2(L);

	return 0;
}

static int r3dBody_GetTransform(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    const rp3d::Transform t = body->getTransform();
	float mat[16];
	t.getOpenGLMatrix(mat);

	lua_getglobal(L, "Matrix");
	lua_getfield(L, -1, "new");
	lua_call(L, 0, 1);
	lua_getfield(L, -1, "setMatrix");
	lua_pushvalue(L, -2);
	for (size_t k = 0; k < 16; k++)
		lua_pushnumber(L, mat[k]);
	lua_call(L, 17, 0);
	lua_remove(L, -2);
	return 1;
}

static int r3dBody_SetTransform(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Transform transform;
	toTransform(L, 2, transform);
	body->setTransform(transform);
	return 0;
}

static int r3dBody_SetType(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    body->setType((BodyType) luaL_checkinteger(L, 2));
	return 0;
}

static int r3dBody_UpdateMassPropertiesFromColliders(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    body->updateMassPropertiesFromColliders();
	return 0;
}

static int r3dBody_destruct(void *p) {
	if (lua_isclosing(L)) return 0; //Worlds and all their bodies are going to be destroyed anyway
    void* ptr = GIDEROS_DTOR_UDATA(p);
    if (!ptr) return 0; //Was already destroyed
	lua_checkstack(L,16);
	getb2(L,ptr);
    if (lua_isnil(L,-1)) {
      //Body has been GC'ed, check if still live in the world
      rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(ptr);
      rp3d::PhysicsWorld* world = static_cast<rp3d::PhysicsWorld*>(body->getUserData());
      if (world!=NULL) { //Not yet destroyed
          body->setUserData(NULL);
          world->destroyRigidBody(body);
      }
    }
    else {
        lua_getfield(L, -1, "__world");
        if (!lua_isnil(L,-1))
        {
            lua_getfield(L,-1,"destroyBody");
            lua_pushvalue(L,-2);
            lua_pushvalue(L,-4);
            lua_call(L,2,0);
        }
    }
    lua_pop(L,1);
    return 0;
}

static int r3dFixture_GetMaterial(lua_State* L) {
	Binder binder(L);
	rp3d::Collider* shape = static_cast<rp3d::Collider*>(binder.getInstance(
			"r3dFixture", 1));
	rp3d::Material& mat = shape->getMaterial();
	lua_newtable(L);
	lua_pushnumber(L, mat.getBounciness());
	lua_setfield(L, -2, "bounciness");
	lua_pushnumber(L, mat.getFrictionCoefficient());
	lua_setfield(L, -2, "frictionCoefficient");
	lua_pushnumber(L, mat.getMassDensity());
	lua_setfield(L, -2, "massDensity");
	return 1;
}

static int r3dFixture_SetMaterial(lua_State* L) {
	Binder binder(L);
	rp3d::Collider* shape = static_cast<rp3d::Collider*>(binder.getInstance(
			"r3dFixture", 1));
	rp3d::Material& mat = shape->getMaterial();
	lua_getfield(L, 2, "bounciness");
	if (!lua_isnil(L,-1))
		mat.setBounciness(luaL_checknumber(L, -1));
	lua_pop(L, 1);
	lua_getfield(L, 2, "frictionCoefficient");
	if (!lua_isnil(L,-1))
		mat.setFrictionCoefficient(luaL_checknumber(L, -1));
	lua_pop(L, 1);
	lua_getfield(L, 2, "massDensity");
	if (!lua_isnil(L,-1))
		mat.setMassDensity(luaL_checknumber(L, -1));
	lua_pop(L, 1);
	return 0;
}

static int r3dFixture_SetIsTrigger(lua_State* L) {
	Binder binder(L);
	rp3d::Collider* shape = static_cast<rp3d::Collider*>(binder.getInstance(
			"r3dFixture", 1));
	shape->setIsTrigger(lua_toboolean(L, 2));
	return 0;
}

static int r3dBody_EnableGravity(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    body->enableGravity(lua_toboolean(L, 2));
	return 0;
}

#define BODY_SETGETVEC3(fct) OBJ_SETGETVEC3(Body,RigidBody,fct)
#define BODY_SETGETNUM(fct) OBJ_SETGETNUM(Body,RigidBody,fct)
#define BODY_GETVEC3(fct) OBJ_GETVEC3(Body,RigidBody,fct)

BODY_SETGETNUM(LinearDamping)
BODY_SETGETNUM(AngularDamping)
BODY_SETGETNUM(Mass)
BODY_SETGETVEC3(LinearVelocity)
BODY_SETGETVEC3(AngularVelocity)
BODY_GETVEC3(Force)
BODY_GETVEC3(Torque)
BODY_SETGETVEC3(LinearLockAxisFactor)
BODY_SETGETVEC3(AngularLockAxisFactor)

static int r3dBody_SetIsAllowedToSleep(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    body->setIsAllowedToSleep((bool)lua_toboolean(L, 2));
	return 0;
}

static int r3dBody_SetIsActive(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    body->setIsActive((bool)lua_toboolean(L, 2));
	return 0;
}

static int r3dBody_SetIsSleeping(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    body->setIsSleeping((bool)lua_toboolean(L, 2));
	return 0;
}

static int r3dBody_ApplyWorldForceAtWorldPosition(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Vector3 force, point;
	TO_VECTOR(L, 2, force);
	TO_VECTOR(L, 5, point);
	body->applyWorldForceAtWorldPosition(force, point);
	return 0;
}

static int r3dBody_ApplyWorldForceAtLocalPosition(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Vector3 force, point;
	TO_VECTOR(L, 2, force);
	TO_VECTOR(L, 5, point);
	body->applyWorldForceAtLocalPosition(force, point);
	return 0;
}

static int r3dBody_ApplyWorldForceAtCenterOfMass(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Vector3 force, point;
	TO_VECTOR(L, 2, force);
	body->applyWorldForceAtCenterOfMass(force);
	return 0;
}

static int r3dBody_ApplyLocalForceAtWorldPosition(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Vector3 force, point;
	TO_VECTOR(L, 2, force);
	TO_VECTOR(L, 5, point);
	body->applyLocalForceAtWorldPosition(force, point);
	return 0;
}

static int r3dBody_ApplyLocalForceAtLocalPosition(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Vector3 force, point;
	TO_VECTOR(L, 2, force);
	TO_VECTOR(L, 5, point);
	body->applyLocalForceAtLocalPosition(force, point);
	return 0;
}

static int r3dBody_ApplyLocalForceAtCenterOfMass(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Vector3 force, point;
	TO_VECTOR(L, 2, force);
	body->applyLocalForceAtCenterOfMass(force);
	return 0;
}

static int r3dBody_ResetForce(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    body->resetForce();
	return 0;
}

static int r3dBody_ResetTorque(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    body->resetForce();
	return 0;
}

static int r3dBody_RayCast(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Vector3 rs, re;
	TO_VECTOR(L, 2, rs);
	TO_VECTOR(L, 5, re);
	Ray ray(rs, re);
	RaycastInfo ri;
	if (body->raycast(ray, ri))
		push_RayCastInfo(L, ri);
	else
		lua_pushnil(L);
	return 1;
}

static int r3dBody_TestPointInside(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Vector3 pt;
	TO_VECTOR(L, 2, pt);
	lua_pushboolean(L,body->testPointInside(pt));

	return 1;
}

static int r3dBody_ApplyWorldTorque(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Vector3 torque;
	TO_VECTOR(L, 2, torque);
	body->applyWorldTorque(torque);
	return 0;
}

static int r3dBody_ApplyLocalTorque(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
    CHECK_BODY(body,1);
    rp3d::Vector3 torque;
    TO_VECTOR(L, 2, torque);
	body->applyLocalTorque(torque);
	return 0;
}

static int r3dBoxShape_create(lua_State* L) {
	Binder binder(L);

	rp3d::Vector3 sz;
	TO_VECTOR(L, 1, sz);
	rp3d::BoxShape *shape = physicsCommon.createBoxShape(sz);
	shapeType[shape]=SHP_BOX;

	binder.pushInstance("r3dBoxShape", shape);
	return 1;
}

static int r3dBoxShape_SetHalfExtents(lua_State* L) {
	Binder binder(L);
	rp3d::BoxShape *shape  = static_cast<rp3d::BoxShape*>(binder.getInstance(
			"r3dBoxShape", 1));
	rp3d::Vector3 pt;
	TO_VECTOR(L, 2, pt);
	shape->setHalfExtents(pt);
	return 0;
}

static int r3dSphereShape_create(lua_State* L) {
	Binder binder(L);

	rp3d::SphereShape *shape = physicsCommon.createSphereShape(luaL_checknumber(L, 1));
	shapeType[shape]=SHP_SPHERE;

	binder.pushInstance("r3dSphereShape", shape);
	return 1;
}

static int r3dSphereShape_SetRadius(lua_State* L) {
	Binder binder(L);
	rp3d::SphereShape *shape  = static_cast<rp3d::SphereShape*>(binder.getInstance(
			"r3dSphereShape", 1));
	shape->setRadius(luaL_checknumber(L,2));
	return 0;
}

static int r3dCapsuleShape_create(lua_State* L) {
	Binder binder(L);

	rp3d::CapsuleShape *shape = physicsCommon.createCapsuleShape(luaL_checknumber(L, 1),
			luaL_checknumber(L, 2));
	shapeType[shape]=SHP_CAPSULE;

	binder.pushInstance("r3dCapsuleShape", shape);
	return 1;
}

static int r3dCapsuleShape_SetHeight(lua_State* L) {
	Binder binder(L);
	rp3d::CapsuleShape *shape  = static_cast<rp3d::CapsuleShape*>(binder.getInstance(
			"r3dCapsuleShape", 1));
	shape->setHeight(luaL_checknumber(L,2));
	return 0;
}

static int r3dCapsuleShape_SetRadius(lua_State* L) {
	Binder binder(L);
	rp3d::CapsuleShape *shape  = static_cast<rp3d::CapsuleShape*>(binder.getInstance(
			"r3dCapsuleShape", 1));
	shape->setRadius(luaL_checknumber(L,2));
	return 0;
}

class gidMesh : public GidExtraData {
protected:
	float *vertices;
	int *indices;
	int *facesn;
	size_t vc,ic,fc;
public:
	gidMesh(lua_State *L, int n,bool hasFaces) {
		luaL_checktype(L, n, LUA_TTABLE);
		luaL_checktype(L, n + 1, LUA_TTABLE);
		size_t vn = lua_objlen(L, n);
		if (vn % 3)
			luaL_error(L,
					"Vertex array should contain a multiple of 3 items, %d supplied",
					vn);
		size_t in = lua_objlen(L, n + 1);
		if (hasFaces) {
			luaL_checktype(L, n + 2, LUA_TTABLE);
			fc = lua_objlen(L, n + 2);
			facesn = new int[fc];
			int nt=0;
			for (size_t k = 0; k < fc; k++) {
				lua_rawgeti(L, n + 2, k + 1);
				int nk = luaL_optinteger(L, -1, 0);
				lua_pop(L, 1);
				nt+=nk;
				facesn[k] = nk;
			}
			if (nt>in) {
				delete facesn;
				luaL_error(L,
						"Faces reference more indices than supplied (%d>%d)",
						nt,in);
			}
		}
		else
		{
			facesn=NULL;
			if (in % 3)
			luaL_error(L,
					"Index array should contain a multiple of 3 items, %d supplied",
					in);
		}
		vc = vn / 3;
		ic = in;
		indices = new int[in];
		for (size_t k = 0; k < in; k++) {
			lua_rawgeti(L, n + 1, k + 1);
			int ik = luaL_optinteger(L, -1, 0);
			lua_pop(L, 1);
			if ((ik <= 0)||(ik > vc)) {
                delete[] indices;
				if (facesn) delete facesn;
				luaL_error(L,
						"Index array contains an invalid vertice index: %d, max:%d",
						ik, vc);
			}
			indices[k] = ik-1;
		}
		vertices = new float[vn];
		for (size_t k = 0; k < vn; k++) {
			lua_rawgeti(L, n, k + 1);
			vertices[k] = luaL_optnumber(L, -1, 0);
			lua_pop(L, 1);
		}
	}
 ~gidMesh() {
	 delete indices;
	 delete vertices;
	 if (facesn) delete facesn;
 }
};

class gidPolyMesh : public gidMesh {
protected:
	rp3d::PolygonVertexArray::PolygonFace *faces;
	rp3d::PolygonVertexArray *polygonVertexArray;
public:
	rp3d::PolyhedronMesh *polymesh;
	gidPolyMesh(lua_State *L, int n) : gidMesh(L,n,true) {
		faces = new rp3d::PolygonVertexArray::PolygonFace[fc];
		rp3d::PolygonVertexArray::PolygonFace* face = faces;
		int ib=0;
		for (size_t f = 0; f < fc; f++) {
			face->indexBase = ib;
			face->nbVertices = facesn[f];
			ib+=facesn[f];
			face++;
		}

		// Create the polygon vertex array
		polygonVertexArray = new rp3d::PolygonVertexArray(vc, vertices,
				3 * sizeof(float), indices, sizeof(int), fc, faces,
				rp3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
				rp3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

		// Create the polyhedron mesh
#ifndef _NO_THROW
		try {
#endif
			polymesh = physicsCommon.createPolyhedronMesh(polygonVertexArray);
#ifndef _NO_THROW
		} catch (std::runtime_error &e) {
			delete polygonVertexArray;
			delete faces;
			delete indices;
			delete vertices;
			luaL_error(L,"Invalid mesh, probably not convex or ill-formed");
			polymesh=NULL;
		}
#endif
	}
 ~gidPolyMesh() {
	 if (polymesh)
		 physicsCommon.destroyPolyhedronMesh(polymesh);
	 delete polygonVertexArray;
	 delete faces;
 }
};

class gidTriMesh: public gidMesh {
protected:
	rp3d::TriangleVertexArray* triangleArray;
public:
	rp3d::TriangleMesh *trimesh;
	gidTriMesh(lua_State *L, int n) : gidMesh(L,n,false) {
		triangleArray =
		new rp3d::TriangleVertexArray(vc, vertices, 3 * sizeof(float), ic/3,
				indices, 3 * sizeof(int),
				rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
				rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
		trimesh=physicsCommon.createTriangleMesh();
		trimesh->addSubpart(triangleArray);
	}
 ~gidTriMesh() {
	 physicsCommon.destroyTriangleMesh(trimesh);
	 delete triangleArray;
 }
};

static int r3dConvexMeshShape_create(lua_State* L) {
	Binder binder(L);

	gidPolyMesh *gd=new gidPolyMesh(L, 1);
	rp3d::ConvexMeshShape *shape = physicsCommon.createConvexMeshShape(gd->polymesh);
	shapeData[shape]=gd;
	shapeType[shape]=SHP_CONVEX;

	binder.pushInstance("r3dConvexMeshShape", shape);
	return 1;
}

static int r3dConvexMeshShape_SetScale(lua_State* L) {
	Binder binder(L);
	rp3d::ConvexMeshShape *shape  = static_cast<rp3d::ConvexMeshShape*>(binder.getInstance(
			"r3dConvexMeshShape", 1));
	rp3d::Vector3 pt;
	TO_VECTOR(L, 2, pt);
	shape->setScale(pt);
	return 0;
}

static int r3dConcaveMeshShape_create(lua_State* L) {
	Binder binder(L);

	gidTriMesh *gd=new gidTriMesh(L, 1);
	rp3d::ConcaveMeshShape *shape = physicsCommon.createConcaveMeshShape(gd->trimesh);
	shapeData[shape]=gd;
	shapeType[shape]=SHP_CONCAVE;

	binder.pushInstance("r3dConcaveMeshShape", shape);
	return 1;
}

static int r3dConcaveMeshShape_SetScale(lua_State* L) {
	Binder binder(L);
	rp3d::ConcaveMeshShape *shape  = static_cast<rp3d::ConcaveMeshShape*>(binder.getInstance(
			"r3dConcaveMeshShape", 1));
	rp3d::Vector3 pt;
	TO_VECTOR(L, 2, pt);
	shape->setScale(pt);
	return 0;
}

class gidHeightField: public GidExtraData {
protected:
	float *data;
public:
	gidHeightField(float *data) {
		this->data=data;
	}
	~gidHeightField() {
		delete data;
	}
};


static int r3dHeightFieldShape_create(lua_State* L) {
	Binder binder(L);
	int w=luaL_checkinteger(L,1);
	int h=luaL_checkinteger(L,2);
	float ih=luaL_checknumber(L,3);
	float ah=luaL_checknumber(L,4);
	luaL_checktype(L, 5, LUA_TTABLE);
	size_t hn = lua_objlen(L, 5);
	if (hn!=w*h)
		luaL_error(L,"Height field size mismatch (%dx%d!=%d)",w,h,hn);
	float *d=new float[hn];
	for (size_t k = 0; k < hn; k++) {
		lua_rawgeti(L, 5, k + 1);
		d[k] = luaL_optnumber(L, -1, 0);
		lua_pop(L, 1);
	}

	gidHeightField *gd=new gidHeightField(d);
	rp3d::HeightFieldShape *shape = physicsCommon.createHeightFieldShape(w,h,ih,ah, d, rp3d::HeightFieldShape::HeightDataType::HEIGHT_FLOAT_TYPE);
	shapeData[shape]=gd;
	shapeType[shape]=SHP_HEIGHTFIELD;

	binder.pushInstance("r3dHeightFieldShape", shape);
	return 1;
}

static int r3dHeightFieldShape_SetScale(lua_State* L) {
	Binder binder(L);
	rp3d::HeightFieldShape *shape  = static_cast<rp3d::HeightFieldShape*>(binder.getInstance(
			"r3dHeightFieldShape", 1));
	rp3d::Vector3 pt;
	TO_VECTOR(L, 2, pt);
	shape->setScale(pt);
	return 0;
}

static int r3dShape_destruct(void *p) {
	void* ptr = GIDEROS_DTOR_UDATA(p);
	rp3d::CollisionShape* shape = static_cast<rp3d::CollisionShape*>(ptr);

	if (shapeData[shape]) delete shapeData[shape];
	switch (shapeType[shape]) {
	case SHP_BOX: physicsCommon.destroyBoxShape((BoxShape *)shape); break;
	case SHP_SPHERE: physicsCommon.destroySphereShape((SphereShape *)shape); break;
	case SHP_CAPSULE: physicsCommon.destroyCapsuleShape((CapsuleShape *)shape); break;
	case SHP_CONVEX: physicsCommon.destroyConvexMeshShape((ConvexMeshShape *)shape); break;
	case SHP_CONCAVE: physicsCommon.destroyConcaveMeshShape((ConcaveMeshShape *)shape); break;
	case SHP_HEIGHTFIELD: physicsCommon.destroyHeightFieldShape((HeightFieldShape *)shape); break;
	}
	shapeData.erase(shape);
	shapeType.erase(shape);

	return 0;
}

static int r3dFixture_SetCollisionCategoryBits(lua_State* L) {
	Binder binder(L);
	rp3d::Collider* shape = static_cast<rp3d::Collider*>(binder.getInstance(
			"r3dFixture", 1));
	shape->setCollisionCategoryBits(luaL_checkinteger(L, 2));
	return 0;
}

static int r3dFixture_SetCollideWithMaskBits(lua_State* L) {
	Binder binder(L);
	rp3d::Collider* shape = static_cast<rp3d::Collider*>(binder.getInstance(
			"r3dFixture", 1));
	shape->setCollideWithMaskBits(luaL_checkinteger(L, 2));
	return 0;
}

static int r3dFixture_RayCast(lua_State* L) {
	Binder binder(L);
	rp3d::Collider* shape = static_cast<rp3d::Collider*>(binder.getInstance(
			"r3dFixture", 1));
	rp3d::Vector3 rs, re;
	TO_VECTOR(L, 2, rs);
	TO_VECTOR(L, 5, re);
	Ray ray(rs, re);
	RaycastInfo ri;
	if (shape->raycast(ray, ri))
		push_RayCastInfo(L, ri);
	else
		lua_pushnil(L);
	return 1;
}

#define JOINT_GETVEC3(fct) OBJ_GETVEC3(Joint,Joint,fct)

static int r3dJoint_GetReactionForce(lua_State* L) {
	Binder binder(L);
	rp3d::Joint* joint = static_cast<rp3d::Joint*>(binder.getInstance(
			"r3dJoint", 1));

	rp3d::Vector3 v=joint->getReactionForce(luaL_checknumber(L,2));
	lua_pushnumber(L,v.x);
	lua_pushnumber(L,v.y);
	lua_pushnumber(L,v.z);
	return 3;
}

static int r3dJoint_GetReactionTorque(lua_State* L) {
	Binder binder(L);
	rp3d::Joint* joint = static_cast<rp3d::Joint*>(binder.getInstance(
			"r3dJoint", 1));

	rp3d::Vector3 v=joint->getReactionTorque(luaL_checknumber(L,2));
	lua_pushnumber(L,v.x);
	lua_pushnumber(L,v.y);
	lua_pushnumber(L,v.z);
	return 3;
}

OBJ_GETNUM(HingeJoint,HingeJoint,Angle)

static int r3dFixture_TestPointInside(lua_State* L) {
	Binder binder(L);
	rp3d::Collider* shape = static_cast<rp3d::Collider*>(binder.getInstance(
			"r3dFixture", 1));
	rp3d::Vector3 pt;
	TO_VECTOR(L, 2, pt);
	lua_pushboolean(L,shape->testPointInside(pt));

	return 1;
}

static int r3dWorld_DestroyJoint(lua_State* L) {
	Binder binder(L);
	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::Joint* joint = static_cast<rp3d::Joint*>(binder.getInstance(
			"r3dJoint", 2));
	lua_getfield(L, 1, "__joints");
	lua_pushvalue(L, 2);
	lua_rawget(L, -2);
	if (!lua_isnil(L,-1)) {
		world->destroyJoint(joint);
		lua_pushvalue(L, 2);
		lua_pushnil(L);
		lua_rawset(L, -4);
	}

	lua_pop(L, 2);
	return 0;
}

static void world_createJoint(lua_State* L) {
	lua_getfield(L, 1, "__joints");
	lua_pushvalue(L, -2);
	lua_pushvalue(L, -1);
	lua_rawset(L, -3);
	lua_pop(L, 1);
	lua_getfield(L, 2, "__joints");
	lua_pushvalue(L, -2);
	lua_pushvalue(L, -1);
	lua_rawset(L, -3);
	lua_pop(L, 1);
	lua_getfield(L, 3, "__joints");
	lua_pushvalue(L, -2);
	lua_pushvalue(L, -1);
	lua_rawset(L, -3);
	lua_pop(L, 1);
}

static int r3dWorld_CreateBallAndSocketJoint(lua_State* L) {
	Binder binder(L);

	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
    CHECK_BODY(body1,2);
    rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
    CHECK_BODY(body2,3);
    bool hasParams = false;
	if (!lua_isnoneornil(L,7)) {
		luaL_checktype(L, 7, LUA_TTABLE);
		hasParams = true;
	}

	rp3d::Vector3 anc;
	TO_VECTOR(L, 4, anc);
	rp3d::BallAndSocketJointInfo ji(body1, body2, anc);
	if (hasParams) {
		lua_getfield(L, 7, "isCollisionEnabled");
		ji.isCollisionEnabled = lua_toboolean(L, -1);
		lua_pop(L, 1);
	}

	rp3d::BallAndSocketJoint *joint =
			(rp3d::BallAndSocketJoint *) world->createJoint(ji);

	binder.pushInstance("r3dBallAndSocketJoint", joint);
	world_createJoint(L);
	return 1;
}

static int r3dWorld_CreateHingeJoint(lua_State* L) {
	Binder binder(L);

	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
    CHECK_BODY(body1,2);
    rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
    CHECK_BODY(body2,3);
    bool hasParams = false;
	if (!lua_isnoneornil(L,10)) {
		luaL_checktype(L, 10, LUA_TTABLE);
		hasParams = true;
	}
	rp3d::Vector3 anc, axis;
	TO_VECTOR(L, 4, anc);
	TO_VECTOR(L, 7, axis);
	rp3d::HingeJointInfo ji(body1, body2, anc, axis);
	if (hasParams) {
		lua_getfield(L, 10, "isCollisionEnabled");
		ji.isCollisionEnabled = lua_toboolean(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 10, "isLimitEnabled");
		ji.isLimitEnabled = lua_toboolean(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 10, "minAngleLimit");
		ji.minAngleLimit = luaL_optnumber(L, -1, 0);
		lua_pop(L, 1);
		lua_getfield(L, 10, "maxAngleLimit");
		ji.maxAngleLimit = luaL_optnumber(L, -1, 0);
		lua_pop(L, 1);
		lua_getfield(L, 10, "isMotorEnabled");
		ji.isMotorEnabled = lua_toboolean(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 10, "motorSpeed");
		ji.motorSpeed = luaL_optnumber(L, -1, 0);
		lua_pop(L, 1);
		lua_getfield(L, 10, "maxMotorTorque");
		ji.maxMotorTorque = luaL_optnumber(L, -1, 0);
		lua_pop(L, 1);
	}
	rp3d::HingeJoint *joint = (rp3d::HingeJoint *) world->createJoint(ji);

	binder.pushInstance("r3dHingeJoint", joint);
	world_createJoint(L);
	return 1;
}

static int r3dWorld_CreateSliderJoint(lua_State* L) {
	Binder binder(L);

	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
    CHECK_BODY(body1,2);
    rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
    CHECK_BODY(body2,3);
    bool hasParams = false;
	if (!lua_isnoneornil(L,10)) {
		luaL_checktype(L, 10, LUA_TTABLE);
		hasParams = true;
	}
	rp3d::Vector3 anc, axis;
	TO_VECTOR(L, 4, anc);
	TO_VECTOR(L, 7, axis);
	rp3d::SliderJointInfo ji(body1, body2, anc, axis);
	if (hasParams) {
		lua_getfield(L, 10, "isCollisionEnabled");
		ji.isCollisionEnabled = lua_toboolean(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 10, "isLimitEnabled");
		ji.isLimitEnabled = lua_toboolean(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 10, "minTranslationLimit");
		ji.minTranslationLimit = luaL_optnumber(L, -1, 0);
		lua_pop(L, 1);
		lua_getfield(L, 10, "maxTranslationLimit");
		ji.maxTranslationLimit = luaL_optnumber(L, -1, 0);
		lua_pop(L, 1);
		lua_getfield(L, 10, "isMotorEnabled");
		ji.isMotorEnabled = lua_toboolean(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 10, "motorSpeed");
		ji.motorSpeed = luaL_optnumber(L, -1, 0);
		lua_pop(L, 1);
		lua_getfield(L, 10, "maxMotorForce");
		ji.maxMotorForce = luaL_optnumber(L, -1, 0);
		lua_pop(L, 1);
	}
	rp3d::SliderJoint *joint = (rp3d::SliderJoint *) world->createJoint(ji);

	binder.pushInstance("r3dSliderJoint", joint);
	world_createJoint(L);
	return 1;
}

static int r3dWorld_CreateFixedJoint(lua_State* L) {
	Binder binder(L);

	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
    CHECK_BODY(body1,2);
    rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
    CHECK_BODY(body2,3);
    bool hasParams = false;
	if (!lua_isnoneornil(L,7)) {
		luaL_checktype(L, 7, LUA_TTABLE);
		hasParams = true;
	}
	rp3d::Vector3 anc;
	TO_VECTOR(L, 4, anc);
	rp3d::FixedJointInfo ji(body1, body2, anc);
	if (hasParams) {
		lua_getfield(L, 7, "isCollisionEnabled");
		ji.isCollisionEnabled = lua_toboolean(L, -1);
		lua_pop(L, 1);
	}
	rp3d::FixedJoint *joint = (rp3d::FixedJoint *) world->createJoint(ji);

	binder.pushInstance("r3dFixedJoint", joint);
	world_createJoint(L);
	return 1;
}

///DEBUG
class r3dDebugDraw
{
public:
    r3dDebugDraw(LuaApplication* application,rp3d::PhysicsWorld *world1);

	virtual ~r3dDebugDraw();
	SpriteProxy *proxy_;

    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
private:
    struct VColor {
        uint8_t r,g,b,a;
    };
    struct VPos {
    	float x,y,z;
    };
	rp3d::PhysicsWorld* world_;
    VertexBuffer<VPos> vertices;
    VertexBuffer<VColor> colors;
};

void r3dDebugDraw::doDraw(const CurrentTransform& , float _UNUSED(sx), float _UNUSED(sy), float _UNUSED(ex), float _UNUSED(ey))
{
#define SETP(i,n,p) {\
	vertices[i].x = ta[n].point##p.x; \
    vertices[i].y = ta[n].point##p.y; \
    vertices[i].z = ta[n].point##p.z; \
	uint32_t c = ta[n].color##p; \
	colors[i].r = (c >> 16) & 0xFF; \
	colors[i].g = (c >> 8) & 0xFF; \
	colors[i].b = (c >> 0) & 0xFF; \
	colors[i].a = 0xFF; }
	if (world_)
	{
		bool wireframe=true;
		DebugRenderer& dr = world_->getDebugRenderer();
	    ShaderEngine* engine=gtexture_get_engine();
	    ShaderProgram* shp=engine->getDefault(ShaderEngine::STDP_COLOR,ShaderEngine::STDPV_3D);
		size_t nt=dr.getNbTriangles();
		size_t nl=dr.getNbLines();
		size_t ntv=wireframe?0:nt*3;
		size_t nlv=(wireframe?nt*6:0)+nl*2;
		if (nt+nl) {
			vertices.resize(ntv+nlv);
			colors.resize(ntv+nlv);
			int nlb=wireframe?nt*6:ntv;
			if (nt) {
				const DebugRenderer::DebugTriangle *ta=dr.getTrianglesArray();
				for (size_t k=0;k<nt;k++)
				{
					if (wireframe) {
					   SETP(k*6,k,1);
					   SETP(k*6+1,k,2);
					   SETP(k*6+2,k,2);
					   SETP(k*6+3,k,3);
					   SETP(k*6+4,k,3);
					   SETP(k*6+5,k,1);
					}
					else {
					   SETP(k*3,k,1);
					   SETP(k*3+1,k,2);
					   SETP(k*3+2,k,3);
					}
				}
			}
			if (nl) {
				const DebugRenderer::DebugLine *ta=dr.getLinesArray();
				for (size_t k=0;k<nl;k++)
				{
				   SETP(nlb+k*2,k,1);
				   SETP(nlb+k*2+1,k,2);
				}
			}
		   vertices.Update();
		   colors.Update();
		   shp->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT,3, &vertices[0], ntv+nlv, true, NULL);
		   shp->setData(ShaderProgram::DataColor, ShaderProgram::DUBYTE,4, &colors[0], ntv+nlv, true, NULL);
		   if (ntv)
			   shp->drawArrays(ShaderProgram::Triangles, 0, ntv);
		   if (nlv)
			   shp->drawArrays(ShaderProgram::Lines, ntv, nlv);
		}
	}
#undef SETP
}

static void r3dDG_Draw(void *c,const CurrentTransform&t, float sx, float sy, float ex, float ey)
{
	((r3dDebugDraw *)c)->doDraw(t,sx, sy, ex, ey);
}

static void r3dDG_Destroy(void *c)
{
	delete ((r3dDebugDraw *)c);
}


r3dDebugDraw::r3dDebugDraw(LuaApplication* application,rp3d::PhysicsWorld *world1) :
    world_(world1)
{
	proxy_=gtexture_get_spritefactory()->createProxy(application->getApplication(), this, r3dDG_Draw, r3dDG_Destroy);

	world_->setIsDebugRenderingEnabled(true);
	DebugRenderer& debugRenderer = world_->getDebugRenderer();

	debugRenderer.setIsDebugItemDisplayed(DebugRenderer::DebugItem::COLLISION_SHAPE, true);
	debugRenderer.setIsDebugItemDisplayed(DebugRenderer::DebugItem::CONTACT_POINT, true);
	debugRenderer.setIsDebugItemDisplayed(DebugRenderer::DebugItem::CONTACT_NORMAL, true);
}

r3dDebugDraw::~r3dDebugDraw()
{
	world_->setIsDebugRenderingEnabled(false);
}

int r3dDebugDraw_create(lua_State* L)
{
	Binder binder(L);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	rp3d::PhysicsWorld* world =
			static_cast<rp3d::PhysicsWorld*>(binder.getInstance("r3dWorld", 1));

	r3dDebugDraw* debugDraw = new r3dDebugDraw(application,world);
	binder.pushInstance("r3dDebugDraw", debugDraw->proxy_);

	return 1;
}

int r3dDebugDraw_destruct(void *p)
{
	void* ptr = GIDEROS_DTOR_UDATA(p);
	r3dDebugDraw* debugDraw = static_cast<r3dDebugDraw*>(static_cast<SpriteProxy *>(ptr)->getContext());
	debugDraw->proxy_->unref();

	return 0;
}


static int loader(lua_State *L) {
//StackChecker checker(L, "Box2DBinder2::loader", 1);

	Binder binder(L);

	const luaL_Reg r3dWorld_functionList[] = {
			{ "createBody",	r3dWorld_CreateBody },
			{ "destroyBody", r3dWorld_DestroyBody },
			{ "step", r3dWorld_Step },
			{ "raycast", r3dWorld_RayCast },
			{ "testOverlap", r3dWorld_TestOverlap },
			{ "testCollision", r3dWorld_TestCollision },
			{ "setEventListener", r3dWorld_SetEventListener },
			{ "createBallAndSocketJoint", r3dWorld_CreateBallAndSocketJoint },
			{ "createHingeJoint", r3dWorld_CreateHingeJoint },
			{ "createSliderJoint", r3dWorld_CreateSliderJoint },
			{ "createFixedJoint", r3dWorld_CreateFixedJoint },
			{ "destroyJoint", r3dWorld_DestroyJoint },
			{ "setGravity", r3dWorld_SetGravity },
			{ "getGravity", r3dWorld_GetGravity },
			{ NULL, NULL }, };
	binder.createClass("r3dWorld", NULL/*"EventDispatcher"*/, r3dWorld_create,
			r3dWorld_destruct, r3dWorld_functionList);
	const luaL_Reg r3dDebugDraw_functionList[] = {
		{NULL, NULL},
	};
	binder.createClass("r3dDebugDraw", "Sprite", r3dDebugDraw_create, r3dDebugDraw_destruct, r3dDebugDraw_functionList);

	const luaL_Reg r3dBody_functionList[] = {
			{ "createFixture", r3dBody_CreateFixture },
			{ "destroyFixture", r3dBody_DestroyFixture },
			{ "getTransform", r3dBody_GetTransform },
			{ "setTransform", r3dBody_SetTransform },
			{ "getLinearVelocity", r3dBody_GetLinearVelocity },
			{ "setLinearVelocity", r3dBody_SetLinearVelocity },
			{ "getAngularVelocity", r3dBody_GetAngularVelocity },
			{ "setAngularVelocity", r3dBody_SetAngularVelocity },
			{ "getLinearDamping", r3dBody_GetLinearDamping },
			{ "setLinearDamping", r3dBody_SetLinearDamping },
			{ "getAngularDamping", r3dBody_GetAngularDamping },
			{ "setAngularDamping", r3dBody_SetAngularDamping },
			{ "getMass", r3dBody_GetMass },
			{ "setMass", r3dBody_SetMass },
			{ "setType", r3dBody_SetType },
			{ "enableGravity", r3dBody_EnableGravity },
			{ "setIsAllowedToSleep", r3dBody_SetIsAllowedToSleep },
			{ "setIsSleeping", r3dBody_SetIsSleeping },
			{ "setIsActive", r3dBody_SetIsActive },
			{ "applyWorldForceAtWorldPosition", r3dBody_ApplyWorldForceAtWorldPosition },
			{ "applyWorldForceAtLocalPosition", r3dBody_ApplyWorldForceAtLocalPosition },
			{ "applyWorldForceAtCenterOfMass", r3dBody_ApplyWorldForceAtCenterOfMass },
			{ "applyWorldTorque", r3dBody_ApplyWorldTorque },
			{ "applyLocalForceAtWorldPosition", r3dBody_ApplyLocalForceAtWorldPosition },
			{ "applyLocalForceAtLocalPosition", r3dBody_ApplyLocalForceAtLocalPosition },
			{ "applyLocalForceAtCenterOfMass", r3dBody_ApplyLocalForceAtCenterOfMass },
			{ "applyLocalTorque", r3dBody_ApplyLocalTorque },
			{ "resetForce", r3dBody_ResetForce },
			{ "resetTorque", r3dBody_ResetTorque },
			{ "getForce", r3dBody_GetForce },
			{ "getTorque", r3dBody_GetTorque },
			{ "setLinearLockAxisFactor", r3dBody_SetLinearLockAxisFactor },
			{ "setAngularLockAxisFactor", r3dBody_SetAngularLockAxisFactor },
			{ "getLinearLockAxisFactor", r3dBody_GetLinearLockAxisFactor },
			{ "getAngularLockAxisFactor", r3dBody_GetAngularLockAxisFactor },
			{ "raycast", r3dBody_RayCast },
			{ "testPointInside", r3dBody_TestPointInside },
			{ "updateMassPropertiesFromColliders", r3dBody_UpdateMassPropertiesFromColliders },
			{ NULL, NULL }, };
    binder.createClass("r3dBody", NULL/*"EventDispatcher"*/, NULL, r3dBody_destruct,
			r3dBody_functionList);
	const luaL_Reg r3dFixture_functionList[] = {
			{ "setCollisionCategoryBits", r3dFixture_SetCollisionCategoryBits },
			{ "setCollideWithMaskBits",	r3dFixture_SetCollideWithMaskBits },
			{ "raycast", r3dFixture_RayCast },
			{ "testPointInside", r3dFixture_TestPointInside },
			{ "getMaterial", r3dFixture_GetMaterial },
			{ "setMaterial", r3dFixture_SetMaterial },
			{ "setIsTrigger", r3dFixture_SetIsTrigger },
			{ NULL, NULL }, };
	binder.createClass("r3dFixture", NULL/*"EventDispatcher"*/, NULL, NULL,
			r3dFixture_functionList);
	const luaL_Reg r3dShape_functionList[] = { { NULL, NULL }, };
	binder.createClass("r3dShape", NULL/*"EventDispatcher"*/, NULL,
			r3dShape_destruct, r3dShape_functionList);

	const luaL_Reg r3dBoxShape_functionList[] = {
			{ "setHalfExtents", r3dBoxShape_SetHalfExtents },
			{ NULL, NULL }, };
	binder.createClass("r3dBoxShape", "r3dShape", r3dBoxShape_create, NULL,
			r3dBoxShape_functionList);

	const luaL_Reg r3dSphereShape_functionList[] = {
			{ "setRadius", r3dSphereShape_SetRadius },
			{ NULL, NULL }, };
	binder.createClass("r3dSphereShape", "r3dShape", r3dSphereShape_create,
			NULL, r3dSphereShape_functionList);

	const luaL_Reg r3dCapsuleShape_functionList[] = {
			{ "setRadius", r3dCapsuleShape_SetRadius },
			{ "setHeight", r3dCapsuleShape_SetHeight },
			{ NULL, NULL }, };
	binder.createClass("r3dCapsuleShape", "r3dShape", r3dCapsuleShape_create,
			NULL, r3dCapsuleShape_functionList);

	const luaL_Reg r3dConvexMeshShape_functionList[] = {
			{ "setScale", r3dConvexMeshShape_SetScale },
			{ NULL, NULL }, };
	binder.createClass("r3dConvexMeshShape", "r3dShape", r3dConvexMeshShape_create,
				NULL, r3dConvexMeshShape_functionList);
	const luaL_Reg r3dConcaveMeshShape_functionList[] = {
			{ "setScale", r3dConcaveMeshShape_SetScale },
			{ NULL, NULL }, };
	binder.createClass("r3dConcaveMeshShape", "r3dShape", r3dConcaveMeshShape_create,
				NULL, r3dConcaveMeshShape_functionList);
	const luaL_Reg r3dHeightFieldShape_functionList[] = {
			{ "setScale", r3dHeightFieldShape_SetScale },
			{ NULL, NULL }, };
	binder.createClass("r3dHeightFieldShape", "r3dShape", r3dHeightFieldShape_create,
				NULL, r3dHeightFieldShape_functionList);

	const luaL_Reg r3dJoint_functionList[] = {
			{ "getReactionForce", r3dJoint_GetReactionForce },
			{ "getReactionTorque", r3dJoint_GetReactionTorque },
			{ NULL, NULL }, };
	binder.createClass("r3dJoint", NULL/*"EventDispatcher"*/, NULL, NULL,
			r3dJoint_functionList);
	const luaL_Reg r3dBallAndSocketJoint_functionList[] = { { NULL, NULL }, };
	binder.createClass("r3dBoxAndSocketJoint", "r3dJoint", NULL, NULL,
			r3dBallAndSocketJoint_functionList);
	const luaL_Reg r3dHingeJoint_functionList[] = {
			{ "getAngle", r3dHingeJoint_GetAngle },
			{ NULL, NULL }, };
	binder.createClass("r3dHingeJoint", "r3dJoint", NULL, NULL,
			r3dHingeJoint_functionList);
	const luaL_Reg r3dSlider_functionList[] = { { NULL, NULL }, };
	binder.createClass("r3dSliderJoint", "r3dJoint", NULL, NULL,
			r3dSlider_functionList);
	const luaL_Reg r3dFixedJoint_functionList[] = { { NULL, NULL }, };
	binder.createClass("r3dFixedJoint", "r3dJoint", NULL, NULL,
			r3dFixedJoint_functionList);

	lua_getglobal(L, "r3dBody");
	lua_pushinteger(L, (int) BodyType::STATIC);
	lua_setfield(L, -2, "STATIC_BODY");

	lua_pushinteger(L, (int) BodyType::KINEMATIC);
	lua_setfield(L, -2, "KINEMATIC_BODY");

	lua_pushinteger(L, (int) BodyType::DYNAMIC);
	lua_setfield(L, -2, "DYNAMIC_BODY");
	lua_pop(L, 1);

	lua_newtable(L);
#define RELOC_CLASS(n) \
	lua_getglobal(L, "r3d" n); \
	lua_setfield(L, -2, n);\
	lua_pushnil(L);\
	lua_setglobal(L, "r3d" n);
	RELOC_CLASS("World")
	RELOC_CLASS("Body")
	RELOC_CLASS("Shape")
	RELOC_CLASS("Fixture")
	RELOC_CLASS("Joint")
	RELOC_CLASS("BoxShape")
	RELOC_CLASS("SphereShape")
	RELOC_CLASS("CapsuleShape")
	RELOC_CLASS("ConvexMeshShape")
	RELOC_CLASS("ConcaveMeshShape")
	RELOC_CLASS("HeightFieldShape")
	RELOC_CLASS("BallAndSocketJoint")
	RELOC_CLASS("HingeJoint")
	RELOC_CLASS("SliderJoint")
	RELOC_CLASS("FixedJoint")
	RELOC_CLASS("DebugDraw")
	lua_pushvalue(L, -1);
	lua_setglobal(L, "r3d");

	return 1;
}

static void g_initializePlugin(lua_State *L) {
	::L = L;
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcnfunction(L, loader,"plugin_init_reactphysics3d");
	lua_setfield(L, -2, "reactphysics3d");

    lua_pop(L, 2);
    luaL_newweaktable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_b2);
}

static void g_deinitializePlugin(lua_State *_UNUSED(L)) {
}

#ifdef QT_NO_DEBUG
REGISTER_PLUGIN_NAMED("ReactPhysics3D", "0.9.0", reactphysics3d)
#elif defined(TARGET_OS_MAC) || defined(_MSC_VER)
REGISTER_PLUGIN_STATICNAMED_CPP("ReactPhysics3D", "0.9.0", reactphysics3d)
#else
REGISTER_PLUGIN_NAMED("ReactPhysics3D", "0.9.0", reactphysics3d)
#endif
