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
		{"getChildrenAtPoint", SpriteBinder::getChildrenAtPoint},
		{"getParent", SpriteBinder::getParent},
		{"contains", SpriteBinder::contains},
		{"getChildIndex", SpriteBinder::getChildIndex},
		{"removeFromParent", SpriteBinder::removeFromParent},
		{"setClip", SpriteBinder::setClip},
        {"getClip", SpriteBinder::getClip},
        {"setLayoutParameters", SpriteBinder::setLayoutParameters},
        {"setLayoutConstraints", SpriteBinder::setLayoutConstraints},
        {"getLayoutParameters", SpriteBinder::getLayoutParameters},
        {"getLayoutConstraints", SpriteBinder::getLayoutConstraints},
        {"getLayoutInfo", SpriteBinder::getLayoutInfo},
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
		{"isOnStage", SpriteBinder::isOnStage},
		{"getColorTransform", SpriteBinder::getColorTransform},
		{"setColorTransform", SpriteBinder::setColorTransform},
		{"hitTestPoint", SpriteBinder::hitTestPoint},
		{"getWidth", SpriteBinder::getWidth},
		{"getHeight", SpriteBinder::getHeight},
		{"getSize", SpriteBinder::getSize},
		{"getMatrix", SpriteBinder::getMatrix},
		{"setMatrix", SpriteBinder::setMatrix},
		{"getAlpha", SpriteBinder::getAlpha},
		{"setAlpha", SpriteBinder::setAlpha},
		{"getBounds", SpriteBinder::getBounds},
        {"setBlendMode", SpriteBinder::setBlendMode},
        {"clearBlendMode", SpriteBinder::clearBlendMode},
		{"setShader", SpriteBinder::setShader},
		{"setShaderConstant", SpriteBinder::setShaderConstant},
		{"setStencilOperation", SpriteBinder::setStencilOperation},
        {"setStopEventPropagation",setStopEventPropagation},
        {"getDrawCount", getDrawCount},
		{"setEffectStack", SpriteBinder::setEffectStack},
		{"setEffectConstant", SpriteBinder::setEffectConstant},
		{"redrawEffects", SpriteBinder::redrawEffects},
        {"setHiddenChildren", SpriteBinder::setHiddenChildren},
		{"setCheckClip", SpriteBinder::setCheckClip},
        {"clone", SpriteBinder::clone},

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

	lua_pushinteger(L, ShaderEngine::STENCIL_ALWAYS);
	lua_setfield(L, -2, "STENCIL_ALWAYS");
	lua_pushinteger(L, ShaderEngine::STENCIL_DECR);
	lua_setfield(L, -2, "STENCIL_DECR");
	lua_pushinteger(L, ShaderEngine::STENCIL_DECR_WRAP);
	lua_setfield(L, -2, "STENCIL_DECR_WRAP");
	lua_pushinteger(L, ShaderEngine::STENCIL_INCR);
	lua_setfield(L, -2, "STENCIL_INCR");
	lua_pushinteger(L, ShaderEngine::STENCIL_INCR_WRAP);
	lua_setfield(L, -2, "STENCIL_INCR_WRAP");
	lua_pushinteger(L, ShaderEngine::STENCIL_DISABLE);
	lua_setfield(L, -2, "STENCIL_DISABLE");
	lua_pushinteger(L, ShaderEngine::STENCIL_EQUAL);
	lua_setfield(L, -2, "STENCIL_EQUAL");
	lua_pushinteger(L, ShaderEngine::STENCIL_GEQUAL);
	lua_setfield(L, -2, "STENCIL_GEQUAL");
	lua_pushinteger(L, ShaderEngine::STENCIL_INVERT);
	lua_setfield(L, -2, "STENCIL_INVERT");
	lua_pushinteger(L, ShaderEngine::STENCIL_KEEP);
	lua_setfield(L, -2, "STENCIL_KEEP");
	lua_pushinteger(L, ShaderEngine::STENCIL_LEQUAL);
	lua_setfield(L, -2, "STENCIL_LEQUAL");
	lua_pushinteger(L, ShaderEngine::STENCIL_LESS);
	lua_setfield(L, -2, "STENCIL_LESS");
	lua_pushinteger(L, ShaderEngine::STENCIL_NEVER);
	lua_setfield(L, -2, "STENCIL_NEVER");
	lua_pushinteger(L, ShaderEngine::STENCIL_NOTEQUAL);
	lua_setfield(L, -2, "STENCIL_NOTEQUAL");
	lua_pushinteger(L, ShaderEngine::STENCIL_REPLACE);
	lua_setfield(L, -2, "STENCIL_REPLACE");
	lua_pushinteger(L, ShaderEngine::STENCIL_ZERO);
	lua_setfield(L, -2, "STENCIL_ZERO");

	lua_pushinteger(L, ShaderEngine::CULL_NONE);
	lua_setfield(L, -2, "CULL_NONE");
	lua_pushinteger(L, ShaderEngine::CULL_FRONT);
	lua_setfield(L, -2, "CULL_FRONT");
	lua_pushinteger(L, ShaderEngine::CULL_BACK);
	lua_setfield(L, -2, "CULL_BACK");

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

