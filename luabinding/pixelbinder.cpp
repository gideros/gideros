#include "pixelbinder.h"
#include "pixel.h"
#include "bitmapdata.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include "spritebinder.h"
#include <luautil.h>

PixelBinder::PixelBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"setWidth", setWidth},
		{"setHeight", setHeight},
		{"getAnchorPoint", getAnchorPoint},
		{"setAnchorPoint", setAnchorPoint},
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
		{"setNinePatch", setNinePatch},
        {"updateStyle", updateStyle},
        {"__parseGhosts", __parseGhosts},
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

     if (lua_type(L, 3) == LUA_TTABLE) {
         bitmap->setStretching(true);
         int tw,th;

         if (binder.isInstanceOf("TextureRegion", 3)) {
            BitmapData *bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", 3));
 			bitmap->setTextureRegion(bitmapData, 0);
 	    	bitmapData->getRegion(NULL,NULL,&tw,&th, NULL,NULL,NULL,NULL);
         }
         else {
			TextureBase *textureBase = NULL;
            textureBase=static_cast<TextureBase*>(binder.getInstance("TextureBase", 3));
			bitmap->setTexture(textureBase, 0, NULL);
			tw=textureBase->data->width;
			th=textureBase->data->height;
        }
        bitmap->setColor(1, 1, 1, 1);

        lua_Number w = luaL_optnumber(L, 1, tw);
        lua_Number h = luaL_optnumber(L, 2, th);
        bitmap->setDimensions(w, h);

        binder.pushInstance("Pixel", bitmap);
        return 1;
    }

    if (lua_type(L, 1) == LUA_TTABLE) {
        int tw,th;

        if (binder.isInstanceOf("TextureRegion", 1)) {
           BitmapData *bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", 1));
			bitmap->setTextureRegion(bitmapData, 0);
	    	bitmapData->getRegion(NULL,NULL,&tw,&th, NULL,NULL,NULL,NULL);
        }
        else {
			TextureBase *textureBase = NULL;
			textureBase=static_cast<TextureBase*>(binder.getInstance("TextureBase", 1));
			bitmap->setTexture(textureBase, 0, NULL);
			tw=textureBase->data->width;
			th=textureBase->data->height;
       }
        bitmap->setColor(1, 1, 1, 1);

        lua_Number w = luaL_optnumber(L, 2, tw);
        lua_Number h = luaL_optnumber(L, 3, th);
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

    int postCol=3;
    float cvec[4];
    if (lua_tocolorf(L,1,cvec,0)) {
        postCol=2;
        bitmap->setColor(cvec[0],cvec[1],cvec[2],cvec[3]);
    }
    else {
        unsigned int color = luaL_optinteger(L, 1, 0xffffff);
        lua_Number alpha = luaL_optnumber(L, 2, 1.0);
        int r = (color >> 16) & 0xff;
        int g = (color >> 8) & 0xff;
        int b = color & 0xff;
        bitmap->setColor(r/255.f,g/255.f,b/255.f,alpha);
    }

    lua_Number w = luaL_optnumber(L, postCol, 1.0);
    lua_Number h = luaL_optnumber(L, postCol+1, w);
	bitmap->setDimensions(w,h);

	binder.pushInstance("Pixel", bitmap);
	return 1;
}

int PixelBinder::destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
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

int PixelBinder::setAnchorPoint(lua_State* L)
{
	StackChecker checker(L, "PixelBinder::setAnchorPoint", 0);

	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);

	bitmap->setAnchorPoint(x, y);

	return 0;
}

int PixelBinder::getAnchorPoint(lua_State* L)
{
	StackChecker checker(L, "PixelBinder::getAnchorPoint", 2);

	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

	float x, y;
	bitmap->getAnchorPoint(&x, &y);

	lua_pushnumber(L, x);
	lua_pushnumber(L, y);

	return 2;
}

