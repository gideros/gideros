#include "spritebinder.h"
#include "sprite.h"
#include "matrix.h"
#include "colortransform.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>

SpriteBinder::SpriteBinder(lua_State* L)
{
	Binder binder(L);
	
	static const luaL_Reg functionList[] = {
		{"addChild", SpriteBinder::addChild},
		{"addChildAt", SpriteBinder::addChildAt},
		{"removeChild", SpriteBinder::removeChild},
		{"removeChildAt", SpriteBinder::removeChildAt},
		{"getNumChildren", SpriteBinder::numChildren},
        {"swapChildren", SpriteBinder::swapChildren},
		{"swapChildrenAt", SpriteBinder::swapChildrenAt},
		{"getChildAt", SpriteBinder::getChildAt},
		{"getParent", SpriteBinder::getParent},
		{"contains", SpriteBinder::contains},
		{"getChildIndex", SpriteBinder::getChildIndex},
		{"removeFromParent", SpriteBinder::removeFromParent},
		{"setClip", SpriteBinder::setClip},
        {"getClip", SpriteBinder::getClip},
		{"getX", SpriteBinder::getX},
		{"getY", SpriteBinder::getY},
		{"getZ", SpriteBinder::getZ},
		{"getRotation", SpriteBinder::getRotation},
		{"getRotationX", SpriteBinder::getRotationX},
		{"getRotationY", SpriteBinder::getRotationY},
		{"getScaleX", SpriteBinder::getScaleX},
		{"getScaleY", SpriteBinder::getScaleY},
		{"getScaleZ", SpriteBinder::getScaleZ},
        {"getSkewX", SpriteBinder::getSkewX},
        {"getSkewY", SpriteBinder::getSkewY},
		{"setX", SpriteBinder::setX},
		{"setY", SpriteBinder::setY},
		{"setZ", SpriteBinder::setZ},
		{"setRotation", SpriteBinder::setRotation},
		{"setRotationX", SpriteBinder::setRotationX},
		{"setRotationY", SpriteBinder::setRotationY},
		{"setScaleX", SpriteBinder::setScaleX},
		{"setScaleY", SpriteBinder::setScaleY},
		{"setScaleZ", SpriteBinder::setScaleZ},
        {"setSkewX", SpriteBinder::setSkewX},
        {"setSkewY", SpriteBinder::setSkewY},
		{"setPosition", SpriteBinder::setPosition},
		{"getPosition", SpriteBinder::getPosition},
        {"setAnchorPosition", SpriteBinder::setAnchorPosition},
        {"getAnchorPosition", SpriteBinder::getAnchorPosition},
        {"setAnchorPoint", SpriteBinder::setAnchorPoint},
        {"getAnchorPoint", SpriteBinder::getAnchorPoint},
		{"setScale", SpriteBinder::setScale},
		{"getScale", SpriteBinder::getScale},
        {"setSkew", SpriteBinder::setSkew},
        {"getSkew", SpriteBinder::getSkew},
		{"localToGlobal", SpriteBinder::localToGlobal},
		{"globalToLocal", SpriteBinder::globalToLocal},
		{"isVisible", SpriteBinder::isVisible},
		{"setVisible", SpriteBinder::setVisible},
		{"getColorTransform", SpriteBinder::getColorTransform},
		{"setColorTransform", SpriteBinder::setColorTransform},
		{"hitTestPoint", SpriteBinder::hitTestPoint},
		{"getWidth", SpriteBinder::getWidth},
		{"getHeight", SpriteBinder::getHeight},
		{"getMatrix", SpriteBinder::getMatrix},
		{"setMatrix", SpriteBinder::setMatrix},
		{"getAlpha", SpriteBinder::getAlpha},
		{"setAlpha", SpriteBinder::setAlpha},
		{"getBounds", SpriteBinder::getBounds},
        {"setBlendMode", SpriteBinder::setBlendMode},
        {"clearBlendMode", SpriteBinder::clearBlendMode},
		{"setShader", SpriteBinder::setShader},
		{"setShaderConstant", SpriteBinder::setShaderConstant},

		{"set", SpriteBinder::set},
		{"get", SpriteBinder::get},
		{NULL, NULL},
	};

	binder.createClass("Sprite", "EventDispatcher", create, destruct, functionList);

    //lua_newtable(L);
    lua_getglobal(L, "Sprite");

	lua_pushinteger(L, ShaderEngine::ZERO);
	lua_setfield(L, -2, "ZERO");
	lua_pushinteger(L, ShaderEngine::ONE);
	lua_setfield(L, -2, "ONE");
	lua_pushinteger(L, ShaderEngine::SRC_COLOR);
	lua_setfield(L, -2, "SRC_COLOR");
	lua_pushinteger(L, ShaderEngine::ONE_MINUS_SRC_COLOR);
	lua_setfield(L, -2, "ONE_MINUS_SRC_COLOR");
	lua_pushinteger(L, ShaderEngine::DST_COLOR);
	lua_setfield(L, -2, "DST_COLOR");
	lua_pushinteger(L, ShaderEngine::ONE_MINUS_DST_COLOR);
	lua_setfield(L, -2, "ONE_MINUS_DST_COLOR");
	lua_pushinteger(L, ShaderEngine::SRC_ALPHA);
	lua_setfield(L, -2, "SRC_ALPHA");
	lua_pushinteger(L, ShaderEngine::ONE_MINUS_SRC_ALPHA);
	lua_setfield(L, -2, "ONE_MINUS_SRC_ALPHA");
	lua_pushinteger(L, ShaderEngine::DST_ALPHA);
	lua_setfield(L, -2, "DST_ALPHA");
	lua_pushinteger(L, ShaderEngine::ONE_MINUS_DST_ALPHA);
	lua_setfield(L, -2, "ONE_MINUS_DST_ALPHA");
	//lua_pushinteger(L, ShaderEngine::CONSTANT_COLOR);
	//lua_setfield(L, -2, "CONSTANT_COLOR");
	//lua_pushinteger(L, ShaderEngine::ONE_MINUS_CONSTANT_COLOR);
	//lua_setfield(L, -2, "ONE_MINUS_CONSTANT_COLOR");
	//lua_pushinteger(L, ShaderEngine::CONSTANT_ALPHA);
	//lua_setfield(L, -2, "CONSTANT_ALPHA");
	//lua_pushinteger(L, ShaderEngine::ONE_MINUS_CONSTANT_ALPHA);
	//lua_setfield(L, -2, "ONE_MINUS_CONSTANT_ALPHA");
	lua_pushinteger(L, ShaderEngine::SRC_ALPHA_SATURATE);
	lua_setfield(L, -2, "SRC_ALPHA_SATURATE");

    lua_pushstring(L, "alpha");
    lua_setfield(L, -2, "ALPHA");
    lua_pushstring(L, "noAlpha");
    lua_setfield(L, -2, "NO_ALPHA");
    lua_pushstring(L, "add");
    lua_setfield(L, -2, "ADD");
    lua_pushstring(L, "multiply");
    lua_setfield(L, -2, "MULTIPLY");
    lua_pushstring(L, "screen");
    lua_setfield(L, -2, "SCREEN");

    lua_setglobal(L, "Sprite");
}

