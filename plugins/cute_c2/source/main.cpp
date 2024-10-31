//#define IS_BETA_BUILD
#define IMPLEMENT_METAMETHODS

#define _UNUSED(n)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"

#include "lua.hpp"
#include "luautil.h"

#include "gplugin.h"

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"
#include <memory>
#ifdef IS_BETA_BUILD
#define PLUGIN_NAME "CuteC2_beta"
#else
#define PLUGIN_NAME "CuteC2"
#endif

#define LUA_ASSERT(EXP, MSG) if (!(EXP)) { lua_pushstring(L, MSG); lua_error(L); }
#define LUA_ASSERTF(EXP, FMT, ...) if (!(EXP)) { lua_pushfstring(L, FMT, __VA_ARGS__); lua_error(L); }
#define LUA_THROW_ERROR(MSG) lua_pushstring(L, MSG); lua_error(L);
#define LUA_THROW_ERRORF(FMT, ...) lua_pushfstring(L, FMT, __VA_ARGS__); lua_error(L);
#define LUA_PRINTF(FMT, ...) lua_getglobal(L, "print"); lua_pushfstring(L, FMT, __VA_ARGS__); lua_call(L, 1, 0);
#define LUA_PRINT(MSG) lua_getglobal(L, "print"); lua_pushstring(L, MSG); lua_call(L, 1, 0);
#define BIND_IENUM(value, name) lua_pushinteger(L, value); lua_setfield(L, -2, name);
#define BIND_FENUM(value, name) lua_pushnumber(L, value); lua_setfield(L, -2, name);

static lua_State* L;
static char keyWeak = ' ';

namespace cute_c2_impl
{

struct Poly
{
	c2x transform;
	c2Poly body;
	
