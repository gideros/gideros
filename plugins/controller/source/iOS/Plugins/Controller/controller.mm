#include "controller.h"
#include <stdlib.h>
#include <glog.h>
#import "GControllerManager.h"

class GHID
{
public:
	GHID()
	{
		gid_ = g_NextId();
		[GControllerManager init];
	}

	~GHID()
	{
		[GControllerManager cleanup];
		gevent_RemoveEventsWithGid(gid_);
	}
	
	int isAnyAvailable()
	{
		return [GControllerManager isAnyAvailable];
	}
	
	int getPlayerCount()
	{
		return [GControllerManager getPlayerCount];
	}
	
	const char* getControllerName(int player)
	{
		return [[GControllerManager getControllerName:[NSNumber numberWithInt:player]] UTF8String];
	}
	
	void vibrate(int player, long ms)
	{
		
	}
	
	int* getPlayers(int* size)
	{
		
		return NULL;
	}
	
	void onKeyDownEvent(int keyCode, int playerId)
	{

		ghid_KeyEvent *event = (ghid_KeyEvent*)malloc(sizeof(ghid_KeyEvent));
		event->keyCode = keyCode;
		event->playerId = playerId;
		gevent_EnqueueEvent(gid_, callback_s, GHID_KEY_DOWN_EVENT, event, 1, this);
	}
	
	void onKeyUpEvent(int keyCode, int playerId)
	{
		ghid_KeyEvent *event = (ghid_KeyEvent*)malloc(sizeof(ghid_KeyEvent));
		event->keyCode = keyCode;
		event->playerId = playerId;
		gevent_EnqueueEvent(gid_, callback_s, GHID_KEY_UP_EVENT, event, 1, this);
	}
	
	void onRightJoystick(float x, float y, double angle, double strength, int playerId)
	{
		ghid_JoystickEvent *event = (ghid_JoystickEvent*)malloc(sizeof(ghid_JoystickEvent));
		event->x = x;
		event->y = y;
		event->angle = angle;
		event->playerId = playerId;
		event->strength = strength;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_RIGHT_JOYSTICK_EVENT, event, 1, this);
	}
	
	void onLeftJoystick(float x, float y, double angle, double strength, int playerId)
	{

		ghid_JoystickEvent *event = (ghid_JoystickEvent*)malloc(sizeof(ghid_JoystickEvent));
		event->x = x;
		event->y = y;
		event->angle = angle;
		event->playerId = playerId;
		event->strength = strength;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_LEFT_JOYSTICK_EVENT, event, 1, this);
	}
	
	void onRightTrigger(double strength, int playerId)
	{

		ghid_TriggerEvent *event = (ghid_TriggerEvent*)malloc(sizeof(ghid_TriggerEvent));
		event->strength = strength;
		event->playerId = playerId;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_RIGHT_TRIGGER_EVENT, event, 1, this);
	}
	
	void onLeftTrigger(double strength, int playerId)
	{
		ghid_TriggerEvent *event = (ghid_TriggerEvent*)malloc(sizeof(ghid_TriggerEvent));
		event->strength = strength;
		event->playerId = playerId;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_LEFT_TRIGGER_EVENT, event, 1, this);
	}
	
	void onConnected(int playerId)
	{

		ghid_DeviceEvent *event = (ghid_DeviceEvent*)malloc(sizeof(ghid_DeviceEvent));
		event->playerId = playerId;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_CONNECTED_EVENT, event, 1, this);
	}
	
	void onDisconnected(int playerId)
	{

		ghid_DeviceEvent *event = (ghid_DeviceEvent*)malloc(sizeof(ghid_DeviceEvent));
		event->playerId = playerId;
		
		gevent_EnqueueEvent(gid_, callback_s, GHID_DISCONNECTED_EVENT, event, 1, this);
	}
	
	g_id addCallback(gevent_Callback callback, void *udata)
	{
		return callbackList_.addCallback(callback, udata);
	}
	void removeCallback(gevent_Callback callback, void *udata)
	{
		callbackList_.removeCallback(callback, udata);
	}
	void removeCallbackWithGid(g_id gid)
	{
		callbackList_.removeCallbackWithGid(gid);
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		((GHID*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	g_id gid_;
};

static GHID *s_ghid = NULL;

extern "C" {

void ghid_init()
{
	s_ghid = new GHID;
}

void ghid_cleanup()
{
	delete s_ghid;
	s_ghid = NULL;
}

int ghid_isAnyAvailable()
{
	return s_ghid->isAnyAvailable();
}

int ghid_getPlayerCount()
{
	return s_ghid->getPlayerCount();
}

const char* ghid_getControllerName(int player)
{
	return s_ghid->getControllerName(player);
}

void ghid_vibrate(int player, long ms)
{
	s_ghid->vibrate(player, ms);
}

int* ghid_getPlayers(int* size)
{
	return s_ghid->getPlayers(size);
}
    
void ghid_onKeyDownEvent(int keyCode, int playerId)
{
    s_ghid->onKeyDownEvent(keyCode, playerId);
}
	
void ghid_onKeyUpEvent(int keyCode, int playerId)
{
    s_ghid->onKeyUpEvent(keyCode, playerId);
}
	
void ghid_onRightJoystick(float x, float y, double angle, double strength, int playerId)
{
    s_ghid->onRightJoystick(x, y, angle, strength, playerId);
}
	
void ghid_onLeftJoystick(float x, float y, double angle, double strength, int playerId)
{
    s_ghid->onLeftJoystick(x, y, angle, strength, playerId);
}
	
void ghid_onRightTrigger(double strength, int playerId)
{
    s_ghid->onRightTrigger(strength, playerId);
}
	
void ghid_onLeftTrigger(double strength, int playerId)
{
    s_ghid->onLeftTrigger(strength, playerId);
}
	
void ghid_onConnected(int playerId)
{
    s_ghid->onConnected(playerId);
}
	
void ghid_onDisconnected(int playerId)
{
    s_ghid->onDisconnected(playerId);
}

g_id ghid_addCallback(gevent_Callback callback, void *udata)
{
	return s_ghid->addCallback(callback, udata);
}

void ghid_removeCallback(gevent_Callback callback, void *udata)
{
	s_ghid->removeCallback(callback, udata);
}

void ghid_removeCallbackWithGid(g_id gid)
{
	s_ghid->removeCallbackWithGid(gid);
}

}