int SpriteBinder::create(lua_State* L)
{
	StackChecker checker(L, "Sprite", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	Binder binder(L);
    Sprite* sprite = new Sprite(application->getApplication());
	binder.pushInstance("Sprite", sprite);

/*	lua_getglobal(L, "Graphics");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);
	lua_pushvalue(L, -2);
	lua_call(L, 1, 1);
	lua_setfield(L, -2, "graphics"); // sprite.graphics = Graphics.new(sprite)
	*/

	return 1;
}

int SpriteBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Sprite* sprite = static_cast<Sprite*>(ptr);
	sprite->unref();

	return 0;
}

static void createChildrenTable(lua_State* L)
{
	StackChecker checker(L, "createChildrenTable");
	
	lua_getfield(L, 1, "__children");
	if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);

		lua_newtable(L);
		lua_setfield(L, 1, "__children");
	}
	else
		lua_pop(L, 1);
}

int SpriteBinder::addChild(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::addChild", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	Sprite* child = static_cast<Sprite*>(binder.getInstance("Sprite", 2));

	GStatus status;
	if (sprite->canChildBeAdded(child, &status) == false)
		return luaL_error(L, status.errorString());

	if (child->parent() == sprite)
	{
		sprite->addChild(child);
		return 0;
	}

	if (child->parent())
	{
		// remove from parent's __children table
		lua_getfield(L, 2, "__parent");			// push child.__parent
		lua_getfield(L, -1, "__children");		// push child.__parent.__children

		lua_pushlightuserdata(L, child);
		lua_pushnil(L);
		lua_rawset(L, -3);						// child.__parent.__children[child] = nil

		lua_pop(L, 2);							// pop child.__parent and child.__parent.__children
	}

	// set new parent
	lua_pushvalue(L, 1);
	lua_setfield(L, 2, "__parent");				// child.__parent = sprite

	createChildrenTable(L);

	// add to __children table
	lua_getfield(L, 1, "__children");			// push sprite.__children
	lua_pushlightuserdata(L, child);
	lua_pushvalue(L, 2);
	lua_rawset(L, -3);							// sprite.__children[child] = child
	lua_pop(L, 1);								// pop sprite.__children

	sprite->addChild(child);

	return 0;
}


