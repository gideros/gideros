/*
 
 This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
 (C) 2010 - 2012 Gideros Mobile 
 
 */

#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"

#import <iAd/iAd.h>

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


/*
 we have two tables:

 1.strong (table -> boolean)
 show -> add, hide - > remove
 to disable GC for visible banners

 2. weak (ptr -> table)
 to access table from pointer
 
 */

static char keyStrong = ' ';
static char keyWeak = ' ';

static const char *BANNER_ACTION_BEGIN = "bannerActionBegin";
static const char *BANNER_ACTION_FINISHED = "bannerActionFinished";
static const char *BANNER_AD_LOADED = "bannerAdLoaded";
static const char *BANNER_AD_FAILED = "bannerAdFailed";

static const char *TOP = "top";
static const char *BOTTOM = "bottom";
static const char *PORTRAIT = "portrait";
static const char *LANDSCAPE = "landscape";

const int kTOP = 0;
const int kBOTTOM = 1;
const int kPORTRAIT = 0;
const int kLANDSCAPE = 1;

/* 
 require "iad"
 
 iad.isAvailable()
  
 local banner = iad.Banner.new(iad.Banner.TOP, iad.Banner.PORTRAIT)
 banner:addEventListener(Event.BANNER_ACTION_BEGIN, ...)
 banner:addEventListener(Event.BANNER_ACTION_FINISHED, ...)
 banner:addEventListener(Event.BANNER_AD_LOADED, ...)
 banner:addEventListener(Event.BANNER_AD_FAILED, ...)
 banner:show()
 banner:hide()
 banner:setAlignment(iad.Banner.BOTTOM)
*/

static bool isiAdAvailable()
{
	return NSClassFromString(@"ADBannerView") != nil;
}

static NSString *getContentSizeIdentifierPortrait()
{
    return ADBannerContentSizeIdentifierPortrait;
}

static NSString *getContentSizeIdentifierLandscape()
{
    return ADBannerContentSizeIdentifierLandscape;
}

class Banner;

@interface BannerDelegate : NSObject<ADBannerViewDelegate>
{
}

- (id)initWithBanner:(Banner *)banner;

@property (nonatomic, assign) Banner *banner;

@end


class Banner : public GEventDispatcherProxy
{
public:
	Banner(lua_State *L, int alignment, int orientation) : L(L)
	{
		alignment_ = alignment;
		orientation_ = orientation;

		view_ = [[ADBannerView alloc] initWithFrame:CGRectZero];

		if (orientation_ == kPORTRAIT)
			view_.currentContentSizeIdentifier = getContentSizeIdentifierPortrait();
		else
			view_.currentContentSizeIdentifier = getContentSizeIdentifierLandscape();
		
		delegate_ = [[BannerDelegate alloc] initWithBanner:this];
		view_.delegate = delegate_;

		showWhenAvailable_ = false;
	}

	virtual ~Banner()
	{
		view_.delegate = nil;
        delegate_.banner = NULL;
		[view_ cancelBannerViewAction];
		[view_ removeFromSuperview];
		[view_ release];
		[delegate_ release];
	}
	
	void show()
	{
		showWhenAvailable_ = true;
		
		if (view_.bannerLoaded)
		{
			if (view_.superview == nil)
			{
				UIViewController *viewController = g_getRootViewController();
				[viewController.view addSubview:view_];
			}

			updateFramePosition();
		}
	}
	
	void hide()
	{
		showWhenAvailable_ = false;
		[view_ removeFromSuperview];
	}
	
	void setAlignment(int alignment)
	{
		alignment_ = alignment;

		if (view_.superview != nil)
			updateFramePosition();
	}
	
	bool isBannerLoaded() const
	{
		return view_.bannerLoaded;
	}

	void adLoaded()
	{
		if (showWhenAvailable_)
			show();

		dispatchEvent(BANNER_AD_LOADED, NULL, NULL);
	}
	
	void actionBegin(BOOL willLeaveApplication)
	{
		dispatchEvent(BANNER_ACTION_BEGIN, NULL, &willLeaveApplication);
	}
	
	void actionFinished()
	{
		dispatchEvent(BANNER_ACTION_FINISHED, NULL, NULL);
	}