	Poly()
	{
		body = c2Poly();
		transform = c2xIdentity();
	}
};

////////////////////////////////////////////////////////////////////////////////
///
/// DEBUG TOOL
///
////////////////////////////////////////////////////////////////////////////////

static int DUMP_INDEX = 0;

static void stackDump(lua_State* L, const char* prefix = "")
{
	int i = lua_gettop(L);
	LUA_PRINTF("----------------	%d	----------------\n>%s\n---------------- Stack Dump ----------------", DUMP_INDEX, prefix);
	while (i)
	{
		int t = lua_type(L, i);
		lua_getglobal(L, "print");
		switch (t)
		{
			case LUA_TSTRING: lua_pushfstring(L, "[S] %d:'%s'", i, lua_tostring(L, i));
			case LUA_TBOOLEAN: lua_pushfstring(L, "[B] %d: %s", i, lua_toboolean(L, i) ? "true" : "false");
			case LUA_TNUMBER: lua_pushfstring(L, "[N] %d: %f", i, lua_tonumber(L, i));
			case LUA_TVECTOR:
			{
				const float* vec = lua_tovector(L, i);
				lua_pushfstring(L, "[V] %d: {%f, %f, %f}", i, vec[0], vec[1], vec[2]);
			}
			break;
			default: lua_pushfstring(L, "[D] %d: %s", i, lua_typename(L, t));
		}
		i--;
		lua_call(L, 1, 0);
	}
	LUA_PRINT("------------ Stack Dump Finished ------------\n");

	DUMP_INDEX++;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// HELPERS
///
/////////////////////////////////////////////////////////////////////////////////////////////

void* getRawPtr(lua_State* L, int idx)
{
	lua_getfield(L, idx, "__userdata"); // get adress
	if (lua_isnil(L, -1) != 0)
	{
		lua_pop(L, 1);
		luaL_error(L, "index '__userdata' cannot be found");
	}
	
	void* ptr = *(void**)lua_touserdata(L, -1);
	lua_pop(L, 1);
	
	return ptr;
}

void* getRawPtrCheck(lua_State* L, int idx, c2x** transform)
{
	if (g_isInstanceOf(L, "c2Poly", idx))
	{
		Poly* poly = static_cast<Poly*>(g_getInstance(L, "c2Poly", idx));
		*transform = &poly->transform;
		return (void*)(&poly->body);
	}
	
	return getRawPtr(L, idx);
}

C2_TYPE getShapeType(lua_State* L, int idx)
{
	lua_rawgetfield(L, idx, "__shapeType");
	C2_TYPE shapeType = (C2_TYPE)lua_tointeger(L, -1);
	lua_pop(L, 1);
	return shapeType;
}

void pushPoint(lua_State* L, float x, float y)
{
	lua_createtable(L, 0, 2);
	
	lua_pushnumber(L, x);
	lua_setfield(L, -2, "x");
	
	lua_pushnumber(L, y);
	lua_setfield(L, -2, "y");
}

int pushManifold(lua_State* L, c2Manifold m)
{
	// {count = value, depth = {value1, value2}, contact_points = {{x = value, y = value}, {x = value, y = value}}, normal = {x = value, y = value}}
	lua_createtable(L, 0, 4);
	lua_pushnumber(L, m.count);
	lua_setfield(L, -2, "count");
	
	lua_createtable(L, 2, 0);
	lua_pushnumber(L, m.depths[0]);
	lua_rawseti(L, -2, 1);
	lua_pushnumber(L, m.depths[1]);
	lua_rawseti(L, -2, 2);
	lua_setfield(L, -2, "depths");
	
	lua_createtable(L, 2, 0);
	
	pushPoint(L, m.contact_points[0].x, m.contact_points[0].y);
	lua_rawseti(L, -2, 1);
	
	pushPoint(L, m.contact_points[1].x, m.contact_points[1].y);
	lua_rawseti(L, -2, 2);
	
	lua_setfield(L, -2, "contact_points");
	
	pushPoint(L, m.n.x, m.n.y);
	lua_setfield(L, -2, "normal");
	
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
///
/// TEMPLATES
///
////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T* getPtr(lua_State* L, const char* name, int idx = 1)
{
	return static_cast<T*>(g_getInstance(L, name, idx));
}

inline void setPtr(lua_State* L, void* ptr)
{
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, ptr);
	lua_pop(L, 1);
}

template<class T>
int objRayTest(lua_State* L, const char* classname, C2_TYPE obj_type)
{
	T* obj = getPtr<T>(L, classname, 1);
	c2Raycast out;
	c2Ray ray;
	
	if (lua_gettop(L) > 3)
	{
		ray = c2Ray();
		ray.p = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
		ray.d = c2V(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
		ray.t = luaL_checknumber(L, 6);
	}
	else
	{
		ray = *getPtr<c2Ray>(L, "c2Ray", 2);
	}
	
	int result = c2CastRay(ray, obj, NULL, obj_type, &out);
	
	lua_pushboolean(L, result);
	lua_pushnumber(L, out.n.x);
	lua_pushnumber(L, out.n.y);
	lua_pushnumber(L, out.t);
	return 4;
}

template<class T>
void objInflate(lua_State* L, const char* classname, C2_TYPE obj_type)
{
	T* obj = getPtr<T>(L, classname, 1);
	float skin_factor = luaL_checknumber(L, 2);
	c2Inflate(obj, obj_type, skin_factor);
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// ENUMS
///
/////////////////////////////////////////////////////////////////////////////////////////////

void bindEnums(lua_State* L)
{
	lua_getglobal(L, "CuteC2");

	BIND_IENUM(C2_TYPE_NONE, "TYPE_NONE");
	BIND_IENUM(C2_TYPE_CIRCLE, "TYPE_CIRCLE");
	BIND_IENUM(C2_TYPE_AABB, "TYPE_AABB");
	BIND_IENUM(C2_TYPE_CAPSULE, "TYPE_CAPSULE");
	BIND_IENUM(C2_TYPE_POLY, "TYPE_POLY");
	BIND_IENUM(C2_MAX_POLYGON_VERTS, "MAX_POLYGON_VERTS");

	lua_pop(L, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Shapes
///
/////////////////////////////////////////////////////////////////////////////////////////////


// used by generic functions: GJK, TOI, Collide, Collided
void checkABShape(lua_State* L, void** A, int indA, c2x** ax, void** B, int indB, c2x** bx)
{
	c2x identity = c2xIdentity();
	std::shared_ptr<c2x> def = std::make_shared<c2x>(identity);
	
	if (g_isInstanceOf(L, "c2Poly", indA))
	{
		Poly* poly = getPtr<Poly>(L, "c2Poly", indA);
		*A = (void*)(&poly->body);
		*ax = &poly->transform;
	}
	else
	{
		*A = getRawPtr(L, indA);
		*ax = def.get();
	}
	
	if (g_isInstanceOf(L, "c2Poly", indB))
	{
		Poly* poly = getPtr<Poly>(L, "c2Poly", indB);
		*B = (void*)(&poly->body);
		*bx = &poly->transform;
	}
	else
	{
		*B = getRawPtr(L, indB);
		*bx = def.get();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// CIRCLE
///
/////////////////////////////////////////////////////////////////////////////////////////////

int createCircle(lua_State* L)
{
	c2Circle* circle = new c2Circle();
	circle->p = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	circle->r = luaL_checknumber(L, 3);
	g_pushInstance(L, "c2Circle", circle);
	
	// Store shape directly in table to be able to
	// use global raycast & inflate functions (e.g. CuteC2.raycast(myShape, myRay))
	lua_pushinteger(L, C2_TYPE_CIRCLE);
	lua_setfield(L, -2, "__shapeType");
	
	setPtr(L, circle);
	
	return 1;
}

int setCirclePosition(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	circle->p.x = luaL_checknumber(L, 2);
	circle->p.y = luaL_checknumber(L, 3);
	return 0;
}

int getCirclePosition(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	lua_pushnumber(L, circle->p.x );
	lua_pushnumber(L, circle->p.y );
	return 2;
}

int setCirclePositionX(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	circle->p.x = luaL_checknumber(L, 2);
	return 0;
}

int getCirclePositionX(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	lua_pushnumber(L, circle->p.x);
	return 1;
}

int setCirclePositionY(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	circle->p.y = luaL_checknumber(L, 2);
	return 0;
}

int getCirclePositionY(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	lua_pushnumber(L, circle->p.y);
	return 1;
}

int setCircleRadius(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	circle->r = luaL_checknumber(L, 2);
	return 0;
}

int getCircleRadius(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	lua_pushnumber(L, circle->r);
	return 1;
}

int cirlceGetData(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	lua_pushnumber(L, circle->p.x);
	lua_pushnumber(L, circle->p.y);
	lua_pushnumber(L, circle->r);
	return 3;
}

int getCirlceBoundingBox(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	float size = circle->r * 2.0f;
	
	lua_pushnumber(L, circle->p.x - circle->r);
	lua_pushnumber(L, circle->p.y - circle->r);
	lua_pushnumber(L, circle->p.x + circle->r);
	lua_pushnumber(L, circle->p.x + circle->r);
	lua_pushnumber(L, size);
	lua_pushnumber(L, size);
	return 6;
}

int cirlceRayTest(lua_State* L)
{
	return objRayTest<c2Circle>(L, "c2Circle", C2_TYPE_CIRCLE);
}

int circleInflate(lua_State* L)
{
	objInflate<c2Circle>(L, "c2Circle", C2_TYPE_CIRCLE);
	return 0;
}

int circleHitTest(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	c2v point = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	int result = c2CircleToPoint(*circle, point);
	lua_pushboolean(L, result);
	return 1;
}

int circleMove(lua_State* L)
{
	c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
	float dx = luaL_checknumber(L, 2);
	float dy = luaL_checknumber(L, 3);
	circle->p.x += dx;
	circle->p.y += dy;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// AABB
///
/////////////////////////////////////////////////////////////////////////////////////////////

int createAABB(lua_State* L)
{
	c2AABB* aabb = new c2AABB();
	aabb->min = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	aabb->max = c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
	g_pushInstance(L, "c2AABB", aabb);

	lua_pushinteger(L, C2_TYPE_AABB);
	lua_setfield(L, -2, "__shapeType");
	
	setPtr(L, aabb);

	return 1;
}

int setAABBMinPosition(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	aabb->min.x = x;
	aabb->min.y = y;
	return 0;
}

int getAABBMinPosition(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	lua_pushnumber(L, aabb->min.x);
	lua_pushnumber(L, aabb->min.y);
	return 2;
}

int setAABBMaxPosition(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	aabb->max.x = x;
	aabb->max.y = y;
	return 0;
}

int getAABBMaxPosition(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	lua_pushnumber(L, aabb->max.x);
	lua_pushnumber(L, aabb->max.y);
	return 0;
}

int setAABBCenterPosition(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	c2v hsize = c2Mulvs(c2Sub(aabb->max, aabb->min), 0.5f);
	aabb->min.x = x - hsize.x;
	aabb->min.y = y - hsize.y;
	aabb->max.x = x + hsize.x;
	aabb->max.y = y + hsize.y;
	return 0;
}

int getAABBCenterPosition(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	c2v hsize = c2Mulvs(c2Sub(aabb->max, aabb->min), 0.5f);
	lua_pushnumber(L, aabb->min.x + hsize.x);
	lua_pushnumber(L, aabb->min.y + hsize.y);
	return 2;
}


int setAABBMinX(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	aabb->min.x = luaL_checknumber(L, 2);
	return 0;
}

int getAABBMinX(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	lua_pushnumber(L, aabb->min.x);
	return 1;
}

int setAABBMinY(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	aabb->min.y = luaL_checknumber(L, 2);
	return 0;
}

int getAABBMinY(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	lua_pushnumber(L, aabb->min.y);
	return 1;
}

int setAABBMaxX(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	aabb->max.x = luaL_checknumber(L, 2);
	return 0;
}

int getAABBMaxX(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	lua_pushnumber(L, aabb->max.x);
	return 1;
}

int setAABBMaxY(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	aabb->max.y = luaL_checknumber(L, 2);
	return 0;
}

int getAABBMaxY(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	lua_pushnumber(L, aabb->max.y);
	return 1;
}

int setAABBCenterPositionX(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float x = luaL_checknumber(L, 2);
	float hw = (aabb->max.x - aabb->min.x) * 0.5f;
	aabb->min.x = x - hw;
	aabb->max.x = x + hw;
	return 0;
}

int getAABBCenterPositionX(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float hw = (aabb->max.x - aabb->min.x) * 0.5f;
	lua_pushnumber(L, aabb->min.x + hw);
	return 1;
}

int setAABBCenterPositionY(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float y = luaL_checknumber(L, 2);
	float hh = (aabb->max.y - aabb->min.y) * 0.5f;
	aabb->min.y = y - hh;
	aabb->max.y = y + hh;
	return 0;
}

int getAABBCenterPositionY(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float hh = (aabb->max.y - aabb->min.y) * 0.5f;
	lua_pushnumber(L, aabb->min.y + hh);
	return 1;
}


int setAABBSize(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	c2v newSize= c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	c2v oldSize = c2Sub(aabb->max, aabb->min);
	c2v diff = c2Mulvs(c2Sub(newSize, oldSize), 0.5f);
	aabb->min.x = aabb->min.x - diff.x;
	aabb->min.y = aabb->min.y - diff.y;
	aabb->max.x = aabb->max.x + diff.x;
	aabb->max.y = aabb->max.y + diff.y;
	return 0;
}

int getAABBSize(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	c2v size = c2Sub(aabb->max, aabb->min);
	lua_pushnumber(L, size.x);
	lua_pushnumber(L, size.y);
	return 2;
}

int setAABBhalfSize(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	c2v newSize= c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	c2v oldSize = c2Sub(aabb->max, aabb->min);
	c2v diff = c2Sub(newSize, oldSize);
	aabb->min.x = aabb->min.x - diff.x;
	aabb->min.y = aabb->min.y - diff.y;
	aabb->max.x = aabb->max.x + diff.x;
	aabb->max.y = aabb->max.y + diff.y;
	return 0;
}

int getAABBhalfSize(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	c2v size = c2Mulvs(c2Sub(aabb->max, aabb->min), 0.5f);
	lua_pushnumber(L, size.x);
	lua_pushnumber(L, size.y);
	return 2;
}

int setAABBWidth(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float newW = luaL_checknumber(L, 2);
	float oldW = aabb->max.x - aabb->min.x;
	float d = (oldW - newW) * 0.5f;
	aabb->min.x = aabb->min.x - d;
	aabb->max.x = aabb->max.x + d;
	return 0;
}

int getAABBWidth(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float w = aabb->max.x - aabb->min.x;
	lua_pushnumber(L, w);
	return 1;
}

int setAABBhalfWidth(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float newW = luaL_checknumber(L, 2);
	float oldW = aabb->max.x - aabb->min.x;
	float diff = newW - oldW;
	aabb->min.x = aabb->min.x - diff;
	aabb->max.x = aabb->max.x + diff;
	return 0;
}

int getAABBhalfWidth(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float w = (aabb->max.x - aabb->min.x) * 0.5f;
	lua_pushnumber(L, w);
	return 1;
}

int setAABBHeight(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float newH = luaL_checknumber(L, 2);
	float oldH = aabb->max.y - aabb->min.y;
	float d = (oldH - newH) * 0.5f;
	aabb->min.y = aabb->min.y - d;
	aabb->max.y = aabb->max.y + d;
	return 0;
}

int getAABBHeight(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float h = aabb->max.y - aabb->min.y;
	lua_pushnumber(L, h);
	return 1;
}

int setAABBhalfHeight(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float newH = luaL_checknumber(L, 2);
	float oldH = aabb->max.y - aabb->min.y;
	float diff = newH - oldH;
	aabb->min.y = aabb->min.y - diff;
	aabb->max.y = aabb->max.y + diff;
	return 0;
}

int getAABBhalfHeight(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float h = (aabb->max.y - aabb->min.y) * 0.5f;
	lua_pushnumber(L, h);
	return 1;
}

int getAABBData(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	lua_pushnumber(L, aabb->min.x);
	lua_pushnumber(L, aabb->min.y);
	lua_pushnumber(L, aabb->max.x);
	lua_pushnumber(L, aabb->max.y);
	lua_pushnumber(L, aabb->max.x - aabb->min.x);
	lua_pushnumber(L, aabb->max.y - aabb->min.y);
	return 6;
}

int getAABBboundingBox(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	lua_pushnumber(L, aabb->min.x);
	lua_pushnumber(L, aabb->min.y);
	lua_pushnumber(L, aabb->max.x);
	lua_pushnumber(L, aabb->max.y);
	return 4;
}

int AABBRayTest(lua_State* L)
{
	return objRayTest<c2AABB>(L, "c2AABB", C2_TYPE_AABB);
}

int AABBInflate(lua_State* L)
{
	objInflate<c2AABB>(L, "c2AABB", C2_TYPE_AABB);
	return 0;
}

int AABBHitTest(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	c2v point = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	int result = c2AABBtoPoint(*aabb, point);
	lua_pushboolean(L, result);
	return 1;
}

int AABBmove(lua_State* L)
{
	c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
	float dx = luaL_checknumber(L, 2);
	float dy = luaL_checknumber(L, 3);
	aabb->min.x += dx;
	aabb->min.y += dy;
	aabb->max.x += dx;
	aabb->max.y += dy;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// CAPSULE
///
/////////////////////////////////////////////////////////////////////////////////////////////

int createCapsule(lua_State* L)
{
	c2Capsule* capsule = new c2Capsule();
	c2v pos = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	float h = luaL_checknumber(L, 3);
	capsule->a = pos;
	capsule->b = c2V(pos.x, pos.y + h);
	capsule->r = luaL_checknumber(L, 4);
	g_pushInstance(L, "c2Capsule", capsule);

	lua_pushinteger(L, C2_TYPE_CAPSULE);
	lua_setfield(L, -2, "__shapeType");
	
	setPtr(L, capsule);

	return 1;
}

int setCapsulePosition(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float hh = (capsule->b.y - capsule->a.y) * 0.5f;
	capsule->a.x = x;
	capsule->a.y = y - hh;
	capsule->b.x = x;
	capsule->b.y = y + hh;
	return 0;
}

int getCapsulePosition(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float hh = (capsule->b.y - capsule->a.y) * 0.5f;
	lua_pushnumber(L, capsule->a.x);
	lua_pushnumber(L, capsule->a.y + hh);
	return 2;
}

int setCapsuleTipPosition(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float h = capsule->b.y - capsule->a.y;
	capsule->a.x = x;
	capsule->a.y = y;
	capsule->b.x = x;
	capsule->b.y = y + h;
	return 0;
}

int getCapsuleTipPosition(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->a.x);
	lua_pushnumber(L, capsule->a.y);
	return 2;
}

int setCapsuleBasePosition(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float h = capsule->b.y - capsule->a.y;
	capsule->a.x = x;
	capsule->a.y = y - h;
	capsule->b.x = x;
	capsule->b.y = y;
	return 0;
}

int getCapsuleBasePosition(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->b.x);
	lua_pushnumber(L, capsule->b.y);
	return 2;
}

int setCapsuleX(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float x = luaL_checknumber(L, 2);
	capsule->a.x = x;
	capsule->b.x = x;
	return 0;
}

int getCapsuleX(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->a.x);
	return 1;
}

int setCapsuleY(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float y = luaL_checknumber(L, 2);
	float hh = (capsule->b.y - capsule->a.y) * 0.5f;
	capsule->a.y = y - hh;
	capsule->b.y = y + hh;
	return 0;
}

int getCapsuleY(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float hh = (capsule->b.y - capsule->a.y) * 0.5f;
	lua_pushnumber(L, capsule->a.y + hh);
	return 1;
}

int setCapsuleTipX(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float x = luaL_checknumber(L, 2);
	capsule->a.x = x;
	capsule->b.x = x;
	return 0;
}

int getCapsuleTipX(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->a.x);
	return 1;
}

int setCapsuleTipY(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float y = luaL_checknumber(L, 2);
	float h = capsule->b.y - capsule->a.y;
	capsule->a.y = y;
	capsule->b.y = y + h;
	return 0;
}

int getCapsuleTipY(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->a.y);
	return 1;
}

int setCapsuleBaseX(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float x = luaL_checknumber(L, 2);
	capsule->b.x = x;
	capsule->a.x = x;
	return 0;
}

int getCapsuleBaseX(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->b.x);
	return 1;
}

int setCapsuleBaseY(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float y = luaL_checknumber(L, 2);
	float h = capsule->b.y - capsule->a.y;
	capsule->b.y = y;
	capsule->a.y = y - h;
	return 0;
}

int getCapsuleBaseY(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->b.y);
	return 1;
}

int setCapsuleHeight(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float h = luaL_checknumber(L, 2);
	capsule->b.y = capsule->a.y + h;
	return 0;
}

int getCapsuleHeight(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->b.y - capsule->a.y);
	return 1;
}

int setCapsuleHalfHeight(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float hh = luaL_checknumber(L, 2);
	capsule->b.y = capsule->a.y + hh + hh;
	return 0;
}

int getCapsuleHalfHeight(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float hh = (capsule->b.y - capsule->a.y) * 0.5f;
	lua_pushnumber(L, hh);
	return 1;
}

int setCapsuleRadius(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	capsule->r = luaL_checknumber(L, 2);
	return 0;
}

int getCapsuleRadius(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->r);
	return 1;
}

int getCapsuleSize(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->r);
	lua_pushnumber(L, capsule->b.y - capsule->a.y);
	return 2;
}

