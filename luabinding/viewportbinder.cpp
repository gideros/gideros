#include "viewportbinder.h"
#include <viewport.h>
#include <string.h>
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>


ViewportBinder::ViewportBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
			{"setContent", setContent},
			{"setTransform", setTransform},
			{"setProjection", setProjection},
			{"getContent", getContent},
			{"getTransform", getTransform},
			{"getProjection", getProjection},
			{"lookAt", lookAt},
			{"lookAngles", lookAngles},
			{NULL, NULL},
	};

	binder.createClass("Viewport", "Sprite", create, destruct, functionList);
}

int ViewportBinder::create(lua_State* L)
{
	StackChecker checker(L, "ViewportBinder::create", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

    Viewport* shape = new Viewport(application->getApplication());
	binder.pushInstance("Viewport", shape);

	return 1;
}

int ViewportBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Viewport* shape = static_cast<Viewport*>(ptr);
	shape->unref();

	return 0;
}

int ViewportBinder::setTransform(lua_State* L)
{
	StackChecker checker(L, "ViewportBinder::setTransform", 0);
	
	Binder binder(L);
	Viewport* shape = static_cast<Viewport*>(binder.getInstance("Viewport", 1));
	const Matrix4 *matrix = NULL;
	if (!lua_isnone(L, 2))
	{
		Transform *matrix2 = static_cast<Transform*>(binder.getInstance("Matrix", 2));
		if (matrix2)
			matrix=&matrix2->matrix();
	}
	shape->setTransform(matrix);

	return 0;
}

int ViewportBinder::setProjection(lua_State* L)
{
	StackChecker checker(L, "ViewportBinder::setProjection", 0);

	Binder binder(L);
	Viewport* shape = static_cast<Viewport*>(binder.getInstance("Viewport", 1));
	const Matrix4 *matrix = NULL;
	if (!lua_isnone(L, 2))
	{
		Transform *matrix2 = static_cast<Transform*>(binder.getInstance("Matrix", 2));
		if (matrix2)
			matrix=&matrix2->matrix();
	}
	shape->setProjection(matrix);

	return 0;
}

int ViewportBinder::setContent(lua_State* L)
{
	StackChecker checker(L, "ViewportBinder::setContent", 0);

	Binder binder(L);
	Viewport* shape = static_cast<Viewport*>(binder.getInstance("Viewport"));
	Sprite* s = static_cast<Sprite*>(binder.getInstance("Sprite", 2));
	shape->setContent(s);

	return 0;
}

int ViewportBinder::getTransform(lua_State* L)
{
	StackChecker checker(L, "ViewportBinder::getTransform", 0);

	Binder binder(L);
	Viewport* shape = static_cast<Viewport*>(binder.getInstance("Viewport", 1));
	Matrix4 m=shape->getTransform();

	Transform *t=new Transform();
	t->setMatrix(m.data());

    binder.pushInstance("Matrix", t);
	return 1;
}

int ViewportBinder::getProjection(lua_State* L)
{
	StackChecker checker(L, "ViewportBinder::getProjection", 0);

	Binder binder(L);
	Viewport* shape = static_cast<Viewport*>(binder.getInstance("Viewport", 1));
	Matrix4 m=shape->getProjection();

	Transform *t=new Transform();
	t->setMatrix(m.data());

    binder.pushInstance("Matrix", t);
	return 1;
}

int ViewportBinder::getContent(lua_State* L)
{
	StackChecker checker(L, "ViewportBinder::getContent", 0);

	Binder binder(L);
	Viewport* shape = static_cast<Viewport*>(binder.getInstance("Viewport"));
	Sprite* s = shape->getContent();
	if (s)
	    binder.pushInstance("Sprite", s);
	else
		lua_pushnil(L);
	return 1;
}

int ViewportBinder::lookAt(lua_State* L)
{
	StackChecker checker(L, "ViewportBinder::setTransform", 0);

	Binder binder(L);
	Viewport* shape = static_cast<Viewport*>(binder.getInstance("Viewport", 1));

	shape->lookAt(luaL_optnumber(L,2,0),luaL_optnumber(L,3,0),luaL_optnumber(L,4,0),
			luaL_optnumber(L,5,0),luaL_optnumber(L,6,0),luaL_optnumber(L,7,0),
			luaL_optnumber(L,8,0),luaL_optnumber(L,9,0),luaL_optnumber(L,10,0));

	return 0;
}

int ViewportBinder::lookAngles(lua_State* L)
{
	StackChecker checker(L, "ViewportBinder::setTransform", 0);

	Binder binder(L);
	Viewport* shape = static_cast<Viewport*>(binder.getInstance("Viewport", 1));

	shape->lookAngles(luaL_optnumber(L,2,0),luaL_optnumber(L,3,0),luaL_optnumber(L,4,0),
			luaL_optnumber(L,5,0),luaL_optnumber(L,6,0),luaL_optnumber(L,7,0));

	return 0;
}
