#include "spritebinder.h"
#include "sprite.h"
#include "matrix.h"
#include "colortransform.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>


int SpriteBinder::layoutStrings[64];
size_t SpriteBinder::tokenChildren;
size_t SpriteBinder::tokenParent;
size_t SpriteBinder::tokenUserdata;
size_t SpriteBinder::tokenNewclone;

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
        {"spriteToLocal", SpriteBinder::spriteToLocal},
        {"spriteToLocalMatrix", SpriteBinder::spriteToLocalMatrix},
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
        {"setWorldAlign", SpriteBinder::setWorldAlign},
        {"setStyle", SpriteBinder::setStyle},
        {"resolveStyle", SpriteBinder::resolveStyle},
        {"updateStyle", SpriteBinder::updateStyle},
        {"setGhosts", SpriteBinder::setGhosts},
        {"lookAt", SpriteBinder::lookAt},
        {"setAutoSort", SpriteBinder::setAutoSort},
        {"__parseGhosts", SpriteBinder::__parseGhosts},

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
	lua_pushinteger(L, ShaderEngine::STENCIL_GREATER);
	lua_setfield(L, -2, "STENCIL_GREATER");
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
    for (int k=0;k<64;k++)
    	layoutStrings[k]=-1;
    tokenChildren=lua_newtoken(L,"__children");
    tokenParent=lua_newtoken(L,"__parent");
    tokenNewclone=lua_newtoken(L,"newClone");
    tokenUserdata=lua_newtoken(L,"__userdata");
}

int SpriteBinder::create(lua_State* L)
{
	StackChecker checker(L, "Sprite", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	Binder binder(L);
    Sprite* sprite = new Sprite(application->getApplication());
	binder.pushInstance("Sprite", sprite);

	return 1;
}

static void fixupClone(lua_State *L,Sprite *o,Sprite *c,int oidx,int cidx,int fidx) {
    Binder binder(L);
    lua_checkstack(L,8);
    lua_pushvalue(L,oidx);
    lua_pushvalue(L,cidx-1);
    lua_rawset(L,fidx-2);

    lua_rawgettoken(L, oidx, SpriteBinder::tokenUserdata);
    lua_rawgettoken(L, cidx-1, SpriteBinder::tokenUserdata);
    if (lua_getmetatable(L,-2))
        lua_setmetatable(L,-2);
    lua_pop(L,2);
    if (lua_getmetatable(L,oidx))
        lua_setmetatable(L,cidx-1);

    int cc=c->childCount();
    if (cc>0) {
        lua_rawgettoken(L,oidx,SpriteBinder::tokenChildren); //OCL
        lua_createtable(L,0,cc); //OCL,CCL
        for(int k=0;k<cc;k++) {
            Sprite *sl=o->child(k);
            Sprite *cl=c->child(k);
            lua_pushlightuserdata(L, sl); //OCL,CCL,OUD
            lua_rawget(L,-3); //OCL,CCL,OCO
            cl->ref();
            lua_clonetable(L,-1); //OCL, CCL, OCO, CCO
            binder.makeInstance(NULL, cl); //OCL,CCL,OCO,CCO
            fixupClone(L,sl,cl,-2,-1,fidx-4);
            lua_pushvalue(L,cidx-4);
            lua_rawsettoken(L, -2, SpriteBinder::tokenParent);
            lua_pushlightuserdata(L, cl);
            lua_pushvalue(L,-2); //OCL,CCL,OCO,CCO,CLK,CCO
            lua_rawset(L,-5); //OCL,CCL,OCO,CCO
            lua_rawset(L,fidx-4); //OCL,CCL [fidx[oco]=cco]
        }
        lua_rawsettoken(L, cidx-2, SpriteBinder::tokenChildren); //OCL
        lua_pop(L,1);
    }

    //Last step, clone/map original data to cloned table
    lua_remaptable(L,cidx,fidx);
    /*
    lua_rawgettoken(L, cidx, SpriteBinder::tokenChildren); //-1:Save __children
    lua_rawgetfield(L, cidx-1, "__userdata"); //-2:Save __userdata
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
    lua_rawsetfield(L, cidx-2, "__userdata"); //-1:Restore __userdata
    lua_rawsettoken(L, cidx-1, SpriteBinder::tokenChildren); //0:Restore __children
    */
    lua_pushnil(L);
    lua_rawsettoken(L, cidx-1, SpriteBinder::tokenParent);

    if (lua_gettoken(L,cidx,SpriteBinder::tokenNewclone)!=LUA_TFUNCTION)
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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Sprite* clone = sprite->clone();

    lua_clonetable(L,1);
    binder.makeInstance(NULL, clone); //OCL,CCL,OCO,CCO //TOSEE: Sprite ?
    //binder.pushInstance("Sprite", clone);
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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    int mask=lua_tointeger(L,2);
	if (mask==0) //if zero, it may have been something not convertible, check as boolean
		mask=lua_toboolean(L,2)?-1:0;
	sprite->setStopPropagationMask(mask);
	return 0;
}

static void createChildrenTable(lua_State* L)
{
	StackChecker checker(L, "createChildrenTable");
	
    lua_rawgettoken(L, 1, SpriteBinder::tokenChildren);
	if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);

		lua_newtable(L);
        lua_rawsettoken(L, 1, SpriteBinder::tokenChildren);
	}
	else
		lua_pop(L, 1);
}

int SpriteBinder::addChild(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::addChild", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Sprite* child = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 2));

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
        lua_rawgettoken(L, 2, tokenParent);			// push child.__parent
        lua_rawgettoken(L, -1, tokenChildren);		// push child.__parent.__children

		lua_pushlightuserdata(L, child);
		lua_pushnil(L);
		lua_rawset(L, -3);						// child.__parent.__children[child] = nil

		lua_pop(L, 2);							// pop child.__parent and child.__parent.__children
	}

	// set new parent
	lua_pushvalue(L, 1);
    lua_rawsettoken(L, 2, tokenParent);				// child.__parent = sprite

	createChildrenTable(L);

	// add to __children table
    lua_rawgettoken(L, 1, tokenChildren);			// push sprite.__children
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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Sprite* child = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 2));
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
        lua_rawgettoken(L, 2, tokenParent);			// push child.__parent
        lua_rawgettoken(L, -1, tokenChildren);		// push child.__parent.__children

		lua_pushlightuserdata(L, child);
		lua_pushnil(L);
		lua_rawset(L, -3);						// child.__parent.__children[child] = nil

		lua_pop(L, 2);							// pop child.__parent and child.__parent.__children
	}

	// set new parent
	lua_pushvalue(L, 1);
    lua_rawsettoken(L, 2, tokenParent);				// child.__parent = sprite

	createChildrenTable(L);

	// add to __children table
    lua_rawgettoken(L, 1, tokenChildren);			// push sprite.__children
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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
	int index = luaL_checknumber(L, 2);
	if (index < 0) index = sprite->childCount() + index + 1;
	if (index < 1 || index > sprite->childCount())
        luaL_error(L, "%s", GStatus(2006).errorString());	// Error #2006: The supplied index is out of bounds.

	Sprite* child = sprite->getChildAt(index - 1);

    lua_rawgettoken(L, 1, tokenChildren);	// push sprite.__children
	lua_pushlightuserdata(L, child);
	lua_rawget(L, -2);					// push sprite.__children[child]
	lua_pushnil(L);
    lua_rawsettoken(L, -2, tokenParent);	// sprite.__children[child].__parent = nil
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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Sprite* child = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 2));

	GStatus status;
	int index = sprite->getChildIndex(child, &status);
	if (status.error() == true)
        luaL_error(L, "%s", status.errorString());

	lua_pushnil(L);
    lua_rawsettoken(L, 2, tokenParent);		// child.__parent = nil

    lua_rawgettoken(L, 1, tokenChildren);	// push sprite.__children
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
        Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
        Sprite* sprite1 = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 2));
        Sprite* sprite2 = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 3));

        sprite->swapChildren(sprite1,sprite2);

        return 0;
}

int SpriteBinder::swapChildrenAt(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::swapChildrenAt",0);
	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    int index = luaL_checkinteger(L, 2);
	if (index < 0) index = sprite->childCount() + index + 1;
	if (index < 1 || index > sprite->childCount())
        luaL_error(L, "%s", GStatus(2006).errorString());	// Error #2006: The supplied index is out of bounds.

	Sprite* child = sprite->getChildAt(index - 1);

    lua_rawgettoken(L, 1, tokenChildren);	// push sprite.__children
	lua_pushlightuserdata(L, child);
	lua_rawget(L, -2);					// push sprite.__children[child]
	lua_remove(L, -2);					// pop sprite.__children

	return 1;
}

int SpriteBinder::setClip(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setClip", 0);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    lua_pushnumber(L, sprite->clipX());
    lua_pushnumber(L, sprite->clipY());
    lua_pushnumber(L, sprite->clipW());
    lua_pushnumber(L, sprite->clipH());

    return 4;
}

#define FKEY(n) (STRKEY_LAYOUT_##n)
#define FSKEY(n) (#n)
#define FMKEY(n) (1LL<<FKEY(n))
#define FRESOLVE(n,t) if (lua_type(L,-1)==LUA_TSTRING) { const char *key=luaL_checkstring(L,-1); p->resolvedMap|=FMKEY(n); p->resolved[FKEY(n)]=key; lua_pushvalue(L,t-1); LuaApplication::resolveStyle(L,key,-2); lua_remove(L,-2);
#define ARESOLVE(n,t,i) if (lua_type(L,-1)==LUA_TSTRING) { const char *key=luaL_checkstring(L,-1); p->resolvedMap|=FMKEY(n); lua_pushvalue(L,t-1); LuaApplication::resolveStyle(L,key,-2); lua_remove(L,-2); p->resolvedArray[FKEY(n)][i]=key; }
#define RESOLVE(n) FRESOLVE(n,-1) } else { p->resolvedMap&=~FMKEY(n); p->resolved.erase(FKEY(n)); }
#define RESOLVED(n) if ((!raw)&&(p->resolvedMap&FMKEY(n))) lua_pushstring(L,p->resolved[FKEY(n)].c_str()); else
#define ARESOLVED(n,i) if ((!raw)&&(p->resolvedMap&FMKEY(n))&&p->resolvedArray[FKEY(n)].count(i)) lua_pushstring(L,p->resolvedArray[FKEY(n)][i].c_str()); else
#define FILL_NUM_ARRAY(n,f) \
        lua_getfield(L,2,FSKEY(n)); if (!lua_isnoneornil(L,-1)) { \
			luaL_checktype(L,-1, LUA_TTABLE); \
			p->f.resize(lua_objlen(L,-1)); \
            p->resolvedMap&=~FMKEY(n); \
            p->resolvedArray.erase(FKEY(n)); \
            for (size_t k=1;k<=(size_t)lua_objlen(L,-1);k++) { \
                lua_rawgeti(L,-1,k); ARESOLVE(n,-2,k-1); \
                p->f[k-1]=lua_tonumber(L,-1); \
                lua_pop(L,1); } \
		} lua_pop(L,1);
#define FETCH_KEY(n) if (layoutStrings[FKEY(n)]==-1) layoutStrings[FKEY(n)]=lua_newtoken(L,FSKEY(n));
#define FETCH_VAL(n) FETCH_KEY(n) lua_rawgettoken(L,2,layoutStrings[FKEY(n)]);
#define STORE_VAL(n) FETCH_KEY(n) lua_rawsettoken(L,-2,layoutStrings[FKEY(n)]);
#define FILL_INT(n,f) FETCH_VAL(n); if (!lua_isnoneornil(L,-1)) { RESOLVE(n); p->f=lua_tointeger(L,-1); } lua_pop(L,1);
#define FILL_INTT(n,f,t) FETCH_VAL(n); if (!lua_isnoneornil(L,-1)) { RESOLVE(n); p->f=(t) lua_tointeger(L,-1); } lua_pop(L,1);
#define FILL_NUM(n,f) FETCH_VAL(n); if (!lua_isnoneornil(L,-1)) { RESOLVE(n); p->f=lua_tonumber(L,-1); } lua_pop(L,1);
#define FILL_BOOL(n,f) FETCH_VAL(n); if (!lua_isnoneornil(L,-1)) p->f=lua_toboolean(L,-1); lua_pop(L,1);
#define STOR_NUM_ARRAY(n,f) \
        lua_newtable(L); \
        for (size_t k=0;k<p->f.size();k++) { ARESOLVED(n,k) lua_pushnumber(L,p->f[k]); lua_rawseti(L,-2,k+1); }\
        STORE_VAL(n);
#define STOR_NUM_ARRAYN(n,f,fl) \
        lua_newtable(L); \
        for (size_t k=0;k<p->fl;k++) { ARESOLVED(n,k) lua_pushnumber(L,p->f[k]); lua_rawseti(L,-2,k+1); }\
        STORE_VAL(n);
#define STOR_INT(n,f) RESOLVED(n) lua_pushinteger(L,p->f); STORE_VAL(n);
#define STOR_INTT(n,f,t) RESOLVED(n) lua_pushinteger(L,(int)p->f); STORE_VAL(n);
#define STOR_NUM(n,f) RESOLVED(n) lua_pushnumber(L,p->f); STORE_VAL(n);
#define STOR_BOOL(n,f) lua_pushboolean(L,p->f); STORE_VAL(n);