#define FCONSTANT(name,v)	lua_pushinteger(L, v); lua_setfield(L, -2, "LAYOUT_FILL_"#name);
#define ACONSTANT(name)	lua_pushinteger(L, GridBagConstraints::name); lua_setfield(L, -2, "LAYOUT_ANCHOR_"#name);
    FCONSTANT(NONE,0);
    FCONSTANT(BOTH,1);
    FCONSTANT(HORIZONTAL,2);
    FCONSTANT(VERTICAL,3);
    ACONSTANT(NORTHWEST);
    ACONSTANT(NORTH);
    ACONSTANT(NORTHEAST);
    ACONSTANT(WEST);
    ACONSTANT(CENTER);
    ACONSTANT(EAST);
    ACONSTANT(SOUTHWEST);
    ACONSTANT(SOUTH);
    ACONSTANT(SOUTHEAST);
#undef FCONSTANT
#undef ACONSTANT

	lua_pushinteger(L, SPRITE_EVENTMASK_MOUSE);
	lua_setfield(L, -2, "EVENTMASK_MOUSE");
	lua_pushinteger(L, SPRITE_EVENTMASK_TOUCH);
	lua_setfield(L, -2, "EVENTMASK_TOUCH");
	lua_pushinteger(L, SPRITE_EVENTMASK_KEY);
	lua_setfield(L, -2, "EVENTMASK_KEY");

	lua_pushinteger(L, 0);
	lua_setfield(L, -2, "LAYOUT_INFO_CURRENT");
	lua_pushinteger(L, 1);
	lua_setfield(L, -2, "LAYOUT_INFO_MINIMAL");
	lua_pushinteger(L, 2);
	lua_setfield(L, -2, "LAYOUT_INFO_PREFERRED");
	lua_pushinteger(L, 3);
	lua_setfield(L, -2, "LAYOUT_INFO_BEST");

	lua_pushinteger(L, Sprite::CONTINUOUS);
	lua_setfield(L, -2, "EFFECT_MODE_CONTINUOUS");
	lua_pushinteger(L, Sprite::AUTOMATIC);
	lua_setfield(L, -2, "EFFECT_MODE_AUTOMATIC");
	lua_pushinteger(L, Sprite::TRIGGERED);
	lua_setfield(L, -2, "EFFECT_MODE_TRIGGERED");

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

static void fixupClone(lua_State *L,Sprite *o,Sprite *c,int oidx,int cidx,int fidx) {
    Binder binder(L);
    lua_checkstack(L,8);
    lua_pushvalue(L,oidx);
    lua_pushvalue(L,cidx-1);
    lua_rawset(L,fidx-2);

    lua_getfield(L, oidx, "__userdata");
    lua_getfield(L, cidx-1, "__userdata");
    if (lua_getmetatable(L,-2))
        lua_setmetatable(L,-2);
    lua_pop(L,2);
    if (lua_getmetatable(L,oidx))
        lua_setmetatable(L,cidx-1);

    int cc=c->childCount();
    if (cc>0) {
        lua_getfield(L,oidx,"__children");
        lua_createtable(L,0,cc);
        for(int k=0;k<cc;k++) {
            Sprite *sl=o->child(k);
            Sprite *cl=c->child(k);
            lua_pushlightuserdata(L, sl);
            lua_rawget(L,-3);
            cl->ref();
            binder.pushInstance("Sprite", cl); //OCL,CCL,OCO,CCO
            fixupClone(L,sl,cl,-2,-1,fidx-4);
            lua_pushvalue(L,cidx-4);
            lua_setfield(L, -2, "__parent");
            lua_pushlightuserdata(L, cl);
            lua_pushvalue(L,-2); //OCL,CCL,OCO,CCO,CLK,CCO
            lua_rawset(L,-5); //OCL,CCL,OCO,CCO
            lua_rawset(L,fidx-4); //OCL,CCL [fidx[oco]=cco]
        }
        lua_setfield(L, cidx-2, "__children"); //OCL
        lua_pop(L,1);
    }

    //Last step, clone/map original data to cloned table
    lua_getfield(L, cidx, "__children"); //-1:Save __children
    lua_getfield(L, cidx-1, "__userdata"); //-2:Save __userdata
    lua_pushnil(L); //-3: Pairs key
    while (lua_next(L,oidx-3)) { //-4: K,V
        lua_pushvalue(L,-2); //-5:K,V,K
        lua_insert(L,-2); //-5:K,K,V
        lua_pushvalue(L,-1); //-6:K,K,V,V
        lua_rawget(L,fidx-6); //-6:K,K,V,M(V)
        if (lua_isnil(L,-1))
            lua_pop(L,1); //-5:K,K,V
        else
            lua_remove(L,-2); //-5:K,K,M(V)
        lua_rawset(L,cidx-5); //-3:K
    }
    lua_setfield(L, cidx-2, "__userdata"); //-1:Restore __userdata
    lua_setfield(L, cidx-1, "__children"); //0:Restore __children
    lua_getfield(L,cidx,"newClone");
    if (!lua_isfunction(L,-1))
        lua_pop(L,1);
    else {
        lua_pushvalue(L,cidx-1);
        lua_call(L,1,0);
    }

}

