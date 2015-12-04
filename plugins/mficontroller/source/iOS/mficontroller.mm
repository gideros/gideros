/*
 
 This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
 (C) 2015 Antonio De Calatrava
 
 */



#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"

#include <GameController/GameController.h>

static NSString * const kGameToggleMenuNotification = @"GameToggleMenuNotification";

static const char KEY_OBJECTS = ' ';

static const char* BUTTON_EVENT = "button";
static const char* CONNECTED_GC_EVENT = "connectedGameController";
static const char* DISCONNECTED_GC_EVENT = "disConnectedGameController";
static const char* PAUSE_EVENT = "pause";
static const char* MENU_EVENT = "menu";
static const char* ANALOG_EVENT = "analog";

static const int BUTTON_A = 1;
static const int BUTTON_B = 2;
static const int BUTTON_X = 3;
static const int BUTTON_Y = 4;

static const int BUTTON_LEFT_SHOULDER = 10;
static const int BUTTON_RIGHT_SHOULDER = 11;

static const int DPAD_UP = 20;
static const int DPAD_DOWN = 21;
static const int DPAD_LEFT = 22;
static const int DPAD_RIGHT = 23;


#pragma mark GGameController
class GGameController;

struct GGameControllerEvent
{
    GGameControllerEvent()
    {
    }
    
    GGameControllerEvent(GGameController *gameController,
                         char const *type,
                         NSError *error,
                         NSInteger controllerIndex,
                         NSInteger buttonIndex,
                         BOOL isPressed) :
    gameController(gameController),
    type(type),
    error(error),
    controllerIndex(controllerIndex),
    buttonIndex(buttonIndex),
    isPressed(isPressed)
    {
    }
    
    GGameController *gameController;
    char const *type;
    NSError *error;
    NSInteger controllerIndex;
    NSInteger buttonIndex;
    BOOL isPressed;
};

#pragma mark GGameControllerEventPerformer
@interface GGameControllerEventPerformer : NSObject
{
    GGameControllerEvent event;
}

- (id)init:(GGameControllerEvent)event;

@end

#pragma mark GGameControllerHelper
@interface GGameControllerHelper : NSObject

@property (nonatomic, strong) GCController *controller;
@property (nonatomic, strong) NSArray *controllers;
@property (nonatomic, strong) id connectObserver;
@property (nonatomic, strong) id disconnectObserver;
@property (nonatomic, strong) id pauseToggleObserver;

@end

@implementation GGameControllerHelper

- (BOOL)isAvailable {
    NSLog(@"count GCCOntroller %lu", (unsigned long)[[GCController controllers] count]);
    
    if ([[GCController controllers] count]) {
        self.controllers = [GCController controllers];
        self.controller = [GCController controllers][0];
        return YES;
    }
    
    return NO;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self.pauseToggleObserver];
    [super dealloc];
}

- (NSInteger)amountControllers {
    if ([[GCController controllers] count]) {
        return [[GCController controllers] count];
    }
    
    return 0;
}

- (GCController *)getControllerAtIndex:(NSInteger)index {
    if (index >= [[GCController controllers] count]) return nil;
    return [GCController controllers][index];
}