int SpriteBinder::setLayoutParameters(lua_State *L)
{
    StackChecker checker(L, "SpriteBinder::setLayoutParameters", 0);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    if (lua_isnoneornil(L,2))
		sprite->clearLayoutState();
	else {
        luaL_checktype(L, 2, LUA_TTABLE);
        LuaApplication::getStyleTable(L,1);
		GridBagLayout *p=sprite->getLayoutState();
        FILL_NUM_ARRAY(columnWidths,columnWidths);
        FILL_NUM_ARRAY(rowHeights,rowHeights);
        FILL_NUM_ARRAY(columnWeights,columnWeights);
        FILL_NUM_ARRAY(rowWeights,rowWeights);

		lua_getfield(L,2,"insets");
        if (!lua_isnoneornil(L,-1)) {
            FRESOLVE(insetTop,-1)
                p->resolved[FKEY(insetLeft)]=p->resolved[FKEY(insetTop)];
                p->resolved[FKEY(insetBottom)]=p->resolved[FKEY(insetTop)];
                p->resolved[FKEY(insetRight)]=p->resolved[FKEY(insetTop)];
            }
			else {
                p->resolved.erase(FKEY(insetTop));
                p->resolved.erase(FKEY(insetLeft));
                p->resolved.erase(FKEY(insetBottom));
                p->resolved.erase(FKEY(insetRight));
			}
            p->pInsets.left=p->pInsets.right=p->pInsets.top=p->pInsets.bottom=luaL_checknumber(L,-1);
        }
		lua_pop(L,1);
        FILL_NUM(insetTop,pInsets.top); FILL_NUM(insetLeft,pInsets.left);
        FILL_NUM(insetBottom,pInsets.bottom); FILL_NUM(insetRight,pInsets.right);

        FILL_BOOL(equalizeCells,equalizeCells);
        FILL_BOOL(resizeContainer,resizeContainer);
        FILL_BOOL(worldAlign,worldAlign);
        FILL_BOOL(fixedGrid,fixedGrid);
        FILL_NUM(cellSpacingX,cellSpacingX); FILL_NUM(cellSpacingY,cellSpacingY);
        FILL_NUM(gridAnchorX,gridAnchorX); FILL_NUM(gridAnchorY,gridAnchorY);
        FILL_NUM(offsetx,offsetX); FILL_NUM(offsety,offsetY);
        FILL_NUM(originx,originX); FILL_NUM(originy,originY);
        FILL_NUM(zOffset,zOffset);

        lua_pop(L,1);
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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    if (lua_isnoneornil(L,2))
		sprite->clearLayoutConstraints();
	else {
        luaL_checktype(L, 2, LUA_TTABLE);
        LuaApplication::getStyleTable(L,1);
        GridBagConstraints *p=sprite->getLayoutConstraints();

        FILL_INT(gridx,gridx); FILL_INT(gridy,gridy);
        FILL_INT(gridwidth,gridwidth); FILL_INT(gridheight,gridheight);
        FILL_BOOL(gridRelative,gridRelative);
        FILL_INT(overflowMode,overflowMode);
        FILL_INT(hidePriority,hidePriority);
        FILL_NUM(weightx,weightx); FILL_NUM(weighty,weighty);
        FILL_INTT(anchor,anchor,GridBagConstraints::_Anchor);
        lua_getfield(L,2,"fill");
        if (!lua_isnoneornil(L,-1)) {
            int fill=luaL_checkinteger(L,-1);
            p->fillX=(fill==1)||(fill==2)?1:0;
            p->fillY=(fill==1)||(fill==3)?1:0;
        }
        lua_pop(L,1);
        FILL_NUM(fillx,fillX); FILL_NUM(filly,fillY);
        FILL_NUM(aspectRatio,aspectRatio);
        FILL_NUM(anchorx,anchorX); FILL_NUM(anchory,anchorY);
        FILL_NUM(extraw,extraW); FILL_NUM(extrah,extraH);
        FILL_NUM(offsetx,offsetX); FILL_NUM(offsety,offsetY);
        FILL_NUM(originx,originX); FILL_NUM(originy,originY);
        FILL_NUM(ipadx,ipadx); FILL_NUM(ipady,ipady);
        lua_getfield(L,2,"width");
        if (!lua_isnoneornil(L,-1)) {
            FRESOLVE(minWidth,-1)
                p->resolved[FKEY(prefWidth)]=p->resolved[FKEY(minWidth)];
            }
        	else {
                p->resolved.erase(FKEY(minWidth));
                p->resolved.erase(FKEY(prefWidth));
        	}
            float width=luaL_checknumber(L,-1);
            p->aminWidth=width; p->prefWidth=width;
        }
        lua_pop(L,1);
        lua_getfield(L,2,"height");
        if (!lua_isnoneornil(L,-1)) {
            FRESOLVE(minHeight,-1)
                p->resolved[FKEY(prefHeight)]=p->resolved[FKEY(minHeight)];
            }
			else {
                p->resolved.erase(FKEY(minHeight));
                p->resolved.erase(FKEY(prefHeight));
			}
            float height=luaL_checknumber(L,-1);
            p->aminHeight=height; p->prefHeight=height;
        }
        lua_pop(L,1);
        FILL_NUM(minWidth,aminWidth); FILL_NUM(minHeight,aminHeight);
        FILL_NUM(prefWidth,prefWidth); FILL_NUM(prefHeight,prefHeight);
        FILL_BOOL(shrink,optimizeSize);
        FILL_BOOL(group,group);
        FILL_BOOL(autoclip,autoClip);

		lua_getfield(L,2,"insets");
        if (!lua_isnoneornil(L,-1)) {
            FRESOLVE(insetTop,-1)
                p->resolved[FKEY(insetLeft)]=p->resolved[FKEY(insetTop)];
                p->resolved[FKEY(insetBottom)]=p->resolved[FKEY(insetTop)];
                p->resolved[FKEY(insetRight)]=p->resolved[FKEY(insetTop)];
            }
            else {
                p->resolved.erase(FKEY(insetTop));
                p->resolved.erase(FKEY(insetLeft));
                p->resolved.erase(FKEY(insetBottom));
                p->resolved.erase(FKEY(insetRight));
            }
            p->insets.left=p->insets.right=p->insets.top=p->insets.bottom=luaL_checknumber(L,-1);
        }
		lua_pop(L,1);
        FILL_NUM(insetTop,insets.top); FILL_NUM(insetLeft,insets.left);
        FILL_NUM(insetBottom,insets.bottom); FILL_NUM(insetRight,insets.right);
        lua_pop(L,1);
        sprite->invalidate(Sprite::INV_CONSTRAINTS);
	}
	return 0;
}