int setCapsuleSize(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	capsule->r = luaL_checknumber(L, 2);
	float h = luaL_checknumber(L, 3);
	capsule->b.y = capsule->a.y + h;
	return 0;
}

int getCapsuleBoundingBox(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	
	lua_pushnumber(L, capsule->a.x - capsule->r);
	lua_pushnumber(L, capsule->a.y - capsule->r);
	lua_pushnumber(L, capsule->b.x + capsule->r);
	lua_pushnumber(L, capsule->b.y + capsule->r);
	return 4;
}

int getCapsuleData(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	lua_pushnumber(L, capsule->a.x);
	lua_pushnumber(L, capsule->a.y);
	lua_pushnumber(L, capsule->b.x);
	lua_pushnumber(L, capsule->b.y);
	lua_pushnumber(L, capsule->r);
	return 5;
}

int capsuleRayTest(lua_State* L)
{
	return objRayTest<c2Capsule>(L, "c2Capsule", C2_TYPE_CAPSULE);
}

int capsuleInflate(lua_State* L)
{
	objInflate<c2Capsule>(L, "c2Capsule", C2_TYPE_CAPSULE);
	return 0;
}

int capsuleHitTest(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	c2v point = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	c2AABB aabb = c2AABB();
	aabb.min = c2V(capsule->a.x - capsule->r, capsule->a.y);
	aabb.max = c2V(capsule->a.x + capsule->r, capsule->b.y);
	
	if (c2AABBtoPoint(aabb, point))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	
	c2Circle circleA = c2Circle();
	circleA.p = capsule->a;
	circleA.r = capsule->r;
	
	if (c2CircleToPoint(circleA, point))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	
	c2Circle circleB = c2Circle();
	circleB.p = capsule->b;
	circleB.r = capsule->r;
	
	if (c2CircleToPoint(circleB, point))
	{
		lua_pushboolean(L, true);
		return 1;
	}
	
	lua_pushboolean(L, false);
	return 1;
}

