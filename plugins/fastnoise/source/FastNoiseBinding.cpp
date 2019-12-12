#define _USE_MATH_DEFINES
#define FN_USE_DOUBLES

#include <gglobal.h>
#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"
#include "FastNoise.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592654
#endif

//----------------------------------------------------------------
//                      Helper functions
//----------------------------------------------------------------
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

static void my_assert(lua_State* L, bool condition, const char* message)
{
    if (!condition)
    {
        lua_pushstring(L, message);
        lua_error(L);
    }
}

static void assertIsPositiveNumber(lua_State *L, int narg, const char *name) {
    lua_Integer d = lua_tointeger(L, narg);
    if (d <= 0) {
        lua_pushfstring(L, "%s must be a positive integer, but was %s (a %s)",
                name, lua_tostring(L, narg), lua_typename(L, narg));
        lua_error(L);
    }
}

static double map(double v, double minC, double maxC, double minD, double maxD)
{
    return (v - minC) / (maxC - minC) * (maxD - minD) + minD;
}

static unsigned char getColor(lua_State* L, int ind)
{
    lua_rawgeti(L, -1, ind);
    double cv = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return (unsigned char)cv;
}

static void loadNoiseParams(lua_State* L, int index, double &xoff, double &yoff, double &zoff, double &min, double &max )
{
    xoff = 0.0;
    yoff = 0.0;
    zoff = 0.0;
    min = -1.0;
    max = 1.0;
    if (lua_istable(L, index))
    {
        lua_getfield(L, index, "xoff");
        if (!lua_isnil(L, -1)) xoff = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, index, "yoff");
        if (!lua_isnil(L, -1)) yoff = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, index, "zoff");
        if (!lua_isnil(L, -1)) zoff = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, index, "min");
        if (!lua_isnil(L, -1)) min = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, index, "max");
        if (!lua_isnil(L, -1)) max = luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
}

struct Color
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A;
    Color()
    {
        R = 255; G = 255; B = 255;  A = 255;
    }
    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        R = r; G = g; B = b; A = a;
    }
};

