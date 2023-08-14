#include "shapebinder.h"
#include <shape.h>
#include <string.h>
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>


#define EVEN_ODD "evenOdd"
#define NON_ZERO "nonZero"

#define NONE "none"
#define SOLID "solid"
#define TEXTURE "texture"

ShapeBinder::ShapeBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"setLineStyle", setLineStyle},
		{"setFillStyle", setFillStyle},
		{"beginPath", beginPath},
		{"moveTo", moveTo},
		{"lineTo", lineTo},
		{"endPath", endPath},
		{"closePath", closePath},
        {"clear", clear},
        {"tesselate", tesselate},
        {NULL, NULL},
	};

	binder.createClass("Shape", "Sprite", create, destruct, functionList);

	lua_getglobal(L, "Shape");	// get Shape metatable

	lua_pushstring(L, NONE);
	lua_setfield(L, -2, "NONE");

	lua_pushstring(L, SOLID);
	lua_setfield(L, -2, "SOLID");

	lua_pushstring(L, TEXTURE);
	lua_setfield(L, -2, "TEXTURE");

	lua_pushstring(L, EVEN_ODD);
	lua_setfield(L, -2, "EVEN_ODD");

	lua_pushstring(L, NON_ZERO);
	lua_setfield(L, -2, "NON_ZERO");

	lua_pop(L, 1);
}

int ShapeBinder::create(lua_State* L)
{
	StackChecker checker(L, "ShapeBinder::create", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

    Shape* shape = new Shape(application->getApplication());
	binder.pushInstance("Shape", shape);

	return 1;
}

int ShapeBinder::destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
	Shape* shape = static_cast<Shape*>(ptr);
	shape->unref();

	return 0;
}

int ShapeBinder::setFillStyle(lua_State* L)
{
	Binder binder(L);
	Shape* shape = static_cast<Shape*>(binder.getInstance("Shape", 1));
	
	const char* type = luaL_checkstring(L, 2);

	if (strcmp(type, NONE) == 0)
	{
		shape->clearFillStyle();
	}
	else if (strcmp(type, SOLID) == 0)
	{
		unsigned int color = luaL_optinteger(L, 3, 0);
		lua_Number alpha = luaL_optnumber(L, 4, 1.0);
		shape->setSolidFillStyle(color, alpha);
	}
	else if (strcmp(type, TEXTURE) == 0)
	{
		TextureBase* texture = static_cast<TextureBase*>(binder.getInstance("TextureBase", 3));
        const Matrix* matrix = NULL;
		if (!lua_isnone(L, 4))
			matrix = &((static_cast<Transform*>(binder.getInstance("Matrix", 4)))->matrix());
		shape->setTextureFillStyle(texture, matrix);
	}
	else
	{
		GStatus status(2008, "fillStyle");		// Error #2008: Parameter %s must be one of the accepted values.
		luaL_error(L, "%s", status.errorString());
		return 0;
	}

	return 0;
}

int ShapeBinder::setLineStyle(lua_State* L)
{
	StackChecker checker(L, "ShapeBinder::setLineStyle", 0);

	Binder binder(L);
	Shape* shape = static_cast<Shape*>(binder.getInstance("Shape"));

	double thickness = luaL_checknumber(L, 2);
	unsigned int color = luaL_optinteger(L, 3, 0);
	lua_Number alpha = luaL_optnumber(L, 4, 1.0);

	shape->setLineStyle(thickness, color, alpha);

	return 0;
}

int ShapeBinder::tesselate(lua_State* L)
{
    Binder binder(L);

    luaL_checktype(L,1,LUA_TTABLE);
    Shape::WindingRule windingRule = Shape::eEvenOdd;
    if (!lua_isnone(L, 2))
    {
        const char* winding = luaL_checkstring(L, 2);
        if (strcmp(winding, EVEN_ODD) == 0)
        {
            windingRule = Shape::eEvenOdd;
        }
        else if (strcmp(winding, NON_ZERO) == 0)
        {
            windingRule = Shape::eNonZero;
        }
        else
        {
            GStatus status(2008, "winding");		// Error #2008: Parameter %s must be one of the accepted values.
            luaL_error(L, "%s", status.errorString());
            return 0;
        }
    }
    std::vector<std::vector<Point2f>> contours;
    int cn=lua_objlen(L,1);
    for (int k=1;k<=cn;k++) {
        std::vector<Point2f> ctr;
        lua_rawgeti(L,1,k);
        int ln=lua_objlen(L,-1);
        for (int l=1;l<=ln;l+=2) {
            lua_rawgeti(L,-1,l);
            double x=luaL_checknumber(L,-1);
            lua_rawgeti(L,-2,l+1);
            double y=luaL_checknumber(L,-1);
            lua_pop(L,2);
            ctr.push_back(Point2f(x,y));
        }
        contours.push_back(ctr);
        lua_pop(L,1);
    }

    std::vector<double> ts=Shape::tesselate(contours,windingRule);
    lua_createtable(L,ts.size(),0);
    for (size_t k=0;k<ts.size();k++)
    {
        lua_pushnumber(L,ts[k]);
        lua_rawseti(L,-2,k+1);
    }

    return 1;
}

int ShapeBinder::beginPath(lua_State* L)
{
	Binder binder(L);
	Shape* shape = static_cast<Shape*>(binder.getInstance("Shape", 1));

	Shape::WindingRule windingRule = Shape::eEvenOdd;
	if (!lua_isnone(L, 2))
	{
		const char* winding = luaL_checkstring(L, 2);
		if (strcmp(winding, EVEN_ODD) == 0)
		{
			windingRule = Shape::eEvenOdd;
		}
		else if (strcmp(winding, NON_ZERO) == 0)
		{
			windingRule = Shape::eNonZero;
		}
		else
		{
			GStatus status(2008, "winding");		// Error #2008: Parameter %s must be one of the accepted values.
			luaL_error(L, "%s", status.errorString());
			return 0;
		}
	}

	shape->beginPath(windingRule);

	return 0;
}
int ShapeBinder::moveTo(lua_State* L)
{
	Binder binder(L);
	Shape* shape = static_cast<Shape*>(binder.getInstance("Shape"));

	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);

	shape->moveTo(x, y);

	return 0;
}
int ShapeBinder::lineTo(lua_State* L)
{
	Binder binder(L);
	Shape* shape = static_cast<Shape*>(binder.getInstance("Shape"));

	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);

	shape->lineTo(x, y);

	return 0;
}
int ShapeBinder::endPath(lua_State* L)
{
	Binder binder(L);
	Shape* shape = static_cast<Shape*>(binder.getInstance("Shape"));

	shape->endPath();

	return 0;
}
int ShapeBinder::closePath(lua_State* L)
{
	Binder binder(L);
	Shape* shape = static_cast<Shape*>(binder.getInstance("Shape"));

	shape->closePath();

	return 0;
}

int ShapeBinder::clear(lua_State* L)
{
	Binder binder(L);
	Shape* shape = static_cast<Shape*>(binder.getInstance("Shape"));

	shape->clear();

	return 0;
}
