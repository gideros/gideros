#include <gglobal.h>
#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"
#include "clipper.hpp"

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

static void luaL_newweaktable(lua_State *L, const char *mode)
{
    lua_newtable(L);			// create table for instance list
    lua_pushstring(L, mode);
    lua_setfield(L, -2, "__mode");	  // set as weak-value table
    lua_pushvalue(L, -1);             // duplicate table
    lua_setmetatable(L, -2);          // set itself as metatable
}

static void luaL_rawgetptr(lua_State *L, int idx, void *ptr)
{
    idx = abs_index(L, idx);
    lua_pushlightuserdata(L, ptr);
    lua_rawget(L, idx);
}

static void luaL_rawsetptr(lua_State *L, int idx, void *ptr)
{
    idx = abs_index(L, idx);
    lua_pushlightuserdata(L, ptr);
    lua_insert(L, -2);
    lua_rawset(L, idx);
}

static char keyWeak = ' ';

using namespace ClipperLib;

class GPoly : public GProxy
{
public:

    GPoly(){}

    GPoly(Polygon n){
        p = n;
    }

    ~GPoly(){
        p.clear();
    }

    void addPoint(int x, int y){
        p.push_back(IntPoint(x,y));
    }

    Polygon getPoly(){
        return p;
    }

    double getArea(){
        return Area(p);
    }

    void clean(double distance = 1.415){
        CleanPolygon(p, p, distance);
    }

    void reverse(){
        ReversePolygon(p);
    }

    Polygons simplify(PolyFillType type){
        ps.clear();
        SimplifyPolygon(p, ps, type);
        return ps;
    }

    bool isClockwise(){
        return Orientation(p);
    }

    Polygons getOffsetPolyline(double delta, JoinType jointype = jtSquare, EndType endtype = etSquare, double limit = 0.0){
        ps.clear();
        ps.push_back(p);
        OffsetPolyLines(ps, ps, delta, jointype, endtype, limit);
        return ps;
    }

    Polygons getOffsetPolygon(double delta, JoinType jointype = jtSquare, double limit = 0.0, bool autoFix = true){
        ps.clear();
        ps.push_back(p);
        OffsetPolygons(ps, ps, delta, jointype, limit, autoFix);
        return ps;
    }

private:
    Polygon p;
    Polygons ps;
};

static void pushPolygons(lua_State *L, Polygons p)
{
    lua_newtable(L);
    for(int i = 0; i < p.size(); i++)
    {
        //index
        lua_pushinteger(L, i+1);

        GPoly *c = new GPoly(p[i]);
        g_pushInstance(L, "Polygon", c->object());

        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
        lua_pushvalue(L, -2);
        luaL_rawsetptr(L, -2, c);
        lua_pop(L, 1);

        //back to table
        lua_settable(L, -3);
    }
    lua_pushvalue(L, -1);
}


static int destructPoly(lua_State* L)
{
    void *ptr = *(void**)lua_touserdata(L, 1);
    GReferenced* object = static_cast<GReferenced*>(ptr);
    GPoly *n = static_cast<GPoly*>(object->proxy());

    n->unref();

    return 0;
}


static GPoly *getPolyInstance(lua_State* L, int index)
{
    GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Polygon", index));
    GPoly *c = static_cast<GPoly*>(object->proxy());

    return c;
}

static int initPoly(lua_State *L)
{
    GPoly *n = new GPoly();
    g_pushInstance(L, "Polygon", n->object());

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, n);
    lua_pop(L, 1);

    lua_pushvalue(L, -1);
    return 1;
}

static int addPoint(lua_State *L)
{
    GPoly *p = getPolyInstance(L, 1);
    int x = luaL_checknumber(L, 2);
    int y = luaL_checknumber(L, 3);
    p->addPoint(x, y);
    return 0;
}

static int getPoints(lua_State *L)
{
    GPoly *p = getPolyInstance(L, 1);
    Polygon pol = p->getPoly();
    lua_newtable(L);
    for (int i = 0; i < pol.size(); ++i)
    {
        //index
        lua_pushinteger(L, i+1);
        //create sub table
        lua_newtable(L);

        //set x
        lua_pushinteger(L, 1);
        lua_pushnumber(L, pol[i].X);

        //back to table
        lua_settable(L, -3);

        //set y
        lua_pushinteger(L, 2);
        lua_pushnumber(L, pol[i].Y);

        //back to table
        lua_settable(L, -3);

        //outer table
        lua_settable(L, -3);
    }
    lua_pushvalue(L, -1);
    return 1;
}

static int getArea(lua_State *L)
{
    GPoly *p = getPolyInstance(L, 1);
    lua_pushnumber(L, p->getArea());
    return 1;
}

