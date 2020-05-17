#include <gamepad/Gamepad.h>
#include <gamepad/Gamepad_private.h>

#include <wrl.h>
#include <collection.h>
#include <concrt.h>
#include <algorithm>
#include <windows.gaming.input.h>
#include <windows.foundation.collections.h>

#include <controller.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cvt/wstring>
#include <codecvt>

using namespace Concurrency;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Gaming::Input;

struct Gamepad_devicePrivate {
    RawGameController^ rgc;
    Platform::Array<GameControllerSwitchPosition>^ lastSwitchReading;
    Platform::Array<bool>^ lastButtonReading;
    Platform::Array<double>^ lastAxisReading;
    int povAxisIndex;
    Gamepad_devicePrivate() : rgc(nullptr), lastSwitchReading(nullptr), lastButtonReading(nullptr), lastAxisReading(nullptr) {};
};

static struct Gamepad_device ** devices = NULL;
static unsigned int numDevices = 0;
static unsigned int nextDeviceID = 0;

static bool inited = false;

void Gamepad_init() {
    if (!inited) {
        inited = true;
    }
}

static void disposeDevice(struct Gamepad_device * deviceRecord) {
    free(deviceRecord->privateData);

    free((void *) deviceRecord->description);
    free(deviceRecord->axisStates);
    free(deviceRecord->buttonStates);

    free(deviceRecord);
}

void Gamepad_shutdown() {
    unsigned int deviceIndex;

    if (inited) {
        for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
            disposeDevice(devices[deviceIndex]);
        }
        free(devices);
        devices = NULL;
        numDevices = 0;
        inited = false;
    }
}

unsigned int Gamepad_numDevices() {
    return numDevices;
}

struct Gamepad_device * Gamepad_deviceAtIndex(unsigned int deviceIndex) {
    if (deviceIndex >= numDevices) {
        return NULL;
    }
    return devices[deviceIndex];
}

static char * getDeviceDescription(RawGameController ^rgc) {
    stdext::cvt::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    std::string name = convert.to_bytes(rgc->DisplayName->Data());
    char* description = _strdup(name.c_str());
    return description;
}

void Gamepad_detectDevices() {
    unsigned int deviceIndex, deviceIndex2;
    bool duplicate;
    struct Gamepad_device * deviceRecord;
    struct Gamepad_devicePrivate * deviceRecordPrivate;
    UINT joystickID;
    int axisIndex;

    if (!inited) {
        return;
    }

    for (RawGameController^ gamepad : RawGameController::RawGameControllers)
    {
            duplicate = false;
            for (deviceIndex2 = 0; deviceIndex2 < numDevices; deviceIndex2++) {
                if (((struct Gamepad_devicePrivate *) devices[deviceIndex2]->privateData)->rgc == gamepad) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) {
                continue;
            }

            deviceRecord = (struct Gamepad_device *) malloc(sizeof(struct Gamepad_device));
            deviceRecord->deviceID = nextDeviceID++;
            deviceRecord->description = getDeviceDescription(gamepad);
            deviceRecord->vendorID = gamepad->HardwareVendorId;
            deviceRecord->productID = gamepad->HardwareProductId;
            deviceRecord->numAxes = gamepad->AxisCount;
            deviceRecord->numButtons = gamepad->ButtonCount;
            deviceRecord->axisStates = (float *) calloc(sizeof(float), deviceRecord->numAxes);
            deviceRecord->buttonStates = (bool *) calloc(sizeof(bool), deviceRecord->numButtons);
            devices = (struct Gamepad_device **) realloc(devices, sizeof(struct Gamepad_device *) * (numDevices + 1));
            devices[numDevices++] = deviceRecord;

            deviceRecordPrivate = new Gamepad_devicePrivate();
            deviceRecordPrivate->rgc = gamepad;
            deviceRecordPrivate->lastSwitchReading=
                ref new Platform::Array<GameControllerSwitchPosition>(gamepad->SwitchCount);
            deviceRecordPrivate->lastButtonReading =
                ref new Platform::Array<bool>(deviceRecord->numButtons);
            deviceRecordPrivate->lastAxisReading = ref new Platform::Array<double>(deviceRecord->numAxes);

            deviceRecordPrivate->povAxisIndex = -1;

            deviceRecord->privateData = deviceRecordPrivate;

            if (Gamepad_deviceAttachCallback != NULL) {
                Gamepad_deviceAttachCallback(deviceRecord, Gamepad_deviceAttachContext);
            }
    }
}

