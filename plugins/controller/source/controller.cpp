#include <controller.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <Gamepad.h>
#include "gcontroller.h"

extern "C" {
void ghid_onConnected(struct Gamepad_device * device, void * context);
void ghid_onDisconnected(struct Gamepad_device * device, void * context);
void ghid_onButtonDown(struct Gamepad_device * device, unsigned int buttonID, double timestamp, void * context);
void ghid_onButtonUp(struct Gamepad_device * device, unsigned int buttonID, double timestamp, void * context);
void ghid_onAxisMoved(struct Gamepad_device * device, unsigned int axisID, float value, float lastValue, double timestamp, void * context);
}

GHID::GHID()
{
    gid_ = g_NextId();
    players.clear();
    Gamepad_deviceAttachFunc(ghid_onConnected, (void *) 0x1);
    Gamepad_deviceRemoveFunc(ghid_onDisconnected, (void *) 0x2);
    Gamepad_buttonDownFunc(ghid_onButtonDown, (void *) 0x3);
    Gamepad_buttonUpFunc(ghid_onButtonUp, (void *) 0x4);
    Gamepad_axisMoveFunc(ghid_onAxisMoved, (void *) 0x5);
    Gamepad_init();
    gevent_AddCallback(onEnterFrame, this);
}

GHID::~GHID()
{
    gevent_RemoveCallback(onEnterFrame, this);
    Gamepad_shutdown();
    std::map<int, GController*>::iterator iter, e = players.end();
    for (iter = players.begin(); iter != e; ++iter)
    {
        delete iter->second;
    }
    players.clear();
    gevent_RemoveEventsWithGid(gid_);
}

int GHID::isAnyAvailable()
{
    if(players.size() > 0){
        return 1;
    }
    return 0;
}

int GHID::getPlayerCount()
{
    return players.size();
}

const char* GHID::getControllerName(int playerId)
{
    std::map<int, GController*>::iterator it = players.find(playerId);
    if (it != players.end()) {
        it->second->getName();
    }
    return "";
}

void GHID::vibrate(int player, long ms)
{

}

int* GHID::getPlayers(int* size)
{
    int s = (int)players.size();
    *size = s;
    int* arr = new int[s];
    int i = 0;
    std::map<int, GController*>::iterator iter, e = players.end();
    for (iter = players.begin(); iter != e; ++iter)
    {
        arr[i] = iter->first;
        i++;
    }
    return &arr[0];
}

void GHID::onKeyDownEvent(int keyCode, int playerId)
{
    ghid_KeyEvent *event = (ghid_KeyEvent*)malloc(sizeof(ghid_KeyEvent));
    event->keyCode = keyCode;
    event->playerId = playerId;
    gevent_EnqueueEvent(gid_, callback_s, GHID_KEY_DOWN_EVENT, event, 1, this);
}

void GHID::onKeyUpEvent(int keyCode, int playerId)
{
    ghid_KeyEvent *event = (ghid_KeyEvent*)malloc(sizeof(ghid_KeyEvent));
    event->keyCode = keyCode;
    event->playerId = playerId;
    gevent_EnqueueEvent(gid_, callback_s, GHID_KEY_UP_EVENT, event, 1, this);
}

void GHID::onRightJoystick(float x, float y, double angle, double strength, int playerId)
{
    ghid_JoystickEvent *event = (ghid_JoystickEvent*)malloc(sizeof(ghid_JoystickEvent));
    event->x = x;
    event->y = y;
    event->angle = angle;
    event->playerId = playerId;
    event->strength = strength;

    gevent_EnqueueEvent(gid_, callback_s, GHID_RIGHT_JOYSTICK_EVENT, event, 1, this);
}

void GHID::onLeftJoystick(float x, float y, double angle, double strength, int playerId)
{
    ghid_JoystickEvent *event = (ghid_JoystickEvent*)malloc(sizeof(ghid_JoystickEvent));
    event->x = x;
    event->y = y;
    event->angle = angle;
    event->playerId = playerId;
    event->strength = strength;

    gevent_EnqueueEvent(gid_, callback_s, GHID_LEFT_JOYSTICK_EVENT, event, 1, this);
}

void GHID::onRightTrigger(double strength, int playerId)
{
    ghid_TriggerEvent *event = (ghid_TriggerEvent*)malloc(sizeof(ghid_TriggerEvent));
    event->strength = strength;
    event->playerId = playerId;

    gevent_EnqueueEvent(gid_, callback_s, GHID_RIGHT_TRIGGER_EVENT, event, 1, this);
}