- (void)activateExtendedController:(GCController *)controller onGGameController:(GGameController *)gameController {
    int a = (int)[self.controllers indexOfObject:controller];
#ifdef __IPHONE_9_0
    [controller setPlayerIndex:(GCControllerPlayerIndex)a];
#else
    [controller setPlayerIndex:(NSInteger)a];
#endif
    
    
    __weak typeof(self) weakself = self;
    self.pauseToggleObserver = [[NSNotificationCenter defaultCenter] addObserverForName:kGameToggleMenuNotification object:nil queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification *note) {
        NSLog(@"toggleMenu");
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, MENU_EVENT, nil, a, 0, YES)] autorelease];
    }];

    NSLog(@"activateExtendedController GC index %i",a);
    
    [[controller extendedGamepad].buttonA setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_A, pressed)] autorelease];
    }];
    [[controller extendedGamepad].buttonB setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_B, pressed)] autorelease];
    }];
    [[controller extendedGamepad].buttonX setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_X, pressed)] autorelease];
    }];
    [[controller extendedGamepad].buttonY setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_Y, pressed)] autorelease];
    }];
    
    [[controller extendedGamepad].leftShoulder setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_LEFT_SHOULDER, pressed)] autorelease];
    }];
    [[controller extendedGamepad].rightShoulder setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_RIGHT_SHOULDER, pressed)] autorelease];
    }];
    
    [[controller extendedGamepad].dpad.up setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_UP, pressed)] autorelease];
    }];
    [[controller extendedGamepad].dpad.down setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_DOWN, pressed)] autorelease];
    }];
    [[controller extendedGamepad].dpad.left setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_LEFT, pressed)] autorelease];
    }];
    [[controller extendedGamepad].dpad.right setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_RIGHT, pressed)] autorelease];
    }];
    
    [controller setControllerPausedHandler:^(GCController *controller) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, PAUSE_EVENT, nil, a, 0, FALSE)] autorelease];
    }];
    
}

- (void)activateStandardController:(GCController *)controller onGGameController:(GGameController *)gameController {
    int a = (int)[self.controllers indexOfObject:controller];
#ifdef __IPHONE_9_0
    [controller setPlayerIndex:(GCControllerPlayerIndex)a];
#else
    [controller setPlayerIndex:(NSInteger)a];
#endif
    
    NSLog(@"activateStandardController GC index %i",a);
    
    __weak typeof(self) weakself = self;
    self.pauseToggleObserver = [[NSNotificationCenter defaultCenter] addObserverForName:kGameToggleMenuNotification object:nil queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification *note) {
        NSLog(@"toggleMenu");
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, MENU_EVENT, nil, a, 0, YES)] autorelease];
    }];

    [controller.gamepad.buttonA setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_A, pressed)] autorelease];
    }];
    [controller.gamepad.buttonB setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_B, pressed)] autorelease];
    }];
    [controller.gamepad.buttonX setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_X, pressed)] autorelease];
    }];
    [controller.gamepad.buttonY setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_Y, pressed)] autorelease];
    }];
    
    [controller.gamepad.leftShoulder setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_LEFT_SHOULDER, pressed)] autorelease];
    }];
    [controller.gamepad.rightShoulder setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_RIGHT_SHOULDER, pressed)] autorelease];
    }];
    
    [controller.gamepad.dpad.up setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_UP, pressed)] autorelease];
    }];
    [controller.gamepad.dpad.down setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_DOWN, pressed)] autorelease];
    }];
    [controller.gamepad.dpad.left setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_LEFT, pressed)] autorelease];
    }];
    [controller.gamepad.dpad.right setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_RIGHT, pressed)] autorelease];
    }];
    
    [controller setControllerPausedHandler:^(GCController *controller) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, PAUSE_EVENT, nil, a, 0, FALSE)] autorelease];
    }];
    
}

#if TARGET_OS_TV==1
- (void)activateMicroController:(GCController *)controller onGGameController:(GGameController *)gameController {
    int a = (int)[self.controllers indexOfObject:controller];
    [controller setPlayerIndex:(GCControllerPlayerIndex)a];
    [controller.microGamepad setAllowsRotation:YES];
    
    NSLog(@"activateMicroGamepadController GC index %i",a);
    
    __weak typeof(self) weakself = self;
    self.pauseToggleObserver = [[NSNotificationCenter defaultCenter] addObserverForName:kGameToggleMenuNotification object:nil queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification *note) {
        NSLog(@"toggleMenu");
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, MENU_EVENT, nil, a, 0, YES)] autorelease];
    }];

    [controller.microGamepad.buttonA setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_A, pressed)] autorelease];
    }];
    [controller.microGamepad.buttonX setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, BUTTON_X, pressed)] autorelease];
    }];
    
    [controller.microGamepad.dpad.up setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_UP, pressed)] autorelease];
    }];
    [controller.microGamepad.dpad.down setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_DOWN, pressed)] autorelease];
    }];
    [controller.microGamepad.dpad.left setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_LEFT, pressed)] autorelease];
    }];
    [controller.microGamepad.dpad.right setValueChangedHandler:^(GCControllerButtonInput *button, float value, BOOL pressed) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, BUTTON_EVENT, nil, a, DPAD_RIGHT, pressed)] autorelease];
    }];
    
    [controller setControllerPausedHandler:^(GCController *controller) {
        [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(gameController, PAUSE_EVENT, nil, a, 0, FALSE)] autorelease];
    }];
    
}
#endif