	void adFailed(NSError *error)
	{
		[view_ removeFromSuperview];
		
		dispatchEvent(BANNER_AD_FAILED, error, NULL);
	}
	
	void dispatchEvent(const char *type, NSError *error, BOOL *willLeaveApplication)
	{		
        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
        luaL_rawgetptr(L, -1, this);
		
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 2);
            return;
        }

        lua_getfield(L, -1, "dispatchEvent");
		
        lua_pushvalue(L, -2);

        lua_getglobal(L, "Event");
        lua_getfield(L, -1, "new");
        lua_remove(L, -2);
	
        lua_pushstring(L, type);
        lua_call(L, 1, 1);
		
		if (error)
		{
			lua_pushinteger(L, error.code);
			lua_setfield(L, -2, "errorCode");
			
			lua_pushstring(L, [error.localizedDescription UTF8String]);
			lua_setfield(L, -2, "errorDescription");			
		}
		
		if (willLeaveApplication)
		{
			lua_pushboolean(L, *willLeaveApplication);
			lua_setfield(L, -2, "willLeaveApplication");
		}
		
		if (lua_pcall(L, 2, 0, 0) != 0)
			g_error(L, lua_tostring(L, -1));
		
		lua_pop(L, 2);
	}
	
	void updateFramePosition()
	{
		CGRect frame = view_.frame;
		if (alignment_ == kTOP)
		{
			frame.origin = CGPointMake(0, 0);
		}
		else
		{
			int height;
			CGRect screenRect = [[UIScreen mainScreen] bounds];
			if (UIInterfaceOrientationIsPortrait([[UIApplication sharedApplication] statusBarOrientation]))
				height = screenRect.size.height;
			else
				height = screenRect.size.width;
			
			if (orientation_ == kPORTRAIT)
			{
				CGSize size = [ADBannerView sizeFromBannerContentSizeIdentifier:getContentSizeIdentifierPortrait()];
				frame.origin = CGPointMake(0, height - size.height);
			}
			else
			{
				CGSize size = [ADBannerView sizeFromBannerContentSizeIdentifier:getContentSizeIdentifierLandscape()];
				frame.origin = CGPointMake(0, height - size.height);
			}
		}
		view_.frame = frame;		
	}
	
private:
	lua_State *L;
	
	bool showWhenAvailable_;

	BannerDelegate *delegate_;
	ADBannerView *view_;
	
	int alignment_;
	int orientation_;
};

@implementation BannerDelegate

@synthesize banner = banner_;

- (id)initWithBanner:(Banner *)banner
{
	if (self = [super init])
	{
        banner_ = banner;
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationDidChange:) name:UIDeviceOrientationDidChangeNotification object:nil];        
		[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	}
	
	return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIDeviceOrientationDidChangeNotification object:nil];
    [[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];

    [super dealloc];
}

- (void)orientationDidChange:(NSNotification *)notification
{
    if (banner_)
        banner_->updateFramePosition();
}

- (void)bannerViewWillLoadAd:(ADBannerView *)banner
{
}

- (void)bannerViewDidLoadAd:(ADBannerView *)banner
{
    if (banner_)
        banner_->adLoaded();
}

- (BOOL)bannerViewActionShouldBegin:(ADBannerView *)banner willLeaveApplication:(BOOL)willLeave
{
    if (banner_)
        banner_->actionBegin(willLeave);
	return YES;
}

- (void)bannerViewActionDidFinish:(ADBannerView *)banner
{
    if (banner_)
        banner_->actionFinished();
}

- (void)bannerView:(ADBannerView *)banner didFailToReceiveAdWithError:(NSError *)error
{
    if (banner_)
        banner_->adFailed(error);
}

@end

static int create(lua_State *L)
{
	if (!isiAdAvailable())
		return luaL_error(L, "iAd framework is not available.");

	int alignment;
	const char *alignmentstr = luaL_checkstring(L, 1);
	if (strcmp(alignmentstr, TOP) == 0)
		alignment = kTOP;
	else if (strcmp(alignmentstr, BOTTOM) == 0)
		alignment = kBOTTOM;
	else
		return luaL_error(L, "Parameter 'alignment' must be one of the accepted values.");

	int orientation;
	const char *orientationstr = luaL_checkstring(L, 2);
	if (strcmp(orientationstr, PORTRAIT) == 0)
		orientation = kPORTRAIT;
	else if (strcmp(orientationstr, LANDSCAPE) == 0)
		orientation = kLANDSCAPE;
	else
		return luaL_error(L, "Parameter 'orientation' must be one of the accepted values.");
		
	Banner *banner = new Banner(L, alignment, orientation);
	g_pushInstance(L, "iad.Banner", banner->object());
	
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, banner);
	lua_pop(L, 1);
	
	return 1;
}