static int clean(lua_State *L)
{
    GPoly *p = getPolyInstance(L, 1);

    double distance = 1.415;
    if(lua_isnumber(L, 2))
        distance = luaL_checknumber(L, 2);
    p->clean(distance);
    return 0;
}

static int reverse(lua_State *L)
{
    GPoly *p = getPolyInstance(L, 1);
    p->reverse();
    return 0;
}

static int simplify(lua_State *L)
{
    GPoly *p = getPolyInstance(L, 1);
    PolyFillType type = pftEvenOdd;
    if(lua_isnumber(L, 2))
        type = (PolyFillType)luaL_checknumber(L, 2);
    Polygons ps = p->simplify(type);
    pushPolygons(L, ps);
    return 1;
}

static int isClockwise(lua_State *L)
{
    GPoly *p = getPolyInstance(L, 1);
    lua_pushboolean(L, p->isClockwise());
    return 1;
}

static int getLineOffset(lua_State *L)
{
    GPoly *p = getPolyInstance(L, 1);
    double delta = luaL_checknumber(L, 2);
    JoinType jt = jtSquare;
    if(lua_isnumber(L, 3))
        jt = (JoinType)luaL_checknumber(L, 3);
    EndType et = etSquare;
    if(lua_isnumber(L, 4))
        et = (EndType)luaL_checknumber(L, 4);
    double limit = 0;
    if(lua_isnumber(L, 5))
        limit = luaL_checknumber(L, 5);
    Polygons ps = p->getOffsetPolyline(delta, jt, et, limit);
    pushPolygons(L, ps);
    return 1;
}

static int getPolygonOffset(lua_State *L)
{
    GPoly *p = getPolyInstance(L, 1);
    double delta = luaL_checknumber(L, 2);
    JoinType jt = jtSquare;
    if(lua_isnumber(L, 3))
        jt = (JoinType)luaL_checknumber(L, 3);
    double limit = 0;
    if(lua_isnumber(L, 4))
        limit = luaL_checknumber(L, 4);
    bool autoFix = true;
    if(lua_isboolean(L, 5))
        autoFix = lua_toboolean(L, 5);
    Polygons ps = p->getOffsetPolygon(delta, jtSquare, limit, autoFix);
    pushPolygons(L, ps);
    return 1;
}


class GClipper : public GProxy
{
public:
    GClipper(){}
    ~GClipper(){}

    void addSubjectPoly(Polygon p)
    {
        c.AddPolygon(p, ptSubject);
    }

    void addClipPoly(Polygon p)
    {
        c.AddPolygon(p, ptClip);
    }

    void clear()
    {
        c.Clear();
    }

    Polygons execute(ClipType ctype, PolyFillType subtype, PolyFillType cliptype)
    {
        solution.clear();
        c.Execute(ctype, solution, subtype, cliptype);
        return solution;
    }

    IntRect getBounds()
    {
        return c.GetBounds();
    }

private:
    Clipper c;
    Polygons solution;
};

static int destructClipper(lua_State* L)
{
    void *ptr = *(void**)lua_touserdata(L, 1);
    GReferenced* object = static_cast<GReferenced*>(ptr);
    GClipper *c = static_cast<GClipper*>(object->proxy());

    c->unref();

    return 0;
}

static GClipper *getClipperInstance(lua_State* L, int index)
{
    GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Clipper", index));
    GClipper *c = static_cast<GClipper*>(object->proxy());

    return c;
}

static int initClipper(lua_State *L)
{
    GClipper *c = new GClipper();
    g_pushInstance(L, "Clipper", c->object());

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, c);
    lua_pop(L, 1);

    lua_pushvalue(L, -1);
    return 1;
}

static int addSubjectPolygon(lua_State *L)
{
    GClipper *c = getClipperInstance(L, 1);
    GPoly *p = getPolyInstance(L, 2);
    c->addSubjectPoly(p->getPoly());
    return 0;
}

static int addClipPolygon(lua_State *L)
{
    GClipper *c = getClipperInstance(L, 1);
    GPoly *p = getPolyInstance(L, 2);
    c->addClipPoly(p->getPoly());
    return 0;
}

static int clear(lua_State *L)
{
    GClipper *c = getClipperInstance(L, 1);
    c->clear();
    return 0;
}

static int getIntersection(lua_State *L)
{
    GClipper *c = getClipperInstance(L, 1);
    PolyFillType type1 = pftEvenOdd;
    if(lua_isnumber(L, 2))
        type1 = (PolyFillType)luaL_checknumber(L, 2);
    PolyFillType type2 = pftEvenOdd;
    if(lua_isnumber(L, 3))
        type2 = (PolyFillType)luaL_checknumber(L, 3);
    Polygons p = c->execute(ctIntersection, type1, type2);
    pushPolygons(L, p);
    return 1;
}