int SpriteBinder::clone(lua_State *L)
{
    StackChecker checker(L, "SpriteBinder::clone", 1);
    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
    Sprite* clone = sprite->clone();

    binder.pushInstance("Sprite", clone);
    lua_newtable(L);
    lua_pushvalue(L,1);
    fixupClone(L,sprite, clone,-1,-3,-2);
    lua_pop(L,2);

    return 1;
}

int SpriteBinder::destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
	Sprite* sprite = static_cast<Sprite*>(ptr);
	sprite->unref();

	return 0;
}

int SpriteBinder::setStopEventPropagation(lua_State *L) {
	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	int mask=lua_tointeger(L,2);
	if (mask==0) //if zero, it may have been something not convertible, check as boolean
		mask=lua_toboolean(L,2)?-1:0;
	sprite->setStopPropagationMask(mask);
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
    StackChecker checker(L, "SpriteBinder::addChild", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	Sprite* child = static_cast<Sprite*>(binder.getInstance("Sprite", 2));

	GStatus status;
	if (sprite->canChildBeAdded(child, &status) == false)
        luaL_error(L, "%s", status.errorString());

	if (child->parent() == sprite)
	{
        lua_pushinteger(L,1+sprite->addChild(child));
        return 1;
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

    lua_pushinteger(L,1+sprite->addChild(child));

    return 1;
}


int SpriteBinder::addChildAt(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::addChildAt", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	Sprite* child = static_cast<Sprite*>(binder.getInstance("Sprite", 2));
	int index = luaL_checkinteger(L, 3);

	GStatus status;
    if (sprite->canChildBeAddedAt(child, index - 1, &status) == false)
        luaL_error(L, "%s", status.errorString());

	if (child->parent() == sprite)
	{
        lua_pushinteger(L,1+sprite->addChildAt(child, index - 1));
        return 1;
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

    lua_pushinteger(L,1+sprite->addChildAt(child, index - 1));

    return 1;
}


int SpriteBinder::removeChildAt(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::removeChildAt", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	int index = luaL_checknumber(L, 2);
	if (index < 0) index = sprite->childCount() + index + 1;
	if (index < 1 || index > sprite->childCount())
        luaL_error(L, "%s", GStatus(2006).errorString());	// Error #2006: The supplied index is out of bounds.

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
        luaL_error(L, "%s", status.errorString());

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
        luaL_error(L, "%s", GStatus(2006).errorString());	// Error #2006: The supplied index1 is out of bounds.
        int index2 = luaL_checknumber(L, 3);
        if (index2 < 0) index2 = sprite->childCount() + index2 + 1;
	if (index2 < 1 || index2 > sprite->childCount())
        luaL_error(L, "%s", GStatus(2006).errorString());	// Error #2006: The supplied index2 is out of bounds.

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
        luaL_error(L, "%s", GStatus(2006).errorString());	// Error #2006: The supplied index is out of bounds.

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

#define FILL_NUM_ARRAY(n,f) \
		lua_getfield(L,2,n); if (!lua_isnoneornil(L,-1)) { \
			luaL_checktype(L,-1, LUA_TTABLE); \
			p->f.resize(lua_objlen(L,-1)); \
			for (size_t k=1;k<=lua_objlen(L,-1);k++) { lua_rawgeti(L,-1,k); p->f[k-1]=luaL_checknumber(L,-1); lua_pop(L,1); } \
		} lua_pop(L,1);
#define FILL_INT(n,f) lua_getfield(L,2,n); if (!lua_isnoneornil(L,-1)) p->f=luaL_checkinteger(L,-1); lua_pop(L,1);
#define FILL_INTT(n,f,t) lua_getfield(L,2,n); if (!lua_isnoneornil(L,-1)) p->f=(t) luaL_checkinteger(L,-1); lua_pop(L,1);
#define FILL_NUM(n,f) lua_getfield(L,2,n); if (!lua_isnoneornil(L,-1)) p->f=luaL_checknumber(L,-1); lua_pop(L,1);
#define FILL_BOOL(n,f) lua_getfield(L,2,n); p->f=lua_toboolean(L,-1); lua_pop(L,1);
#define STOR_NUM_ARRAY(n,f) \
		lua_newtable(L); \
		for (size_t k=0;k<p->f.size();k++) { lua_pushnumber(L,p->f[k]); lua_rawseti(L,-2,k+1); }\
		lua_setfield(L,-2,n);
#define STOR_INT(n,f) lua_pushinteger(L,p->f); lua_setfield(L,-2,n);
#define STOR_INTT(n,f,t) lua_pushinteger(L,(int)p->f); lua_setfield(L,-2,n);
#define STOR_NUM(n,f) lua_pushnumber(L,p->f); lua_setfield(L,-2,n);
#define STOR_BOOL(n,f) lua_pushboolean(L,p->f); lua_setfield(L,-2,n);

int SpriteBinder::setLayoutParameters(lua_State *L)
{
    StackChecker checker(L, "SpriteBinder::setLayoutParameters", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));
	if (lua_isnoneornil(L,2))
		sprite->clearLayoutState();
	else {
		luaL_checktype(L, 2, LUA_TTABLE);
		GridBagLayout *p=sprite->getLayoutState();
		FILL_NUM_ARRAY("columnWidths",columnWidths);
		FILL_NUM_ARRAY("rowHeights",rowHeights);
		FILL_NUM_ARRAY("columnWeights",columnWeights);
		FILL_NUM_ARRAY("rowWeights",rowWeights);

		lua_getfield(L,2,"insets");
		if (!lua_isnoneornil(L,-1))
			p->pInsets.left=p->pInsets.right=p->pInsets.top=p->pInsets.bottom=luaL_checknumber(L,-1);
		lua_pop(L,1);
        FILL_NUM("insetTop",pInsets.top); FILL_NUM("insetLeft",pInsets.left);
        FILL_NUM("insetBottom",pInsets.bottom); FILL_NUM("insetRight",pInsets.right);

        FILL_BOOL("equalizeCells",equalizeCells);
        FILL_BOOL("resizeContainer",resizeContainer);
        FILL_NUM("cellSpacingX",cellSpacingX); FILL_NUM("cellSpacingY",cellSpacingY);
        FILL_NUM("gridAnchorX",gridAnchorX); FILL_NUM("gridAnchorY",gridAnchorY);
        FILL_NUM("zOffset",zOffset);
        p->dirty=true;
        p->layoutInfoCache[0].valid=false;
        p->layoutInfoCache[1].valid=false;
        sprite->invalidate(Sprite::INV_CONSTRAINTS);
    }
	return 0;
}

int SpriteBinder::setLayoutConstraints(lua_State *L)
{
    StackChecker checker(L, "SpriteBinder::setLayoutConstraints", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));
	if (lua_isnoneornil(L,2))
		sprite->clearLayoutConstraints();
	else {
		luaL_checktype(L, 2, LUA_TTABLE);
		GridBagConstraints *p=sprite->getLayoutConstraints();

		FILL_INT("gridx",gridx); FILL_INT("gridy",gridy);
		FILL_INT("gridwidth",gridwidth); FILL_INT("gridheight",gridheight);
		FILL_NUM("weightx",weightx); FILL_NUM("weighty",weighty);
        FILL_INTT("anchor",anchor,GridBagConstraints::_Anchor);
        lua_getfield(L,2,"fill");
        if (!lua_isnoneornil(L,-1)) {
            int fill=luaL_checkinteger(L,-1);
            p->fillX=(fill==1)||(fill==2)?1:0;
            p->fillY=(fill==1)||(fill==3)?1:0;
        }
        lua_pop(L,1);
        FILL_NUM("fillx",fillX); FILL_NUM("filly",fillY);
        FILL_NUM("aspectRatio",aspectRatio);
        FILL_NUM("anchorx",anchorX); FILL_NUM("anchory",anchorY);
        FILL_NUM("offsetx",offsetX); FILL_NUM("offsety",offsetY);
        FILL_NUM("originx",originX); FILL_NUM("originy",originY);
        FILL_NUM("ipadx",ipadx); FILL_NUM("ipady",ipady);
        lua_getfield(L,2,"width");
        if (!lua_isnoneornil(L,-1)) {
            float width=luaL_checknumber(L,-1);
            p->aminWidth=width; p->prefWidth=width;
        }
        lua_pop(L,1);
        lua_getfield(L,2,"height");
        if (!lua_isnoneornil(L,-1)) {
            float height=luaL_checknumber(L,-1);
            p->aminHeight=height; p->prefHeight=height;
        }
        lua_pop(L,1);
        FILL_NUM("minWidth",aminWidth); FILL_NUM("minHeight",aminHeight);
		FILL_NUM("prefWidth",prefWidth); FILL_NUM("prefHeight",prefHeight);
        FILL_BOOL("shrink",optimizeSize);
        FILL_BOOL("group",group);

		lua_getfield(L,2,"insets");
		if (!lua_isnoneornil(L,-1))
			p->insets.left=p->insets.right=p->insets.top=p->insets.bottom=luaL_checknumber(L,-1);
		lua_pop(L,1);
		FILL_NUM("insetTop",insets.top); FILL_NUM("insetLeft",insets.left);
		FILL_NUM("insetBottom",insets.bottom); FILL_NUM("insetRight",insets.right);
        sprite->invalidate(Sprite::INV_CONSTRAINTS);
	}
	return 0;
}

int SpriteBinder::getLayoutParameters(lua_State *L)
{
    StackChecker checker(L, "SpriteBinder::getLayoutParameters", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));
	if (sprite->hasLayoutState())
	{
		GridBagLayout *p=sprite->getLayoutState();
		lua_newtable(L);
		STOR_NUM_ARRAY("columnWidths",columnWidths);
		STOR_NUM_ARRAY("rowHeights",rowHeights);
		STOR_NUM_ARRAY("columnWeights",columnWeights);
		STOR_NUM_ARRAY("rowWeights",rowWeights);
        STOR_NUM("insetTop",pInsets.top); STOR_NUM("insetLeft",pInsets.left);
        STOR_NUM("insetBottom",pInsets.bottom); STOR_NUM("insetRight",pInsets.right);
        STOR_BOOL("equalizeCells",equalizeCells);
        STOR_BOOL("resizeContainer",resizeContainer);
        STOR_NUM("cellSpacingX",cellSpacingX); STOR_NUM("cellSpacingY",cellSpacingY);
        STOR_NUM("gridAnchorX",gridAnchorX); STOR_NUM("gridAnchorY",gridAnchorY);
        STOR_NUM("zOffset",zOffset);
	}
	else
		lua_pushnil(L);
	return 1;
}

int SpriteBinder::getLayoutConstraints(lua_State *L)
{
    StackChecker checker(L, "SpriteBinder::getLayoutConstraints", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));
	if (sprite->hasLayoutConstraints())
	{
		GridBagConstraints *p=sprite->getLayoutConstraints();
		lua_newtable(L);
		STOR_INT("gridx",gridx); STOR_INT("gridy",gridy);
		STOR_INT("gridwidth",gridwidth); STOR_INT("gridheight",gridheight);
		STOR_NUM("weightx",weightx); STOR_NUM("weighty",weighty);
        STOR_INTT("anchor",anchor,GridBagConstraints::_Anchor);
        STOR_NUM("fillx",fillX); STOR_NUM("filly",fillY);
        STOR_NUM("aspectRatio",aspectRatio);
        STOR_NUM("anchorx",anchorX); STOR_NUM("anchory",anchorY);
        STOR_NUM("ipadx",ipadx); STOR_NUM("ipady",ipady);
		STOR_NUM("minWidth",aminWidth); STOR_NUM("minHeight",aminHeight);
		STOR_NUM("prefWidth",prefWidth); STOR_NUM("prefHeight",prefHeight);
		STOR_NUM("insetTop",insets.top); STOR_NUM("insetLeft",insets.left);
		STOR_NUM("insetBottom",insets.bottom); STOR_NUM("insetRight",insets.right);
        STOR_NUM("offsetx",offsetX); STOR_NUM("offsety",offsetY);
        STOR_NUM("originx",originX); STOR_NUM("originy",originY);
		STOR_BOOL("shrink",optimizeSize);
	}
	else
		lua_pushnil(L);
	return 1;
}