static Color matchColor(lua_State* L, double noiseValue)
{
    Color color = Color();
    bool flag = false;
    for (unsigned int i = 0; i < lua_objlen(L, -1); i++)
    {
        lua_rawgeti(L, -1, i+1);
        lua_getfield(L, -1, "h");
        double hh = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        if (noiseValue <= hh)
        {
            flag = true;

            lua_getfield(L, -1, "color");
            color.R = getColor(L, 1);
            color.G = getColor(L, 2);
            color.B = getColor(L, 3);
            color.A = getColor(L, 4);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
        if (flag) break;
    }

    return color;
}

//----------------------------------------------------------------
//                          GProxy
//----------------------------------------------------------------

class GNoise : public GProxy
{
private:
    FastNoise noise;
public:
    GNoise(){}
    GNoise(FastNoise *n) { noise = *n; }
    ~GNoise(){}
    FastNoise* GetFNoise() { return &noise; }

    // for bindings
    void setSeed(int seed) { noise.SetSeed(seed); }
    int getSeed() const { return noise.GetSeed(); }
    void setFrequency(FN_DECIMAL frequency) { noise.SetFrequency(frequency); }
    FN_DECIMAL getFrequency() const { return noise.GetFrequency(); }

    FN_DECIMAL getNoise(FN_DECIMAL x, FN_DECIMAL y) { return noise.GetNoise(x,y); }
    FN_DECIMAL getNoise(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) { return noise.GetNoise(x,y,z); }

    void setInterp(int interp){ noise.SetInterp((FastNoise::Interp)interp); }
    int getInterp() { return (int)noise.GetInterp(); }
    void setNoiseType(int noiseType) { noise.SetNoiseType((FastNoise::NoiseType) noiseType); }
    int getNoiseType() { return (int)noise.GetNoiseType(); }
    void setFractalOctaves(int octaves) { noise.SetFractalOctaves(octaves); }
    int getFractalOctaves() const { return noise.GetFractalOctaves(); }

    void setFractalLacunarity(FN_DECIMAL lacunarity) { noise.SetFractalLacunarity(lacunarity); }
    FN_DECIMAL getFractalLacunarity() { return noise.GetFractalLacunarity(); }
    void setFractalGain(FN_DECIMAL gain) { noise.SetFractalGain(gain); }
    FN_DECIMAL getFractalGain() { return noise.GetFractalGain(); }
    void setFractalType(int fractalType) { noise.SetFractalType((FastNoise::FractalType) fractalType); }
    int getFractalType() { return (int)noise.GetFractalType(); }
    void setCellularDistanceFunction(int df) { noise.SetCellularDistanceFunction((FastNoise::CellularDistanceFunction)df);  }
    int getCellularDistanceFunction() { return (int)noise.GetCellularDistanceFunction();  }
    void setCellularReturnType(int rt) { noise.SetCellularReturnType((FastNoise::CellularReturnType)rt); }
    int getCellularReturnType() { return (int)noise.GetCellularReturnType(); }
    void setCellularNoiseLookup(GNoise *n) { noise.SetCellularNoiseLookup(n->GetFNoise()); }
    GNoise *getCellularNoiseLookup() { GNoise *n = new GNoise(noise.GetCellularNoiseLookup()); return n; }
    void setCellularDistance2Indices(int i0, int i1) { noise.SetCellularDistance2Indices(i0, i1); }
    void setCellularJitter(FN_DECIMAL jitter) { noise.SetCellularJitter(jitter); }
    FN_DECIMAL getCellularJitter() { return noise.GetCellularJitter(); }
    void setGradientPerturbAmp(FN_DECIMAL gamp) { noise.SetGradientPerturbAmp(gamp); }
    FN_DECIMAL getGradientPerturbAmp() { return noise.GetGradientPerturbAmp(); }

    void gradientPerturb(FN_DECIMAL& x, FN_DECIMAL& y) const { noise.GradientPerturb(x,y); }
    void gradientPerturbFractal(FN_DECIMAL& x, FN_DECIMAL& y) const { noise.GradientPerturbFractal(x,y); }

    void gradientPerturb(FN_DECIMAL& x, FN_DECIMAL& y, FN_DECIMAL& z) const { noise.GradientPerturb(x,y,z); }
    void gradientPerturbFractal(FN_DECIMAL& x, FN_DECIMAL& y, FN_DECIMAL& z) const { noise.GradientPerturbFractal(x,y,z); }

    FN_DECIMAL getSimplex(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL w) const { return noise.GetSimplex(x,y,z,w); }

    FN_DECIMAL getWhiteNoise(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL w) const { return noise.GetWhiteNoise(x,y,z,w); }

    FN_DECIMAL getWhiteNoiseInt(int x, int y) const { return noise.GetWhiteNoiseInt(x,y); }
    FN_DECIMAL getWhiteNoiseInt(int x, int y, int z) const { return noise.GetWhiteNoiseInt(x,y,z); }
    FN_DECIMAL getWhiteNoiseInt(int x, int y, int z, int w) const { return noise.GetWhiteNoiseInt(x,y,z,w); }
};

static int destructNoise(lua_State* L)
{
    void *ptr = *(void**)lua_touserdata(L, 1);
    GReferenced* object = static_cast<GReferenced*>(ptr);
    GNoise *n = static_cast<GNoise*>(object->proxy());

    n->unref();

    return 0;
}

static GNoise *getNoiseInstance(lua_State* L, int index)
{
    GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Noise", index));
    GNoise *c = static_cast<GNoise*>(object->proxy());

    return c;
}

static int initNoise(lua_State *L)
{
    GNoise *n = new GNoise();
    g_pushInstance(L, "Noise", n->object());

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, n);
    lua_pop(L, 1);

    lua_pushvalue(L, -1);
    return 1;
}

//----------------------------------------------------------------
//                         Noise Settings
//----------------------------------------------------------------

static int setSeed(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int seed = luaL_checkinteger(L, 2);
    n->setSeed(seed);
    return 0;
}

static int getSeed(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushinteger(L, n->getSeed());
    return 1;
}

static int setFrequency(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL f = luaL_checknumber(L, 2);
    n->setFrequency(f);
    return 0;
}

static int getFrequency(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushnumber(L, n->getFrequency());
    return 1;
}

static int setInterp(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int interp = lua_tointeger(L, -1);
    n->setInterp(interp);
    return 0;
}

static int getInterp(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushinteger(L, n->getInterp());
    return 1;
}

static int setNoiseType(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int nt = lua_tointeger(L, -1);
    n->setNoiseType(nt);
    return 0;
}

static int getNoiseType(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushinteger(L, n->getNoiseType());
    return 1;
}

static int setFractalOctaves(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int octaves = lua_tointeger(L, -1);
    n->setFractalOctaves(octaves);
    return 0;
}

static int getFractalOctaves(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushinteger(L, n->getFractalOctaves());
    return 1;
}

static int setFractalLacunarity(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL lacunarity = luaL_checknumber(L, 2);
    n->setFractalLacunarity(lacunarity);
    return 0;
}

static int getFractalLacunarity(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushnumber(L, n->getFractalLacunarity());
    return 1;
}

static int setFractalGain(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL gain = luaL_checknumber(L, 2);
    n->setFractalGain(gain);
    return 0;
}

static int getFractalGain(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushnumber(L, n->getFractalGain());
    return 1;
}

static int setFractalType(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int fractalType = luaL_checkinteger(L, 2);
    n->setFractalType(fractalType);
    return 0;
}

static int getFractalType(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushinteger(L, n->getFractalType());
    return 1;
}

static int setCellularDistanceFunction(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int df = luaL_checkinteger(L, 2);
    n->setCellularDistanceFunction(df);
    return 0;
}

static int getCellularDistanceFunction(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushinteger(L, n->getCellularDistanceFunction());
    return 1;
}

static int setCellularReturnType(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int rt = luaL_checkinteger(L, 2);
    if (rt == (int)FastNoise::NoiseLookup){
        auto lookUp = n->GetFNoise()->GetCellularNoiseLookup();
        my_assert(L, lookUp != NULL, "Cellular noise lookup object is not set! Use Noise:setCellularNoiseLookup(Noise).");
    }

    n->setCellularReturnType(rt);
    return 0;
}

static int getCellularReturnType(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushinteger(L, n->getCellularReturnType());
    return 1;
}

static int setCellularNoiseLookup(lua_State* L)
{
    GNoise *n  = getNoiseInstance(L, 1);
    GNoise *sn = getNoiseInstance(L, 2);
    n->setCellularNoiseLookup(sn);
    return 0;
}

static int getCellularNoiseLookup(lua_State* L)
{
    GNoise *n  = getNoiseInstance(L, 1);
    GNoise *ns = n->getCellularNoiseLookup();

    g_pushInstance(L, "Noise", ns->object());

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, n);
    lua_pop(L, 1);

    lua_pushvalue(L, -1);
    return 1;
}


static int setCellularDistance2Indices(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int i0 = luaL_checkinteger(L, 2);
    int i1 = luaL_checkinteger(L, 2);
    n->setCellularDistance2Indices(i0, i1);
    return 0;
}

static int setCellularJitter(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int jitter = luaL_checknumber(L, 2);
    n->setCellularJitter(jitter);
    return 0;
}

static int getCellularJitter(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushnumber(L, n->getCellularJitter());
    return 1;
}

static int setGradientPerturbAmp(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL gamp = luaL_checknumber(L, 2);
    n->setGradientPerturbAmp(gamp);
    return 0;
}

static int getGradientPerturbAmp(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    lua_pushnumber(L, n->getGradientPerturbAmp());
    return 1;
}

//----------------------------------------------------------------
//                      Noise functions
//----------------------------------------------------------------

static int noise(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL x = lua_tonumber(L, 2);
    FN_DECIMAL y = lua_tonumber(L, 3);
    FN_DECIMAL z = lua_tonumber(L, 4);
    FN_DECIMAL v = n->getNoise(x, y, z);
    lua_pushnumber(L, v);
    return 1;
}

static int noise2D(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL x = luaL_checknumber(L, 2);
    FN_DECIMAL y = luaL_checknumber(L, 3);
    FN_DECIMAL v = n->getNoise(x, y);
    lua_pushnumber(L, v);
    return 1;
}

static int noise3D(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL x = luaL_checknumber(L, 2);
    FN_DECIMAL y = luaL_checknumber(L, 3);
    FN_DECIMAL z = luaL_checknumber(L, 4);
    FN_DECIMAL v = n->getNoise(x, y, z);
    lua_pushnumber(L, v);
    return 1;
}


static int gradientPerturb2D(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL x = lua_tonumber(L, 2);
    FN_DECIMAL y = lua_tonumber(L, 3);
    n->gradientPerturb(x,y);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}

static int gradientPerturb3D(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL x = lua_tonumber(L, 2);
    FN_DECIMAL y = lua_tonumber(L, 3);
    FN_DECIMAL z = lua_tonumber(L, 4);
    n->gradientPerturb(x,y,z);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 3;
}


static int gradientPerturbFractal2D(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL x = lua_tonumber(L, 2);
    FN_DECIMAL y = lua_tonumber(L, 3);
    n->gradientPerturbFractal(x,y);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}

