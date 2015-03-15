#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gglobal.h>
#include <gevent.h>
#include <string>

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

G_API g_id ghid_addCallback(gevent_Callback callback, void *udata);
G_API void ghid_removeCallback(gevent_Callback callback, void *udata);
G_API void ghid_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif