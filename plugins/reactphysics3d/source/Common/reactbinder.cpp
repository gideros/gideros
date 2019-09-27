#include "gstdio.h"
#include "application.h"
#include "lua.hpp"
#include "luaapplication.h"
#include "luautil.h"
#include "gplugin.h"
#include "binder.h"
#include "gglobal.h"

#include "reactphysics3d.h"
using namespace reactphysics3d;


#define _UNUSED(n)


typedef unsigned char Uint8;
static Application* application;
static lua_State *L;


static int r3dWorld_create(lua_State* L)
{
	Binder binder(L);

	rp3d::Vector3 gravity;
	gravity.x=luaL_checknumber(L,1);
	gravity.y=luaL_checknumber(L,2);
	gravity.z=luaL_checknumber(L,3);
	rp3d::DynamicsWorld *world=new rp3d::DynamicsWorld(gravity);

	binder.pushInstance("r3dWorld", world);
/*
	lua_newtable(L);
	lua_setfield(L, -2, "__bodies");

	lua_newtable(L);
	lua_setfield(L, -2, "__joints");

	lua_pushlightuserdata(L, world);
	lua_pushvalue(L, -2);
	setb2(L);
*/
/*
	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);				// remove global "Event"

	lua_pushvalue(L, -1);	// duplicate Event.new
	lua_pushstring(L, b2WorldED::BEGIN_CONTACT.type());
	lua_call(L, 1, 1); // call Event.new
	lua_setfield(L, -3, "__beginContactEvent");
*/

	return 1;
}

int r3dWorld_destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	rp3d::DynamicsWorld* world = static_cast<rp3d::DynamicsWorld*>(ptr);
	delete world;

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

static void toTransform(lua_State *L,int n,rp3d::Transform &transform) {
	Binder binder(L);
	lua_pushvalue(L,n);
	if (binder.isInstanceOf("Matrix",-1)) {
		float mat[16];
		lua_getfield(L,-1,"getMatrix");
		lua_pushvalue(L,-2);
		lua_call(L,1,16);
		for (size_t k=0;k<16;k++)
			mat[k]=luaL_optnumber(L,-16+k,0);
		lua_pop(L,17);
		transform.setFromOpenGL(mat);
	}
}

int r3dWorld_CreateBody(lua_State* L)
{
	Binder binder(L);
	rp3d::DynamicsWorld* world = static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::Transform transform;
	toTransform(L,2,transform);
	rp3d::RigidBody *body=world->createRigidBody(transform);
	binder.pushInstance("r3dBody", body);
	lua_newtable(L);
	lua_setfield(L,-2,"__fixtures");

	return 1;
}

int r3dWorld_DestroyBody(lua_State* L)
{
	Binder binder(L);
	rp3d::DynamicsWorld* world = static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance("r3dBody", 2));
	world->destroyRigidBody(body);
	binder.setInstance(2,NULL);

	return 0;
}

int r3dWorld_Step(lua_State* L)
{
	Binder binder(L);
	rp3d::DynamicsWorld* world = static_cast<rp3d::DynamicsWorld*>(binder.getInstance("r3dWorld", 1));
	world->update(luaL_checknumber(L,2));

	return 0;
}

int r3dBody_CreateFixture(lua_State* L)
{
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance("r3dBody", 1));
	rp3d::CollisionShape* shape = static_cast<rp3d::CollisionShape*>(binder.getInstance("r3dShape", 2));
	rp3d::Transform transform;
	toTransform(L,3,transform);
	float mass=luaL_optnumber(L,4,1.0);
	rp3d::ProxyShape *fixture=body->addCollisionShape(shape,transform,mass);
	binder.pushInstance("r3dFixture", fixture);
	//Link shape to body
	lua_getfield(L,1,"__fixtures");
	lua_pushvalue(L,-2);
	lua_pushvalue(L,2);
	lua_settable(L,-3);
	lua_pop(L,1);

	return 1;
}

int r3dBody_DestroyFixture(lua_State* L)
{
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance("r3dBody", 1));
	rp3d::ProxyShape* shape = static_cast<rp3d::ProxyShape*>(binder.getInstance("r3dFixture", 2));
	body->removeCollisionShape(shape);

	//Unlink shape from body
	lua_getfield(L,1,"__fixtures");
	lua_pushvalue(L,2);
	lua_pushnil(L);
	lua_settable(L,-3);
	lua_pop(L,1);

	binder.setInstance(2,NULL);
	return 0;
}