static int gradientPerturbFractal3D(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL x = lua_tonumber(L, 2);
    FN_DECIMAL y = lua_tonumber(L, 3);
    FN_DECIMAL z = lua_tonumber(L, 4);
    n->gradientPerturbFractal(x,y,z);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 3;
}

static int simplex4D(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL x = luaL_checknumber(L, 2);
    FN_DECIMAL y = luaL_checknumber(L, 3);
    FN_DECIMAL z = luaL_checknumber(L, 4);
    FN_DECIMAL w = luaL_checknumber(L, 5);
    lua_pushnumber(L, n->getSimplex(x,y,z,w));
    return 1;
}


static int whiteNoise4D(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    FN_DECIMAL x = luaL_checknumber(L, 2);
    FN_DECIMAL y = luaL_checknumber(L, 3);
    FN_DECIMAL z = luaL_checknumber(L, 4);
    FN_DECIMAL w = luaL_checknumber(L, 5);
    lua_pushnumber(L, n->getWhiteNoise(x,y,z,w));
    return 1;
}

static int whiteNoise2DInt(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    lua_pushnumber(L, n->getWhiteNoiseInt(x,y));
    return 1;
}

static int whiteNoise3DInt(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int z = luaL_checkinteger(L, 4);
    lua_pushnumber(L, n->getWhiteNoiseInt(x,y,z));
    return 1;
}

static int whiteNoise4DInt(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int z = luaL_checkinteger(L, 4);
    int w = luaL_checkinteger(L, 5);
    lua_pushnumber(L, n->getWhiteNoiseInt(x,y,z,w));
    return 1;
}

//-------------------------------------------------------
//                      Additions
//-------------------------------------------------------

static int generateArray(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);

    assertIsPositiveNumber(L, 2, "Array size");
    int size = lua_tointeger(L, 2);
    double yoff = luaL_optnumber(L, 3, 0);

    lua_createtable(L, 0, size);
    for (int i = 0; i < size; i++)
    {
        FN_DECIMAL v = n->getNoise(i, yoff);
        lua_pushnumber(L, i+1);
        lua_pushnumber(L, v);
        lua_rawset(L, -3);
    }

    return 1;
}

static int generateTexture(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);

    // check if texture size is > 0
    assertIsPositiveNumber(L, 2, "Texture width");
    assertIsPositiveNumber(L, 3, "Texture height");
    // get texture size
    int w = luaL_checkinteger(L, 2);
    int h = luaL_checkinteger(L, 3);
    // get optional filtering parameter
    bool filtering = lua_toboolean(L, 4);
    // define optional noise parameters
    double xoff,yoff,zoff,min,max;
    // get parameters
    loadNoiseParams(L, 6, xoff, yoff, zoff, min, max);

    unsigned char *data=new unsigned char[w*h*4];
    unsigned char *ptr=data;

    for (int y=0;y<h;y++)
        for (int x=0;x<w;x++)
        {
            float noise=map(n->getNoise(x + xoff, y + yoff, zoff), min, max, 0, 1);
            unsigned char lum=(unsigned char)(noise*255);
            Color clr = Color(lum,lum,lum,255);

            if (lua_istable(L, 6))
            {
                lua_getfield(L, 6, "colors");
                if (lua_objlen(L, -1) > 0)
                    clr = matchColor(L, noise);
                lua_pop(L, 1);
            }
            *ptr++=clr.R;
            *ptr++=clr.G;
            *ptr++=clr.B;
            *ptr++=clr.A;
        }

    // Use our data array
    lua_getglobal(L, "Texture");
    lua_getfield(L, -1, "new");
    lua_pushlstring(L,(char *)data,4*w*h);
    lua_pushinteger(L,w);
    lua_pushinteger(L,h);
    lua_pushboolean(L,filtering);

    // load options table
    if (lua_istable(L, 5))
    {
        lua_pushvalue(L, 5);
        lua_call(L,5,1);
    }
    else
        lua_call(L,4,1);
    // Newly created texture is on stack already, just return it
    //Delete our data array
    delete[] data;
    lua_remove(L, -2);
    return 1;
}