int PixelBinder::setNinePatch(lua_State *L) {
	Binder binder(L);
	int argc=lua_gettop(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));
	if ((argc==1)||(argc==2)) {
		if (lua_isboolean(L,2)) {
			bitmap->setStretching(lua_toboolean(L,2));
			bitmap->setNinePatch(0,0,0,0,0,0,0,0);
		}
		else {
			lua_Number i = luaL_optnumber(L, 2,-1);
			bitmap->setStretching(i==0);
			if (i>=0)
				bitmap->setNinePatch(i,i,i,i,i,i,i,i);
		}
	} else if (argc==3) {
		lua_Number iv = luaL_checknumber(L, 2);
		lua_Number it = luaL_checknumber(L, 3);
		bitmap->setNinePatch(iv,iv,iv,iv,it,it,it,it);
	}
	else if (argc<=5) {
		lua_Number l = luaL_checknumber(L, 2);
		lua_Number r = luaL_checknumber(L, 3);
		lua_Number t = luaL_checknumber(L, 4);
		lua_Number b = luaL_checknumber(L, 5);
		bitmap->setNinePatch(l,r,t,b,l,r,t,b);
	}
	else {
		lua_Number vl = luaL_checknumber(L, 2);
		lua_Number vr = luaL_checknumber(L, 3);
		lua_Number vt = luaL_checknumber(L, 4);
		lua_Number vb = luaL_checknumber(L, 5);
		lua_Number tl = luaL_checknumber(L, 6);
		lua_Number tr = luaL_checknumber(L, 7);
		lua_Number tt = luaL_checknumber(L, 8);
		lua_Number tb = luaL_checknumber(L, 9);
		bitmap->setNinePatch(vl,vr,vt,vb,tl,tr,tt,tb);
	}

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
        bitmap->setTextureRegion(bitmapData, luaL_optinteger(L,3,0));
        return 0;
    }

    TextureBase *textureBase = NULL;
    if (!lua_isnoneornil(L, 2))
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
#define COLVEC(var,idx) float var[4]; LuaApplication::resolveColor(L,1,idx,var,bitmap->styCache_##var);
#define COLARG(var) (((int)(var[0]*0xFF0000))&0xFF0000)|(((int)(var[1]*0xFF00))&0xFF00)|((int)((var[2]*0xFF))&0xFF),var[3]
	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));
    bitmap->styCache_color.clear();
    bitmap->styCache_c1.clear();
    bitmap->styCache_c2.clear();
    bitmap->styCache_c3.clear();
    bitmap->styCache_c4.clear();
    int ctype=lua_type(L,2);
    if ((ctype==LUA_TVECTOR)||(ctype==LUA_TCOLOR)||(ctype==LUA_TSTRING)||(ctype==LUA_TUSERDATA)) { //Vector or resolvables colors
		if (lua_gettop(L) == 5) {
			COLVEC(c1,2);
			COLVEC(c2,3);
			COLVEC(c3,4);
			COLVEC(c4,5);
			bitmap->setGradient(COLARG(c1),COLARG(c2),COLARG(c3),COLARG(c4));
		}
	    else if (lua_gettop(L) == 3) {
			COLVEC(c1,2);
			COLVEC(c2,3);
	    	bitmap->setGradient(COLARG(c1),COLARG(c2),COLARG(c1),COLARG(c2));
	    }
	    else if (lua_gettop(L) == 4) {
			COLVEC(c1,2);
			COLVEC(c2,3);
	    	bitmap->setGradientWithAngle(COLARG(c1),COLARG(c2),
	                luaL_checknumber(L, 4));
	    }
	    else {
			COLVEC(color,2);
	        bitmap->setColor(color[0],color[1],color[2],color[3]);
	        bitmap->clearGradient();
	    }
#undef COLVEC
#undef COLARG
	}
	else if (lua_gettop(L) == 9) bitmap->setGradient(
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

#define HASCOL(var) (!bitmap->styCache_##var.empty())
#define COLVEC(var) float var[4]; LuaApplication::resolveColor(L,1,0,var,bitmap->styCache_##var);
int PixelBinder::updateStyle(lua_State* L)
{
    StackChecker checker(L, "PixelBinder::updateStyle", 0);
    Binder binder(L);
    Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));
    SpriteBinder::updateStyle(L);

//TODO Handle gradient
    if (HASCOL(color)) {
        COLVEC(color);
        bitmap->setColor(color[0],color[1],color[2],color[3]);
    }
    return 0;
}
#undef HASCOL
#undef COLVEC

int PixelBinder::__parseGhosts(lua_State* L)
{
    Binder binder(L);
    Pixel* model = static_cast<Pixel*>(binder.getInstanceOfType("Pixel", GREFERENCED_TYPEMAP_PIXEL, 2));
    GhostPixel *ghost=new GhostPixel(model);
    //Color
    lua_rawgetfield(L,1,"color");
    if (lua_isnoneornil(L,-1))
        ghost->hasColor=false;
    else {
        ghost->hasColor=true;
        std::string ccache;
        float cc[4];
        LuaApplication::resolveColor(L,2,-1,cc,ccache);
        ghost->color[0]=cc[0]*255;
        ghost->color[1]=cc[1]*255;
        ghost->color[2]=cc[2]*255;
        ghost->color[3]=cc[3]*255;
    }
    lua_pop(L,1);
    SpriteBinder::__parseGhost(ghost,L);
    lua_pushlightuserdata(L,ghost);
    return 1;
}