int SpriteBinder::getLayoutInfo(lua_State *L)
{
    StackChecker checker(L, "SpriteBinder::getLayoutInfo", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));
	float epw=luaL_optnumber(L,2,-1);
	float eph=luaL_optnumber(L,3,-1);
	int type=luaL_optinteger(L,4,0);

	if (sprite->hasLayoutState())
	{
		GridBagLayout *sp=sprite->getLayoutState();
		GridBagLayoutInfo pi;
		GridBagLayoutInfo *p=&pi;
		if (type==0) { //Current
			int loops=100; //Detect endless loops while forcing immediate layout
			while(sp->dirty&&(loops--))
			{
				sp->dirty=false;
				float pwidth,pheight;
				sprite->getDimensions(pwidth, pheight);
				if (epw>=0) pwidth=epw;
				if (eph>=0) pheight=eph;
				sp->ArrangeGrid(sprite,pwidth,pheight);
			}
			if (loops==0) //Gave up, mark as clean to prevent going through endless loop again
				sp->dirty=false;
			p=sp->getCurrentLayoutInfo();
		} else
		{
			float dw,dh;
			pi = sp->getLayoutInfo(sprite, (type>=2)?2:type); //PREFERRED or BEST
		    sp->getMinSize(sprite, pi, dw, dh, sp->pInsets);
		    if (type==3) {//BEST
				float pwidth,pheight;
				sprite->getDimensions(pwidth, pheight);
				if (pwidth < dw || pheight < dh) {
					pi = sp->getLayoutInfo(sprite, 1);
					sp->getMinSize(sprite, pi, dw, dh, sp->pInsets);
				}
		    }
		    pi.reqWidth=dw;
		    pi.reqHeight=dh;
		}
		lua_newtable(L);

		STOR_INT("width",width); STOR_INT("height",height);
		STOR_NUM("reqWidth",reqWidth); STOR_NUM("reqHeight",reqHeight);
		STOR_NUM("startx",startx); STOR_NUM("starty",starty);
		STOR_NUM_ARRAY("minWidth",minWidth);
		STOR_NUM_ARRAY("minHeight",minHeight);
		STOR_NUM_ARRAY("weightX",weightX);
		STOR_NUM_ARRAY("weightY",weightY);
	}
	else
		lua_pushnil(L);
	return 1;
}