static int generateTileableTexture(lua_State* L)
{
    GNoise *n = getNoiseInstance(L, 1);

    // check if texture size is > 0
    assertIsPositiveNumber(L, 2, "Texture width");
    assertIsPositiveNumber(L, 3, "Texture height");
    // get texture size
    int w = luaL_checkinteger(L, 2);
    int h = luaL_checkinteger(L, 3);
    // get optional filtering parameter
    bool filtering = lua_toboolean(L, 4);
    // define optional noise parameters
    double xoff,yoff,zoff,min,max;
    // get optional noise parameters
    loadNoiseParams(L, 6, xoff, yoff, zoff, min, max);

    unsigned char *data=new unsigned char[w*h*4];
    unsigned char *ptr=data;

    double pi2 = 2*M_PI;
    for (int y=0;y<h;y++)
        for (int x=0;x<w;x++)
        {
            double s = (double)x / (double)w;
            double t = (double)y / (double)h;

            double nx=cos(s*pi2)*-w/pi2;
            double ny=cos(t*pi2)*-h/pi2;
            double nz=-1.0+sin(s*pi2)*-w/pi2;
            double nw=-1.0+sin(t*pi2)*-h/pi2;
            double noise = n->getSimplex(nx + xoff, ny + yoff, nz + zoff,nw);

            noise=map(noise, min, max, 0, 1);

            unsigned char lum=(unsigned char)(noise*255);
            Color clr = Color(lum,lum,lum,255);

            if (lua_istable(L, 6))
            {
                lua_getfield(L, 6, "colors");
                if (lua_objlen(L, -1) > 0)
                    clr = matchColor(L, noise);
                lua_pop(L, 1);
            }
            *ptr++=clr.R;
            *ptr++=clr.G;
            *ptr++=clr.B;
            *ptr++=clr.A;
        }
    // Use our data array
    lua_getglobal(L, "Texture");
    lua_getfield(L, -1, "new");
    lua_pushlstring(L,(char *)data,4*w*h);
    lua_pushinteger(L,w);
    lua_pushinteger(L,h);
    lua_pushboolean(L,filtering);

    // load options table

    // load options table
    if (lua_istable(L, 5))
    {
        lua_pushvalue(L, 5);
        lua_call(L,5,1);
    }
    else
        lua_call(L,4,1);
    // Newly created texture is on stack already, just return it
    //Delete our data array
    delete[] data;
    lua_remove(L, -2);
    return 1;
}

static int reset(lua_State* L)
{
    GNoise *noise = getNoiseInstance(L, 1);
    noise->setFrequency(0.01);
    noise->setInterp((int)FastNoise::Quintic);
    noise->setNoiseType((int)FastNoise::Simplex);
    noise->setFractalOctaves(3);
    noise->setFractalLacunarity(2.0);
    noise->setFractalGain(0.5);
    noise->setFractalType((int)FastNoise::FBM);
    noise->setCellularDistanceFunction((int)FastNoise::Euclidean);
    noise->setCellularReturnType((int)FastNoise::CellValue);
    noise->setCellularDistance2Indices(0, 1);
    noise->setCellularJitter(0.45);
    noise->setGradientPerturbAmp(1.0);
    return 0;
}

//-------------------------------------------------------

