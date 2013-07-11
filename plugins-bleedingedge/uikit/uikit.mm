#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"

static const char KEY_OBJECTS = ' ';

// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of KEY_OBJECTS
static void createObjectsTable(lua_State* L)
{
	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_newtable(L);                  // create a table
	lua_pushliteral(L, "v");
	lua_setfield(L, -2, "__mode");    // set as weak-value table
	lua_pushvalue(L, -1);             // duplicate table
	lua_setmetatable(L, -2);          // set itself as metatable
	lua_rawset(L, LUA_REGISTRYINDEX);	
}

static void setObject(lua_State* L, void* ptr)
{
	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, ptr);
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 1);
}

static void getObject(lua_State* L, void* ptr)
{
	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, ptr);
	lua_rawget(L, -2);
	lua_remove(L, -2);
}

class View : public GEventDispatcherProxy
{
public:
	View(lua_State* L)
	{
	
	}
	
	virtual ~View()
	{
		[uiView release];
	}
	
	virtual void create()
	{	
		uiView = [[UIView alloc] init];
	}

	void addView(View* view)
	{
		[uiView addSubview:view->uiView];
	}
	
	void removeFromParent()
	{
		[uiView removeFromSuperview];
	}
	
	void setPosition(int x, int y)
	{
		CGRect frame = uiView.frame;
		frame.origin.x = x;
		frame.origin.y = y;
		uiView.frame = frame;	
	}
	
	void setSize(int width, int height)
	{
		CGRect frame = uiView.frame;
		frame.size.width = width;
		frame.size.height = height;
		uiView.frame = frame;			
	}
	
	UIView* uiView;
};


class RootView : public View
{
public:
	RootView(lua_State* L) : View(L)
	{
		
	}
	
	virtual ~RootView()
	{
		[uiView release];
	}
	
	virtual void create()
	{	
		UIViewController* controller = g_getRootViewController();	
		uiView = controller.view;
		[uiView retain];
	}
};

@interface ButtonTarget : NSObject
{
	GReferenced* target;
	lua_State* L;
}

@property (nonatomic, assign) GReferenced* target;
@property (nonatomic, assign) lua_State* L;

-(void)event:(id)sender;

@end

@implementation ButtonTarget

@synthesize target;
@synthesize L;

-(void)event:(id)sender
{
	getObject(L, target);

	if (!lua_isnil(L, -1))
	{
		lua_getfield(L, -1, "dispatchEvent");
		
		lua_pushvalue(L, -2);
		
		lua_getglobal(L, "Event");
		lua_getfield(L, -1, "new");
		lua_remove(L, -2);
		
		lua_pushstring(L, "click");
		lua_call(L, 1, 1);

		if (lua_pcall(L, 2, 0, 0) != 0)
		{
			g_error(L, lua_tostring(L, -1));
			return;
		}
	}
	
	lua_pop(L, 1);

}

@end

class Button : public View
{
public:
	Button(lua_State* L) : View(L)
	{
		target = [[ButtonTarget alloc] init];
	}
	
	virtual ~Button()
	{
		[target release];		
	}

	virtual void create()	
	{
		UIButton* button = [[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
		[button addTarget:target action:@selector(event:) forControlEvents:UIControlEventTouchUpInside];
		uiView = button;
	}	

	void setTitle(NSString* title)
	{
		[(UIButton*)uiView setTitle:title forState:UIControlStateNormal];
	}
	
private:
	ButtonTarget* target;
};


class ViewBinder
{
public:
	ViewBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);

	static int addView(lua_State* L);
	static int removeFromParent(lua_State* L);
	static int setPosition(lua_State* L);
	static int setSize(lua_State* L);
};

ViewBinder::ViewBinder(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{"addView", addView},
		{"removeFromParent", removeFromParent},
		{"setPosition", setPosition},
		{"setSize", setSize},
		{NULL, NULL},
	};
	
	g_createClass(L, "ui.View", "EventDispatcher", create, destruct, functionlist);
}

int ViewBinder::create(lua_State* L)
{
	View* view = new View(L);
	view->create();
	
	g_pushInstance(L, "ui.View", view->object());
	
	setObject(L, view);
	
	return 1;
}

int ViewBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	View* view = static_cast<View*>(object->proxy());
	
	view->unref(); 
	
	return 0;
}


