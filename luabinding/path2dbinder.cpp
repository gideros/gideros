#include "path2dbinder.h"
#include <path.h>
#include <string.h>
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>
#include <ttfont.h>

Path2DBinder::Path2DBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"setLineColor", setLineColor},
		{"setFillColor", setFillColor},
		{"setPath", setPath},
		{"setSvgPath", setSvgPath},
		{"setFontPath", setFontPath},
		{"setTexture", setTexture},
		{"setLineThickness", setLineThickness },
        {"setConvex", setConvex },
		{"getPathPoints", getPathPoints },
		{NULL, NULL},
	};

	binder.createClass("Path2D", "Sprite", create, destruct, functionList);

	lua_getglobal(L, "Path2D");	// get Path2D metatable

/*	lua_pushstring(L, NONE);
	lua_setfield(L, -2, "NONE");

	lua_pushstring(L, SOLID);
	lua_setfield(L, -2, "SOLID");

	lua_pushstring(L, TEXTURE);
	lua_setfield(L, -2, "TEXTURE");

	lua_pushstring(L, EVEN_ODD);
	lua_setfield(L, -2, "EVEN_ODD");

	lua_pushstring(L, NON_ZERO);
	lua_setfield(L, -2, "NON_ZERO");
*/
	lua_pop(L, 1);
}

int Path2DBinder::create(lua_State* L)
{
	StackChecker checker(L, "Path2DBinder::create", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

    Path2D* shape = new Path2D(application->getApplication());
	binder.pushInstance("Path2D", shape);

	return 1;
}

int Path2DBinder::destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
	Path2D* shape = static_cast<Path2D*>(ptr);
	shape->unref();

	return 0;
}

int Path2DBinder::setConvex(lua_State* L)
{
	Binder binder(L);
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D", 1));

	bool convex = lua_toboolean(L, 2);
	shape->setConvex(convex);

	return 0;
}

int Path2DBinder::setFillColor(lua_State* L)
{
	Binder binder(L);
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D", 1));
#define COLVEC(var,idx) float var[4]; luaL_checkcolorf(L,idx,var);
#define COLARG(var) (((int)(var[0]*0xFF0000))&0xFF0000)|(((int)(var[1]*0xFF00))&0xFF00)|((int)((var[2]*0xFF))&0xFF),var[3]
    if (lua_iscolor(L,2)) { //Vector colors
        if (lua_gettop(L) == 5) {
            COLVEC(c1,2);
            COLVEC(c2,3);
            COLVEC(c3,4);
            COLVEC(c4,5);
            shape->setGradient(COLARG(c1),COLARG(c2),COLARG(c3),COLARG(c4));
        }
        else if (lua_gettop(L) == 3) {
            COLVEC(c1,2);
            COLVEC(c2,3);
            shape->setGradient(COLARG(c1),COLARG(c2),COLARG(c1),COLARG(c2));
        }
        else if (lua_gettop(L) == 4) {
            COLVEC(c1,2);
            COLVEC(c2,3);
            shape->setGradientWithAngle(COLARG(c1),COLARG(c2),
            luaL_checknumber(L, 4));
        }
        else {
            COLVEC(color,2);
            shape->setFillColor(COLARG(color));
            shape->clearGradient();
        }
#undef COLVEC
#undef COLARG
    }
    else if (lua_gettop(L) == 9) shape->setGradient(
                luaL_checknumber(L, 2), luaL_checknumber(L, 3),
                luaL_checknumber(L, 4), luaL_checknumber(L, 5),
                luaL_checknumber(L, 6), luaL_checknumber(L, 7),
                luaL_checknumber(L, 8), luaL_checknumber(L, 9));
    else if (lua_gettop(L) == 5) shape->setGradient(
                luaL_checknumber(L, 2), luaL_checknumber(L, 3),
                luaL_checknumber(L, 2), luaL_checknumber(L, 3),
                luaL_checknumber(L, 4), luaL_checknumber(L, 5),
                luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    else if (lua_gettop(L) == 6) shape->setGradientWithAngle(
                luaL_checknumber(L, 2), luaL_checknumber(L, 3),
                luaL_checknumber(L, 4), luaL_checknumber(L, 5),
                luaL_checknumber(L, 6));
    else {
		unsigned int color = luaL_optinteger(L, 2, 0);
		lua_Number alpha = luaL_optnumber(L, 3, 1.0);
		shape->setFillColor(color, alpha);
		shape->clearGradient();
    }

	return 0;
}

int Path2DBinder::setLineColor(lua_State* L)
{
	StackChecker checker(L, "Path2DBinder::setLineStyle", 0);

	Binder binder(L);
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D"));

    float cvec[4];
    if (lua_tocolorf(L,2,cvec,0)) {
        unsigned int col = ((((int)(cvec[0]*255))&0xFF)<<16)|((((int)(cvec[1]*255))&0xFF)<<8)|((((int)(cvec[2]*255))&0xFF)<<0);
        shape->setLineColor(col, cvec[3]);
    }
    else {
        unsigned int color = luaL_optinteger(L, 2, 0);
        lua_Number alpha = luaL_optnumber(L, 3, 1.0);

        shape->setLineColor(color, alpha);
    }

	return 0;
}

int Path2DBinder::setTexture(lua_State *L)
{
    Binder binder(L);

    Path2D *bitmap = static_cast<Path2D*>(binder.getInstance("Path2D", 1));
    TextureBase *textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 2));
	Transform* matrix = NULL;
	if (!lua_isnone(L, 3))
		matrix = static_cast<Transform*>(binder.getInstance("Matrix", 3));
    bitmap->setTexture(textureBase, matrix?&matrix->matrix():NULL);

    return 0;
}