int SpriteBinder::getLayoutParameters(lua_State *L)
{
    StackChecker checker(L, "SpriteBinder::getLayoutParameters", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    if (sprite->hasLayoutState())
	{
		GridBagLayout *p=sprite->getLayoutState();
        bool raw=lua_toboolean(L,2);
		lua_newtable(L);
        STOR_NUM_ARRAY(columnWidths,columnWidths);
        STOR_NUM_ARRAY(rowHeights,rowHeights);
        STOR_NUM_ARRAY(columnWeights,columnWeights);
        STOR_NUM_ARRAY(rowWeights,rowWeights);
        STOR_NUM(insetTop,pInsets.top); STOR_NUM(insetLeft,pInsets.left);
        STOR_NUM(insetBottom,pInsets.bottom); STOR_NUM(insetRight,pInsets.right);
        STOR_BOOL(equalizeCells,equalizeCells);
        STOR_BOOL(worldAlign,worldAlign);
        STOR_BOOL(fixedGrid,fixedGrid);
        STOR_BOOL(resizeContainer,resizeContainer);
        STOR_NUM(cellSpacingX,cellSpacingX); STOR_NUM(cellSpacingY,cellSpacingY);
        STOR_NUM(gridAnchorX,gridAnchorX); STOR_NUM(gridAnchorY,gridAnchorY);
        STOR_NUM(offsetx,offsetX); STOR_NUM(offsety,offsetY);
        STOR_NUM(originx,originX); STOR_NUM(originy,originY);
        STOR_NUM(zOffset,zOffset);
	}
	else
		lua_pushnil(L);
	return 1;
}

int SpriteBinder::getLayoutConstraints(lua_State *L)
{
    StackChecker checker(L, "SpriteBinder::getLayoutConstraints", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    if (sprite->hasLayoutConstraints())
	{
		GridBagConstraints *p=sprite->getLayoutConstraints();
        bool raw=lua_toboolean(L,2);
        lua_newtable(L);
        STOR_INT(gridx,gridx); STOR_INT(gridy,gridy);
        STOR_INT(gridwidth,gridwidth); STOR_INT(gridheight,gridheight);
        STOR_NUM(weightx,weightx); STOR_NUM(weighty,weighty);
        STOR_BOOL(gridRelative,gridRelative);
        STOR_INT(overflowMode,overflowMode);
        STOR_INT(hidePriority,hidePriority);
        STOR_INTT(anchor,anchor,GridBagConstraints::_Anchor);
        STOR_NUM(fillx,fillX); STOR_NUM(filly,fillY);
        STOR_NUM(aspectRatio,aspectRatio);
        STOR_NUM(anchorx,anchorX); STOR_NUM(anchory,anchorY);
        STOR_NUM(ipadx,ipadx); STOR_NUM(ipady,ipady);
        STOR_NUM(minWidth,aminWidth); STOR_NUM(minHeight,aminHeight);
        STOR_NUM(prefWidth,prefWidth); STOR_NUM(prefHeight,prefHeight);
        STOR_NUM(insetTop,insets.top); STOR_NUM(insetLeft,insets.left);
        STOR_NUM(insetBottom,insets.bottom); STOR_NUM(insetRight,insets.right);
        STOR_NUM(extraw,extraW); STOR_NUM(extrah,extraH);
        STOR_NUM(offsetx,offsetX); STOR_NUM(offsety,offsetY);
        STOR_NUM(originx,originX); STOR_NUM(originy,originY);
        STOR_BOOL(shrink,optimizeSize);
        STOR_BOOL(group,group);
        STOR_BOOL(autoclip,autoClip);
	}
	else
		lua_pushnil(L);
	return 1;
}

#undef RESOLVED
#undef ARESOLVED
#define RESOLVED(n)
#define ARESOLVED(n,i)

int SpriteBinder::getLayoutInfo(lua_State *L)
{
    StackChecker checker(L, "SpriteBinder::getLayoutInfo", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    float epw=luaL_optnumber(L,2,-1);
	float eph=luaL_optnumber(L,3,-1);
	int type=luaL_optinteger(L,4,0);

	if (sprite->hasLayoutState())
	{
		GridBagLayout *sp=sprite->getLayoutState();
        GridBagLayoutInfo *p=nullptr;
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
        }
        if (!p)
		{
			float dw,dh;
            float pwidth,pheight;
            if (type==0) type=3;
            sprite->getDimensions(pwidth, pheight);
            p = sp->getLayoutInfo(sprite, (type>=2)?2:type, pwidth, pheight, nullptr); //PREFERRED or BEST
            sp->getMinSize(sprite, p, dw, dh, sp->pInsets);
		    if (type==3) {//BEST
				if (pwidth < dw || pheight < dh) {
                    p = sp->getLayoutInfo(sprite, 1, pwidth, pheight, nullptr);
                    sp->getMinSize(sprite, p, dw, dh, sp->pInsets);
				}
		    }
            p->reqWidth=dw;
            p->reqHeight=dh;
		}
		lua_newtable(L);

        STOR_INT(width,width); STOR_INT(height,height);
        STOR_NUM(reqWidth,reqWidth); STOR_NUM(reqHeight,reqHeight);
        STOR_NUM(startx,startx); STOR_NUM(starty,starty);
        STOR_NUM(cellSpacingX,cellSpacingX); STOR_NUM(cellSpacingY,cellSpacingY);
        STOR_NUM_ARRAYN(minWidth,minWidth,width);
        STOR_NUM_ARRAYN(minHeight,minHeight,height);
        STOR_NUM_ARRAYN(weightX,weightX,width);
        STOR_NUM_ARRAYN(weightY,weightY,height);
	}
	else
		lua_pushnil(L);
	return 1;
}

#undef FRESOLVE
#undef RESOLVE
#undef RESOLVED
#undef ARESOLVE
#undef ARESOLVED
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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->x());

	return 1;
}

int SpriteBinder::getY(lua_State* L)
{
	StackChecker checker(L, "getY", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->y());

	return 1;
}

int SpriteBinder::getZ(lua_State* L)
{
	StackChecker checker(L, "getZ", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->z());

	return 1;
}

int SpriteBinder::getRotation(lua_State* L)
{
	StackChecker checker(L, "getRotation", 1);
	
	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->rotation());

	return 1;
}

int SpriteBinder::getRotationX(lua_State* L)
{
	StackChecker checker(L, "getRotationX", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->rotationX());

	return 1;
}

int SpriteBinder::getRotationY(lua_State* L)
{
	StackChecker checker(L, "getRotationY", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->rotationY());

	return 1;
}

int SpriteBinder::getScaleX(lua_State* L)
{
	StackChecker checker(L, "getScaleX", 1);
	
	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->scaleX());

	return 1;
}

int SpriteBinder::getScaleY(lua_State* L)
{
	StackChecker checker(L, "getScaleY", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->scaleY());

	return 1;
}

int SpriteBinder::getScaleZ(lua_State* L)
{
	StackChecker checker(L, "getScaleZ", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->scaleZ());

	return 1;
}

int SpriteBinder::getSkewX(lua_State* L)
{
    StackChecker checker(L, "getSkewX", 1);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    lua_pushnumber(L, sprite->skewX());

    return 1;
}

int SpriteBinder::getSkewY(lua_State* L)
{
    StackChecker checker(L, "getSkewY", 1);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    lua_pushnumber(L, sprite->skewY());

    return 1;
}