#undef FILL_INT
#undef FILL_NUM
#undef FILL_NUM_ARRAY
#undef FILL_INTT
#undef STOR_INT
#undef STOR_NUM
#undef STOR_NUM_ARRAY
#undef STOR_INTT

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
    float width = x1 < x2 ? x2 - x1 : 0;
    float height = y1 < y2 ? y2 - y1 : 0;

    sprite->setRefXY(x * width, y * height);

    return 0;
}

int SpriteBinder::getAnchorPoint(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::getAnchorPoint", 2);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

    float x1, y1, x2, y2;
    sprite->objectBounds(&x1, &y1, &x2, &y2);

    float width = x1 < x2 ? x2 - x1 : 1;
    float height = y1 < y2 ? y2 - y1 : 1;

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
		luaL_error(L, "%s", status.errorString());
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
	
	bool recursiveCheck = lua_toboolean(L, 2);
	
	if (recursiveCheck)
	{
		Sprite* parent = sprite->parent();
		while (parent)
		{
			if (!parent->visible()) 
			{
				lua_pushboolean(L, false);
				return 1;
			}
			parent = parent->parent();
		}
	}
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

int SpriteBinder::isOnStage(lua_State* L)
{
	StackChecker checker(L, "isOnStage", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	lua_pushboolean(L,sprite->getStage()!=NULL);

	return 1;
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

int SpriteBinder::getChildrenAtPoint(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getChildrenAtPoint", 1);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	bool visible = lua_toboolean(L, 4);
	bool nosubs = lua_toboolean(L, 5);

    std::vector<std::pair<int,Sprite *>>res;
	sprite->getChildrenAtPoint(x, y, visible, nosubs, res);
	int nres=res.size();
	lua_createtable(L,nres,0);
	for (int i=0;i<nres;i++) {
        int pidx=res[i].first;
        if (pidx==0)
            lua_getfield(L, 1, "__children");	// push sprite.__children
        else {
            lua_rawgeti(L,-1,pidx);
            lua_getfield(L, -1, "__children");	// push sprite.__children
            lua_remove(L, -2);					// pop sprite.__children
        }
        lua_pushlightuserdata(L, res[i].second);
		lua_rawget(L, -2);					// push sprite.__children[child]
		lua_remove(L, -2);					// pop sprite.__children
		lua_rawseti(L,-2,i+1);
	}

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

/*
	simply return the width and height of the sprite
	DO NOT add extra parameter check to return the non-transformed values.
	turn to getBounds if you need that.
*/
int SpriteBinder::getSize(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getSize", 2);
	
	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

    float minx, miny, maxx, maxy;
	
	sprite->localBounds(&minx, &miny, &maxx, &maxy);
		
	if (minx > maxx || miny > maxy)
	{
		// empty bounds
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
	}
	else
	{
		lua_pushnumber(L, maxx - minx);
		lua_pushnumber(L, maxy - miny);
	}
      
	return 2;
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
        else luaL_error(L, "%s", "Parameter 'blendMode' must be one of the accepted values.");
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
	sprite->setShader(shader,(ShaderEngine::StandardProgram)luaL_optinteger(L,3,0),luaL_optinteger(L,4,0),lua_toboolean(L,5));

	return 0;
}

void parseShaderParam(lua_State* L,int idx,Sprite::ShaderParam &sp,int &shtype,int &shvar) {
	sp.name=luaL_checkstring(L,idx);
	sp.type = (ShaderProgram::ConstantType) luaL_checkinteger(L, idx+1);
	sp.mult = luaL_checknumber(L, idx+2);
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
	int li=lua_istable(L,idx+3)?idx+4:(idx+3+cm);
	shtype=luaL_optinteger(L,li,0);
	shvar=luaL_optinteger(L,li+1,0);
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
				lua_rawgeti(L, idx+3, k+1);
				m[k]=luaL_checkinteger(L,-1);
				lua_pop(L,1);
			}
		}
		else
		{
			for (int k=0;k<cm;k++)
				m[k]=luaL_checkinteger(L,idx+3+k);
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
		if (lua_istable(L,idx+3))
		{
			for (int k=0;k<cm;k++)
			{
				lua_rawgeti(L, idx+3, k+1);
				m[k]=luaL_checknumber(L,-1);
				lua_pop(L,1);
			}
		}
		else
		{
			for (int k=0;k<cm;k++)
				m[k]=luaL_checknumber(L,idx+3+k);
		}
		break;
	}
	case ShaderProgram::CTEXTURE:
		break;
	}
}

int SpriteBinder::setShaderConstant(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setShaderConstant", 0);

	Binder binder(L);

	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

   // virtual void setConstant(int index,ConstantType type,const void *ptr);

	Sprite::ShaderParam sp;
	int shtype,shvar;
	parseShaderParam(L,2,sp,shtype,shvar);

	sprite->setShaderConstant(sp,(ShaderEngine::StandardProgram)shtype,shvar);
	return 0;
}

int SpriteBinder::setEffectStack(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setEffectStack", 0);

	Binder binder(L);
	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	std::vector<Sprite::Effect> effects;
	Sprite::EffectUpdateMode mode=Sprite::CONTINUOUS;
	if (!lua_isnoneornil(L,2)) {
		luaL_checktype(L,2,LUA_TTABLE);
		int ec=lua_objlen(L,2);
		mode=(Sprite::EffectUpdateMode) luaL_optinteger(L,3,0);
		for (int en=0;en<ec;en++)
		{
			Sprite::Effect e;
			e.clearBuffer=(en==0); //Clear buffer by default for first pass, don't clear for subsequent passes
            e.autoBuffer=false;

			lua_rawgeti(L,2,en+1);
			luaL_checktype(L,-1,LUA_TTABLE);

			lua_getfield(L,-1,"buffer");
			e.buffer=static_cast<GRenderTarget*>(binder.getInstance("RenderTarget", -1));
			lua_getfield(L,-2,"shader");
			if (!lua_isnoneornil(L,-1))
				e.shader=static_cast<ShaderProgram*>(binder.getInstance("Shader", -1));
			lua_getfield(L,-3,"transform");
			if (!lua_isnoneornil(L,-1)) {
				Transform *t=static_cast<Transform*>(binder.getInstance("Matrix", -1));
				e.transform=t->matrix();
			}
			lua_getfield(L,-4,"postTransform");
			if (!lua_isnoneornil(L,-1)) {
				Transform *t=static_cast<Transform*>(binder.getInstance("Matrix", -1));
				e.postTransform=t->matrix();
			}
			lua_getfield(L,-5,"textures");
			if (!lua_isnoneornil(L,-1)) {
				luaL_checktype(L,-1,LUA_TTABLE);
				int tc=lua_objlen(L,-1);
				for (int tn=0;tn<tc;tn++)
				{
					lua_rawgeti(L,-1,tn+1);
					TextureBase *tex=static_cast<TextureBase*>(binder.getInstance("TextureBase", -1));
					lua_pop(L,1);
					e.textures.push_back(tex);
				}
			}
			lua_getfield(L,-6,"clear");
			if (!lua_isnoneornil(L,-1))
				e.clearBuffer=lua_toboolean(L,-1);

            lua_getfield(L,-7,"autoBuffer");
            if (!lua_isnoneornil(L,-1))
                e.autoBuffer=lua_toboolean(L,-1);

            lua_getfield(L,-8,"autoTransform");
            if (!lua_isnoneornil(L,-1)) {
                Transform *t=static_cast<Transform*>(binder.getInstance("Matrix", -1));
                e.autoTransform=t->matrix();
            }

            lua_pop(L,9);
			effects.push_back(e);
		}
	}
	sprite->setEffectStack(effects,mode);

	return 0;
}

int SpriteBinder::setEffectConstant(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setEffectConstant", 1);

	Binder binder(L);

	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));

    int num=luaL_checkinteger(L,2);
    if (num<1) {
    	lua_pushstring(L,"Effect index must be positive");
    	lua_error(L);
    }

	Sprite::ShaderParam sp;
	int shtype,shvar;
	parseShaderParam(L,3,sp,shtype,shvar);

	lua_pushboolean(L,sprite->setEffectShaderConstant(num-1,sp));
	return 1;
}

int SpriteBinder::redrawEffects(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::redrawEffects", 0);

	Binder binder(L);

	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	sprite->redrawEffects();
	return 0;
}

int SpriteBinder::setStencilOperation(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setStencilOperation", 0);

	Binder binder(L);

	Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", 1));
	ShaderEngine::DepthStencil ds;

	if (lua_isnoneornil(L,2))
		ds.dTest=false;
	else if (lua_istable(L,2))
	{
		luaL_checktype(L,2,LUA_TTABLE);
		lua_getfield(L,2,"stencilClear");
		if (!lua_isnil(L,-1))
			ds.sClear=lua_toboolean(L,-1);
		lua_pop(L,1);
		lua_getfield(L,2,"stencilMask");
		if (!lua_isnil(L,-1))
			ds.sMask=luaL_checkinteger(L,-1);
		lua_pop(L,1);
		lua_getfield(L,2,"stencilWriteMask");
		if (!lua_isnil(L,-1))
			ds.sWMask=luaL_checkinteger(L,-1);
		lua_pop(L,1);
		lua_getfield(L,2,"stencilRef");
		if (!lua_isnil(L,-1))
			ds.sRef=luaL_checkinteger(L,-1);
		lua_pop(L,1);
		lua_getfield(L,2,"stencilFunc");
		if (!lua_isnil(L,-1))
			ds.sFunc=(ShaderEngine::StencilFunc) luaL_checkinteger(L,-1);
		lua_pop(L,1);
		lua_getfield(L,2,"depthPass");
		if (!lua_isnil(L,-1))
			ds.dPass=(ShaderEngine::StencilOp) luaL_checkinteger(L,-1);
		lua_pop(L,1);
		lua_getfield(L,2,"stencilFail");
		if (!lua_isnil(L,-1))
			ds.sFail=(ShaderEngine::StencilOp) luaL_checkinteger(L,-1);
		lua_pop(L,1);
		lua_getfield(L,2,"depthFail");
		if (!lua_isnil(L,-1))
			ds.dFail=(ShaderEngine::StencilOp) luaL_checkinteger(L,-1);
		lua_pop(L,1);
		lua_getfield(L,2,"cullMode");
		if (!lua_isnil(L,-1))
			ds.cullMode=((ShaderEngine::CullMode)(luaL_checkinteger(L,-1)&3));
		lua_pop(L,1);
		ds.dTest=true;
	}

	sprite->setStencilOperation(ds);

	return 0;
}

int SpriteBinder::getDrawCount(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::getDrawCount", 1);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

    lua_pushinteger(L, sprite->drawCount());

    return 1;
}

int SpriteBinder::setCheckClip(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setCheckClip", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));
    sprite->setCheckClip(lua_toboolean(L,2));
    return 0;
}

int SpriteBinder::setHiddenChildren(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setHiddenChildren", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite"));

    std::vector<char> hset;
    if (lua_type(L,2)==LUA_TTABLE) {
        size_t tl=lua_objlen(L,2);
        for (size_t n=1;n<=tl;n++) {
            lua_rawgeti(L,2,n);
            if (lua_type(L,-1)==LUA_TTABLE) {
                lua_rawgeti(L,-1,1);
                lua_rawgeti(L,-2,2);
                int hf=luaL_optinteger(L,-2,0);
                int hl=luaL_optinteger(L,-1,hf);
                lua_pop(L,2);
                if (hl>0) {
                    if (hset.size()<(size_t)hl)
                        hset.resize(hl);
                    for (int k=hf;k<=hl;k++)
                        hset[k-1]=true;
                }
            }
            else {
                int hf=luaL_checkinteger(L,-1);
                if (hf>0) {
                    if (hset.size()<(size_t)hf)
                        hset.resize(hf);
                    hset[hf-1]=true;
                }
            }
            lua_pop(L,1);
        }
    }
    else {
        int hf=luaL_optinteger(L,2,0);
        int hl=luaL_optinteger(L,3,hf);
        if (hl>0) {
            hset.resize(hl);
            for (int k=hf;k<=hl;k++)
                hset[k-1]=true;
        }
    }
    sprite->setSkipSet(hset);

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
		luaL_error(L, "%s", status.errorString());
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
		luaL_error(L, "%s", status.errorString());
		return 0;
	}

	lua_pushnumber(L, value);

	return 1;
}
