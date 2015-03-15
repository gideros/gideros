#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gglobal.h>
#include <gevent.h>
#include <string>
#include <map>
#include <vector>

class GController;

class GHID
{
public:
    GHID();
    ~GHID();

    int isAnyAvailable();

    int getPlayerCount();

    const char* getControllerName(int playerId);

    void vibrate(int player, long ms);

    int* getPlayers(int* size);

    void onKeyDownEvent(int keyCode, int playerId);

    void onKeyUpEvent(int keyCode, int playerId);

    void onRightJoystick(float x, float y, double angle, double strength, int playerId);

    void onLeftJoystick(float x, float y, double angle, double strength, int playerId);

    void onRightTrigger(double strength, int playerId);

    void onLeftTrigger(double strength, int playerId);

    void onConnected(struct Gamepad_device * device);

    void onDisconnected(struct Gamepad_device * device);

    void onButtonDown(struct Gamepad_device * device, unsigned int buttonID);

    void onButtonUp(struct Gamepad_device * device, unsigned int buttonID);

    void onAxisMoved(struct Gamepad_device * device, unsigned int axisID, float value, float lastValue);

    g_id addCallback(gevent_Callback callback, void *udata);
    void removeCallback(gevent_Callback callback, void *udata);
    void removeCallbackWithGid(g_id gid);

private:
    static void onEnterFrame(int type, void *event, void *udata);

    static void callback_s(int type, void *event, void *udata);

    void callback(int type, void *event);

private:
    gevent_CallbackList callbackList_;

private:
    g_id gid_;
    std::map<int, GController*> players;
};

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

typedef struct ghid_Player
{
    int playerId;
    unsigned short vendor_id;
    unsigned short product_id;
    std::string product;
} ghid_Player;

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

static const int BUTTON_B = 97;
static const int DPAD_DOWN = 20;
static const int DPAD_LEFT = 21;
static const int DPAD_RIGHT = 22;
static const int DPAD_UP = 19;
static const int BUTTON_L1 = 102;
static const int BUTTON_L2 = 104;
static const int BUTTON_L3 = 106;
static const int BUTTON_MENU = 82;
static const int BUTTON_BACK = 4;
static const int BUTTON_A = 96;
static const int BUTTON_R1 = 103;
static const int BUTTON_R2 = 105;
static const int BUTTON_R3 = 107;
static const int BUTTON_X = 99;
static const int BUTTON_Y = 100;

static const int LEFT_STICK_X = 0;
static const int LEFT_STICK_Y = 1;
static const int RIGHT_STICK_X = 2;
static const int RIGHT_STICK_Y = 3;
static const int DPAD_X = 6;
static const int DPAD_Y = 7;
static const int L_TRIGGER = 4;
static const int R_TRIGGER = 5;

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