int SpriteBinder::setX(lua_State* L)
{
	StackChecker checker(L, "setX");

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double x = luaL_checknumber(L, 2);
	sprite->setX(x);

	return 0;
}

int SpriteBinder::setY(lua_State* L)
{
	StackChecker checker(L, "setY");

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double y = luaL_checknumber(L, 2);
	sprite->setY(y);

	return 0;
}

int SpriteBinder::setZ(lua_State* L)
{
	StackChecker checker(L, "setZ");

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double z = luaL_checknumber(L, 2);
	sprite->setZ(z);

	return 0;
}

int SpriteBinder::setRotation(lua_State* L)
{
	StackChecker checker(L, "setRotation");

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double rotation = luaL_checknumber(L, 2);
	sprite->setRotation(rotation);

	return 0;
}

int SpriteBinder::setRotationX(lua_State* L)
{
	StackChecker checker(L, "setRotationX");

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double rotation = luaL_checknumber(L, 2);
	sprite->setRotationX(rotation);

	return 0;
}

int SpriteBinder::setRotationY(lua_State* L)
{
	StackChecker checker(L, "setRotationY");

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double rotation = luaL_checknumber(L, 2);
	sprite->setRotationY(rotation);

	return 0;
}

int SpriteBinder::setScaleX(lua_State* L)
{
	StackChecker checker(L, "setScaleX");

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double scaleX = luaL_checknumber(L, 2);
	sprite->setScaleX(scaleX);

	return 0;
}

int SpriteBinder::setScaleY(lua_State* L)
{
	StackChecker checker(L, "setScaleY");

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double scaleY = luaL_checknumber(L, 2);
	sprite->setScaleY(scaleY);

	return 0;
}

int SpriteBinder::setScaleZ(lua_State* L)
{
	StackChecker checker(L, "setScaleZ");

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double scaleZ = luaL_checknumber(L, 2);
	sprite->setScaleZ(scaleZ);

	return 0;
}

int SpriteBinder::setSkewX(lua_State* L)
{
    StackChecker checker(L, "setScaleX");

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    double skewX = luaL_checknumber(L, 2);
    sprite->setSkewX(skewX);

    return 0;
}

int SpriteBinder::setSkewY(lua_State* L)
{
    StackChecker checker(L, "setScaleY");

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    double skewY = luaL_checknumber(L, 2);
    sprite->setSkewY(skewY);

    return 0;
}

int SpriteBinder::setPosition(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setPosition", 0);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	const float *v=lua_tovector(L,2);
	if (v)
		sprite->setXYZ(v[0], v[1], v[2]);
	else {
		lua_Number x = luaL_checknumber(L, 2);
		lua_Number y = luaL_checknumber(L, 3);
		if (lua_isnoneornil(L, 4))
			sprite->setXY(x, y);
		else
		{
			lua_Number z = luaL_checknumber(L, 4);
			sprite->setXYZ(x, y, z);
		}
	}
	return 0;
}

int SpriteBinder::getPosition(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getPosition", 3);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->x());
	lua_pushnumber(L, sprite->y());
	lua_pushnumber(L, sprite->z());

	return 3;
}

int SpriteBinder::setAnchorPosition(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setAnchorPosition", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	const float *v=lua_tovector(L,2);
	if (v)
		sprite->setRefXYZ(v[0], v[1], v[2]);
	else {
		lua_Number x = luaL_checknumber(L, 2);
		lua_Number y = luaL_checknumber(L, 3);
		if (lua_isnoneornil(L, 4))
			sprite->setRefXY(x, y);
		else
		{
			lua_Number z = luaL_checknumber(L, 4);
			sprite->setRefXYZ(x, y, z);
		}
	}
    return 0;
}

int SpriteBinder::getAnchorPosition(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::getAnchorPosition", 3);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    lua_pushnumber(L, sprite->refX());
    lua_pushnumber(L, sprite->refY());
    lua_pushnumber(L, sprite->refZ());

    return 3;
}

int SpriteBinder::setAnchorPoint(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setAnchorPoint", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	const float *v=lua_tovector(L,2);
	if (v)
		sprite->setScaleXYZ(v[0], v[1], v[2]);
	else {
		lua_Number x = luaL_checknumber(L, 2);
		lua_Number y = lua_isnoneornil(L, 3) ? x : luaL_checknumber(L, 3);
		if (lua_isnoneornil(L, 4)) //No Z
			sprite->setScaleXY(x, y); // Only scale X and Y
		else
		{
			lua_Number z = luaL_checknumber(L, 4);
			sprite->setScaleXYZ(x, y, z);
		}
	}
	return 0;
}

int SpriteBinder::getScale(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getScale", 3);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->scaleX());
	lua_pushnumber(L, sprite->scaleY());
	lua_pushnumber(L, sprite->scaleZ());

	return 3;
}

int SpriteBinder::setSkew(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setSkew", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    lua_Number x = luaL_checknumber(L, 2);
    lua_Number y = luaL_checknumber(L, 3);
    sprite->setSkewXY(x, y);

    return 0;
}

int SpriteBinder::getSkew(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::getSkew", 2);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    lua_pushnumber(L, sprite->skewX());
    lua_pushnumber(L, sprite->skewY());

    return 2;
}

int SpriteBinder::getParent(lua_State* L)
{
	StackChecker checker(L, "getParent", 1);

    lua_rawgettoken(L, 1, tokenParent);

	return 1;
}

int SpriteBinder::contains(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::contains", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Sprite* child = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 2));

	lua_pushboolean(L, sprite->contains(child));
	
	return 1;
}


int SpriteBinder::getChildIndex(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getChildIndex", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Sprite* child = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 2));


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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
	Sprite* parent = sprite->parent();

	if (parent == NULL)
		return 0;

    lua_rawgettoken(L, 1, tokenParent);		// puah sprite.__parent
    lua_rawgettoken(L, -1, tokenChildren);	// push sprite.__parent.__children
	lua_pushlightuserdata(L, sprite);
	lua_pushnil(L);
	lua_rawset(L, -3);					// sprite.__parent.__children[sprite] = nil
	lua_pop(L, 2);						// pop sprite.__parent and sprite.__parent.__children

	lua_pushnil(L);
    lua_rawsettoken(L, 1, tokenParent);		// sprite.__parent = nil

	parent->removeChild(sprite);

	return 0;
}

int SpriteBinder::localToGlobal(lua_State* L)
{
	StackChecker checker(L, "localToGlobal", 3);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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

int SpriteBinder::spriteToLocal(lua_State* L)
{
    StackChecker checker(L, "spriteToLocal", 3);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Sprite* ref = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 2));

    double x = luaL_checknumber(L, 3);
    double y = luaL_checknumber(L, 4);
    double z = luaL_optnumber(L, 5, 0.0);

    float tx, ty, tz;
    if (!sprite->spriteToLocal(ref, x, y, z, &tx, &ty, &tz)) {
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushnil(L);
    }
    else {
		lua_pushnumber(L, tx);
		lua_pushnumber(L, ty);
		lua_pushnumber(L, tz);
    }

    return 3;
}

int SpriteBinder::spriteToLocalMatrix(lua_State* L)
{
    StackChecker checker(L, "spriteToLocalMatrix", 1);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Sprite* ref = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 2));

    Matrix m;
    if (!sprite->spriteToLocalMatrix(ref,m))
        lua_pushnil(L);
    else {
        Transform *t=new Transform();
        t->setMatrix(m.data());
        binder.pushInstance("Matrix", t);
    }

    return 1;
}

int SpriteBinder::isVisible(lua_State* L)
{
	StackChecker checker(L, "isVisible", 1);
	
	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    int visible = lua_toboolean(L, 2);
    sprite->setVisible(visible);

    return 0;
}

int SpriteBinder::setAutoSort(lua_State* L)
{
    StackChecker checker(L, "setAutoSort");

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    sprite->setAutoSort(lua_toboolean(L, 2));

    return 0;
}

int SpriteBinder::isOnStage(lua_State* L)
{
	StackChecker checker(L, "isOnStage", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);

	bool shapeFlag = false;

    int nargs=lua_gettop(L);
    if (nargs >= 4)
        shapeFlag = lua_toboolean(L, 4);
    Sprite *ref=nullptr;
    if ((nargs >= 5)&&(!lua_isnil(L,5)))
        ref = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 5));

    lua_pushboolean(L, sprite->hitTestPoint(x, y, shapeFlag, ref));
	
	return 1;
}