- (NSString *)getTypeOfController:(GCController *)controller {
    
#if TARGET_OS_TV==1
    if ([controller respondsToSelector:@selector(microGamepad)] && controller.microGamepad) {
        return @"MICRO_GAMEPAD";
    }
#endif
    
#ifdef __IPHONE_8_0
    if ([controller respondsToSelector:@selector(motion)] && controller.motion) {
        return @"MOTION_CONTROLLER";
    }
#endif
    
    if ([controller respondsToSelector:@selector(extendedGamepad)] && controller.extendedGamepad) {
        return @"EXTENDED_GAMEPAD";
    }
    
    if ([controller respondsToSelector:@selector(gamepad)] && controller.gamepad) {
        return @"GAMEPAD";
    }
    
    
    
    return @"UNKNOWN";
}

- (void) captureMenuButton {
    UIViewController * mainVC = [[[[UIApplication sharedApplication] delegate] window] rootViewController];
    if ([mainVC respondsToSelector:@selector(captureMenuButton)]) {
        [mainVC captureMenuButton];
    }
}

- (void) releaseMenuButton {
    UIViewController * mainVC = [[[[UIApplication sharedApplication] delegate] window] rootViewController];
    if ([mainVC respondsToSelector:@selector(releaseMenuButton)]) {
        [mainVC releaseMenuButton];
    }
}

@end