void GHID::onLeftTrigger(double strength, int playerId)
{
    ghid_TriggerEvent *event = (ghid_TriggerEvent*)malloc(sizeof(ghid_TriggerEvent));
    event->strength = strength;
    event->playerId = playerId;

    gevent_EnqueueEvent(gid_, callback_s, GHID_LEFT_TRIGGER_EVENT, event, 1, this);
}

void GHID::onConnected(struct Gamepad_device * device)
{
    int playerId = device->deviceID+1;
    GController *c = new GController(this, playerId, device->description, device->numButtons, device->vendorID, device->productID);
    players.insert(std::pair<int, GController*>(playerId,c));
    ghid_DeviceEvent *event = (ghid_DeviceEvent*)malloc(sizeof(ghid_DeviceEvent));
    event->playerId = playerId;

    gevent_EnqueueEvent(gid_, callback_s, GHID_CONNECTED_EVENT, event, 1, this);
}

void GHID::onDisconnected(struct Gamepad_device * device)
{
    int playerId = device->deviceID+1;
    std::map<int, GController*>::iterator it = players.find(playerId);
    if (it != players.end()) {
      delete it->second;
      players.erase(it);
    }
    ghid_DeviceEvent *event = (ghid_DeviceEvent*)malloc(sizeof(ghid_DeviceEvent));
    event->playerId = playerId;

    gevent_EnqueueEvent(gid_, callback_s, GHID_DISCONNECTED_EVENT, event, 1, this);
}

void GHID::onButtonDown(struct Gamepad_device * device, unsigned int buttonID)
{
    int playerId = device->deviceID+1;
    std::map<int,GController*>::iterator it = players.find(playerId);
    if(it != players.end())
    {
        it->second->handleButtonDown(buttonID);
    }
}

void GHID::onButtonUp(struct Gamepad_device * device, unsigned int buttonID)
{
    int playerId = device->deviceID+1;
    std::map<int,GController*>::iterator it = players.find(playerId);
    if(it != players.end())
    {
        it->second->handleButtonUp(buttonID);
    }
}

void GHID::onAxisMoved(struct Gamepad_device * device, unsigned int axisID, float value, float lastValue)
{
    int playerId = device->deviceID+1;
    std::map<int,GController*>::iterator it = players.find(playerId);
    if(it != players.end())
    {
        it->second->handleAxisMove(axisID, value, lastValue);
    }
}

g_id GHID::addCallback(gevent_Callback callback, void *udata)
{
    return callbackList_.addCallback(callback, udata);
}
void GHID::removeCallback(gevent_Callback callback, void *udata)
{
    callbackList_.removeCallback(callback, udata);
}
void GHID::removeCallbackWithGid(g_id gid)
{
    callbackList_.removeCallbackWithGid(gid);
}

void GHID::onEnterFrame(int type, void *event, void *udata)
{
    if(type == GEVENT_PRE_TICK_EVENT)
    {
        Gamepad_detectDevices();
        Gamepad_processEvents();
    }
}

void GHID::callback_s(int type, void *event, void *udata)
{
    ((GHID*)udata)->callback(type, event);
}

void GHID::callback(int type, void *event)
{
    callbackList_.dispatchEvent(type, event);
}

extern "C" {

static GHID *s_ghid = NULL;

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

void ghid_onConnected(struct Gamepad_device * device, void * context)
{
    s_ghid->onConnected(device);
}

void ghid_onDisconnected(struct Gamepad_device * device, void * context)
{
    s_ghid->onDisconnected(device);
}

void ghid_onButtonDown(struct Gamepad_device * device, unsigned int buttonID, double timestamp, void * context) {
    //s_ghid->onKeyDownEvent(buttonID, 1);
    s_ghid->onButtonDown(device, buttonID);
}

void ghid_onButtonUp(struct Gamepad_device * device, unsigned int buttonID, double timestamp, void * context) {
    //s_ghid->onKeyUpEvent(buttonID, 1);
    s_ghid->onButtonUp(device, buttonID);
}

void ghid_onAxisMoved(struct Gamepad_device * device, unsigned int axisID, float value, float lastValue, double timestamp, void * context) {
    //s_ghid->onLeftJoystick(axisID, value, lastValue, 0, 1);
    s_ghid->onAxisMoved(device, axisID, value, lastValue);
}

int* ghid_getPlayers(int* size)
{
	return s_ghid->getPlayers(size);
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