int capsuleMove(lua_State* L)
{
	c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
	float dx = luaL_checknumber(L, 2);
	float dy = luaL_checknumber(L, 3);
	capsule->a.x += dx;
	capsule->a.y += dy;
	capsule->b.x += dx;
	capsule->b.y += dy;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// POLYGON
///
/////////////////////////////////////////////////////////////////////////////////////////////

void updatePointsInternal(lua_State* L, int idx, Poly* poly, bool inGlobalSpace)
{
	luaL_checktype(L, idx, LUA_TTABLE);
	int l = lua_objlen(L, idx);
	LUA_ASSERT(l % 2 == 0, "Incorrect points table size");
	l = c2Min(l, C2_MAX_POLYGON_VERTS * 2);
	poly->body.count = l / 2;

	for (int i = 0, j = 0; i < l; i += 2, ++j)
	{
		lua_rawgeti(L, idx, i + 1);
		float x = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, idx, i + 2);
		float y = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		
		if (inGlobalSpace)
		{
			poly->body.verts[j] = c2V(x - poly->transform.p.x, y - poly->transform.p.y);
		}
		else
		{
			poly->body.verts[j] = c2V(x, y);
		}
	}
	c2MakePoly(&poly->body);
}

void updateVertexNormal(Poly* poly, int idx)
{
	int next = (idx + 1) % poly->body.count;
	c2v e = c2Sub(poly->body.verts[next], poly->body.verts[idx]);
	poly->body.norms[idx] = c2Norm(c2CCW90(e));
}

C2_INLINE int getPointIndex(lua_State* L, int idx)
{
	int index = luaL_checkinteger(L, 2) - 1;
	index = c2Clamp(index, 0, C2_MAX_POLYGON_VERTS);
	return index;
}

int createPoly(lua_State* L)
{
	Poly* poly = new Poly();
	updatePointsInternal(L, 1, poly, false);
	g_pushInstance(L, "c2Poly", poly);
	
	lua_pushinteger(L, C2_TYPE_POLY);
	lua_setfield(L, -2, "__shapeType");
	
	setPtr(L, poly);
	return 1;
}

int polyUpdatePoints(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	bool inGlobalSpace = luaL_optboolean(L, 2, 0);
	
	updatePointsInternal(L, 2, poly, inGlobalSpace);
	return 0;
}

int polyUpdatePointsXY(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	bool inGlobalSpace = luaL_optboolean(L, 2, 0);
	
	luaL_checktype(L, 2, LUA_TTABLE);
	int l = lua_objlen(L, 2);
	LUA_ASSERT(l <= 0, "Incorrect points table size");
	l = c2Min(l, C2_MAX_POLYGON_VERTS);
	poly->body.count = l;

	for (int i = 0; i < l; i++)
	{
		lua_rawgeti(L, 2, i + 1);
		
		lua_rawgetfield(L, -1, "x");
		float x = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		
		lua_rawgetfield(L, -1, "y");
		float y = luaL_checknumber(L, -1);
		lua_pop(L, 2);
		
		if (inGlobalSpace)
		{
			poly->body.verts[i] = c2V(x - poly->transform.p.x, y - poly->transform.p.y);
		}
		else
		{
			poly->body.verts[i] = c2V(x, y);
		}
	}
	c2MakePoly(&poly->body);
	
	return 0;
}

int polySetVertexPosition(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);	
	int index = getPointIndex(L, 2);
	c2v position = c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
	bool inGlobalSpace = luaL_optboolean(L, 5, 0);
	
	if (inGlobalSpace)
	{
		position = c2Sub(position, poly->transform.p);
	}
	
	poly->body.verts[index] = position;
	
	int prev = index - 1 < 0 ? poly->body.count - 1 : index - 1;
	updateVertexNormal(poly, index);
	updateVertexNormal(poly, prev);
	
	return 0;
}