class GGameController : public GEventDispatcherProxy
{
public:
    GGameController(lua_State* L) : L(L)
    {
        helper = [[GGameControllerHelper alloc] init];
        
        helper.connectObserver = [[NSNotificationCenter defaultCenter] addObserverForName:GCControllerDidConnectNotification object:nil queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification *note) {
            //if ([[GCController controllers] count] > 0) {
            NSLog(@"New GC connected");
            [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(this, CONNECTED_GC_EVENT, nil, 0, 0, FALSE)] autorelease];
            //}
            if ([[helper getControllerAtIndex:[helper amountControllers]-1] extendedGamepad]) {
                [helper activateExtendedController:[helper getControllerAtIndex:[helper amountControllers]-1] onGGameController:this];
            }
#if TARGET_OS_TV == 1
            else if ([[helper getControllerAtIndex:[helper amountControllers] -1] microGamepad]) {
                [helper activateMicroController:[helper getControllerAtIndex:[helper amountControllers]-1] onGGameController:this];
            }
#endif
            else
            {
                [helper activateStandardController:[helper getControllerAtIndex:[helper amountControllers]-1] onGGameController:this];
            }
            
        }];
        helper.disconnectObserver = [[NSNotificationCenter defaultCenter] addObserverForName:GCControllerDidDisconnectNotification object:nil queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification *note) {
            //if (![[GCController controllers] count]) {
            NSLog(@"GC disconnected");
            [[[GGameControllerEventPerformer alloc] init:GGameControllerEvent(this, DISCONNECTED_GC_EVENT, nil, 0, 0, FALSE)] autorelease];
            //}
        }];
        
        activate();
        
    }
    
    GGameController()
    {
        [helper release];
    }
    
    BOOL isAvailable()
    {
        return [helper isAvailable];
    }
    
    void activate()
    {
        if (isAvailable() == NO)
            return;
        
        for (int a = 0; a<[helper amountControllers]; a++) {
            
            if ([[helper getControllerAtIndex:a] extendedGamepad]) {
                [helper activateExtendedController:[helper getControllerAtIndex:a] onGGameController:this];
            }
#if TARGET_OS_TV==1
            else if ([[helper getControllerAtIndex:a] microGamepad]) {
                [helper activateMicroController:[helper getControllerAtIndex:a] onGGameController:this];
            }
#endif
            else
            {
                [helper activateStandardController:[helper getControllerAtIndex:a] onGGameController:this];
            }
            
            //if ([helper getControllerAtIndex:a].motion) {
            //NSLog(@"motion available at controller %i",a);
            //}
            
        }
        
        
    }
    
    int getAmountControllers()
    {
        if (isAvailable() == NO) return 0;
        return (int)[helper amountControllers];
    }
    
    const char * getControllerType(int controllerIndex)
    {
        return [[helper getTypeOfController:[helper getControllerAtIndex:controllerIndex]] UTF8String];
    }
    
    void captureMenuButton()
    {
        [helper captureMenuButton];
    }
    
    void releaseMenuButton()
    {
        [helper releaseMenuButton];
    }
    
    
    void dispatchEvent(char const *type, NSError *error, NSInteger controllerIndex, NSInteger buttonIndex, BOOL isPressed)
    {
        lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
        lua_rawget(L, LUA_REGISTRYINDEX);
        
        lua_rawgeti(L, -1, 1);
        
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
        
        if (buttonIndex)
        {
            lua_pushinteger(L, controllerIndex);
            lua_setfield(L, -2, "controllerIndex");
            
            lua_pushinteger(L, buttonIndex);
            lua_setfield(L, -2, "buttonIndex");
            
            lua_pushboolean(L, (isPressed ? 1 : 0));
            lua_setfield(L, -2, "isPressed");
        }
        
        if (strcmp(type, PAUSE_EVENT)==0) {
            lua_pushinteger(L, controllerIndex);
            lua_setfield(L, -2, "controllerIndex");
        }
        
        if (lua_pcall(L, 2, 0, 0) != 0)
            g_error(L, lua_tostring(L, -1));
        
        lua_pop(L, 2);
        
    }
    
private:
    lua_State* L;
    GGameControllerHelper* helper;
    
};

@implementation GGameControllerEventPerformer

- (id)init:(GGameControllerEvent)theEvent
{
    if (self = [super init])
    {
        self->event = theEvent;
        [self performSelectorOnMainThread:@selector(performEvent) withObject:nil waitUntilDone:YES];
    }
    
    return self;
}

- (void)performEvent
{
    event.gameController->dispatchEvent(event.type,
                                        event.error,
                                        event.controllerIndex,
                                        event.buttonIndex,
                                        event.isPressed);
}

@end

static int destruct(lua_State* L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    GReferenced* object = static_cast<GReferenced*>(ptr);
    GGameController* gamecontroller = static_cast<GGameController*>(object->proxy());
    
    gamecontroller->unref();
    
    return 0;
}

static GGameController* getInstance(lua_State* L, int index)
{
    GReferenced* object = static_cast<GReferenced*>(g_getInstance(L, "MFIController", index));
    GGameController* gamecontroller = static_cast<GGameController*>(object->proxy());
    
    return gamecontroller;
}

static int isAvailable(lua_State* L)
{
    GGameController* gamecontroller = getInstance(L, 1);
    lua_pushboolean(L, gamecontroller->isAvailable());
    
    return 1;
}

static int activate(lua_State* L)
{
    GGameController* gamecontroller = getInstance(L, 1);
    gamecontroller->activate();
    
    return 0;
}

static int getAmountControllers(lua_State* L)
{
    GGameController* gamecontroller = getInstance(L, 1);
    lua_pushinteger(L, gamecontroller->getAmountControllers());
    
    return 1;
}

static int getControllerType(lua_State *L)
{
    GGameController* gamecontroller = getInstance(L, 1);
    lua_pushstring(L, gamecontroller->getControllerType(lua_tonumber(L, 2)));
    return 1;
}

