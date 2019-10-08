#include "gstdio.h"
#include "application.h"
#include "lua.hpp"
#include "luaapplication.h"
#include "luautil.h"
#include "gplugin.h"
#include "binder.h"
#include "gglobal.h"
#include "glog.h"

#include "reactphysics3d.h"
using namespace reactphysics3d;
#include <utility>
#include <set>
#include <stdexcept>

#define _UNUSED(n)

static lua_State *L;
char key_b2 = ' ';

void getb2(lua_State* L) {
	//StackChecker checker(L, "getb2", 0);
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_b2);
	lua_pushvalue(L, -2);
	lua_rawget(L, -2);
	lua_remove(L, -2);
	lua_remove(L, -2);
}

void getb2(lua_State* L, const void* ptr) {
	lua_pushlightuserdata(L, (void *) ptr);
	getb2(L);
}

void setb2(lua_State* L) {
	//StackChecker checker(L, "setb2", -2);
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_b2);
	lua_pushvalue(L, -3);
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 3);
}

class GidEventListener: public EventListener {
	typedef std::pair<const void *, const void *> GidContact;
	struct ColInfo {
		/// Pointer to the first body of the contact
		CollisionBody* body1;
		/// Pointer to the second body of the contact
		CollisionBody* body2;
		/// Pointer to the proxy shape of first body
		const ProxyShape* proxyShape1;
		/// Pointer to the proxy shape of second body
		const ProxyShape* proxyShape2;
	};
	std::map<GidContact, ColInfo> contacts;
	std::set<GidContact> ncontacts;
	static GidContact contact(const void *a, const void *b) {
		if (a < b)
			return std::make_pair(a, b);
		else
			return std::make_pair(b, a);
	}
public:
	lua_State *L;
	int cbn;
	virtual void newContact(
			const CollisionCallback::CollisionCallbackInfo& ci) {
		GidContact c = contact(ci.proxyShape1, ci.proxyShape2);
		ncontacts.insert(c);
		if (contacts.count(c))
			return;
		ColInfo cli;
		cli.body1 = ci.body1;
		cli.body2 = ci.body2;
		cli.proxyShape1 = ci.proxyShape1;
		cli.proxyShape2 = ci.proxyShape2;
		contacts[c] = cli;
		contactEvent("beginContact", cli);
	}
	void contactEvent(const char *type, ColInfo &cli) {
		lua_getref(L, cbn);
		lua_pushstring(L, type);
		lua_newtable(L);
		getb2(L, cli.body1);
		lua_setfield(L, -2, "body1");
		getb2(L, cli.body2);
		lua_setfield(L, -2, "body2");
		getb2(L, cli.proxyShape1);
		lua_setfield(L, -2, "fixture1");
		getb2(L, cli.proxyShape2);
		lua_setfield(L, -2, "fixture2");
		lua_call(L, 2, 0);
	}
	void endFrame() {
		for (std::map<GidContact, ColInfo>::iterator it = contacts.begin();
				it != contacts.end();) {
			if (!ncontacts.count(it->first)) {
				ColInfo cli = it->second;
				if (!(cli.body1->isSleeping() && cli.body2->isSleeping())) {
					std::map<GidContact, ColInfo>::iterator it2 = it++;
					contactEvent("endContact", cli);
					contacts.erase(it2);
				} else
					it++;
			} else
				it++;
		}
		ncontacts.clear();
	}

};
static std::map<rp3d::DynamicsWorld *, GidEventListener *> events;

static int r3dWorld_create(lua_State* L) {
	Binder binder(L);

	rp3d::Vector3 gravity;
	gravity.x = luaL_checknumber(L, 1);
	gravity.y = luaL_checknumber(L, 2);
	gravity.z = luaL_checknumber(L, 3);
	rp3d::DynamicsWorld *world = new rp3d::DynamicsWorld(gravity);

	binder.pushInstance("r3dWorld", world);
	lua_newtable(L);
	lua_setfield(L, -2, "__joints");

	return 1;
}

