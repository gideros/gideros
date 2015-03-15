#include "gcontroller.h"
#include <controller.h>
#include <stdlib.h>
#include <cmath>
#include <cstdio>
#include <qt/qglobal.h>

static const float DEAD_ZONE = 0.25f;
static const double MATH_PI = atan(1)*4;
static int BUTTONS_DEFAULT[10] = {BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y, BUTTON_L1, BUTTON_R1, BUTTON_BACK, BUTTON_MENU, BUTTON_L3, BUTTON_R3};
static int BUTTONS_XBOX_MAC[15] = {DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT, BUTTON_MENU, BUTTON_BACK, BUTTON_L3, BUTTON_R3, BUTTON_L1, BUTTON_R1, BUTTON_MENU, BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y};
static int BUTTONS_OUYA_MAC[15] = {BUTTON_A, BUTTON_X, BUTTON_Y, BUTTON_B, BUTTON_L1, BUTTON_R1, BUTTON_L3, BUTTON_R3, DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT, -1, -1, BUTTON_MENU};
static int BUTTONS_OUYA_WIN[15] = {BUTTON_A, BUTTON_X, BUTTON_Y, BUTTON_B, BUTTON_L1, BUTTON_R1, BUTTON_L3, BUTTON_R3, DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT, BUTTON_L2, BUTTON_R2, BUTTON_MENU};
static int BUTTONS_PS3_MAC[17] = {BUTTON_BACK, BUTTON_L3, BUTTON_R3, BUTTON_MENU, DPAD_UP, DPAD_RIGHT, DPAD_DOWN, DPAD_LEFT, BUTTON_L2, BUTTON_R2, BUTTON_L1, BUTTON_R1, BUTTON_Y, BUTTON_B, BUTTON_A, BUTTON_X, BUTTON_MENU};