int SpriteBinder::addChildAt(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::addChildAt", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	Sprite* child = static_cast<Sprite*>(binder.getInstance("Sprite", 2));
	int index = luaL_checkinteger(L, 3);

	GStatus status;
    if (sprite->canChildBeAddedAt(child, index - 1, &status) == false)
		return luaL_error(L, status.errorString());

	if (child->parent() == sprite)
	{
        sprite->addChildAt(child, index - 1);
		return 0;
	}

	if (child->parent())
	{
		// remove from parent's __children table
		lua_getfield(L, 2, "__parent");			// push child.__parent
		lua_getfield(L, -1, "__children");		// push child.__parent.__children

		lua_pushlightuserdata(L, child);
		lua_pushnil(L);
		lua_rawset(L, -3);						// child.__parent.__children[child] = nil

		lua_pop(L, 2);							// pop child.__parent and child.__parent.__children
	}

	// set new parent
	lua_pushvalue(L, 1);
	lua_setfield(L, 2, "__parent");				// child.__parent = sprite

	createChildrenTable(L);

	// add to __children table
	lua_getfield(L, 1, "__children");			// push sprite.__children
	lua_pushlightuserdata(L, child);
	lua_pushvalue(L, 2);
	lua_rawset(L, -3);							// sprite.__children[child] = child
	lua_pop(L, 1);								// pop sprite.__children

    sprite->addChildAt(child, index - 1);

	return 0;
}


int SpriteBinder::removeChildAt(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::removeChildAt", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	int index = luaL_checknumber(L, 2);
	if (index < 0) index = sprite->childCount() + index + 1;
	if (index < 1 || index > sprite->childCount())
		return luaL_error(L, GStatus(2006).errorString());	// Error #2006: The supplied index is out of bounds.

	Sprite* child = sprite->getChildAt(index - 1);

	lua_getfield(L, 1, "__children");	// push sprite.__children
	lua_pushlightuserdata(L, child);
	lua_rawget(L, -2);					// push sprite.__children[child]
	lua_pushnil(L);
	lua_setfield(L, -2, "__parent");	// sprite.__children[child].__parent = nil
	lua_pop(L, 1);						// pop sprite.__children[child]

	lua_pushlightuserdata(L, child);
	lua_pushnil(L);
	lua_rawset(L, -3);					// sprite.__children[sprite] = nil
	lua_pop(L, 1);						// pop sprite.__children

	sprite->removeChildAt(index - 1);

	return 0;
}


int SpriteBinder::removeChild(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::removeChild", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	Sprite* child = static_cast<Sprite*>(binder.getInstance("Sprite", 2));

	GStatus status;
	int index = sprite->getChildIndex(child, &status);
	if (status.error() == true)
		return luaL_error(L, status.errorString());

	lua_pushnil(L);
	lua_setfield(L, 2, "__parent");		// child.__parent = nil

	lua_getfield(L, 1, "__children");	// push sprite.__children
	lua_pushlightuserdata(L, child);
	lua_pushnil(L);
	lua_rawset(L, -3);					// sprite.__children[sprite] = nil
	lua_pop(L, 1);						// pop sprite.__children

	sprite->removeChild(index);

	return 0;
}