int SpriteBinder::getChildrenAtPoint(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getChildrenAtPoint", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	bool visible = lua_toboolean(L, 4);
	bool nosubs = lua_toboolean(L, 5);
    int nargs=lua_gettop(L);
    Sprite *ref=nullptr;
    if ((nargs >= 6)&&(!lua_isnil(L,6)))
        ref = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 6));

    std::vector<std::pair<int,Sprite *>>res;
    sprite->getChildrenAtPoint(x, y, ref, visible, nosubs, res);
	int nres=res.size();
	lua_createtable(L,nres,0);
	for (int i=0;i<nres;i++) {
        int pidx=res[i].first;
        if (pidx==0)
            lua_rawgettoken(L, 1, tokenChildren);	// push sprite.__children
        else {
            lua_rawgeti(L,-1,pidx);
            lua_rawgettoken(L, -1, tokenChildren);	// push sprite.__children
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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	Transform *t=new Transform();
	*t=sprite->transform();

    binder.pushInstance("Matrix", t);
	
	return 1;
}

int SpriteBinder::setMatrix(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setMatrix", 0);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Transform* matrix = static_cast<Transform*>(binder.getInstance("Matrix", 2));

	sprite->setMatrix(matrix);

	return 0;
}

int SpriteBinder::getAlpha(lua_State* L)
{
	StackChecker checker(L, "getAlpha", 1);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_pushnumber(L, sprite->alpha());

	return 1;
}

int SpriteBinder::setAlpha(lua_State* L)
{
	StackChecker checker(L, "setAlpha", 0);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

	lua_Number alpha = luaL_checknumber(L, 2);
	sprite->setAlpha(alpha);

	return 0;
}

int SpriteBinder::getBounds(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::getBounds", 4);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Sprite* targetCoordinateSpace = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 2));
	bool visible=lua_toboolean(L,3);

	float minx, miny, maxx, maxy;
	sprite->getBounds(targetCoordinateSpace, &minx, &miny, &maxx, &maxy, visible);

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    sprite->clearBlendFunc();

	return 0;
}

int SpriteBinder::setShader(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setShader", 0);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
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

    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
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

    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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

    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    sprite->redrawEffects();
	return 0;
}

int SpriteBinder::setStencilOperation(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::setStencilOperation", 0);

	Binder binder(L);

    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    ShaderEngine::DepthStencil ds;
    unsigned int sm=0;

    if (lua_istable(L,2))
	{
		luaL_checktype(L,2,LUA_TTABLE);
        lua_getfield(L,2,"depthTest");
        if (!lua_isnil(L,-1)) {
            ds.dTest=lua_toboolean(L,-1);
            sm|=Sprite::STENCILMASK_DTEST;
        }
        lua_pop(L,1);
        lua_getfield(L,2,"depthClear");
        if (!lua_isnil(L,-1)) {
            ds.dClear=lua_toboolean(L,-1);
            sm|=Sprite::STENCILMASK_DCLEAR;
        }
        lua_pop(L,1);
        lua_getfield(L,2,"stencilClear");
        if (!lua_isnil(L,-1)) {
			ds.sClear=lua_toboolean(L,-1);
            sm|=Sprite::STENCILMASK_SCLEAR;
        }
		lua_pop(L,1);
		lua_getfield(L,2,"stencilMask");
        if (!lua_isnil(L,-1)) {
			ds.sMask=luaL_checkinteger(L,-1);
            sm|=Sprite::STENCILMASK_SMASK;
        }
        lua_pop(L,1);
		lua_getfield(L,2,"stencilWriteMask");
        if (!lua_isnil(L,-1)) {
			ds.sWMask=luaL_checkinteger(L,-1);
            sm|=Sprite::STENCILMASK_SWMASK;
        }
        lua_pop(L,1);
		lua_getfield(L,2,"stencilClearValue");
        if (!lua_isnil(L,-1)) {
			ds.sClearValue=luaL_checkinteger(L,-1);
            sm|=Sprite::STENCILMASK_SCLEARVALUE;
        }
        lua_pop(L,1);
		lua_getfield(L,2,"stencilRef");
        if (!lua_isnil(L,-1)) {
			ds.sRef=luaL_checkinteger(L,-1);
            sm|=Sprite::STENCILMASK_SREF;
        }
        lua_pop(L,1);
		lua_getfield(L,2,"stencilFunc");
        if (!lua_isnil(L,-1)) {
			ds.sFunc=(ShaderEngine::StencilFunc) luaL_checkinteger(L,-1);
            sm|=Sprite::STENCILMASK_SFUNC;
        }
        lua_pop(L,1);
		lua_getfield(L,2,"depthPass");
        if (!lua_isnil(L,-1)) {
			ds.dPass=(ShaderEngine::StencilOp) luaL_checkinteger(L,-1);
            sm|=Sprite::STENCILMASK_DPASS;
        }
        lua_pop(L,1);
		lua_getfield(L,2,"stencilFail");
        if (!lua_isnil(L,-1)) {
			ds.sFail=(ShaderEngine::StencilOp) luaL_checkinteger(L,-1);
            sm|=Sprite::STENCILMASK_SFAIL;
        }
        lua_pop(L,1);
		lua_getfield(L,2,"depthFail");
        if (!lua_isnil(L,-1)) {
			ds.dFail=(ShaderEngine::StencilOp) luaL_checkinteger(L,-1);
            sm|=Sprite::STENCILMASK_DFAIL;
        }
        lua_pop(L,1);
		lua_getfield(L,2,"cullMode");
        if (!lua_isnil(L,-1)) {
			ds.cullMode=((ShaderEngine::CullMode)(luaL_checkinteger(L,-1)&3));
            sm|=Sprite::STENCILMASK_CULLMODE;
        }
        lua_pop(L,1);
        lua_getfield(L,2,"depthMask");
        if (!lua_isnil(L,-1)) {
            ds.dMask=lua_toboolean(L,-1);
            sm|=Sprite::STENCILMASK_DMASK;
        }
        lua_pop(L,1);
    }

    sprite->setStencilOperation(ds,sm);

	return 0;
}

int SpriteBinder::getDrawCount(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::getDrawCount", 1);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

    lua_pushinteger(L, sprite->drawCount());

    return 1;
}

int SpriteBinder::setCheckClip(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setCheckClip", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    sprite->setCheckClip(lua_toboolean(L,2));
    return 0;
}

int SpriteBinder::setHiddenChildren(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setHiddenChildren", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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

int SpriteBinder::setWorldAlign(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setWorldAlign", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    sprite->setWorldAlign(lua_toboolean(L,2));
    return 0;
}

static void gatherStyledChildren(lua_State *L,int idx,int tidx)
{
    /*
    if (lua_rawgetfield(L,idx,"__style")==LUA_TTABLE) {
        if (lua_rawgetfield(L,-1,"__Cache")==LUA_TTABLE) {
            lua_pop(L,1);
            lua_newtable(L);
            lua_setfield(L,-2,"__Cache");
        }
        lua_pop(L,1);
    }
    lua_pop(L,1);
    */
    if (lua_rawgettoken(L,idx,SpriteBinder::tokenChildren)!=LUA_TNIL) {
        lua_checkstack(L,8);
        lua_pushnil(L);
        while (lua_next(L,-2)) {
		   gatherStyledChildren(L,-1,tidx-3);
           lua_pushvalue(L,-2);
           lua_rawset(L,tidx-4);
        }
    }
    lua_pop(L,1);
}

int SpriteBinder::setStyle(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setStyle", 0);

    if (!lua_istable(L, 1))	// check if the bottom of stack (first paramater, i.e. self) is table
    {
        luaL_typerror(L, 1, "Sprite");
        return 0;
    }
    if (lua_rawgettoken(L, 1, SpriteBinder::tokenUserdata)!=LUA_TUSERDATA) // get adress
    {
        lua_pop(L, 1);
        luaL_error(L, "Sprite '__userdata' cannot be found");
        return 0;
    }
    void* ptr = *(void**)lua_touserdata(L, -1);
    Sprite* sprite = static_cast<Sprite*>(ptr);
    lua_pop(L, 1);

    if (!lua_isnil(L,2))
        luaL_checktype(L,2,LUA_TTABLE);
    bool propagate=lua_toboolean(L,3);
    lua_pushvalue(L,2);
    lua_rawsetfield(L,1,"__style");
    lua_gettoken(L,LUA_GLOBALSINDEX,LuaApplication::token_application);

    int npop=1;
    if (!lua_isnil(L,-1))
    {
        lua_rawgettoken(L,-1,LuaApplication::token__styleUpdates);
		if (lua_isnil(L,-1))
		{
			lua_pop(L,1);
			lua_newtable(L);
			lua_pushvalue(L,-1);
            lua_rawsettoken(L,-3,LuaApplication::token__styleUpdates);
		}
		npop++;
		lua_pushvalue(L,1);
		lua_pushlightuserdata(L,sprite);
		lua_rawset(L,-3);
		if (propagate)
			gatherStyledChildren(L,1,-1);
    }
    lua_pop(L,npop);
    LuaApplication::hasStyleUpdate=true;
    return 0;
}

int SpriteBinder::resolveStyle(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::resolveStyle", 2);
    int hasTable=!lua_isnoneornil(L,3);
    lua_pushvalue(L,2);
    if (lua_type(L,2)!=LUA_TSTRING) {
        lua_pushstring(L,lua_typename(L,lua_type(L,2)));
        return 2;
    }
    if (hasTable)
        lua_pushvalue(L,3);
    else
    	LuaApplication::getStyleTable(L,1);

    const char *skey=luaL_checkstring(L,2);
    int rtype=LuaApplication::resolveStyle(L,skey,-2);
    lua_remove(L,-2);
    lua_pushstring(L,lua_typename(L,rtype));
    return 2;
}

#define FILL_NUM_ARRAY(n,f) \
        if (p->resolvedMap&FMKEY(n)) { \
            auto &pa=p->resolvedArray[FKEY(n)]; \
            for (size_t k=0;k<p->f.size();k++) { \
                if (pa.count(k)) { \
                    lua_pushvalue(L,-1); \
                    LuaApplication::resolveStyle(L,pa[k].c_str(),0); \
                    lua_Number nn=lua_tonumber(L,-1);\
                    if (nn!=p->f[k]) { p->f[k]=nn; dirty=true; }\
                    lua_pop(L,1); \
                } } }
#define FILL_NUM(n,f) if (p->resolvedMap&FMKEY(n)) { \
    lua_pushvalue(L,-1); \
    LuaApplication::resolveStyle(L,p->resolved[FKEY(n)].c_str(),0);\
    float nn=(float)lua_tonumber(L,-1);\
    if (nn!=p->f) { p->f=nn; dirty=true; }\
    lua_pop(L,1);\
    }

int SpriteBinder::updateStyle(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::updateStyle", 0);
    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    LuaApplication::getStyleTable(L,1);
    bool dirtyConstraints=false;
    {
        GridBagLayout *p=sprite->layoutState;
        if (p&&p->resolvedMap) {
            bool dirty=false;

            FILL_NUM_ARRAY(columnWidths,columnWidths);
            FILL_NUM_ARRAY(rowHeights,rowHeights);
            FILL_NUM_ARRAY(columnWeights,columnWeights);
            FILL_NUM_ARRAY(rowWeights,rowWeights);

            FILL_NUM(insetTop,pInsets.top); FILL_NUM(insetLeft,pInsets.left);
            FILL_NUM(insetBottom,pInsets.bottom); FILL_NUM(insetRight,pInsets.right);

            FILL_NUM(cellSpacingX,cellSpacingX); FILL_NUM(cellSpacingY,cellSpacingY);
            FILL_NUM(gridAnchorX,gridAnchorX); FILL_NUM(gridAnchorY,gridAnchorY);
            FILL_NUM(offsetx,offsetX); FILL_NUM(offsety,offsetY);
            FILL_NUM(originx,originX); FILL_NUM(originy,originY);
            FILL_NUM(zOffset,zOffset);

            if (dirty) {
                p->dirty=true;
                p->layoutInfoCache[0].valid=false;
                p->layoutInfoCache[1].valid=false;
                dirtyConstraints=true;
            }
        }
    }

    //Constraints
    {
        GridBagConstraints *p=sprite->layoutConstraints;
        if (p&&p->resolvedMap) {
            bool dirty=false;

            FILL_NUM(weightx,weightx); FILL_NUM(weighty,weighty);
            FILL_NUM(fillx,fillX); FILL_NUM(filly,fillY);
            FILL_NUM(aspectRatio,aspectRatio);
            FILL_NUM(anchorx,anchorX); FILL_NUM(anchory,anchorY);
            FILL_NUM(extraw,extraW); FILL_NUM(extrah,extraH);
            FILL_NUM(offsetx,offsetX); FILL_NUM(offsety,offsetY);
            FILL_NUM(originx,originX); FILL_NUM(originy,originY);
            FILL_NUM(ipadx,ipadx); FILL_NUM(ipady,ipady);

            FILL_NUM(prefWidth, prefWidth);
            FILL_NUM(minWidth, aminWidth);
            FILL_NUM(prefHeight, prefHeight);
            FILL_NUM(minHeight, aminHeight);

            FILL_NUM(insetTop,insets.top); FILL_NUM(insetLeft,insets.left);
            FILL_NUM(insetBottom,insets.bottom); FILL_NUM(insetRight,insets.right);

            dirtyConstraints|=dirty;
        }
    }

    if (dirtyConstraints)
        sprite->invalidate(Sprite::INV_CONSTRAINTS);
    lua_pop(L,1);
    return 0;
}
#undef FILL_NUM_ARRAY
#undef FILL_NUM

int SpriteBinder::set(lua_State* L)
{
	StackChecker checker(L, "SpriteBinder::set", 0);

	Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));

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