static int AXIS_DEFAULT[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static int AXIS_XBOX_WIN[8] = {0, 1, 4, 3, 2, 2, 5, 6};
static int AXIS_OUYA_WIN[8] = {0, 1, 5, 4, 2, 3, 6, 7};
static int AXIS_MOGA_MAC[8] = {0, 1, 2, 3, 6, 7, 5, 4};
static int AXIS_PS3_MAC[8] = {0, 1, 2, 3, 14, 15, -1, -1};

#ifdef Q_OS_WIN
    static int OS = 1;
#elif defined Q_OS_MAC
    static int OS = 2;
#else
    static int OS = 0;
#endif
GController::GController(GHID *ghid, unsigned int dID, const char* pname, int btnCount, int vID, int pID)
{
    buttonCount = btnCount;
    buttonOffset = 0;
    vendorID = vID;
    productID = pID;
    name = pname;
    dispatchTrigger = true;
    buttons = &BUTTONS_DEFAULT[0];
    axis =&AXIS_DEFAULT[0];
    if(OS == 1)
    {
        if(vendorID == 1118 && productID == 673) //xbox
        {
            axis = &AXIS_XBOX_WIN[0];
        }
        else if(vendorID == 10294 && productID == 1)//ouya
        {
            buttonCount = 15;
            buttons = &BUTTONS_OUYA_WIN[0];
            axis = &AXIS_OUYA_WIN[0];
            dispatchTrigger = false;
        }
    }
    else if(OS == 2)
    {
        if(vendorID == 1118 && productID == 654) //xbox
        {
            buttonOffset = 5;
            buttonCount = 15;
            buttons = &BUTTONS_XBOX_MAC[0];
        }
        else if(vendorID == 8406 && productID == 3501)//Moga
        {
            axis = &AXIS_MOGA_MAC[0];
        }
        else if(vendorID == 10294 && productID == 1)//ouya
        {
            buttonCount = 15;
            buttonOffset = 3;
            buttons = &BUTTONS_OUYA_MAC[0];
        }
        else if(vendorID == 1356 && productID == 616)//PS3
        {
            buttonCount = 17;
            buttonOffset = 0;
            buttons = &BUTTONS_PS3_MAC[0];
            axis = &AXIS_PS3_MAC[0];
        }
    }
    ghid_ = ghid;
    deviceID = dID;
    name = pname;
    xDPad = new GControllerDPadAxis(ghid_, deviceID, true);
    yDPad = new GControllerDPadAxis(ghid_, deviceID, false);
    trigger = new GControllerTrigger(ghid_, deviceID, dispatchTrigger);
    leftStick = new GControllerJoystick(ghid_, deviceID, true);
    rightStick = new GControllerJoystick(ghid_, deviceID, false);
}

GController::~GController(){
    delete xDPad;
    delete yDPad;
    delete trigger;
    delete leftStick;
    delete rightStick;
    xDPad = NULL;
    yDPad = NULL;
    trigger = NULL;
    leftStick = NULL;
    rightStick = NULL;
    buttons = NULL;
    axis = NULL;
}

unsigned int GController::getId(){
    return deviceID;
}

const char* GController::getName(){
    return name.c_str();
}

void GController::handleButtonDown(unsigned int buttonID) {
    if(buttonID-buttonOffset < buttonCount)
        buttonID = buttons[buttonID-buttonOffset];
    ghid_->onKeyDownEvent(buttonID, deviceID);
}

void GController::handleButtonUp(unsigned int buttonID) {
    if(buttonID-buttonOffset < buttonCount)
        buttonID = buttons[buttonID-buttonOffset];
    ghid_->onKeyUpEvent(buttonID, deviceID);
}

void GController::handleAxisMove(unsigned int axisID, float value, float lastValue)
{
    if(axisID == axis[DPAD_X])
    {
        xDPad->handle(value, lastValue);
    }
    else if(axisID == axis[DPAD_Y])
    {
        yDPad->handle(value, lastValue);
    }
    else if(axisID == axis[L_TRIGGER] || axisID == axis[R_TRIGGER]){
        if(axis[L_TRIGGER] == axis[R_TRIGGER])
            trigger->handleBoth(value, lastValue);
        else if(axisID == axis[L_TRIGGER])
            trigger->handleLeft(value, lastValue);
        else if (axisID == axis[R_TRIGGER])
            trigger->handleRight(value, lastValue);
    }
    else if(axisID == axis[LEFT_STICK_X]){
        leftStick->handleXAxis(value,lastValue);
    }
    else if(axisID == axis[LEFT_STICK_Y]){
        leftStick->handleYAxis(value,lastValue);
    }
    else if(axisID == axis[RIGHT_STICK_X]){
        rightStick->handleXAxis(value,lastValue);
    }
    else if(axisID == axis[RIGHT_STICK_Y]){
        rightStick->handleYAxis(value,lastValue);
    }
}

GControllerDPadAxis::GControllerDPadAxis(GHID *ghid, unsigned int dID, bool x)
{
    ghid_ = ghid;
    xAxis = x;
    deviceID = dID;
}

void GControllerDPadAxis::handle(float value, float lastValue){
    if(ghid_ != NULL && lastValue != value)
    {
        if(xAxis)
        {
            if(value == -1)
                ghid_->onKeyDownEvent(DPAD_LEFT, deviceID);
            else if(value == 1)
                ghid_->onKeyDownEvent(DPAD_RIGHT, deviceID);
            else if(value == 0 && lastValue == -1)
                ghid_->onKeyUpEvent(DPAD_LEFT, deviceID);
            else if(value == 0 && lastValue == 1)
                ghid_->onKeyUpEvent(DPAD_RIGHT, deviceID);
        }
        else
        {
            if(value == -1)
                ghid_->onKeyDownEvent(DPAD_UP, deviceID);
            else if(value == 1)
                ghid_->onKeyDownEvent(DPAD_DOWN, deviceID);
            else if(value == 0 && lastValue == -1)
                ghid_->onKeyUpEvent(DPAD_UP, deviceID);
            else if(value == 0 && lastValue == 1)
                ghid_->onKeyUpEvent(DPAD_DOWN, deviceID);
        }
    }
}

GControllerTrigger::GControllerTrigger(GHID *ghid, unsigned int dID, bool dispatch)
{
    ghid_ = ghid;
    deviceID = dID;
    shouldDispatch = dispatch;
}

void GControllerTrigger::handleBoth(float value, float lastValue){
    if(lastValue != value && fabs(value) >= DEAD_ZONE)
    {
        if(value <= 0)
        {
            ghid_->onRightTrigger(fabs(value), deviceID);
            if(shouldDispatch)
            {
                if(!rightDown && value < -0.5)
                {
                    rightDown = true;
                    ghid_->onKeyDownEvent(BUTTON_R2, deviceID);
                }
                else if(rightDown && value >= -0.5)
                {
                    rightDown = false;
                    ghid_->onKeyUpEvent(BUTTON_R2, deviceID);
                }
           }
        }
        else if(value >= 0)
        {
            ghid_->onLeftTrigger(fabs(value), deviceID);
            if(shouldDispatch)
            {
                if(!leftDown && value > 0.5)
                {
                    leftDown = true;
                    ghid_->onKeyDownEvent(BUTTON_L2, deviceID);
                }
                else if(leftDown && value <= 0.5)
                {
                    leftDown = false;
                    ghid_->onKeyUpEvent(BUTTON_L2, deviceID);
                }
            }
        }
    }
}

void GControllerTrigger::handleLeft(float value, float lastValue){
    if(lastValue != value)
    {
        value = (value + 1)/2;
        if(fabs(value) >= DEAD_ZONE)
        {
            ghid_->onLeftTrigger(fabs(value), deviceID);
            if(shouldDispatch)
            {
                if(!leftDown && value > 0.5)
                {
                    leftDown = true;
                    ghid_->onKeyDownEvent(BUTTON_L2, deviceID);
                }
                else if(leftDown && value <= 0.5)
                {
                    leftDown = false;
                    ghid_->onKeyUpEvent(BUTTON_L2, deviceID);
                }
            }
        }
    }
}

void GControllerTrigger::handleRight(float value, float lastValue){
    if(lastValue != value)
    {
        value = (value + 1)/2;
        if(fabs(value) >= DEAD_ZONE)
        {
            ghid_->onRightTrigger(fabs(value), deviceID);
            if(shouldDispatch)
            {
                if(!rightDown && value > 0.5)
                {
                    rightDown = true;
                    ghid_->onKeyDownEvent(BUTTON_R2, deviceID);
                }
                else if(rightDown && value <= 0.5)
                {
                    rightDown = false;
                    ghid_->onKeyUpEvent(BUTTON_R2, deviceID);
                }
            }
        }
    }
}

GControllerJoystick::GControllerJoystick(GHID *ghid, unsigned int dID, bool left)
{
    ghid_ = ghid;
    deviceID = dID;
    xAxis = 0;
    yAxis = 0;
    isLeft = left;
}

void GControllerJoystick::handleXAxis(float value, float lastValue){
    if(lastValue != value)
    {
        xAxis = value;
        handle();
    }
}

void GControllerJoystick::handleYAxis(float value, float lastValue){
    if(lastValue != value)
    {
        yAxis = value;
        handle();
    }
}

void GControllerJoystick::handle(){
    if(xAxis*xAxis + yAxis*yAxis <= DEAD_ZONE * DEAD_ZONE)
    {
        xAxis = 0;
        yAxis = 0;
    }
    if(lastX != xAxis || lastY != yAxis)
    {
        lastX = xAxis;
        lastY = yAxis;
        double strength = sqrt(xAxis*xAxis + yAxis*yAxis);
        double angle = acos(xAxis/strength);
        if(yAxis > 0)
        {
                angle= -angle+2*MATH_PI;
        }
        angle = -angle+2*MATH_PI;
        if(isLeft)
        {
                ghid_->onLeftJoystick(xAxis, yAxis, angle, strength, deviceID);
        }
        else
        {
                ghid_->onRightJoystick(xAxis, yAxis, angle, strength, deviceID);
        }
    }
}