static int getUnion(lua_State *L)
{
    GClipper *c = getClipperInstance(L, 1);
    PolyFillType type1 = pftEvenOdd;
    if(lua_isnumber(L, 2))
        type1 = (PolyFillType)luaL_checknumber(L, 2);
    PolyFillType type2 = pftEvenOdd;
    if(lua_isnumber(L, 3))
        type2 = (PolyFillType)luaL_checknumber(L, 3);
    Polygons p = c->execute(ctUnion, type1, type2);
    pushPolygons(L, p);
    return 1;
}

static int getDifference(lua_State *L)
{
    GClipper *c = getClipperInstance(L, 1);
    PolyFillType type1 = pftEvenOdd;
    if(lua_isnumber(L, 2))
        type1 = (PolyFillType)luaL_checknumber(L, 2);
    PolyFillType type2 = pftEvenOdd;
    if(lua_isnumber(L, 3))
        type2 = (PolyFillType)luaL_checknumber(L, 3);
    Polygons p = c->execute(ctDifference, type1, type2);
    pushPolygons(L, p);
    return 1;
}

static int getExclusion(lua_State *L)
{
    GClipper *c = getClipperInstance(L, 1);
    PolyFillType type1 = pftEvenOdd;
    if(lua_isnumber(L, 2))
        type1 = (PolyFillType)luaL_checknumber(L, 2);
    PolyFillType type2 = pftEvenOdd;
    if(lua_isnumber(L, 3))
        type2 = (PolyFillType)luaL_checknumber(L, 3);
    Polygons p = c->execute(ctXor, type1, type2);
    pushPolygons(L, p);
    return 1;
}

static int getBounds(lua_State *L)
{
    GClipper *c = getClipperInstance(L, 1);
    IntRect r = c->getBounds();
    lua_pushnumber(L, r.left);
    lua_pushnumber(L, r.top);
    lua_pushnumber(L, abs(r.left - r.right));
    lua_pushnumber(L, abs(r.top - r.bottom));
    return 4;
}

static int loader(lua_State *L)
{
    const luaL_Reg polyList[] = {
        {"new", initPoly},
        {"addPoint", addPoint},
        {"getPoints", getPoints},
        {"getArea", getArea},
        {"clean", clean},
        {"reverse", reverse},
        {"simplify", simplify},
        {"isClockwise", isClockwise},
        {"getPoints", getPoints},
        {"getLineOffset", getLineOffset},
        {"getPolygonOffset", getPolygonOffset},
        {NULL, NULL},
    };

    const luaL_Reg clipperList[] = {
        {"new", initClipper},
        {"addSubjectPolygon", addSubjectPolygon},
        {"addClipPolygon", addClipPolygon},
        {"clear", clear},
        {"getIntersection", getIntersection},
        {"getUnion", getUnion},
        {"getDifference", getDifference},
        {"getExclusion", getExclusion},
        {"getBounds", getBounds},
        {NULL, NULL},
    };

    g_createClass(L, "Polygon", NULL, NULL, destructPoly, polyList);
    g_createClass(L, "Clipper", NULL, NULL, destructClipper, clipperList);

    // create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

    lua_getglobal(L, "Polygon");
    lua_pushnumber(L, pftEvenOdd);
    lua_setfield(L, -2, "FILL_EVEN_ODD");
    lua_pushnumber(L, pftNonZero);
    lua_setfield(L, -2, "FILL_NON_ZERO");
    lua_pushnumber(L, pftPositive);
    lua_setfield(L, -2, "FILL_POSITIVE");
    lua_pushnumber(L, pftNegative);
    lua_setfield(L, -2, "FILL_NEGATIVE");

    lua_pushnumber(L, jtSquare);
    lua_setfield(L, -2, "JOINT_SQUARE");
    lua_pushnumber(L, jtRound);
    lua_setfield(L, -2, "JOINT_ROUND");
    lua_pushnumber(L, jtMiter);
    lua_setfield(L, -2, "JOINT_MITER");

    lua_pushnumber(L, etClosed);
    lua_setfield(L, -2, "END_CLOSED");
    lua_pushnumber(L, etButt);
    lua_setfield(L, -2, "END_BUTT");
    lua_pushnumber(L, etSquare);
    lua_setfield(L, -2, "END_SQUARE");
    lua_pushnumber(L, etRound);
    lua_setfield(L, -2, "END_ROUND");
    lua_pop(L, 1);

    return 0;
}

static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "clipper");

    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
}
REGISTER_PLUGIN("Clipper", "1.0")