int polyGetPoints(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	bool inGlobalSpace = luaL_optboolean(L, 2, 0);
	
	lua_createtable(L, poly->body.count * 2, 0);
	
	for (int i = 0; i < poly->body.count; i++)
	{
		c2v& vertex = poly->body.verts[i];
		
		if (inGlobalSpace)
		{
			c2v global = c2Add(vertex, poly->transform.p);
			pushPoint(L, global.x, global.y);
		}
		else
		{
			pushPoint(L, vertex.x, vertex.y);
		}
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

int polyGetRotatedPoints(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	bool inGlobalSpace = luaL_optboolean(L, 2, 0);
	
	lua_createtable(L, poly->body.count, 0);
	
	for (int i = 0; i < poly->body.count; i++)
	{
		c2v& vertex = poly->body.verts[i];
		
		if (inGlobalSpace)
		{
			c2v pt = c2Mulrv(poly->transform.r, vertex);
			c2v gl = c2Add(pt, poly->transform.p);
			pushPoint(L, gl.x, gl.y);
		}
		else
		{
			c2v pt = c2Mulrv(poly->transform.r, vertex);
			pushPoint(L, pt.x, pt.y);
		}
		
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

int polyGetNormals(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	lua_createtable(L, poly->body.count * 2, 0);
	
	for (int i = 0; i < poly->body.count; i++)
	{
		pushPoint(L, poly->body.norms[i].x, poly->body.norms[i].y);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

int polyGetRotatedNormals(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	
	lua_createtable(L, poly->body.count, 0);
	for (int i = 0; i < poly->body.count; i++)
	{
		c2v pt = c2Mulrv(poly->transform.r, poly->body.norms[i]);
		pushPoint(L, pt.x, pt.y);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

int polyGetBoundingBox(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	bool inGlobalSpace = luaL_optboolean(L, 2, 0);
	
	c2v min = c2V(FLT_MAX, FLT_MAX);
	c2v max = c2V(-FLT_MAX, -FLT_MAX);
	c2v& pos = poly->transform.p;
	
	for (int i = 0; i < poly->body.count; i++)
	{
		c2v pr = c2Mulrv(poly->transform.r, poly->body.verts[i]);			
		min.x = c2Min(min.x, pr.x);
		min.y = c2Min(min.y, pr.y);
		max.x = c2Max(max.x, pr.x);
		max.y = c2Max(max.y, pr.y);
	}
	
	if (inGlobalSpace)
	{
		min.x += pos.x;
		min.y += pos.y;
		max.x += pos.x;
		max.y += pos.y;
	}
	
	lua_pushnumber(L, min.x);
	lua_pushnumber(L, min.y);
	lua_pushnumber(L, max.x);
	lua_pushnumber(L, max.y);
	
	return 4;
}

int polyGetData(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	bool inGlobalSpace = luaL_optboolean(L, 2, 0);
	
	lua_createtable(L, 0, 2);
	lua_createtable(L, poly->body.count * 2, 0); // points
	lua_createtable(L, poly->body.count * 2, 0); // normals
	
	for (int i = 0; i < poly->body.count; i++)
	{
		c2v& vertex = poly->body.verts[i];
		c2v& normal = poly->body.norms[i];
		
		if (inGlobalSpace)
		{
			c2v gl = c2Add(vertex, poly->transform.p);
			
			lua_pushnumber(L, gl.x);
			lua_rawseti(L, -3, i * 2 + 1);
	
			lua_pushnumber(L, gl.y);
			lua_rawseti(L, -3, i * 2 + 2);
		}
		else
		{
			lua_pushnumber(L, vertex.x);
			lua_rawseti(L, -3, i * 2 + 1);
	
			lua_pushnumber(L, vertex.y);
			lua_rawseti(L, -3, i * 2 + 2);
		}
		
		lua_pushnumber(L, normal.x);
		lua_rawseti(L, -2, i * 2 + 1);

		lua_pushnumber(L, normal.y);
		lua_rawseti(L, -2, i * 2 + 2);
	}
	lua_setfield(L, -3, "normals");
	lua_setfield(L, -2, "points");
	
	return 1;
}

int polyRayTest(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	c2Raycast out;
	
	if (lua_gettop(L) > 3)
	{
		c2Ray ray = c2Ray();
		ray.p = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
		ray.d = c2V(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
		ray.t = luaL_checknumber(L, 6);
		
		int result = c2CastRay(ray, &poly->body, &poly->transform, C2_TYPE_POLY, &out);
		
		lua_pushinteger(L, result);
		lua_pushnumber(L, out.n.x);
		lua_pushnumber(L, out.n.y);
		lua_pushnumber(L, out.t);
		return 4;
		
	}
	c2Ray A = *getPtr<c2Ray>(L, "c2Ray", 2);
	
	int result = c2CastRay(A, &poly->body, &poly->transform, C2_TYPE_POLY, &out);
	
	lua_pushboolean(L, result);
	lua_pushnumber(L, out.n.x);
	lua_pushnumber(L, out.n.y);
	lua_pushnumber(L, out.t);
	return 4;
}

int polyInflate(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	float skin_factor = luaL_checknumber(L, 2);
	c2Inflate(&poly->body, C2_TYPE_POLY, skin_factor);
	return 0;
}

int polyHitTest(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	c2v point = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	
	c2v lp = c2Sub(point, poly->transform.p);
	
	int prevSide = 0;
	bool result = true;
	
	for (int i = 0; i < poly->body.count; i++)
	{
		c2v v1 = poly->body.verts[i + 0];
		c2v v2 = poly->body.verts[(i + 1) % poly->body.count];
		
		c2v p1 = c2Mulrv(poly->transform.r, v1);
		c2v p2 = c2Mulrv(poly->transform.r, v2);
		
		c2v t1 = c2Sub(p2, p1);
		c2v t2 = c2Sub(lp, p1);
		float d = c2Det2(t1, t2);
		
		if (d == 0.0f)
		{
			result = true;
			break;
		}
		else
		{
			int side = d < 0.0f ? -1 : 1;
			if (prevSide == 0)
			{
				prevSide = side;
			}
			else if (prevSide != side)
			{
				result = false;
				break;
			}
		}
	}
	
	lua_pushboolean(L, result);
	return 1;
}

int polyGetVertexPosition(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	int index = getPointIndex(L, 2);
	bool inGlobalSpace = luaL_optboolean(L, 3, 0);
	
	c2v& vertex = poly->body.verts[index];
	
	if (inGlobalSpace)
	{
		c2v gl = c2Add(vertex, poly->transform.p);
		lua_pushnumber(L, gl.x);
		lua_pushnumber(L, gl.y);
	}
	else
	{
		lua_pushnumber(L, vertex.x);
		lua_pushnumber(L, vertex.y);
	}
	return 2;
}

int polyGetVertexPositionX(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	int index = getPointIndex(L, 2);
	bool inGlobalSpace = luaL_optboolean(L, 3, 0);
	
	if (inGlobalSpace)
		lua_pushnumber(L, poly->body.verts[index].x + poly->transform.p.x);
	else
		lua_pushnumber(L, poly->body.verts[index].x);
	
	return 1;
}

int polyGetVertexPositionY(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	int index = getPointIndex(L, 2);
	bool inGlobalSpace = luaL_optboolean(L, 3, 0);
	
	if (inGlobalSpace)
		lua_pushnumber(L, poly->body.verts[index].y + poly->transform.p.y);
	else
		lua_pushnumber(L, poly->body.verts[index].y);
	return 1;
}

int polyGetRotatedVertexPosition(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	int index = getPointIndex(L, 2);
	bool inGlobalSpace = luaL_optboolean(L, 3, 0);
	
	c2v& vertex = poly->body.verts[index];
	
	if (inGlobalSpace)
	{
		c2v gl = c2Mulrv(poly->transform.r, vertex);
		c2v pt = c2Add(gl, poly->transform.p);
		lua_pushnumber(L, pt.x);
		lua_pushnumber(L, pt.y);
	}
	else
	{
		c2v pt = c2Mulrv(poly->transform.r, vertex);
		lua_pushnumber(L, pt.x);
		lua_pushnumber(L, pt.y);
	}
	return 2;
}

int polyGetRotatedVertexPositionX(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	int index = getPointIndex(L, 2);
	bool inGlobalSpace = luaL_optboolean(L, 3, 0);
	
	c2v& vertex = poly->body.verts[index];
	
	if (inGlobalSpace)
	{
		c2v pt = c2Mulrv(poly->transform.r, vertex);
		c2v gl = c2Add(pt, poly->transform.p);
		lua_pushnumber(L, gl.x);
	}
	else
	{
		c2v pt = c2Mulrv(poly->transform.r, vertex);
		lua_pushnumber(L, pt.x);
	}
	return 1;
}

int polyGetRotatedVertexPositionY(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	int index = getPointIndex(L, 2);
	bool inGlobalSpace = luaL_optboolean(L, 3, 0);
	
	c2v& vertex = poly->body.verts[index];
	
	if (inGlobalSpace)
	{
		c2v pt = c2Mulrv(poly->transform.r, vertex);
		c2v gl = c2Add(pt, poly->transform.p);
		lua_pushnumber(L, gl.x);
	}
	else
	{
		c2v pt = c2Mulrv(poly->transform.r, vertex);
		lua_pushnumber(L, pt.y);
	}
	return 1;
}

int polyGetVertexCount(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	lua_pushinteger(L, poly->body.count);
	return 1;
}

int polyUpdateCenter(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	
	c2v average = poly->body.verts[0];
	for (int i = 1; i < poly->body.count; ++i) 
	{
		average = c2Add(average, poly->body.verts[i]);
	}
	
	average = c2Div(average, (float)poly->body.count);
	
	for (int i = 0; i < poly->body.count; i++) 
	{
		poly->body.verts[i] = c2Sub(poly->body.verts[i], average);
	}
	
	return 0;
}

int polyRemoveVertex(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	int index = getPointIndex(L, 2);
	poly->body.count -= 1;
	LUA_ASSERT(poly->body.count > 2, "Polygon must have atleast 3 vertices!");
	
	for (int i = index; i < poly->body.count; i++)
	{
		poly->body.verts[i] = poly->body.verts[i + 1];
		poly->body.norms[i] = poly->body.norms[i + 1];
	}
	
	index -= 1;
	int prev = index < 0 ? poly->body.count - 1 : index;
	updateVertexNormal(poly, prev);
	
	return 0;
}

int polyInsertVertex(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	LUA_ASSERTF(poly->body.count + 1<= C2_MAX_POLYGON_VERTS, "Maximum vertex count limit reached (%d)!", poly->body.count);
	c2v point = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	bool inGlobalSpace = luaL_optboolean(L, 4, 0);
	int uind = luaL_optnumber(L, 5, poly->body.count) - 1;
	int index = c2Clamp(uind, 0, C2_MAX_POLYGON_VERTS);
	
	if (inGlobalSpace)
	{
		point = c2Sub(point, poly->transform.p);
	}
	
	poly->body.count += 1;
	
	for (int i = poly->body.count - 1;  i > index; i--)
	{
		poly->body.verts[i] = poly->body.verts[i - 1];
		poly->body.norms[i] = poly->body.norms[i - 1];
	}
	
	index += 1;
	poly->body.verts[index] = c2MulrvT(poly->transform.r, point);
	updateVertexNormal(poly, index);
	
	int prev = index - 1 < 0 ? poly->body.count - 1 : index - 1;
	updateVertexNormal(poly, prev);
	return 0;
}

int setPolyPosition(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	poly->transform.p.x = luaL_checknumber(L, 2);
	poly->transform.p.y = luaL_checknumber(L, 3);
	return 0;
}

int getPolyPosition(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	lua_pushnumber(L, poly->transform.p.x);
	lua_pushnumber(L, poly->transform.p.y);
	return 2;
}

int setPolyX(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	poly->transform.p.x = luaL_checknumber(L, 2);
	return 0;
}

int getPolyX(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	lua_pushnumber(L, poly->transform.p.x);
	return 1;
}

int setPolyY(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	poly->transform.p.y = luaL_checknumber(L, 3);
	return 0;
}

int getPolyY(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	lua_pushnumber(L, poly->transform.p.y);
	return 1;
}

int movePoly(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	poly->transform.p.x += luaL_checknumber(L, 2);
	poly->transform.p.y += luaL_checknumber(L, 3);
	return 0;
}

int setPolyRotation(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	poly->transform.r = c2Rot(luaL_checknumber(L, 2));
	return 0;
}

int getPolyRotation(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	lua_pushnumber(L, atan2(poly->transform.r.s, poly->transform.r.c));
	return 1;
}

int getPolyCosSin(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	lua_pushnumber(L, poly->transform.r.c);
	lua_pushnumber(L, poly->transform.r.s);
	return 2;
}

int rotatePoly(lua_State* L)
{
	Poly* poly = getPtr<Poly>(L, "c2Poly", 1);
	float r = atan2(poly->transform.r.s, poly->transform.r.c);
	poly->transform.r = c2Rot(r + luaL_checknumber(L, 2));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// RAY
///
/////////////////////////////////////////////////////////////////////////////////////////////

int createRay(lua_State* L)
{
	c2Ray* ray = new c2Ray();
	ray->p = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	ray->d = c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
	if (lua_gettop(L) < 5 || lua_isnil(L, 5))
	{
		c2v dir = c2Sub(ray->d, ray->p);
		ray->t = c2Len(dir);
	}
	else
	{
		ray->t = luaL_checknumber(L, 5);
	}
	
	g_pushInstance(L, "c2Ray", ray);
	
	setPtr(L, ray);

	return 1;
}

int createRayFromRotation(lua_State* L)
{
	c2Ray* ray = new c2Ray();
	ray->p = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	float rot = luaL_checknumber(L, 3);
	ray->t = luaL_optnumber(L, 4, 1.0f);
	ray->d.x = cosf(rot);
	ray->d.y = sinf(rot);
	
	g_pushInstance(L, "c2Ray", ray);
	
	setPtr(L, ray);

	return 1;
}

int rayGetStartPosition(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	lua_pushnumber(L, ray->p.x);
	lua_pushnumber(L, ray->p.y);
	return 2;
}

int raySetStartPosition(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	ray->p.x = luaL_checknumber(L, 2);
	ray->p.y = luaL_checknumber(L, 3);
	return 0;
}

int raySetStartX(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	ray->p.x = luaL_checknumber(L, 2);
	return 0;
}

int rayGetStartX(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	lua_pushnumber(L, ray->p.x);
	return 1;
}

int raySetStartY(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	ray->p.y = luaL_checknumber(L, 2);
	return 0;
}

int rayGetStartY(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	lua_pushnumber(L, ray->p.y);
	return 1;
}

int rayGetTargetPosition(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	lua_pushnumber(L, ray->d.x);
	lua_pushnumber(L, ray->d.y);
	return 2;
}

int raySetTargetPosition(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	ray->d.x = luaL_checknumber(L, 2);
	ray->d.y = luaL_checknumber(L, 3);
	return 0;
}

int rayGetTargetX(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	lua_pushnumber(L, ray->d.x);
	return 1;
}

int raySetTargetX(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	ray->d.x = luaL_checknumber(L, 2);
	return 0;
}

int rayGetTargetY(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	lua_pushnumber(L, ray->d.y);
	return 1;
}

int raySetTargetY(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	ray->d.y = luaL_checknumber(L, 2);
	return 0;
}

int rayFaceTo(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	c2v p = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	c2v dir = c2Sub(p, ray->p);
	ray->d = c2Norm(dir);
	if (lua_gettop(L) < 4 || lua_isnil(L, 4))
	{
		ray->t = c2Len(dir);
	}
	else
	{
		ray->t = luaL_checknumber(L, 4);
	}
	return 0;
}

int rayGetFacePosition(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	float x = ray->t * ray->d.x + ray->p.x;
	float y = ray->t * ray->d.y + ray->p.y;
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

int rayGetFaceX(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	float x = ray->t * ray->d.x + ray->p.x;
	lua_pushnumber(L, x);
	return 1;
}

int rayGetFaceY(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	float y = ray->t * ray->d.y + ray->p.y;
	lua_pushnumber(L, y);
	return 1;
}

int rayGetLength(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	lua_pushnumber(L, ray->t);
	return 1;
}

int raySetLength(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	ray->t = luaL_checknumber(L, 2);
	return 0;
}

int rayGetDirection(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	float d = atan2f(ray->d.y, ray->d.x);
	lua_pushnumber(L, d);
	return 1;
}

int raySetDirection(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	float ang = luaL_checknumber(L, 2);
	ray->d.x = cosf(ang);
	ray->d.y = sinf(ang);
	return 0;
}

int rayMove(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	float dx = luaL_checknumber(L, 2);
	float dy = luaL_checknumber(L, 3);
	ray->p.x += dx;
	ray->p.y += dy;
	return 0;
}

int rayGetData(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	lua_pushnumber(L, ray->p.x);
	lua_pushnumber(L, ray->p.y);
	lua_pushnumber(L, ray->d.x);
	lua_pushnumber(L, ray->d.y);
	lua_pushnumber(L, ray->t);
	return 5;
}

int rayNormalize(lua_State* L)
{
	c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
	c2v dir = c2Sub(ray->d, ray->p);
	
	if (lua_gettop(L) > 1 && lua_toboolean(L, 2))
	{
		ray->t = c2Len(dir);
	}
	else
		ray->t = 1.0f;
	
	ray->d = c2Norm(dir);
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// CUTE_C2_API
///
/////////////////////////////////////////////////////////////////////////////////////////////

int c2CircleToCircle_lua(lua_State* L)
{
	c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
	c2Circle B = *getPtr<c2Circle>(L, "c2Circle", 2);

	int result = c2CircletoCircle(A, B);
	lua_pushboolean(L, result);
	return 1;
}

int c2CircleToAABB_lua(lua_State* L)
{
	c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
	c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);

	int result = c2CircletoAABB(A, B);
	lua_pushboolean(L, result);
	return 1;
}

int c2CircletoCapsule_lua(lua_State* L)
{
	c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
	c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);

	int result = c2CircletoCapsule(A, B);
	lua_pushboolean(L, result);
	return 1;
}

int c2AABBtoAABB_lua(lua_State* L)
{
	c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
	c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);

	int result = c2AABBtoAABB(A, B);
	lua_pushboolean(L, result);
	return 1;
}

int c2AABBtoCapsule_lua(lua_State* L)
{
	c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
	c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);

	int result = c2AABBtoCapsule(A, B);
	lua_pushboolean(L, result);
	return 1;
}

int c2CapsuletoCapsule_lua(lua_State* L)
{
	c2Capsule A = *getPtr<c2Capsule>(L, "c2Capsule", 1);
	c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);

	int result = c2CapsuletoCapsule(A, B);
	lua_pushboolean(L, result);
	return 1;
}

int c2CircletoPoly_lua(lua_State* L)
{
	c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
	Poly* B = getPtr<Poly>(L, "c2Poly", 2);

	int result = c2CircletoPoly(A, &B->body, &B->transform);
	lua_pushboolean(L, result);
	return 1;
}

int c2AABBtoPoly_lua(lua_State* L)
{
	c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
	Poly* B = getPtr<Poly>(L, "c2Poly", 2);

	int result = c2AABBtoPoly(A, &B->body, &B->transform);
	lua_pushboolean(L, result);
	return 1;
}

int c2CapsuletoPoly_lua(lua_State* L)
{
	c2Capsule A = *getPtr<c2Capsule>(L, "c2Capsule", 1);
	Poly* B = getPtr<Poly>(L, "c2Poly", 2);

	int result = c2CapsuletoPoly(A, &B->body, &B->transform);
	lua_pushboolean(L, result);
	return 1;
}

int c2PolytoPoly_lua(lua_State* L)
{
	Poly* A = getPtr<Poly>(L, "c2Poly", 1);
	Poly* B = getPtr<Poly>(L, "c2Poly", 2);

	int result = c2PolytoPoly(&A->body, &A->transform, &B->body, &B->transform);
	lua_pushboolean(L, result);
	return 1;
}

int c2AABBtoPoint_lua(lua_State* L)
{
	c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	lua_pushboolean(L, c2AABBtoPoint(A, c2V(x, y)));
	return 1;
}

int c2CircleToPoint_lua(lua_State* L)
{
	c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	lua_pushboolean(L, c2CircleToPoint(A, c2V(x, y)));
	return 1;
}

int c2CastRay_lua(lua_State* L)
{
	c2Raycast out;
	c2x* bx = nullptr;
	
	if (lua_gettop(L) > 3)
	{
		c2Ray ray = c2Ray();
		ray.p = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
		ray.d = c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
		ray.t = luaL_checknumber(L, 5);
		
		C2_TYPE shapeType = getShapeType(L, 6);
		
		void* obj = getRawPtrCheck(L, 6, &bx);
		
		int result = c2CastRay(ray, obj, bx, shapeType, &out);
		lua_pushinteger(L, result);
		lua_pushnumber(L, out.n.x);
		lua_pushnumber(L, out.n.y);
		lua_pushnumber(L, out.t);
		return 4;
	}
	
	c2Ray ray = *getPtr<c2Ray>(L, "c2Ray", 1);
	void* obj = getRawPtrCheck(L, 2, &bx);
	
	C2_TYPE shapeType = getShapeType(L, 2);

	int result = c2CastRay(ray, obj, bx, shapeType, &out);
	lua_pushinteger(L, result);
	lua_pushnumber(L, out.n.x);
	lua_pushnumber(L, out.n.y);
	lua_pushnumber(L, out.t);
	return 4;
}

int c2Collide_lua(lua_State* L)
{
	void* A = nullptr;
	c2x* transformA = nullptr;
	void* B = nullptr;
	c2x* transformB = nullptr;
	checkABShape(L, &A, 1, &transformA, &B, 2, &transformB);
	
	C2_TYPE shapeTypeA = getShapeType(L, 1);
	C2_TYPE shapeTypeB = getShapeType(L, 2);
	
	c2Manifold m;
	c2Collide(A, transformA, shapeTypeA, B, transformB, shapeTypeB, &m);
	
	return pushManifold(L, m);
}

int c2Collided_lua(lua_State* L)
{
	void* A = nullptr;
	c2x* transformA = nullptr;
	void* B = nullptr;
	c2x* transformB = nullptr;
	checkABShape(L, &A, 1, &transformA, &B, 2, &transformB);
	
	C2_TYPE shapeTypeA = getShapeType(L, 1);
	C2_TYPE shapeTypeB = getShapeType(L, 2);

	int result = c2Collided(A, transformA, shapeTypeA, B, transformB, shapeTypeB);
	lua_pushboolean(L, result);
	return 1;
}

int c2CircletoCircleManifold_lua(lua_State* L)
{
	c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
	c2Circle B = *getPtr<c2Circle>(L, "c2Circle", 2);
	c2Manifold m;

	c2CircletoCircleManifold(A, B, &m);
	
	return pushManifold(L, m);
}

int c2CircletoAABBManifold_lua(lua_State* L)
{
	c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
	c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);
	c2Manifold m;

	c2CircletoAABBManifold(A, B, &m);
	
	return pushManifold(L, m);
}

int c2CircletoCapsuleManifold_lua(lua_State* L)
{
	c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
	c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);
	c2Manifold m;

	c2CircletoCapsuleManifold(A, B, &m);
	
	return pushManifold(L, m);
}

int c2AABBtoAABBManifold_lua(lua_State* L)
{
	c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
	c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);
	c2Manifold m;

	c2AABBtoAABBManifold(A, B, &m);
	
	return pushManifold(L, m);
}

int c2AABBtoCapsuleManifold_lua(lua_State* L)
{
	c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
	c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);
	c2Manifold m;

	c2AABBtoCapsuleManifold(A, B, &m);
	
	return pushManifold(L, m);
}

int c2CapsuletoCapsuleManifold_lua(lua_State* L)
{
	c2Capsule A = *getPtr<c2Capsule>(L, "c2Capsule", 1);
	c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);
	c2Manifold m;

	c2CapsuletoCapsuleManifold(A, B, &m);
	
	return pushManifold(L, m);
}

int c2CircletoPolyManifold_lua(lua_State* L)
{
	c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
	Poly* B = getPtr<Poly>(L, "c2Poly", 2);
	c2Manifold m;

	c2CircletoPolyManifold(A, &B->body, &B->transform, &m);
	
	return pushManifold(L, m);
}

int c2AABBtoPolyManifold_lua(lua_State* L)
{
	c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
	Poly* B = getPtr<Poly>(L, "c2Poly", 2);
	c2Manifold m;

	c2AABBtoPolyManifold(A, &B->body, &B->transform, &m);
	
	return pushManifold(L, m);
}

int c2CapsuletoPolyManifold_lua(lua_State* L)
{
	c2Capsule A = *getPtr<c2Capsule>(L, "c2Capsule", 1);
	Poly* B = getPtr<Poly>(L, "c2Poly", 2);
	c2Manifold m;

	c2CapsuletoPolyManifold(A, &B->body, &B->transform, &m);
	
	return pushManifold(L, m);
}

int c2PolytoPolyManifold_lua(lua_State* L)
{
	Poly* A = getPtr<Poly>(L, "c2Poly", 1);
	Poly* B = getPtr<Poly>(L, "c2Poly", 2);
	c2Manifold m;

	c2PolytoPolyManifold(&A->body, &A->transform, &B->body, &B->transform, &m);
	
	return pushManifold(L, m);
}

int c2GJK_lua(lua_State* L)
{
	void* A = nullptr;
	c2x* transformA = nullptr;
	void* B = nullptr;
	c2x* transformB = nullptr;
	checkABShape(L, &A, 1, &transformA, &B, 2, &transformB);
	
	C2_TYPE shapeTypeA = getShapeType(L, 1);
	C2_TYPE shapeTypeB = getShapeType(L, 2);
	
	c2v outA;
	c2v outB;
	int use_radius = lua_toboolean(L, 3);
	int iterations;

	float result = c2GJK(
				A, shapeTypeA, transformA,
				B, shapeTypeB, transformB,
				&outA, &outB,
				use_radius,
				&iterations,
				0);
	lua_pushnumber(L, result);
	lua_pushnumber(L, outA.x);
	lua_pushnumber(L, outA.y);
	lua_pushnumber(L, outB.x);
	lua_pushnumber(L, outB.y);
	lua_pushnumber(L, iterations);	
	return 6;
}

int c2TOI_lua(lua_State* L)
{
	void* A = nullptr;
	c2x* transformA = nullptr;
	void* B = nullptr;
	c2x* transformB = nullptr;
	checkABShape(L, &A, 1, &transformA, &B, 4, &transformB);
	
	C2_TYPE shapeTypeA = getShapeType(L, 1);
	c2v vA = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	
	C2_TYPE shapeTypeB = getShapeType(L, 4);
	c2v vB = c2V(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
	
	int use_radius = lua_toboolean(L, 7);
	
	c2TOIResult result = c2TOI(
				A, shapeTypeA, transformA, vA,
				B, shapeTypeB, transformB, vB,
				use_radius);
	
	lua_pushboolean(L, result.hit);
	lua_pushnumber(L, result.toi);
	lua_pushnumber(L, result.n.x);
	lua_pushnumber(L, result.n.y);
	lua_pushnumber(L, result.p.x);
	lua_pushnumber(L, result.p.y);
	lua_pushnumber(L, result.iterations);
	return 7;
}

int c2Impact_lua(lua_State* L)
{
	if (lua_gettop(L) > 2)
	{
		c2Ray ray = c2Ray();
		ray.d = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
		ray.p = c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
		ray.t = luaL_checknumber(L, 5);
		float t = luaL_checknumber(L, 6);
		c2v result = c2Impact(ray, t);
		lua_pushnumber(L, result.x);
		lua_pushnumber(L, result.y);
		return 2;
	}
	c2Ray ray = *getPtr<c2Ray>(L, "c2Ray", 1);
	float t = luaL_checknumber(L, 2);
	c2v result = c2Impact(ray, t);
	lua_pushnumber(L, result.x);
	lua_pushnumber(L, result.y);
	return 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// LOADER
///
/////////////////////////////////////////////////////////////////////////////////////////////

int loader(lua_State* L)
{
	const luaL_Reg circleFunctionsList[] = {
		{"setPosition", setCirclePosition},
		{"getPosition", getCirclePosition},
		
		{"setPositionX", setCirclePositionX},
		{"getPositionX", getCirclePositionX},
		
		{"setPositionY", setCirclePositionY},
		{"getPositionY", getCirclePositionY},
		
		{"setRadius", setCircleRadius},
		{"getRadius", getCircleRadius},
		
		{"getData", cirlceGetData},
		
		{"getBoundingBox", getCirlceBoundingBox},
		
		{"rayTest", cirlceRayTest},
		{"inflate", circleInflate},
		{"hitTest", circleHitTest},
		{"move", circleMove},
		{NULL, NULL}
	};
	g_createClass(L, "c2Circle", NULL, NULL, NULL, circleFunctionsList);

	const luaL_Reg aabbFunctionsList[] = {
		{"setMinPosition", setAABBMinPosition},
		{"getMinPosition", getAABBMinPosition},
		
		{"setMaxPosition", setAABBMaxPosition},
		{"getMaxPosition", getAABBMaxPosition},
		
		{"getBoundingBox", getAABBboundingBox},
		
		{"setPosition", setAABBCenterPosition},
		{"getPosition", getAABBCenterPosition},
		
		{"setMinX", setAABBMinX},
		{"getMinX", getAABBMinX},
		
		{"setMinY", setAABBMinY},
		{"getMinY", getAABBMinY},
		
		{"setMaxX", setAABBMaxX},
		{"getMaxX", getAABBMaxX},
		
		{"setMaxY", setAABBMaxY},
		{"getMaxY", getAABBMaxY},
		
		{"setX", setAABBCenterPositionX},
		{"getX", getAABBCenterPositionX},
		
		{"setY", setAABBCenterPositionY},
		{"getY", getAABBCenterPositionY},
		
		
		{"setHalfSize", setAABBhalfSize},
		{"getHalfSize", getAABBhalfSize},
		
		{"setSize", setAABBSize},
		{"getSize", getAABBSize},
		
		{"setWidth", setAABBWidth},
		{"getWidth", getAABBWidth},
		
		{"setHalfWidth", setAABBhalfWidth},
		{"getHalfWidth", getAABBhalfWidth},
		
		{"setHeight", setAABBHeight},
		{"getHeight", getAABBHeight},
		
		{"setHalfHeight", setAABBhalfHeight},
		{"getHalfHeight", getAABBhalfHeight},
		
		{"getData", getAABBData},
		{"rayTest", AABBRayTest},
		{"inflate", AABBInflate},
		{"hitTest", AABBHitTest},
		{"move", AABBmove},
		{NULL, NULL}
	};
	g_createClass(L, "c2AABB", NULL, NULL, NULL, aabbFunctionsList);

	const luaL_Reg capsuleFunctionsList[] = {
		{"setPosition", setCapsulePosition},
		{"getPosition", getCapsulePosition},
		
		{"setTipPosition", setCapsuleTipPosition},
		{"getTipPosition", getCapsuleTipPosition},
		
		{"setBasePosition", setCapsuleBasePosition},
		{"getBasePosition", getCapsuleBasePosition},
		
		{"setX", setCapsuleX},
		{"getX", getCapsuleX},
		
		{"setY", setCapsuleY},
		{"getY", getCapsuleY},
		
		{"setTipX", setCapsuleTipX},
		{"getTipX", getCapsuleTipX},
		
		{"setTipY", setCapsuleTipY},
		{"getTipY", getCapsuleTipY},
		
		{"setBaseX", setCapsuleBaseX},
		{"getBaseX", getCapsuleBaseX},
		
		{"setBaseY", setCapsuleBaseY},
		{"getBaseY", getCapsuleBaseY},
		
		{"setHeight", setCapsuleHeight},
		{"getHeight", getCapsuleHeight},
		
		{"setHalfHeight", setCapsuleHalfHeight},
		{"getHalfHeight", getCapsuleHalfHeight},
		
		{"setRadius", setCapsuleRadius},
		{"getRadius", getCapsuleRadius},
		
		{"setSize", setCapsuleSize},
		{"getSize", getCapsuleSize},
		
		{"getBoundingBox", getCapsuleBoundingBox},
		
		{"getData", getCapsuleData},
		{"rayTest", capsuleRayTest},
		{"inflate", capsuleInflate},
		{"hitTest", capsuleHitTest},
		{"move", capsuleMove},
		{NULL, NULL}
	};
	g_createClass(L, "c2Capsule", NULL, NULL, NULL, capsuleFunctionsList);

	const luaL_Reg polyFunctionsList[] = {
		{"setPoints", polyUpdatePoints},
		{"setPointsXY", polyUpdatePointsXY},
		{"setVertex", polySetVertexPosition},
		{"getPoints", polyGetPoints},
		{"getBoundingBox", polyGetBoundingBox},
		{"getNormals", polyGetNormals},
		{"getData", polyGetData},
		{"rayTest", polyRayTest},
		{"inflate", polyInflate},
		{"hitTest", polyHitTest},
		{"getRotatedPoints", polyGetRotatedPoints},
		{"getRotatedNormals", polyGetRotatedNormals},
		
		{"getVertex", polyGetVertexPosition},
		{"getVertexX", polyGetVertexPositionX},
		{"getVertexY", polyGetVertexPositionY},
		
		{"getRotatedVertex", polyGetRotatedVertexPosition},
		{"getRotatedVertexX", polyGetRotatedVertexPositionX},
		{"getRotatedVertexY", polyGetRotatedVertexPositionY},
		
		{"getVertexCount", polyGetVertexCount},
		
		{"updateCenter", polyUpdateCenter},
		{"removeVertex", polyRemoveVertex},
		{"insertVertex", polyInsertVertex},
		
		{"setPosition", setPolyPosition},
		{"getPosition", getPolyPosition},
		{"setX", setPolyX},
		{"getX", getPolyX},
		{"setY", setPolyY},
		{"getY", getPolyY},
		{"move", movePoly},

		{"setRotation", setPolyRotation},
		{"getRotation", getPolyRotation},
		{"getCosSin", getPolyCosSin},
		
		{"rotate", rotatePoly},
		
		{NULL, NULL}
	};
	g_createClass(L, "c2Poly", NULL, NULL, NULL, polyFunctionsList);

	const luaL_Reg rayFunctionsList[] = {
		{"setPosition", raySetStartPosition},
		{"getPosition", rayGetStartPosition},
		
		{"setX", raySetStartX},
		{"getX", rayGetStartX},
		{"setY", raySetStartY},
		{"getY", rayGetStartY},
		
		{"setTargetPosition", raySetTargetPosition},
		{"getTargetPosition", rayGetTargetPosition},
		
		{"getTargetX", rayGetTargetX},
		{"getTargetY", rayGetTargetY},
		{"setTargetX", raySetTargetX},
		{"setTargetY", raySetTargetY},
		
		{"faceTo", rayFaceTo},
		{"getFacePosition", rayGetFacePosition},
		{"getFaceX", rayGetFaceX},
		{"getFaceY", rayGetFaceY},
		
		{"getLength", rayGetLength},
		{"setLength", raySetLength},
		
		{"getDirection", rayGetDirection},
		{"setDirection", raySetDirection},
		
		{"move", rayMove},
		{"getData", rayGetData},
		{"normalize", rayNormalize},
		{NULL, NULL}
	};
	g_createClass(L, "c2Ray", NULL, NULL, NULL, rayFunctionsList);

	const luaL_Reg cuteC2FunctionsList[] = {
		{"circle", createCircle},
		{"aabb", createAABB},
		{"capsule", createCapsule},
		{"poly", createPoly},
		{"ray", createRay},
		{"rayFromRotation", createRayFromRotation},

		{"circleToCircle", c2CircleToCircle_lua},
		{"circleToAABB", c2CircleToAABB_lua},
		{"circleToCapsule", c2CircletoCapsule_lua},
		{"AABBtoAABB", c2AABBtoAABB_lua},
		{"AABBtoCapsule", c2AABBtoCapsule_lua},
		{"capsuleToCapsule", c2CapsuletoCapsule_lua},
		{"circleToPoly", c2CircletoPoly_lua},
		{"AABBtoPoly", c2AABBtoPoly_lua},
		{"capsuleToPoly", c2CapsuletoPoly_lua},
		{"polyToPoly", c2PolytoPoly_lua},
		
		{"AABBtoPoint", c2AABBtoPoint_lua},
		{"circleToPoint", c2CircleToPoint_lua},
		{"polyToPoint", polyHitTest},
		{"capsuleToPoint", capsuleHitTest},
		
		{"castRay", c2CastRay_lua},
		{"collide", c2Collide_lua},
		{"collided", c2Collided_lua},

		{"circleToCircleManifold", c2CircletoCircleManifold_lua},
		{"circleToAABBManifold", c2CircletoAABBManifold_lua},
		{"circleToCapsuleManifold", c2CircletoCapsuleManifold_lua},
		{"AABBtoAABBManifold", c2AABBtoAABBManifold_lua},
		{"AABBtoCapsuleManifold", c2AABBtoCapsuleManifold_lua},
		{"capsuleToCapsuleManifold", c2CapsuletoCapsuleManifold_lua},
		{"circletoPolyManifold", c2CircletoPolyManifold_lua},
		{"AABBtoPolyManifold", c2AABBtoPolyManifold_lua},
		{"capsuleToPolyManifold", c2CapsuletoPolyManifold_lua},
		{"polyToPolyManifold", c2PolytoPolyManifold_lua},
		
		{"impact", c2Impact_lua},
		
		{"GJK", c2GJK_lua},
		{"TOI", c2TOI_lua},
		{NULL, NULL}
	};

	g_createClass(L, "CuteC2", NULL, NULL, NULL, cuteC2FunctionsList);

	luaL_newweaktable(L);
	luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

	bindEnums(L);

	return 1;
}

}

static void g_initializePlugin(lua_State* L)
{
	::L = L;

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, cute_c2_impl::loader, "loader");
	lua_setfield(L, -2, PLUGIN_NAME);

	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State* _UNUSED(L)) { }

#ifdef IS_BETA_BUILD
REGISTER_PLUGIN_NAMED(PLUGIN_NAME, "1.0.0", CuteC2_beta)
#else
REGISTER_PLUGIN_NAMED(PLUGIN_NAME, "1.0.0", CuteC2)
#endif