int r3dWorld_destruct(lua_State* L) {
	void* ptr = *(void**) lua_touserdata(L, 1);
	rp3d::DynamicsWorld* world = static_cast<rp3d::DynamicsWorld*>(ptr);
	GidEventListener *e = events[world];
	if (e) {
		lua_unref(L, e->cbn);
		delete e;
		events[world] = NULL;
	}
	delete world;

	return 0;
}

int r3dWorld_SetEventListener(lua_State* L) {
	Binder binder(L);
	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
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
		lua_pushvalue(L, 2);
		e->cbn = lua_ref(L,1);
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
}

int r3dWorld_CreateBody(lua_State* L) {
	Binder binder(L);
	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::Transform transform;
	toTransform(L, 2, transform);
	rp3d::RigidBody *body = world->createRigidBody(transform);
	binder.pushInstance("r3dBody", body);
	lua_newtable(L);
	lua_setfield(L, -2, "__fixtures");
	lua_newtable(L);
	lua_setfield(L, -2, "__joints");

	lua_pushlightuserdata(L, body);
	lua_pushvalue(L, -2);
	setb2(L);

	return 1;
}

int r3dWorld_DestroyBody(lua_State* L) {
	Binder binder(L);
	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
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

	world->destroyRigidBody(body);
	binder.setInstance(2, NULL);

	lua_pushlightuserdata(L, body);
	lua_pushnil(L);
	setb2(L);

	return 0;
}

int r3dWorld_Step(lua_State* L) {
	Binder binder(L);
	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	GidEventListener *e = events[world];
	if (e)
		e->L = L;
	try {
		world->update(luaL_checknumber(L, 2));
	} catch (std::runtime_error &e) {
		luaL_error(L,"Failed to step world, something is not set up correctly");
	}
	if (e)
		e->endFrame();

	return 0;
}


class GidCollisionCallback: public CollisionCallback {
	lua_State *L;
	int cbn;
public:
	GidCollisionCallback(lua_State *L,int cbn) { this->L=L; this->cbn=cbn; }
	void notifyContact(const CollisionCallbackInfo& cli) {
		lua_pushvalue(L, cbn);
		lua_newtable(L);
		getb2(L, cli.body1);
		lua_setfield(L, -2, "body1");
		getb2(L, cli.body2);
		lua_setfield(L, -2, "body2");
		getb2(L, cli.proxyShape1);
		lua_setfield(L, -2, "fixture1");
		getb2(L, cli.proxyShape2);
		lua_setfield(L, -2, "fixture2");
		lua_call(L, 1, 0);
	}
};

int r3dWorld_TestOverlap(lua_State* L) {
	Binder binder(L);
	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
	rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
	lua_pushboolean(L,world->testOverlap(body1, body2));

	return 1;
}

int r3dWorld_TestCollision(lua_State* L) {
	Binder binder(L);
	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	if (lua_type(L,2)==LUA_TFUNCTION) {
		GidCollisionCallback gcb(L,2);
		world->testCollision(&gcb);
	}
	else {
		rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
				"r3dBody", 2));
		if (lua_type(L,3)==LUA_TFUNCTION) {
			GidCollisionCallback gcb(L,3);
			unsigned short cat = luaL_optinteger(L, 4, 0xFFFF);
			world->testCollision(body1, &gcb, cat);
		}
		else {
			rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
					"r3dBody", 3));
			luaL_checktype(L,4,LUA_TFUNCTION);
			GidCollisionCallback gcb(L,4);
			world->testCollision(body1, body2, &gcb);

		}
	}
	return 0;
}