static double currentTime() {
    // HACK: No timestamp data from joyGetInfoEx, so we make it up
    static LARGE_INTEGER frequency;
    LARGE_INTEGER ccurrentTime;

    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }
    QueryPerformanceCounter(&ccurrentTime);

    return (double) ccurrentTime.QuadPart / frequency.QuadPart;
}

static void handleAxisChange(struct Gamepad_device * device, int axisIndex, float value) {
    float lastValue;

    if (axisIndex < 0 || axisIndex >= (int) device->numAxes) {
        return;
    }

    lastValue = device->axisStates[axisIndex];
    device->axisStates[axisIndex] = value;
    if ((Gamepad_axisMoveCallback != NULL)&&(lastValue!=value)) {
        Gamepad_axisMoveCallback(device, axisIndex, value, lastValue, currentTime(), Gamepad_axisMoveContext);
    }
}

static void handleButtonChange(struct Gamepad_device * device, unsigned int buttonIndex, bool down) {
    if (device->buttonStates[buttonIndex] != down) {
        device->buttonStates[buttonIndex] = down;
        if (down && Gamepad_buttonDownCallback != NULL) {
            Gamepad_buttonDownCallback(device, buttonIndex, currentTime(), Gamepad_buttonDownContext);
        }
        else if (!down && Gamepad_buttonUpCallback != NULL) {
            Gamepad_buttonUpCallback(device, buttonIndex, currentTime(), Gamepad_buttonUpContext);
        }
    }
}

/*
static void povToXY(DWORD pov, int * outX, int * outY) {
    if (pov == JOY_POVCENTERED) {
        *outX = *outY = 0;

    } else {
        if (pov > JOY_POVFORWARD && pov < JOY_POVBACKWARD) {
            *outX = 1;

        } else if (pov > JOY_POVBACKWARD) {
            *outX = -1;

        } else {
            *outX = 0;
        }

        if (pov > JOY_POVLEFT || pov < JOY_POVRIGHT) {
            *outY = -1;

        } else if (pov > JOY_POVRIGHT && pov < JOY_POVLEFT) {
            *outY = 1;

        } else {
            *outY = 0;
        }
    }
}

static void handlePOVChange(struct Gamepad_device * device, DWORD lastValue, DWORD value) {
    struct Gamepad_devicePrivate * devicePrivate;
    int lastX, lastY, newX, newY;

    devicePrivate = (struct Gamepad_devicePrivate *) device->privateData;

    if (devicePrivate->povXAxisIndex == -1 || devicePrivate->povYAxisIndex == -1) {
        return;
    }

    povToXY(lastValue, &lastX, &lastY);
    povToXY(value, &newX, &newY);

    if (newX != lastX) {
        device->axisStates[devicePrivate->povXAxisIndex] = newX;
        if (Gamepad_axisMoveCallback != NULL) {
            Gamepad_axisMoveCallback(device, devicePrivate->povXAxisIndex, newX, lastX, currentTime(), Gamepad_axisMoveContext);
        }
    }
    if (newY != lastY) {
        device->axisStates[devicePrivate->povYAxisIndex] = newY;
        if (Gamepad_axisMoveCallback != NULL) {
            Gamepad_axisMoveCallback(device, devicePrivate->povYAxisIndex, newY, lastY, currentTime(), Gamepad_axisMoveContext);
        }
    }
}
*/
void Gamepad_processEvents() {
    unsigned int deviceIndex;
    static bool inProcessEvents;
    struct Gamepad_device * device;
    struct Gamepad_devicePrivate * devicePrivate;

    if (!inited || inProcessEvents) {
        return;
    }

    inProcessEvents = true;
    for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
        device = devices[deviceIndex];
        devicePrivate = (Gamepad_devicePrivate*)device->privateData;

        unsigned long long ts=devicePrivate->rgc->GetCurrentReading(
            devicePrivate->lastButtonReading,
            devicePrivate->lastSwitchReading,
            devicePrivate->lastAxisReading);
        /*        if (result == JOYERR_UNPLUGGED) {
                    if (Gamepad_deviceRemoveCallback != NULL) {
                        Gamepad_deviceRemoveCallback(device, Gamepad_deviceRemoveContext);
                    }

                    disposeDevice(device);
                    numDevices--;
                    for (; deviceIndex < numDevices; deviceIndex++) {
                        devices[deviceIndex] = devices[deviceIndex + 1];
                    }

                } */
        for (unsigned int i = 0; i < device->numAxes; i++)
            handleAxisChange(device, i, devicePrivate->lastAxisReading[i]);
        for (unsigned int i = 0; i < device->numButtons; i++)
            handleButtonChange(device, i, devicePrivate->lastButtonReading[i]);
    }
    inProcessEvents = false;
}
