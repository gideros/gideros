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
            {"setTarget", setTarget},
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

int ViewportBinder::destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
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
    Sprite* os = shape->getContent();
    shape->setContent(s);

    if (s!=os) {
        lua_getfield(L, 1, "__children");
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L,-1);
            lua_setfield(L, 1, "__children");
        }

        if (os) {
            lua_pushlightuserdata(L, os);
            lua_pushnil(L);
            lua_rawset(L, -3);							// sprite.__children[child] = nil
        }

        if (s) {
            lua_pushlightuserdata(L, s);
            lua_pushvalue(L, 2);
            lua_rawset(L, -3);							// sprite.__children[child] = child
        }
        lua_pop(L, 1);								// pop sprite.__children
    }
    return 0;
}

int ViewportBinder::setTarget(lua_State* L)
{
    StackChecker checker(L, "ViewportBinder::setTarget", 0);

    Binder binder(L);
    Viewport* shape = static_cast<Viewport*>(binder.getInstance("Viewport"));
    GRenderTarget* s = static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", 2));
    const float *ccol=lua_tovector(L,4);
    shape->setTarget(s,lua_toboolean(L,3),ccol);
    return 0;
}

int ViewportBinder::getTransform(lua_State* L)
{
	StackChecker checker(L, "ViewportBinder::getTransform", 1);

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
	StackChecker checker(L, "ViewportBinder::getProjection", 1);

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
	StackChecker checker(L, "ViewportBinder::getContent", 1);

	Binder binder(L);
	Viewport* shape = static_cast<Viewport*>(binder.getInstance("Viewport"));
	Sprite* s = shape->getContent();
    if (s) {
        lua_getfield(L, 1, "__children");
        lua_pushlightuserdata(L, s);
        lua_rawget(L,-2);
        lua_remove(L,-2);
    }
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