int r3dWorld_TestAABBOverlap(lua_State* L) {
	Binder binder(L);
	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
	rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
	lua_pushboolean(L,world->testAABBOverlap(body1, body2));

	return 1;
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
	getb2(L, info.proxyShape);
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

int r3dWorld_RayCast(lua_State* L) {
	Binder binder(L);
	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
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

int r3dBody_CreateFixture(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	rp3d::CollisionShape* shape =
			static_cast<rp3d::CollisionShape*>(binder.getInstance("r3dShape", 2));
	rp3d::Transform transform;
	toTransform(L, 3, transform);
	float mass = luaL_optnumber(L, 4, 1.0);
	rp3d::ProxyShape *fixture = body->addCollisionShape(shape, transform, mass);
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

int r3dBody_DestroyFixture(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	rp3d::ProxyShape* shape = static_cast<rp3d::ProxyShape*>(binder.getInstance(
			"r3dFixture", 2));
	body->removeCollisionShape(shape);

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

int r3dBody_GetTransform(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
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

int r3dBody_SetTransform(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	rp3d::Transform transform;
	toTransform(L, 2, transform);
	body->setTransform(transform);
	return 0;
}

int r3dBody_SetType(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	body->setType((BodyType) luaL_checkinteger(L, 2));
	return 0;
}

int r3dBody_GetMaterial(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	rp3d::Material& mat = body->getMaterial();
	lua_newtable(L);
	lua_pushnumber(L, mat.getBounciness());
	lua_setfield(L, -2, "bounciness");
	lua_pushnumber(L, mat.getFrictionCoefficient());
	lua_setfield(L, -2, "frictionCoefficient");
	lua_pushnumber(L, mat.getRollingResistance());
	lua_setfield(L, -2, "rollingResistance");
	return 1;
}

int r3dBody_SetMaterial(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	rp3d::Material& mat = body->getMaterial();
	lua_getfield(L, 2, "bounciness");
	if (!lua_isnil(L,-1))
		mat.setBounciness(luaL_checknumber(L, -1));
	lua_pop(L, 1);
	lua_getfield(L, 2, "frictionCoefficient");
	if (!lua_isnil(L,-1))
		mat.setFrictionCoefficient(luaL_checknumber(L, -1));
	lua_pop(L, 1);
	lua_getfield(L, 2, "rollingResistance");
	if (!lua_isnil(L,-1))
		mat.setRollingResistance(luaL_checknumber(L, -1));
	lua_pop(L, 1);
	return 0;
}

int r3dBody_EnableGravity(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	body->enableGravity(lua_toboolean(L, 2));
	return 0;
}

int r3dBody_SetIsAllowedToSleep(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	body->setIsAllowedToSleep(lua_toboolean(L, 2));
	return 0;
}

int r3dBody_ApplyForce(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	rp3d::Vector3 force, point;
	bool center = lua_isnoneornil(L,5);
	TO_VECTOR(L, 2, force);
	if (center)
		body->applyForceToCenterOfMass(force);
	else {
		TO_VECTOR(L, 5, point);
		body->applyForce(force, point);
	}
	return 0;
}

int r3dBody_RayCast(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
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

int r3dBody_TestPointInside(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	rp3d::Vector3 pt;
	TO_VECTOR(L, 2, pt);
	lua_pushboolean(L,body->testPointInside(pt));

	return 1;
}

int r3dBody_ApplyTorque(lua_State* L) {
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 1));
	rp3d::Vector3 torque;
	TO_VECTOR(L, 2, torque);
	body->applyTorque(torque);
	return 0;
}

static int r3dBoxShape_create(lua_State* L) {
	Binder binder(L);

	rp3d::Vector3 sz;
	TO_VECTOR(L, 1, sz);
	rp3d::BoxShape *shape = new rp3d::BoxShape(sz);

	binder.pushInstance("r3dBoxShape", shape);
	return 1;
}
static int r3dSphereShape_create(lua_State* L) {
	Binder binder(L);

	rp3d::SphereShape *shape = new rp3d::SphereShape(luaL_checknumber(L, 1));

	binder.pushInstance("r3dSphereShape", shape);
	return 1;
}
static int r3dCapsuleShape_create(lua_State* L) {
	Binder binder(L);

	rp3d::CapsuleShape *shape = new rp3d::CapsuleShape(luaL_checknumber(L, 1),
			luaL_checknumber(L, 2));

	binder.pushInstance("r3dCapsuleShape", shape);
	return 1;
}

class gidMesh {
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
		if (in % 3)
			luaL_error(L,
					"Index array should contain a multiple of 3 items, %d supplied",
					in);
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
			facesn=NULL;
		vc = vn / 3;
		ic = in / 3;
		indices = new int[in];
		for (size_t k = 0; k < in; k++) {
			lua_rawgeti(L, n + 1, k + 1);
			int ik = luaL_optinteger(L, -1, 0);
			lua_pop(L, 1);
			if ((ik <= 0)||(ik > vc)) {
				delete indices;
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
		for (size_t f = 0; f < ic; f++) {
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
		try {
			polymesh = new rp3d::PolyhedronMesh(polygonVertexArray);
		} catch (std::runtime_error &e) {
			delete polygonVertexArray;
			delete faces;
			delete indices;
			delete vertices;
			luaL_error(L,"Invalid mesh, probably not convex or ill-formed");
		}
	}
 ~gidPolyMesh() {
	 delete polymesh;
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
		new rp3d::TriangleVertexArray(vc, vertices, 3 * sizeof(float), ic,
				indices, 3 * sizeof(int),
				rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
				rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
		trimesh=new rp3d::TriangleMesh();
		trimesh->addSubpart(triangleArray);
	}
 ~gidTriMesh() {
	 delete trimesh;
	 delete triangleArray;
 }
};

class gidConvexMesh: public rp3d::ConvexMeshShape {
protected:
	gidPolyMesh *mesh;
public:
	gidConvexMesh(gidPolyMesh *m) : rp3d::ConvexMeshShape(m->polymesh) {
		mesh=m;
	}
	~gidConvexMesh() {
		delete mesh;
	}
};

static int r3dConvexMeshShape_create(lua_State* L) {
	Binder binder(L);

	gidConvexMesh *shape = new gidConvexMesh(new gidPolyMesh(L, 1));

	binder.pushInstance("r3dConvexMeshShape", shape);
	return 1;
}

class gidConcaveMesh: public rp3d::ConcaveMeshShape {
protected:
	gidTriMesh *mesh;
public:
	gidConcaveMesh(gidTriMesh *m) : rp3d::ConcaveMeshShape(m->trimesh) {
		mesh=m;
	}
	~gidConcaveMesh() {
		delete mesh;
	}
};

static int r3dConcaveMeshShape_create(lua_State* L) {
	Binder binder(L);

	gidConcaveMesh *shape = new gidConcaveMesh(new gidTriMesh(L, 1));

	binder.pushInstance("r3dConcaveMeshShape", shape);
	return 1;
}

class gidHeightField: public rp3d::HeightFieldShape {
protected:
	float *data;
public:
	gidHeightField(int w,int h,float ih,float ah,float *data) : rp3d::HeightFieldShape(w,h,ih,ah, data, rp3d::HeightFieldShape::HeightDataType::HEIGHT_FLOAT_TYPE) {
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

	gidHeightField *shape = new gidHeightField(w,h,ih,ah,d);

	binder.pushInstance("r3dHeightFieldShape", shape);
	return 1;
}

static int r3dShape_destruct(lua_State* L) {
	void* ptr = *(void**) lua_touserdata(L, 1);
	rp3d::CollisionShape* shape = static_cast<rp3d::CollisionShape*>(ptr);
	delete shape;

	return 0;
}

int r3dFixture_SetCollisionCategoryBits(lua_State* L) {
	Binder binder(L);
	rp3d::ProxyShape* shape = static_cast<rp3d::ProxyShape*>(binder.getInstance(
			"r3dFixture", 1));
	shape->setCollisionCategoryBits(luaL_checkinteger(L, 2));
	return 0;
}

int r3dFixture_SetCollideWithMaskBits(lua_State* L) {
	Binder binder(L);
	rp3d::ProxyShape* shape = static_cast<rp3d::ProxyShape*>(binder.getInstance(
			"r3dFixture", 1));
	shape->setCollideWithMaskBits(luaL_checkinteger(L, 2));
	return 0;
}

int r3dFixture_RayCast(lua_State* L) {
	Binder binder(L);
	rp3d::ProxyShape* shape = static_cast<rp3d::ProxyShape*>(binder.getInstance(
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

int r3dFixture_TestPointInside(lua_State* L) {
	Binder binder(L);
	rp3d::ProxyShape* shape = static_cast<rp3d::ProxyShape*>(binder.getInstance(
			"r3dFixture", 1));
	rp3d::Vector3 pt;
	TO_VECTOR(L, 2, pt);
	lua_pushboolean(L,shape->testPointInside(pt));

	return 1;
}

static int r3dWorld_DestroyJoint(lua_State* L) {
	Binder binder(L);
	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::Joint* joint = static_cast<rp3d::Joint*>(binder.getInstance(
			"r3dJoint", 2));
	lua_getfield(L, 1, "joints_");
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
	lua_getfield(L, 1, "joints_");
	lua_pushvalue(L, -2);
	lua_pushvalue(L, -1);
	lua_rawset(L, -3);
	lua_pop(L, 1);
	lua_getfield(L, 2, "joints_");
	lua_pushvalue(L, -2);
	lua_pushvalue(L, -1);
	lua_rawset(L, -3);
	lua_pop(L, 1);
	lua_getfield(L, 3, "joints_");
	lua_pushvalue(L, -2);
	lua_pushvalue(L, -1);
	lua_rawset(L, -3);
	lua_pop(L, 1);
}

static int r3dWorld_CreateBallAndSocketJoint(lua_State* L) {
	Binder binder(L);

	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
	rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
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

	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
	rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
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

	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
	rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
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

	rp3d::DynamicsWorld* world =
			static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body1 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 2));
	rp3d::RigidBody* body2 = static_cast<rp3d::RigidBody*>(binder.getInstance(
			"r3dBody", 3));
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

int loader(lua_State *L) {
//StackChecker checker(L, "Box2DBinder2::loader", 1);

	Binder binder(L);

	const luaL_Reg r3dWorld_functionList[] = {
			{ "createBody",	r3dWorld_CreateBody },
			{ "destroyBody", r3dWorld_DestroyBody },
			{ "step", r3dWorld_Step },
			{ "raycast", r3dWorld_RayCast },
			{ "testOverlap", r3dWorld_TestOverlap },
			{ "testCollision", r3dWorld_TestCollision },
			{ "testAABBOverlap", r3dWorld_TestAABBOverlap },
			{ "setEventListener", r3dWorld_SetEventListener },
			{ "createBallAndSocketJoint", r3dWorld_CreateBallAndSocketJoint },
			{ "createHingeJoint", r3dWorld_CreateHingeJoint },
			{ "createSliderJoint", r3dWorld_CreateSliderJoint },
			{ "createFixedJoint", r3dWorld_CreateFixedJoint },
			{ "destroyJoint", r3dWorld_DestroyJoint },
			{ NULL, NULL }, };
	binder.createClass("r3dWorld", NULL/*"EventDispatcher"*/, r3dWorld_create,
			r3dWorld_destruct, r3dWorld_functionList);
	const luaL_Reg r3dBody_functionList[] = {
			{ "createFixture", r3dBody_CreateFixture },
			{ "destroyFixture", r3dBody_DestroyFixture },
			{ "getTransform", r3dBody_GetTransform },
			{ "setTransform", r3dBody_SetTransform },
			{ "getMaterial", r3dBody_GetMaterial },
			{ "setMaterial", r3dBody_SetMaterial },
			{ "setType", r3dBody_SetType },
			{ "enableGravity", r3dBody_EnableGravity },
			{ "setIsAllowedToSleep", r3dBody_SetIsAllowedToSleep },
			{ "applyForce", r3dBody_ApplyForce },
			{ "applyTorque", r3dBody_ApplyTorque },
			{ "raycast", r3dBody_RayCast },
			{ "testPointInside", r3dBody_TestPointInside },
			{ NULL, NULL }, };
	binder.createClass("r3dBody", NULL/*"EventDispatcher"*/, NULL, NULL,
			r3dBody_functionList);
	const luaL_Reg r3dFixture_functionList[] = {
			{ "setCollisionCategoryBits", r3dFixture_SetCollisionCategoryBits },
			{ "setCollideWithMaskBits",	r3dFixture_SetCollideWithMaskBits },
			{ "raycast", r3dFixture_RayCast },
			{ "testPointInside", r3dFixture_TestPointInside },
			{ NULL, NULL }, };
	binder.createClass("r3dFixture", NULL/*"EventDispatcher"*/, NULL, NULL,
			r3dFixture_functionList);
	const luaL_Reg r3dShape_functionList[] = { { NULL, NULL }, };
	binder.createClass("r3dShape", NULL/*"EventDispatcher"*/, NULL,
			r3dShape_destruct, r3dShape_functionList);
	const luaL_Reg r3dBoxShape_functionList[] = { { NULL, NULL }, };
	binder.createClass("r3dBoxShape", "r3dShape", r3dBoxShape_create, NULL,
			r3dBoxShape_functionList);
	binder.createClass("r3dSphereShape", "r3dShape", r3dSphereShape_create,
			NULL, r3dBoxShape_functionList);
	binder.createClass("r3dCapsuleShape", "r3dShape", r3dCapsuleShape_create,
			NULL, r3dBoxShape_functionList);
	binder.createClass("r3dConvexMeshShape", "r3dShape", r3dConvexMeshShape_create,
				NULL, r3dBoxShape_functionList);
	binder.createClass("r3dConcaveMeshShape", "r3dShape", r3dConcaveMeshShape_create,
				NULL, r3dBoxShape_functionList);
	binder.createClass("r3dHeightFieldShape", "r3dShape", r3dHeightFieldShape_create,
				NULL, r3dBoxShape_functionList);

	const luaL_Reg r3dJoint_functionList[] = { { NULL, NULL }, };
	binder.createClass("r3dJoint", NULL/*"EventDispatcher"*/, NULL, NULL,
			r3dJoint_functionList);
	const luaL_Reg r3dBallAndSocketJoint_functionList[] = { { NULL, NULL }, };
	binder.createClass("r3dBoxAndSocketJoint", "r3dJoint", NULL, NULL,
			r3dBallAndSocketJoint_functionList);
	const luaL_Reg r3dHingeJoint_functionList[] = { { NULL, NULL }, };
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
	lua_pushvalue(L, -1);
	lua_setglobal(L, "r3d");

	return 1;
}

static void g_initializePlugin(lua_State *L) {
	::L = L;
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "reactphysics3d");

	lua_pop(L, 2);
	luaL_newweaktable(L);
	luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_b2);
}

static void g_deinitializePlugin(lua_State *_UNUSED(L)) {
}

#ifdef QT_NO_DEBUG
REGISTER_PLUGIN_NAMED("ReactPhysics3D", "0.7.1", reactphysics3d)
#elif defined(TARGET_OS_MAC) || defined(_MSC_VER)
REGISTER_PLUGIN_STATICNAMED_CPP("ReactPhysics3D", "0.7.1", reactphysics3d)
#else
REGISTER_PLUGIN_NAMED("ReactPhysics3D", "0.7.1", reactphysics3d)
#endif