int ViewBinder::addView(lua_State* L)
{
	GReferenced* viewObject = static_cast<GReferenced*>(g_getInstance(L, "View", 1));
	GReferenced* childViewObject = static_cast<GReferenced*>(g_getInstance(L, "View", 2));
	
	View* view = static_cast<View*>(viewObject->proxy());
	View* childView = static_cast<View*>(childViewObject->proxy());
	
	if (childView->uiView.superview != nil)
	{
		// TODO
		return luaL_error(L, "already have parent hebe gube");
	}
	
	if ([view->uiView isDescendantOfView:childView->uiView] == YES)
	{
		return luaL_error(L, "cannot add hebe gube");
	}
	
	lua_getfield(L, 1, "__children");
	if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setfield(L, 1, "__children");
	}
	
	lua_pushlightuserdata(L, childView);
	lua_pushvalue(L, 2);
	lua_settable(L, -3);
	lua_pop(L, 1);
		
	view->addView(childView);
	
	return 0;
}

int ViewBinder::removeFromParent(lua_State* L)
{
	GReferenced* viewObject = static_cast<GReferenced*>(g_getInstance(L, "View", 1));
	View* view = static_cast<View*>(viewObject->proxy());
	
	lua_getfield(L, 1, "__parent");
	if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);
		return 0;		
	}
	
	lua_getfield(L, -1, "__children");
	lua_pushlightuserdata(L, view);
	lua_pushnil(L);
	lua_settable(L, -3);

	lua_pop(L, 2);

	view->removeFromParent();
	
	return 0;
}

int ViewBinder::setPosition(lua_State* L)
{
	GReferenced* viewObject = static_cast<GReferenced*>(g_getInstance(L, "View", 1));
	View* view = static_cast<View*>(viewObject->proxy());
	
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	view->setPosition(x, y);
	
	return 0;
}

int ViewBinder::setSize(lua_State* L)
{
	GReferenced* viewObject = static_cast<GReferenced*>(g_getInstance(L, "View", 1));
	View* view = static_cast<View*>(viewObject->proxy());
	
	int width = luaL_checkinteger(L, 2);
	int height = luaL_checkinteger(L, 3);
	view->setSize(width, height);
	
	return 0;	
}

class RootViewBinder
{
public:
	RootViewBinder(lua_State* L);
	
private:
	static int destruct(lua_State* L);
};

RootViewBinder::RootViewBinder(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{NULL, NULL},
	};
	
	g_createClass(L, "ui.RootView", "ui.View", NULL, destruct, functionlist);	
	
	RootView* rootView = new RootView(L);
	rootView->create();
	
	lua_getglobal(L, "ui");
	g_pushInstance(L, "ui.RootView", rootView->object());
	lua_setfield(L, -2, "rootView");
	lua_pop(L, 1);
}

int RootViewBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	RootView* rootView = static_cast<RootView*>(object->proxy());
	
	rootView->unref(); 
	
	return 0;
}

class ButtonBinder
{
public:
	ButtonBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);

	static int setTitle(lua_State* L);
};


ButtonBinder::ButtonBinder(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{"setTitle", setTitle},
		{NULL, NULL},
	};
	
	g_createClass(L, "ui.Button", "ui.View", create, destruct, functionlist);	
}

int ButtonBinder::create(lua_State* L)
{
	Button* button = new Button(L);
	button->create();
	
	g_pushInstance(L, "ui.Button", button->object());
	
	setObject(L, button);
	
	return 1;
}

int ButtonBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	Button* button = static_cast<Button*>(object->proxy());
	
	button->unref(); 
	
	return 0;
}

int ButtonBinder::setTitle(lua_State* L)
{
	GReferenced* buttonObject = static_cast<GReferenced*>(g_getInstance(L, "ui.Button", 1));
	Button* button = static_cast<Button*>(buttonObject->proxy());
	
	const char* title = luaL_checkstring(L, 2);
	button->setTitle([NSString stringWithUTF8String:title]);
	
	return 0;
}


static int loader(lua_State* L)
{
	lua_newtable(L);

	lua_pushvalue(L, -1);
	lua_setglobal(L, "ui");

	ViewBinder viewBinder(L);
	RootViewBinder rootViewBinder(L);
	ButtonBinder buttonBinder(L);

	createObjectsTable(L);

	return 1;
}

static void g_initializePlugin(lua_State* L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "ui");
	
	lua_pop(L, 2);	
}

static void g_deinitializePlugin(lua_State *L)
{
	
}

REGISTER_PLUGIN("UIKit", "1.0")