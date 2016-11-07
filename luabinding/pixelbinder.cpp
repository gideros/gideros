#include "pixelbinder.h"
#include "pixel.h"
#include "bitmapdata.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>

PixelBinder::PixelBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"setWidth", setWidth},
		{"setHeight", setHeight},
        {"setDimensions", setDimensions},
        {"getDimensions", getDimensions},
        {"setColor", setColor},
        {"getColor", getColor},
        {"setTexture", setTexture},
        {"setTextureRegion", setTexture},
        {"setTextureMatrix", setTextureMatrix},
        {"setTexturePosition", setTexturePosition},
        {"getTexturePosition", getTexturePosition},
        {"setTextureScale", setTextureScale},
        {"getTextureScale", getTextureScale},
        {NULL, NULL},
	};

	binder.createClass("Pixel", "Sprite", create, destruct, functionList);
    //binder.createClass("Bitmap", "Sprite", create, destruct, functionList);
}

int PixelBinder::create(lua_State* L)
{
	StackChecker checker(L, "PixelBinder::create", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

    Pixel* bitmap = new Pixel(application->getApplication());

    if (binder.isInstanceOf("TextureRegion", 1)) {
        BitmapData *bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", 1));

        bitmap->setTexture(bitmapData->texture(), 0, NULL);
        bitmap->setColor(1, 1, 1, 1);

        int x, y, w, h, dx1, dy1, dx2, dy2;
        bitmapData->getRegion(&x, &y, &w, &h, &dx1, &dy1, &dx2, &dy2);
        bitmap->setTexturePosition(x, y);
        bitmap->setDimensions(w, h);

        binder.pushInstance("Pixel", bitmap);
        return 1;
    }

    if (lua_type(L, 3) == LUA_TTABLE) {
         bitmap->setStretching(true);

        TextureBase *textureBase = NULL;
        textureBase=static_cast<TextureBase*>(binder.getInstance("TextureBase", 3));

        bitmap->setTexture(textureBase, 0, NULL);
        bitmap->setColor(1, 1, 1, 1);

        lua_Number w = luaL_optnumber(L, 1, textureBase->data->width);
        lua_Number h = luaL_optnumber(L, 2, textureBase->data->height);
        bitmap->setDimensions(w, h);

        binder.pushInstance("Pixel", bitmap);
        return 1;
    }

    if (lua_type(L, 1) == LUA_TTABLE) {
        TextureBase *textureBase = NULL;
        textureBase=static_cast<TextureBase*>(binder.getInstance("TextureBase", 1));

        bitmap->setTexture(textureBase, 0, NULL);
        bitmap->setColor(1, 1, 1, 1);

        lua_Number w = luaL_optnumber(L, 2, textureBase->data->width);
        lua_Number h = luaL_optnumber(L, 3, textureBase->data->height);
        bitmap->setDimensions(w, h);

        lua_Number sx = luaL_optnumber(L, 4, 1.0);
        lua_Number sy = luaL_optnumber(L, 5, 1.0);
        bitmap->setTextureScale(sx, sy);

        lua_Number x = luaL_optnumber(L, 6, 0.0);
        lua_Number y = luaL_optnumber(L, 7, 0.0);
        bitmap->setTexturePosition(x, y);

        binder.pushInstance("Pixel", bitmap);
        return 1;
    }

    unsigned int color = luaL_optinteger(L, 1, 0xffffff);
	lua_Number alpha = luaL_optnumber(L, 2, 1.0);
	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color & 0xff;
	bitmap->setColor(r/255.f,g/255.f,b/255.f,alpha);

    lua_Number w = luaL_optnumber(L, 3, 1.0);
    lua_Number h = luaL_optnumber(L, 4, w);
	bitmap->setDimensions(w,h);

	binder.pushInstance("Pixel", bitmap);
	return 1;
}

int PixelBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Pixel* bitmap = static_cast<Pixel*>(ptr);
	bitmap->unref();

	return 0;
}

int PixelBinder::setWidth(lua_State* L)
{
	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

	lua_Number w = luaL_checknumber(L, 2);

	bitmap->setWidth(w);

	return 0;
}

int PixelBinder::setHeight(lua_State* L)
{
	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

	lua_Number h = luaL_checknumber(L, 2);

	bitmap->setHeight(h);

	return 0;
}

int PixelBinder::setDimensions(lua_State* L)
{
	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

	lua_Number w = luaL_checknumber(L, 2);
	lua_Number h = luaL_checknumber(L, 3);

	bitmap->setDimensions(w,h);

	return 0;
}

int PixelBinder::getDimensions(lua_State* L)
{
    Binder binder(L);

    Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

    float w, h;
    bitmap->getDimensions(w, h);

    lua_pushnumber(L, w);
    lua_pushnumber(L, h);

    return 2;
}