int Path2DBinder::setLineThickness(lua_State* L)
{
	StackChecker checker(L, "Path2DBinder::setLineStyle", 0);

	Binder binder(L);
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D"));

	double thickness = luaL_checknumber(L, 2);
    double feather = luaL_optnumber(L, 3, -1);
    double margin = luaL_optnumber(L, 4, -1);
    double flatness = luaL_optnumber(L, 5, -1);

	shape->setLineThickness(thickness,feather,margin,flatness);

	return 0;
}

int Path2DBinder::getPathPoints(lua_State* L)
{
	Binder binder(L);
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D", 1));

	std::vector<Path2D::PathPoint> points;

    shape->getPathPoints(luaL_optnumber(L, 3, 0),luaL_optnumber(L,2,0),luaL_optinteger(L, 4, 1000),
			luaL_optnumber(L, 5, 1),luaL_optinteger(L, 6, 10),points);

	lua_createtable(L,points.size(),0);
    for (size_t i=0;i<points.size();i++) {
		lua_createtable(L,0,3);
		lua_pushnumber(L,points[i].x); lua_setfield(L,-2,"x");
		lua_pushnumber(L,points[i].y); lua_setfield(L,-2,"y");
		lua_pushnumber(L,points[i].offset); lua_setfield(L,-2,"offset");
        lua_pushnumber(L,(180.0*points[i].angle)/3.141592654); lua_setfield(L,-2,"angle");
		lua_rawseti(L,-2,i+1);
	}

	return 1;
}

int Path2DBinder::setPath(lua_State* L)
{
	Binder binder(L);
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D", 1));

	const char* commands = luaL_optstring(L, 2,"ML*");

	std::vector<float> coords;
    if (lua_type(L, 3) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 3);
        coords.resize(n);
        for (int i = 0; i < n; ++i)
        {
            lua_rawgeti(L, 3, i + 1);
            coords[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
    }
    else
    {
        int n = lua_gettop(L) - 2;
        coords.resize(n);
        for (int i = 0; i < n; ++i)
            coords[i] = luaL_checknumber(L, i + 3);
    }

    shape->setPath(strlen(commands),(const unsigned char *)commands,coords.size(),coords.data());

	return 0;
}

int Path2DBinder::setSvgPath(lua_State* L)
{
    Binder binder(L);
    Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D", 1));

    int num = lua_gettop(L);

    std::string spath = luaL_checkstring(L, 2);

    for (int i = 3; i <= num; i++)
    {
        spath += luaL_checkstring(L, i);
        spath += " ";
    }

    PrPath *pr=prParseSvgPath(spath.c_str());
    if (pr)
    {
        shape->setPath(pr);
        prFreePath(pr);
    }

    return 0;
}

int Path2DBinder::setFontPath(lua_State* L)
{
	Binder binder(L);
	FontBase* font = NULL;
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D", 1));
	font = static_cast<FontBase*>(binder.getInstance("FontBase", 2));
	if (font->getType()!=FontBase::eTTFont)
	{
		lua_pushstring(L,"TTFont required");
		lua_error(L);
	}
	int ch= luaL_checkinteger(L,3);
    TTFont *tf=static_cast<TTFont*>(font);
    FT_UInt glyphIndex;
    FT_Face face=tf->getFace(ch,glyphIndex);

    if ((glyphIndex != 0)&&(!FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT)))
    {
        if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
        {
        	PrPath *pr=prParseFtGlyph(&(face->glyph->outline));
        	if (pr)
        	{
        		shape->setPath(pr);
        		prFreePath(pr);
        	}
        }
    }

    return 0;
}
