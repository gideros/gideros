#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gglobal.h>
#include <gevent.h>

static const int BUTTON_B = 97;
static const int DPAD_DOWN = 20;
static const int DPAD_LEFT = 21;
static const int DPAD_RIGHT = 22;
static const int DPAD_UP = 19;
static const int BUTTON_L1 = 102;
static const int BUTTON_L2 = 104;
static const int BUTTON_L3 = 106;
static const int BUTTON_MENU = 82;
static const int BUTTON_START = 108;
static const int BUTTON_BACK = 4;
static const int BUTTON_A = 96;
static const int BUTTON_R1 = 103;
static const int BUTTON_R2 = 105;
static const int BUTTON_R3 = 107;
static const int BUTTON_X = 99;
static const int BUTTON_Y = 100;

typedef struct ghid_KeyEvent
{
	int keyCode;
	int playerId;
} ghid_KeyEvent;

typedef struct ghid_JoystickEvent
{
	float x;
	float y;
	double angle;
	double strength;
	int playerId;
} ghid_JoystickEvent;

typedef struct ghid_TriggerEvent
{
	double strength;
	int playerId;
} ghid_TriggerEvent;

typedef struct ghid_DeviceEvent
{
	int playerId;
} ghid_DeviceEvent;

enum
{
	GHID_KEY_DOWN_EVENT,
	GHID_KEY_UP_EVENT,
	GHID_RIGHT_JOYSTICK_EVENT,
	GHID_LEFT_JOYSTICK_EVENT,
	GHID_RIGHT_TRIGGER_EVENT,
	GHID_LEFT_TRIGGER_EVENT,
	GHID_CONNECTED_EVENT,
	GHID_DISCONNECTED_EVENT
};


#ifdef __cplusplus
extern "C" {
#endif

G_API void ghid_init();
G_API void ghid_cleanup();

G_API int ghid_isAnyAvailable();
G_API int ghid_getPlayerCount();
G_API const char* ghid_getControllerName(int player);
G_API void ghid_vibrate(int player, long ms);
G_API int* ghid_getPlayers(int* size);
    
G_API void ghid_onKeyDownEvent(int keyCode, int playerId);
G_API void ghid_onKeyUpEvent(int keyCode, int playerId);
G_API void ghid_onRightJoystick(float x, float y, double angle, double strength, int playerId);
G_API void ghid_onLeftJoystick(float x, float y, double angle, double strength, int playerId);
G_API void ghid_onRightTrigger(double strength, int playerId);
G_API void ghid_onLeftTrigger(double strength, int playerId);
G_API void ghid_onConnected(int playerId);
G_API void ghid_onDisconnected(int playerId);

G_API g_id ghid_addCallback(gevent_Callback callback, void *udata);
G_API void ghid_removeCallback(gevent_Callback callback, void *udata);
G_API void ghid_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif