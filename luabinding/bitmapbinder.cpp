#include "bitmapbinder.h"
#include "bitmap.h"
#include "bitmapdata.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>

BitmapBinder::BitmapBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"getAnchorPoint", getAnchorPoint},
		{"setAnchorPoint", setAnchorPoint},
        {"setTexture", setTexture},
        {"setTextureRegion", setTextureRegion},
        {NULL, NULL},
	};

	binder.createClass("Bitmap", "Sprite", create, destruct, functionList);
}

int BitmapBinder::create(lua_State* L)
{
	StackChecker checker(L, "BitmapBinder::create", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

	if (binder.isInstanceOf("TextureBase", 1))
	{
		TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 1));

        Bitmap* bitmap = new Bitmap(application->getApplication(), textureBase);
		binder.pushInstance("Bitmap", bitmap);
	}
	else if (binder.isInstanceOf("TextureRegion", 1))
	{
		BitmapData* bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", 1));

        Bitmap* bitmap = new Bitmap(application->getApplication(), bitmapData);
		binder.pushInstance("Bitmap", bitmap);
	}
	else
	{
		luaL_typerror(L, 1, "TextureBase or TextureRegion");
		return 0;
	}

	//TODO: BitmapData'yi da __bitmapData icinde tutmak lazim. yoksa gc'de kaybolur gider
	//Bunu neden daha once dusunmemisim ya? Belki de tutmamamin bi nedeni vardir.
	//Test etmek lazim.
	//Hatirladim... lua tarafinda gc'ile collect edilse bile, c++ tarafinda silinmeyecegi icin
	//hic bir sorun olmuyor. eger getBitmapData ya da setBitmapData gibi fonksiyonlar yoksa
	//__bitmapData icinde tutmanin bi manasi yok.
	
	return 1;
}

int BitmapBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Bitmap* bitmap = static_cast<Bitmap*>(ptr);
	bitmap->unref();

	return 0;
}

int BitmapBinder::setAnchorPoint(lua_State* L)
{
	StackChecker checker(L, "BitmapBinder::setAnchorPoint", 0);

	Binder binder(L);

	Bitmap* bitmap = static_cast<Bitmap*>(binder.getInstance("Bitmap", 1));

	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);

	bitmap->setAnchorPoint(x, y);

	return 0;
}

int BitmapBinder::getAnchorPoint(lua_State* L)
{
	StackChecker checker(L, "BitmapBinder::getAnchorPoint", 2);

	Binder binder(L);

	Bitmap* bitmap = static_cast<Bitmap*>(binder.getInstance("Bitmap", 1));

	float x, y;
	bitmap->getAnchorPoint(&x, &y);

	lua_pushnumber(L, x);
	lua_pushnumber(L, y);

	return 2;
}

int BitmapBinder::setTexture(lua_State *L)
{
    Binder binder(L);

    Bitmap *bitmap = static_cast<Bitmap*>(binder.getInstance("Bitmap", 1));
    TextureBase *textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 2));
    bitmap->setTexture(textureBase);

    return 0;
}

int BitmapBinder::setTextureRegion(lua_State *L)
{
    Binder binder(L);

    Bitmap *bitmap = static_cast<Bitmap*>(binder.getInstance("Bitmap", 1));
    BitmapData *bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", 2));
    bitmap->setTextureRegion(bitmapData);

    return 0;
}