int r3dBody_GetTransform(lua_State* L)
{
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance("r3dBody", 1));
	const rp3d::Transform t=body->getTransform();
	float mat[16];
	t.getOpenGLMatrix(mat);

	lua_getglobal(L,"Matrix");
	lua_getfield(L,-1,"new");
	lua_call(L,0,1);
	lua_getfield(L,-1,"setMatrix");
	lua_pushvalue(L,-2);
	for (size_t k=0;k<16;k++)
		lua_pushnumber(L,mat[k]);
	lua_call(L,17,0);
	lua_remove(L,-2);
	return 1;
}

int r3dBody_SetType(lua_State* L)
{
	Binder binder(L);
	rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(binder.getInstance("r3dBody", 1));
	body->setType((BodyType)luaL_checkinteger(L,2));
	return 0;
}

static int r3dBoxShape_create(lua_State* L)
{
	Binder binder(L);

	rp3d::Vector3 sz;
	sz.x=luaL_checknumber(L,1);
	sz.y=luaL_checknumber(L,2);
	sz.z=luaL_checknumber(L,3);
	rp3d::BoxShape *shape=new rp3d::BoxShape(sz);

	binder.pushInstance("r3dBoxShape", shape);
	return 1;
}
static int r3dSphereShape_create(lua_State* L)
{
	Binder binder(L);

	rp3d::SphereShape *shape=new rp3d::SphereShape(luaL_checknumber(L,1));

	binder.pushInstance("r3dSphereShape", shape);
	return 1;
}
static int r3dCapsuleShape_create(lua_State* L)
{
	Binder binder(L);

	rp3d::CapsuleShape *shape=new rp3d::CapsuleShape(luaL_checknumber(L,1),luaL_checknumber(L,2));

	binder.pushInstance("r3dCapsuleShape", shape);
	return 1;
}

static int r3dShape_destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	rp3d::CollisionShape* shape = static_cast<rp3d::CollisionShape*>(ptr);
	delete shape;

	return 0;
}

int loader(lua_State *L) {
//StackChecker checker(L, "Box2DBinder2::loader", 1);

	Binder binder(L);

	const luaL_Reg r3dWorld_functionList[] = {
		{"createBody", r3dWorld_CreateBody},
		{"destroyBody", r3dWorld_DestroyBody},
		{"step", r3dWorld_Step},
		{NULL, NULL},
	};
	binder.createClass("r3dWorld", NULL/*"EventDispatcher"*/, r3dWorld_create, r3dWorld_destruct, r3dWorld_functionList);
	const luaL_Reg r3dBody_functionList[] = {
			{"createFixture", r3dBody_CreateFixture},
			{"destroyFixture", r3dBody_DestroyFixture},
			{"getTransform", r3dBody_GetTransform},
			{"setType", r3dBody_SetType},
		{NULL, NULL},
	};
	binder.createClass("r3dBody", NULL/*"EventDispatcher"*/, NULL,NULL, r3dBody_functionList);
	const luaL_Reg r3dFixture_functionList[] = {
		{NULL, NULL},
	};
	binder.createClass("r3dFixture", NULL/*"EventDispatcher"*/, NULL,NULL, r3dFixture_functionList);
	const luaL_Reg r3dShape_functionList[] = {
		{NULL, NULL},
	};
	binder.createClass("r3dShape", NULL/*"EventDispatcher"*/, NULL,r3dShape_destruct, r3dShape_functionList);
	const luaL_Reg r3dBoxShape_functionList[] = {
		{NULL, NULL},
	};
	binder.createClass("r3dBoxShape","r3dShape", r3dBoxShape_create,NULL, r3dBoxShape_functionList);
	binder.createClass("r3dSphereShape","r3dShape", r3dSphereShape_create,NULL, r3dBoxShape_functionList);
	binder.createClass("r3dCapsuleShape","r3dShape", r3dCapsuleShape_create,NULL, r3dBoxShape_functionList);

	lua_getglobal(L,"r3dBody");
	lua_pushinteger(L, (int)BodyType::STATIC);
	lua_setfield(L, -2, "STATIC_BODY");

	lua_pushinteger(L, (int)BodyType::KINEMATIC);
	lua_setfield(L, -2, "KINEMATIC_BODY");

	lua_pushinteger(L, (int)BodyType::DYNAMIC);
	lua_setfield(L, -2, "DYNAMIC_BODY");
	lua_pop(L,1);

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
	RELOC_CLASS("BoxShape")
	RELOC_CLASS("SphereShape")
	RELOC_CLASS("CapsuleShape")
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