static int loader(lua_State* L)
{
    const luaL_Reg functionlist[] = {
        {"new", initNoise},
        {"reset", reset},

        {"generateTexture", generateTexture},
        {"generateTileableTexture", generateTileableTexture},
        {"generateArray", generateArray},

        {"noise", noise},
        {"noise2D", noise2D},
        {"noise3D", noise3D},

        {"gradientPerturb2D", gradientPerturb2D},
        {"gradientPerturb3D", gradientPerturb3D},

        {"gradientPerturbFractal2D", gradientPerturbFractal2D},
        {"gradientPerturbFractal3D", gradientPerturbFractal3D},

        {"whiteNoise4D", whiteNoise4D},
        {"whiteNoise2DInt", whiteNoise2DInt},
        {"whiteNoise3DInt", whiteNoise3DInt},
        {"whiteNoise4DInt", whiteNoise4DInt},

        {"simplex4D", simplex4D},

        {"setNoiseType", setNoiseType}, {"getNoiseType", getNoiseType},
        {"setFractalOctaves", setFractalOctaves}, {"getFractalOctaves", getFractalOctaves},
        {"setInterp", setInterp}, {"getInterp", getInterp},
        {"setSeed", setSeed},  {"getSeed", getSeed},
        {"setFrequency", setFrequency},	{"getFrequency", getFrequency},
        {"setFractalLacunarity", setFractalLacunarity}, {"getFractalLacunarity", getFractalLacunarity},
        {"setFractalGain", setFractalGain}, {"getFractalGain", getFractalGain},
        {"setFractalType", setFractalType}, {"getFractalType", getFractalType},
        {"setCellularDistanceFunction", setCellularDistanceFunction}, {"getCellularDistanceFunction", getCellularDistanceFunction},
        {"setCellularReturnType", setCellularReturnType}, {"getCellularReturnType", getCellularReturnType},
        {"setCellularNoiseLookup", setCellularNoiseLookup}, {"getCellularNoiseLookup", getCellularNoiseLookup},
        {"setCellularDistance2Indices", setCellularDistance2Indices},
        {"setCellularJitter", setCellularJitter}, {"getCellularJitter", getCellularJitter},
        {"setGradientPerturbAmp", setGradientPerturbAmp}, {"getGradientPerturbAmp", getGradientPerturbAmp},
        {NULL, NULL}
    };

    g_createClass(L, "Noise", NULL, NULL, destructNoise, functionlist);

    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

    lua_getglobal(L, "Noise");

    lua_pushnumber(L, FastNoise::FBM);
    lua_setfield(L, -2, "FBM");

    lua_pushnumber(L, FastNoise::RigidMulti);
    lua_setfield(L, -2, "RIGID_MULTI");

    lua_pushnumber(L, FastNoise::Billow);
    lua_setfield(L, -2, "BILLOW");

    lua_pushnumber(L, FastNoise::Simplex);
    lua_setfield(L, -2, "SIMPLEX");

    lua_pushnumber(L, FastNoise::CubicFractal);
    lua_setfield(L, -2, "CUBIC_FRACTAL");

    lua_pushnumber(L, FastNoise::ValueFractal);
    lua_setfield(L, -2, "VALUE_FRACTAL");

    lua_pushnumber(L, FastNoise::SimplexFractal);
    lua_setfield(L, -2, "SIMPLEX_FRACTAL");

    lua_pushnumber(L, FastNoise::Cellular);
    lua_setfield(L, -2, "CELLULAR");

    lua_pushnumber(L, FastNoise::Cubic);
    lua_setfield(L, -2, "CUBIC");

    lua_pushnumber(L, FastNoise::Value);
    lua_setfield(L, -2, "VALUE");

    lua_pushnumber(L, FastNoise::WhiteNoise);
    lua_setfield(L, -2, "WHITE_NOISE");

    lua_pushnumber(L, FastNoise::Perlin);
    lua_setfield(L, -2, "PERLIN");

    lua_pushnumber(L, FastNoise::PerlinFractal);
    lua_setfield(L, -2, "PERLIN_FRACTAL");

    lua_pushnumber(L, FastNoise::Distance2Div);
    lua_setfield(L, -2, "DISTANCE_2_DIV");

    lua_pushnumber(L, FastNoise::NoiseLookup);
    lua_setfield(L, -2, "NOISE_LOOKUP");

    lua_pushnumber(L, FastNoise::CellValue);
    lua_setfield(L, -2, "CELL_VALUE");

    lua_pushnumber(L, FastNoise::Distance2);
    lua_setfield(L, -2, "DISTANCE_2");

    lua_pushnumber(L, FastNoise::Distance2Add);
    lua_setfield(L, -2, "DISTANCE_2_ADD");

    lua_pushnumber(L, FastNoise::Distance);
    lua_setfield(L, -2, "DISTANCE");

    lua_pushnumber(L, FastNoise::Distance2Sub);
    lua_setfield(L, -2, "DISTANCE_2_SUB");

    lua_pushnumber(L, FastNoise::Distance2Mul);
    lua_setfield(L, -2, "DISTANCE_2_MUL");

    lua_pushnumber(L, FastNoise::Hermite);
    lua_setfield(L, -2, "HERMITE");

    lua_pushnumber(L, FastNoise::Linear);
    lua_setfield(L, -2, "LINEAR");

    lua_pushnumber(L, FastNoise::Quintic);
    lua_setfield(L, -2, "QUINTIC");

    lua_pushnumber(L, FastNoise::Manhattan);
    lua_setfield(L, -2, "MANHATTAN");

    lua_pushnumber(L, FastNoise::Euclidean);
    lua_setfield(L, -2, "EUCLIDEAN");

    lua_pushnumber(L, FastNoise::Natural);
    lua_setfield(L, -2, "NATURAL");

    lua_pop(L, 1);

    return 0;
}

static void g_initializePlugin(lua_State* L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "FastNoise");

    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L) {
}
REGISTER_PLUGIN_NAMED("FastNoise", "0.1a",FastNoisePlugin);