static int captureMenuButton(lua_State *L)
{
    GGameController* gamecontroller = getInstance(L, 1);
    gamecontroller->captureMenuButton();
    return 1;
}

static int releaseMenuButton(lua_State *L)
{
    GGameController* gamecontroller = getInstance(L, 1);
    gamecontroller->releaseMenuButton();
    return 1;
}

static int loader(lua_State *L)
{
    //This is a list of functions that can be called from Lua
    const luaL_Reg functionlist[] = {
        {"isAvailable", isAvailable},
        {"activate", activate},
        {"getAmountControllers", getAmountControllers},
        {"getControllerType", getControllerType},
        {"captureMenuButton", captureMenuButton},
        {"releaseMenuButton", releaseMenuButton},
        {NULL, NULL},
    };
    
    g_createClass(L, "MFIController", "EventDispatcher", NULL, destruct, functionlist);
    
    lua_getglobal(L, "Event");
    lua_pushstring(L, BUTTON_EVENT);
    lua_setfield(L, -2, "BUTTON_EVENT");
    lua_pushstring(L, CONNECTED_GC_EVENT);
    lua_setfield(L, -2, "CONNECTED_GC_EVENT");
    lua_pushstring(L, DISCONNECTED_GC_EVENT);
    lua_setfield(L, -2, "DISCONNECTED_GC_EVENT");
    lua_pushstring(L, PAUSE_EVENT);
    lua_setfield(L, -2, "PAUSE_EVENT");
    lua_pushstring(L, MENU_EVENT);
    lua_setfield(L, -2, "MENU_EVENT");
    lua_pop(L, 1);
    
    lua_newtable(L);
    lua_pushnumber(L, BUTTON_A);
    lua_setfield(L, -2, "BUTTON_A");
    lua_pushnumber(L, BUTTON_B);
    lua_setfield(L, -2, "BUTTON_B");
    lua_pushnumber(L, BUTTON_X);
    lua_setfield(L, -2, "BUTTON_X");
    lua_pushnumber(L, BUTTON_RIGHT_SHOULDER);
    lua_setfield(L, -2, "BUTTON_RIGHT_SHOULDER");
    lua_pushnumber(L, BUTTON_LEFT_SHOULDER);
    lua_setfield(L, -2, "BUTTON_LEFT_SHOULDER");
    lua_pushnumber(L, DPAD_UP);
    lua_setfield(L, -2, "DPAD_UP");
    lua_pushnumber(L, DPAD_DOWN);
    lua_setfield(L, -2, "DPAD_DOWN");
    lua_pushnumber(L, DPAD_LEFT);
    lua_setfield(L, -2, "DPAD_LEFT");
    lua_pushnumber(L, DPAD_RIGHT);
    lua_setfield(L, -2, "DPAD_RIGHT");
    lua_pushvalue(L, -1);
    lua_setglobal(L, "ButtonCode");
    
    // create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of KEY_OBJECTS
    lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
    lua_newtable(L);                  // create a table
    lua_pushliteral(L, "v");
    lua_setfield(L, -2, "__mode");    // set as weak-value table
    lua_pushvalue(L, -1);             // duplicate table
    lua_setmetatable(L, -2);          // set itself as metatable
    lua_rawset(L, LUA_REGISTRYINDEX);
    
    
    GGameController* gamecontroller = new GGameController(L);
    g_pushInstance(L, "MFIController", gamecontroller->object());
    
    lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, -2);
    lua_rawseti(L, -2, 1);
    lua_pop(L, 1);
    
    lua_pushvalue(L, -1);
    lua_setglobal(L, "mficontroller");
    
    
    //return the pointer to the plugin
    return 1;
}
static void g_initializePlugin(lua_State* L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    
    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "mficontroller");
    
    lua_pop(L, 2);
}
static void g_deinitializePlugin(lua_State *L)
{   }
REGISTER_PLUGIN("MFIControllerPlugin", "1.0")