int SpriteBinder::numChildren(lua_State* L)
{
	StackChecker checker(L, "numChildren", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushinteger(L, sprite->childCount());
	
	return 1;
}

int SpriteBinder::swapChildren(lua_State* L)
{
        StackChecker checker(L, "SpriteBinder::swapChildren",0);
        Binder binder(L);
        Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
        Sprite* sprite1 = static_cast<Sprite*>(binder.getInstance("Sprite", 2));
        Sprite* sprite2 = static_cast<Sprite*>(binder.getInstance("Sprite", 3));

        sprite->swapChildren(sprite1,sprite2);

        return 0;
}

int SpriteBinder::swapChildrenAt(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::swapChildrenAt",0);
	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	int index1 = luaL_checknumber(L, 2);
	if (index1 < 0) index1 = sprite->childCount() + index1 + 1;
	if (index1 < 1 || index1 > sprite->childCount())
		return luaL_error(L, GStatus(2006).errorString());	// Error #2006: The supplied index1 is out of bounds.
        int index2 = luaL_checknumber(L, 3);
        if (index2 < 0) index2 = sprite->childCount() + index2 + 1;
	if (index2 < 1 || index2 > sprite->childCount())
		return luaL_error(L, GStatus(2006).errorString());	// Error #2006: The supplied index2 is out of bounds.

	sprite->swapChildrenAt(index1-1,index2-1);

	return 0;
}

int SpriteBinder::getChildAt(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getChildAt", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));
	int index = luaL_checkinteger(L, 2);
	if (index < 0) index = sprite->childCount() + index + 1;
	if (index < 1 || index > sprite->childCount())
		return luaL_error(L, GStatus(2006).errorString());	// Error #2006: The supplied index is out of bounds.

	Sprite* child = sprite->getChildAt(index - 1);

	lua_getfield(L, 1, "__children");	// push sprite.__children
	lua_pushlightuserdata(L, child);
	lua_rawget(L, -2);					// push sprite.__children[child]
	lua_remove(L, -2);					// pop sprite.__children

	return 1;
}

int SpriteBinder::setClip(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setClip", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	lua_Number w = luaL_checknumber(L, 4);
	lua_Number h = luaL_checknumber(L, 5);
	sprite->setClip(x, y, w, h);

	return 0;
}

int SpriteBinder::getClip(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::getClip", 4);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

    lua_pushnumber(L, sprite->clipX());
    lua_pushnumber(L, sprite->clipY());
    lua_pushnumber(L, sprite->clipW());
    lua_pushnumber(L, sprite->clipH());

    return 4;
}

int SpriteBinder::getX(lua_State* L)
{
	StackChecker checker(L, "getX", 1);
	
	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushnumber(L, sprite->x());

	return 1;
}

int SpriteBinder::getY(lua_State* L)
{
	StackChecker checker(L, "getY", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushnumber(L, sprite->y());

	return 1;
}

int SpriteBinder::getZ(lua_State* L)
{
	StackChecker checker(L, "getZ", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushnumber(L, sprite->z());

	return 1;
}

int SpriteBinder::getRotation(lua_State* L)
{
	StackChecker checker(L, "getRotation", 1);
	
	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushnumber(L, sprite->rotation());

	return 1;
}

int SpriteBinder::getRotationX(lua_State* L)
{
	StackChecker checker(L, "getRotationX", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushnumber(L, sprite->rotationX());

	return 1;
}

int SpriteBinder::getRotationY(lua_State* L)
{
	StackChecker checker(L, "getRotationY", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushnumber(L, sprite->rotationY());

	return 1;
}

int SpriteBinder::getScaleX(lua_State* L)
{
	StackChecker checker(L, "getScaleX", 1);
	
	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushnumber(L, sprite->scaleX());

	return 1;
}

int SpriteBinder::getScaleY(lua_State* L)
{
	StackChecker checker(L, "getScaleY", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushnumber(L, sprite->scaleY());

	return 1;
}

int SpriteBinder::getScaleZ(lua_State* L)
{
	StackChecker checker(L, "getScaleZ", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushnumber(L, sprite->scaleZ());

	return 1;
}

int SpriteBinder::getSkewX(lua_State* L)
{
    StackChecker checker(L, "getSkewX", 1);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

    lua_pushnumber(L, sprite->skewX());

    return 1;
}

int SpriteBinder::getSkewY(lua_State* L)
{
    StackChecker checker(L, "getSkewY", 1);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

    lua_pushnumber(L, sprite->skewY());

    return 1;
}

int SpriteBinder::setX(lua_State* L)
{
	StackChecker checker(L, "setX");

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double x = luaL_checknumber(L, 2);
	sprite->setX(x);

	return 0;
}

int SpriteBinder::setY(lua_State* L)
{
	StackChecker checker(L, "setY");

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double y = luaL_checknumber(L, 2);
	sprite->setY(y);

	return 0;
}

int SpriteBinder::setZ(lua_State* L)
{
	StackChecker checker(L, "setZ");

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double z = luaL_checknumber(L, 2);
	sprite->setZ(z);

	return 0;
}

int SpriteBinder::setRotation(lua_State* L)
{
	StackChecker checker(L, "setRotation");

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double rotation = luaL_checknumber(L, 2);
	sprite->setRotation(rotation);

	return 0;
}

int SpriteBinder::setRotationX(lua_State* L)
{
	StackChecker checker(L, "setRotationX");

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double rotation = luaL_checknumber(L, 2);
	sprite->setRotationX(rotation);

	return 0;
}

int SpriteBinder::setRotationY(lua_State* L)
{
	StackChecker checker(L, "setRotationY");

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double rotation = luaL_checknumber(L, 2);
	sprite->setRotationY(rotation);

	return 0;
}

int SpriteBinder::setScaleX(lua_State* L)
{
	StackChecker checker(L, "setScaleX");

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double scaleX = luaL_checknumber(L, 2);
	sprite->setScaleX(scaleX);

	return 0;
}

int SpriteBinder::setScaleY(lua_State* L)
{
	StackChecker checker(L, "setScaleY");

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double scaleY = luaL_checknumber(L, 2);
	sprite->setScaleY(scaleY);

	return 0;
}

int SpriteBinder::setScaleZ(lua_State* L)
{
	StackChecker checker(L, "setScaleZ");

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double scaleZ = luaL_checknumber(L, 2);
	sprite->setScaleZ(scaleZ);

	return 0;
}

int SpriteBinder::setSkewX(lua_State* L)
{
    StackChecker checker(L, "setScaleX");

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

    double skewX = luaL_checknumber(L, 2);
    sprite->setSkewX(skewX);

    return 0;
}

int SpriteBinder::setSkewY(lua_State* L)
{
    StackChecker checker(L, "setScaleY");

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

    double skewY = luaL_checknumber(L, 2);
    sprite->setSkewY(skewY);

    return 0;
}

int SpriteBinder::setPosition(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setPosition", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	if (lua_isnoneornil(L, 4))
		sprite->setXY(x, y);
	else
	{
		lua_Number z = luaL_checknumber(L, 4);
		sprite->setXYZ(x, y, z);
	}

	return 0;
}

int SpriteBinder::getPosition(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getPosition", 3);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

	lua_pushnumber(L, sprite->x());
	lua_pushnumber(L, sprite->y());
	lua_pushnumber(L, sprite->z());

	return 3;
}

int SpriteBinder::setAnchorPosition(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setAnchorPosition", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

    lua_Number x = luaL_checknumber(L, 2);
    lua_Number y = luaL_checknumber(L, 3);
	if (lua_isnoneornil(L, 4))
	    sprite->setRefXY(x, y);
	else
	{
		lua_Number z = luaL_checknumber(L, 4);
		sprite->setRefXYZ(x, y, z);
	}

    return 0;
}

int SpriteBinder::getAnchorPosition(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::getAnchorPosition", 3);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

    lua_pushnumber(L, sprite->refX());
    lua_pushnumber(L, sprite->refY());
    lua_pushnumber(L, sprite->refZ());

    return 3;
}

int SpriteBinder::setAnchorPoint(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setAnchorPoint", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

    lua_Number x = luaL_checknumber(L, 2);
    lua_Number y = luaL_checknumber(L, 3);

    float x1, y1, x2, y2;
    sprite->objectBounds(&x1, &y1, &x2, &y2);

    sprite->setRefXY(x * (x2 - x1), y * (y2 - y1));

    return 0;
}

int SpriteBinder::getAnchorPoint(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::getAnchorPoint", 2);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

    float x1, y1, x2, y2;
    sprite->objectBounds(&x1, &y1, &x2, &y2);

    float width = x1 != x2 ? x2 - x1 : 1;
    float height = y1 != y2 ? y2 - y1 : 1;

    lua_pushnumber(L, sprite->refX() / width);
    lua_pushnumber(L, sprite->refY() / height);

    return 2;
}

int SpriteBinder::setScale(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setScale", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = lua_isnoneornil(L, 3) ? x : luaL_checknumber(L, 3);
	if (lua_isnoneornil(L, 4)) //No Z
		sprite->setScaleXY(x, y); // Only scale X and Y
	else
	{
		lua_Number z = luaL_checknumber(L, 4);
		sprite->setScaleXYZ(x, y, z);
	}

	return 0;
}

int SpriteBinder::getScale(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getScale", 3);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

	lua_pushnumber(L, sprite->scaleX());
	lua_pushnumber(L, sprite->scaleY());
	lua_pushnumber(L, sprite->scaleZ());

	return 3;
}

int SpriteBinder::setSkew(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setSkew", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

    lua_Number x = luaL_checknumber(L, 2);
    lua_Number y = luaL_checknumber(L, 3);
    sprite->setSkewXY(x, y);

    return 0;
}

int SpriteBinder::getSkew(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::getSkew", 2);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

    lua_pushnumber(L, sprite->skewX());
    lua_pushnumber(L, sprite->skewY());

    return 2;
}

int SpriteBinder::getParent(lua_State* L)
{
	StackChecker checker(L, "getParent", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_getfield(L, 1, "__parent");

	return 1;
}

int SpriteBinder::contains(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::contains", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	Sprite* child = static_cast<Sprite*>(binder.getInstance("Sprite", 2));

	lua_pushboolean(L, sprite->contains(child));
	
	return 1;
}


int SpriteBinder::getChildIndex(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getChildIndex", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	Sprite* child = static_cast<Sprite*>(binder.getInstance("Sprite", 2));


	GStatus status;
	int index = sprite->getChildIndex(child, &status);

	if (status.error() == true)
	{
		luaL_error(L, status.errorString());
		return 0;
	}

	lua_pushinteger(L, index + 1);

	return 1;
}


int SpriteBinder::removeFromParent(lua_State* L)
{
	StackChecker checker(L, "removeFromParent", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	Sprite* parent = sprite->parent();

	if (parent == NULL)
		return 0;

	lua_getfield(L, 1, "__parent");		// puah sprite.__parent
	lua_getfield(L, -1, "__children");	// push sprite.__parent.__children
	lua_pushlightuserdata(L, sprite);
	lua_pushnil(L);
	lua_rawset(L, -3);					// sprite.__parent.__children[sprite] = nil
	lua_pop(L, 2);						// pop sprite.__parent and sprite.__parent.__children

	lua_pushnil(L);
	lua_setfield(L, 1, "__parent");		// sprite.__parent = nil

	parent->removeChild(sprite);

	return 0;
}

int SpriteBinder::localToGlobal(lua_State* L)
{
	StackChecker checker(L, "localToGlobal", 3);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	double z = luaL_optnumber(L, 4, 0.0);

	float tx, ty, tz;
	sprite->localToGlobal(x, y, z, &tx, &ty, &tz);

	lua_pushnumber(L, tx);
	lua_pushnumber(L, ty);
	lua_pushnumber(L, tz);

	return 3;
}

int SpriteBinder::globalToLocal(lua_State* L)
{
	StackChecker checker(L, "globalToLocal", 3);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	double z = luaL_optnumber(L, 4, 0.0);

	float tx, ty, tz;
	sprite->globalToLocal(x, y, z, &tx, &ty, &tz);

	lua_pushnumber(L, tx);
	lua_pushnumber(L, ty);
	lua_pushnumber(L, tz);

	return 3;
}

int SpriteBinder::isVisible(lua_State* L)
{
	StackChecker checker(L, "isVisible", 1);
	
	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushboolean(L, sprite->visible());

	return 1;
}

int SpriteBinder::setVisible(lua_State* L)
{
	StackChecker checker(L, "setVisible");

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	int visible = lua_toboolean(L, 2);
	sprite->setVisible(visible);

	return 0;
}

int SpriteBinder::getColorTransform(lua_State* L)
{
#if 0
	StackChecker checker(L, "SpriteBinder::getColorTransform", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_getglobal(L, "ColorTransform");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);
	lua_pushnumber(L, sprite->colorTransform().redMultiplier());
	lua_pushnumber(L, sprite->colorTransform().greenMultiplier());
	lua_pushnumber(L, sprite->colorTransform().blueMultiplier());
	lua_pushnumber(L, sprite->colorTransform().alphaMultiplier());
	lua_pushnumber(L, sprite->colorTransform().redOffset());
	lua_pushnumber(L, sprite->colorTransform().greenOffset());
	lua_pushnumber(L, sprite->colorTransform().blueOffset());
	lua_pushnumber(L, sprite->colorTransform().alphaOffset());
	lua_call(L, 8, 1);
	
	return 1;
#else
	StackChecker checker(L, "SpriteBinder::getColorTransform", 4);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

	lua_pushnumber(L, sprite->colorTransform().redMultiplier());
	lua_pushnumber(L, sprite->colorTransform().greenMultiplier());
	lua_pushnumber(L, sprite->colorTransform().blueMultiplier());
	lua_pushnumber(L, sprite->colorTransform().alphaMultiplier());

	return 4;
#endif
}

int SpriteBinder::setColorTransform(lua_State* L)
{
#if 0
	StackChecker checker(L, "setColorTransform", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	ColorTransform* colorTransform = static_cast<ColorTransform*>(binder.getInstance("ColorTransform", 2));

	lua_getfield(L, 2, "redMultiplier");
	lua_Number redMultiplier = luaL_checknumber(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, 2, "greenMultiplier");
	lua_Number greenMultiplier = luaL_checknumber(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, 2, "blueMultiplier");
	lua_Number blueMultiplier = luaL_checknumber(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, 2, "alphaMultiplier");
	lua_Number alphaMultiplier = luaL_checknumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, 2, "redOffset");
	lua_Number redOffset = luaL_checknumber(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, 2, "greenOffset");
	lua_Number greenOffset = luaL_checknumber(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, 2, "blueOffset");
	lua_Number blueOffset = luaL_checknumber(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, 2, "alphaOffset");
	lua_Number alphaOffset = luaL_checknumber(L, -1);
	lua_pop(L, 1);

	sprite->setColorTransform(ColorTransform(redMultiplier,
											 greenMultiplier,
											 blueMultiplier,
											 alphaMultiplier,
											 redOffset,
											 greenOffset,
											 blueOffset,
											 alphaOffset));
	return 0;
#else

	StackChecker checker(L, "SpriteBinder::setColorTransform", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

	lua_Number redMultiplier = luaL_optnumber(L, 2, 1.0);
	lua_Number greenMultiplier = luaL_optnumber(L, 3, 1.0);
	lua_Number blueMultiplier = luaL_optnumber(L, 4, 1.0);
	lua_Number alphaMultiplier = luaL_optnumber(L, 5, 1.0);

	sprite->setColorTransform(ColorTransform(redMultiplier,
											 greenMultiplier,
											 blueMultiplier,
											 alphaMultiplier));

	return 0;

#endif
}

//TODO: shapeFlag'i implement et
int SpriteBinder::hitTestPoint(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::hitTestPoint", 1);
	
	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);

	bool shapeFlag = false;

	if (lua_gettop(L) >= 4)
		shapeFlag = lua_toboolean(L, 4);

	lua_pushboolean(L, sprite->hitTestPoint(x, y, shapeFlag));
	
	return 1;
}

int SpriteBinder::getWidth(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getWidth", 1);
	
	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

    if (lua_isboolean(L, 2) && lua_toboolean(L, 2)) {
        float x1, y1, x2, y2;
        sprite->objectBounds(&x1, &y1, &x2, &y2);
        lua_pushnumber(L, x2 > x1 ? x2 - x1 : 0);
    } else
        lua_pushnumber(L, sprite->width());

	return 1;
}

int SpriteBinder::getHeight(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getHeight", 1);
	
	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

    if (lua_isboolean(L, 2) && lua_toboolean(L, 2)) {
        float x1, y1, x2, y2;
        sprite->objectBounds(&x1, &y1, &x2, &y2);
        lua_pushnumber(L, y2 > y1 ? y2 - y1 : 0);
    } else
        lua_pushnumber(L, sprite->height());

	return 1;
}


int SpriteBinder::getMatrix(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getMatrix", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	Transform *t=new Transform();
	*t=sprite->transform();

    binder.pushInstance("Matrix", t);
	
	return 1;
}

int SpriteBinder::setMatrix(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setMatrix", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	Transform* matrix = static_cast<Transform*>(binder.getInstance("Matrix", 2));

	sprite->setMatrix(matrix);

	return 0;
}

int SpriteBinder::getAlpha(lua_State* L)
{
	StackChecker checker(L, "getAlpha", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushnumber(L, sprite->alpha());

	return 1;
}

int SpriteBinder::setAlpha(lua_State* L)
{
	StackChecker checker(L, "setAlpha", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_Number alpha = luaL_checknumber(L, 2);
	sprite->setAlpha(alpha);

	return 0;
}

int SpriteBinder::getBounds(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getBounds", 4);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	Sprite* targetCoordinateSpace = static_cast<Sprite*>(binder.getInstance("Sprite", 2));

	float minx, miny, maxx, maxy;
	sprite->getBounds(targetCoordinateSpace, &minx, &miny, &maxx, &maxy);

	if (minx > maxx || miny > maxy)
	{
		// empty bounds
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
	}
	else
	{
		lua_pushnumber(L, minx);
		lua_pushnumber(L, miny);
		lua_pushnumber(L, maxx - minx);
		lua_pushnumber(L, maxy - miny);
	}

	return 4;
}

int SpriteBinder::setBlendMode(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setBlendFunc", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

    if (lua_type(L, 2) == LUA_TSTRING) {
        std::string m = lua_tostring(L, 2);
        if (m == "alpha") sprite->setBlendFunc(
            ShaderEngine::SRC_COLOR, ShaderEngine::ONE_MINUS_SRC_ALPHA);
        else if (m == "noAlpha") sprite->setBlendFunc(
            ShaderEngine::ONE, ShaderEngine::ZERO);
        else if (m == "add") sprite->setBlendFunc(
            ShaderEngine::ONE, ShaderEngine::ONE);
        else if (m == "multiply") sprite->setBlendFunc(
            ShaderEngine::DST_COLOR, ShaderEngine::ONE_MINUS_SRC_ALPHA);
        else if (m == "screen") sprite->setBlendFunc(
            ShaderEngine::ONE, ShaderEngine::ONE_MINUS_SRC_COLOR);
        else luaL_error(L, "Parameter 'blendMode' must be one of the accepted values.");
    } else {
        int src = luaL_checkinteger(L, 2);
        int dst = luaL_checkinteger(L, 3);
        sprite->setBlendFunc(static_cast<ShaderEngine::BlendFactor>(src),
                             static_cast<ShaderEngine::BlendFactor>(dst));
    }

    return 0;
}


int SpriteBinder::clearBlendMode(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::clearBlendFunc", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	sprite->clearBlendFunc();

	return 0;
}

int SpriteBinder::setShader(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setShader", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	ShaderProgram* shader = NULL;
	if (!lua_isnoneornil(L,2))
		shader=static_cast<ShaderProgram*>(binder.getInstance("Shader", 2));
	sprite->setShader(shader);

	return 0;
}

int SpriteBinder::setShaderConstant(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setShaderConstant", 0);

	Binder binder(L);

	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

   // virtual void setConstant(int index,ConstantType type,const void *ptr);

	Sprite::ShaderParam sp;


	sp.name=luaL_checkstring(L,2);
	sp.type = (ShaderProgram::ConstantType) luaL_checkinteger(L, 3);
	sp.mult = luaL_checknumber(L, 4);
	int cm=1;
	switch (sp.type)
	{
	case ShaderProgram::CFLOAT2: cm=2; break;
	case ShaderProgram::CFLOAT3: cm=3; break;
	case ShaderProgram::CFLOAT4: cm=4; break;
	case ShaderProgram::CMATRIX: cm=16; break;
	default: cm=1;
	}

	cm*=sp.mult;
	switch (sp.type)
	{
	case ShaderProgram::CINT:
	{
		sp.data.resize((sizeof(int)*cm+sizeof(int)-1)/sizeof(float));
		int *m=(int *)(&(sp.data[0]));
		if (lua_istable(L,5))
		{
			for (int k=0;k<cm;k++)
			{
				lua_rawgeti(L, 5, k+1);
				m[k]=luaL_checkinteger(L,-1);
				lua_pop(L,1);
			}
		}
		else
		{
			for (int k=0;k<cm;k++)
				m[k]=luaL_checkinteger(L,5+k);
		}
		break;
	}
	case ShaderProgram::CFLOAT:
	case ShaderProgram::CFLOAT2:
	case ShaderProgram::CFLOAT3:
	case ShaderProgram::CFLOAT4:
	case ShaderProgram::CMATRIX:
	{
		sp.data.resize(cm);
		float *m=&(sp.data[0]);
		if (lua_istable(L,5))
		{
			for (int k=0;k<cm;k++)
			{
				lua_rawgeti(L, 5, k+1);
				m[k]=luaL_checknumber(L,-1);
				lua_pop(L,1);
			}
		}
		else
		{
			for (int k=0;k<cm;k++)
				m[k]=luaL_checknumber(L,5+k);
		}
		break;
	}
	case ShaderProgram::CTEXTURE:
		break;
	}

	sprite->setShaderConstant(sp);
	return 0;
}

int SpriteBinder::set(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::set", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

	const char* param = luaL_checkstring(L, 2);
	lua_Number value = luaL_checknumber(L, 3);

	GStatus status;
	sprite->set(param, value, &status);

	if (status.error() == true)
	{
		luaL_error(L, status.errorString());
		return 0;
	}

	return 0;
}

int SpriteBinder::get(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::get", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

	const char* param = luaL_checkstring(L, 2);

	GStatus status;
	float value = sprite->get(param, &status);

	if (status.error() == true)
	{
		luaL_error(L, status.errorString());
		return 0;
	}

	lua_pushnumber(L, value);

	return 1;
}