int SpriteBinder::setGhosts(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::setGhosts", 0);

    Binder binder(L);
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    if (lua_isnoneornil(L,2))
        sprite->setGhosts(nullptr);
    else
    {
        luaL_checktype(L,2,LUA_TTABLE);
        int nghosts=lua_objlen(L,2);
        std::vector<GhostSprite *> ghosts;
        for (int i=0;i<nghosts;i++) {
            lua_rawgeti(L,2,i+1); //T
            luaL_checktype(L,-1,LUA_TTABLE);
            lua_rawgetfield(L,-1,"model"); //T,M
            luaL_checktype(L,-1,LUA_TTABLE);
            lua_getfield(L,-1,"__parseGhosts"); //T,M,F
            luaL_checktype(L,-1,LUA_TFUNCTION);
            lua_insert(L,-3); //F,T,M
            lua_call(L,2,1); //G
            GhostSprite *ghost=(GhostSprite *)lua_touserdata(L,-1);
            lua_pop(L,1);
            ghosts.push_back(ghost);
        }
        sprite->setGhosts(new std::vector<GhostSprite *>(ghosts.cbegin(),ghosts.cend()));
    }

    return 0;
}

int SpriteBinder::lookAt(lua_State* L)
{
    StackChecker checker(L, "SpriteBinder::lookAt", 0);

    Binder binder(L);
    Sprite *ref=NULL;
    int nargs=lua_gettop(L);
    int as=2;
    if (nargs>8)
        as+=3;
    if (nargs>=(as+6))
        ref = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, as+6));
    Sprite* sprite = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 1));
    Sprite *parent=sprite->parent();
    if (ref&&(parent==nullptr)) return 0;

    float ex=sprite->x();
    float ey=sprite->y();
    float ez=sprite->z();
    if (as>2)
    {
        ex=luaL_checknumber(L,2);
        ey=luaL_checknumber(L,3);
        ez=luaL_checknumber(L,4);
    }
    float cx=luaL_checknumber(L,as);
    float cy=luaL_checknumber(L,as+1);
    float cz=luaL_checknumber(L,as+2);
    float ux=luaL_checknumber(L,as+3);
    float uy=luaL_checknumber(L,as+4);
    float uz=luaL_checknumber(L,as+5);
    if (ref) {
        if (as>2)
            parent->spriteToLocal(ref,ex,ey,ez,&ex,&ey,&ez);
        parent->spriteToLocal(ref,cx,cy,cz,&cx,&cy,&cz);
        parent->spriteToLocal(ref,ux,uy,uz,&ux,&uy,&uz);
    }
    sprite->lookAt(ex,ey,ez,cx,cy,cz,ux,uy,uz);

    return 0;
}

void SpriteBinder::__parseGhost(GhostSprite *g,lua_State* L)
{
    Binder binder(L);
    lua_rawgetfield(L,1,"transform");
    if (lua_istable(L,-1)) {
        Transform* matrix = static_cast<Transform*>(binder.getInstance("Matrix", -1));
        g->matrix=new float[16];
        memcpy(g->matrix,matrix->matrix().data(),sizeof(float)*16);
    }

    lua_rawgetfield(L,1,"gridx");
    int gridx=lua_tonumber(L,-1);
    lua_rawgetfield(L,1,"gridy");
    int gridy=lua_tonumber(L,-1);
    lua_pop(L,2);
    g->gridx=gridx;
    g->gridy=gridy;
    // Children
    lua_rawgetfield(L,1,"children");
    if (lua_istable(L,-1))
    {
        int nghosts=lua_objlen(L,-1);
        lua_rawgettoken(L,2,SpriteBinder::tokenChildren);
        if (lua_istable(L,-1))
        {
            for (int i=0;i<nghosts;i++) {
                lua_rawgeti(L,-2,i+1); //GC,MC,T
                luaL_checktype(L,-1,LUA_TTABLE);
                Sprite* child = g->getModel()->getChildAt(i);
                if (child) {
                    lua_pushlightuserdata(L, child); //GC,MC,T,UC
                    lua_rawget(L, -3); //GC,MC,T,M
                    lua_getfield(L,-1,"__parseGhosts"); //T,M,F
                    luaL_checktype(L,-1,LUA_TFUNCTION);
                    lua_insert(L,-3); //F,T,M
                    lua_call(L,2,1); //G
                    GhostSprite *ghost=(GhostSprite *)lua_touserdata(L,-1);
                    lua_pop(L,1);
                    if (!g->children) g->children=new std::vector<GhostSprite *>();
                    g->children->push_back(ghost);
                }
                else
                    lua_pop(L,1);
            }
        }
        lua_pop(L,1);
    }
    lua_pop(L,1);
}

int SpriteBinder::__parseGhosts(lua_State* L)
{
    Binder binder(L);
    Sprite* model = static_cast<Sprite*>(binder.getInstanceOfType("Sprite", GREFERENCED_TYPEMAP_SPRITE, 2));
    GhostSprite *ghost=new GhostSprite(model);
    __parseGhost(ghost,L);
    lua_pushlightuserdata(L,ghost);
    return 1;
}