static int destruct(lua_State *L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced *object = static_cast<GReferenced*>(ptr);
	Banner *banner = static_cast<Banner*>(object->proxy());
	
	banner->unref();
	
	return 0;
}

static Banner *getBannerInstance(lua_State *L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "iad.Banner", index));
	Banner *banner = static_cast<Banner*>(object->proxy());
	
	return banner;
}

static int show(lua_State *L)
{
	Banner *banner = getBannerInstance(L, 1);
	banner->show();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    lua_pushvalue(L, 1);
	lua_pushboolean(L, 1);
    lua_settable(L, -3);
    lua_pop(L, 1);

	return 0;
}

static int hide(lua_State *L)
{
	Banner *banner = getBannerInstance(L, 1);
	banner->hide();
		
    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyStrong);
    lua_pushvalue(L, 1);
	lua_pushnil(L);
    lua_settable(L, -3);
    lua_pop(L, 1);
	
	return 0;
}

static int setAlignment(lua_State *L)
{
	Banner *banner = getBannerInstance(L, 1);
	
	int alignment;
	const char *alignmentstr = luaL_checkstring(L, 2);
	if (strcmp(alignmentstr, TOP) == 0)
		alignment = kTOP;
	else if (strcmp(alignmentstr, BOTTOM) == 0)
		alignment = kBOTTOM;
	else
		return luaL_error(L, "Parameter 'alignment' must be one of the accepted values.");
	
	banner->setAlignment(alignment);

	return 0;
}


static int isBannerLoaded(lua_State* L)
{
	Banner *banner = getBannerInstance(L, 1);
	lua_pushboolean(L, banner->isBannerLoaded());
	
	return 1;
	
}

static int isAvailable(lua_State *L)
{
	lua_pushboolean(L, isiAdAvailable());	
	return 1;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
		{"show", show},
		{"hide", hide},
		{"setAlignment", setAlignment},
		{"isBannerLoaded", isBannerLoaded},
		{NULL, NULL},
	};
	
	g_createClass(L, "iad.Banner", "EventDispatcher", create, destruct, functionlist);

    lua_newtable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyStrong);

    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

	lua_getglobal(L, "Event");
	lua_pushstring(L, BANNER_ACTION_BEGIN);
	lua_setfield(L, -2, "BANNER_ACTION_BEGIN");
	lua_pushstring(L, BANNER_ACTION_FINISHED);
	lua_setfield(L, -2, "BANNER_ACTION_FINISHED");
	lua_pushstring(L, BANNER_AD_LOADED);
	lua_setfield(L, -2, "BANNER_AD_LOADED");
	lua_pushstring(L, BANNER_AD_FAILED);
	lua_setfield(L, -2, "BANNER_AD_FAILED");
	lua_pop(L, 1);
	
	lua_getglobal(L, "iad");
	lua_getfield(L, -1, "Banner");
	lua_pushstring(L, TOP);
	lua_setfield(L, -2, "TOP");
	lua_pushstring(L, BOTTOM);
	lua_setfield(L, -2, "BOTTOM");
	lua_pushstring(L, PORTRAIT);
	lua_setfield(L, -2, "PORTRAIT");
	lua_pushstring(L, LANDSCAPE);
	lua_setfield(L, -2, "LANDSCAPE");
	lua_pop(L, 2);
	
	lua_getglobal(L, "iad");
	lua_pushcfunction(L, isAvailable);
	lua_setfield(L, -2, "isAvailable");
	return 1;
}

static void g_initializePlugin(lua_State* L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "iad");
	
	lua_pop(L, 2);	
}

static void g_deinitializePlugin(lua_State *L)
{
	
}

REGISTER_PLUGIN("iAd", "1.0")