int PixelBinder::setTexture(lua_State *L)
{
    Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

    if (binder.isInstanceOf("TextureRegion", 2)) {
        BitmapData *bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", 2));
        TextureBase *textureBase = bitmapData->texture();
        bitmap->setStretching(false);
        bitmap->setTexture(textureBase, 0, NULL);
        int x, y, w, h, dx1, dy1, dx2, dy2;
        bitmapData->getRegion(&x, &y, &w, &h, &dx1, &dy1, &dx2, &dy2);
        bitmap->setTexturePosition(x, y);
        bitmap->setDimensions(w, h);
        return 0;
    }

    TextureBase *textureBase = NULL;
    if (!lua_isnone(L, 2))
    	textureBase=static_cast<TextureBase*>(binder.getInstance("TextureBase", 2));
    int slot=luaL_optinteger(L,3,0);
	Transform* matrix = NULL;
	if (!lua_isnone(L, 4))
		matrix = static_cast<Transform*>(binder.getInstance("Matrix", 4));
    bitmap->setTexture(textureBase, slot,matrix?&matrix->matrix():NULL);

    return 0;
}

int PixelBinder::setTextureMatrix(lua_State *L)
{
    Binder binder(L);

    Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

    Transform* matrix = static_cast<Transform*>(binder.getInstance("Matrix", 2));
    bitmap->setTextureMatrix(&matrix->matrix());

    return 0;
}

int PixelBinder::setColor(lua_State* L)
{
	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

    if (lua_gettop(L) == 9) bitmap->setGradient(
                luaL_checknumber(L, 2), luaL_checknumber(L, 3),
                luaL_checknumber(L, 4), luaL_checknumber(L, 5),
                luaL_checknumber(L, 6), luaL_checknumber(L, 7),
                luaL_checknumber(L, 8), luaL_checknumber(L, 9));
    else if (lua_gettop(L) == 5) bitmap->setGradient(
                luaL_checknumber(L, 2), luaL_checknumber(L, 3),
                luaL_checknumber(L, 2), luaL_checknumber(L, 3),
                luaL_checknumber(L, 4), luaL_checknumber(L, 5),
                luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    else if (lua_gettop(L) == 6) bitmap->setGradientWithAngle(
                luaL_checknumber(L, 2), luaL_checknumber(L, 3),
                luaL_checknumber(L, 4), luaL_checknumber(L, 5),
                luaL_checknumber(L, 6));
    else {
        unsigned int color = luaL_optinteger(L, 2, 0);
        lua_Number alpha = luaL_optnumber(L, 3, 1.0);

        int r = (color >> 16) & 0xff;
        int g = (color >> 8) & 0xff;
        int b = color & 0xff;

        bitmap->setColor(r/255.f,g/255.f,b/255.f,alpha);
        bitmap->clearGradient();
    }

	return 0;
}

int PixelBinder::getColor(lua_State* L)
{
	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

    if (bitmap->hasGradient()) {
        int c1, c2, c3, c4;
        float a1, a2, a3, a4;
        bitmap->getGradient(c1, a1, c2, a2, c3, a3, c4, a4);
        lua_pushnumber(L,c1); lua_pushnumber(L,a1);
        lua_pushnumber(L,c2); lua_pushnumber(L,a2);
        lua_pushnumber(L,c3); lua_pushnumber(L,a3);
        lua_pushnumber(L,c4); lua_pushnumber(L,a4);
        return 8;
    }

	float r,g,b,a;
	bitmap->getColor(r,g,b,a);

	unsigned int color = ((((int)(r*255))&0xFF)<<16)|((((int)(g*255))&0xFF)<<8)|((((int)(b*255))&0xFF)<<0);
	lua_pushnumber(L,color);
	lua_pushnumber(L,a);

	return 2;
}

int PixelBinder::setTexturePosition(lua_State* L)
{
    Binder binder(L);

    Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

    lua_Number x = luaL_checknumber(L, 2);
    lua_Number y = luaL_checknumber(L, 3);

    bitmap->setTexturePosition(x,y);

    return 0;
}

int PixelBinder::getTexturePosition(lua_State* L)
{
    Binder binder(L);

    Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

    float x, y;
    bitmap->getTexturePosition(x, y);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);

    return 2;
}

int PixelBinder::setTextureScale(lua_State* L)
{
    Binder binder(L);

    Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

    lua_Number sx = luaL_checknumber(L, 2);
    lua_Number sy = luaL_checknumber(L, 3);

    bitmap->setTextureScale(sx,sy);

    return 0;
}

int PixelBinder::getTextureScale(lua_State* L)
{
    Binder binder(L);

    Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

    float sx, sy;
    bitmap->getTextureScale(sx, sy);

    lua_pushnumber(L, sx);
    lua_pushnumber(L, sy);

    return 2;
}
