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
#include <glog.h>

using namespace Concurrency;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Gaming::Input;

struct Gamepad_devicePrivate {
    RawGameController^ rgc;
    Gamepad^ rgamepad;
    Platform::Array<GameControllerSwitchPosition>^ lastSwitchReading;
    Platform::Array<bool>^ lastButtonReading;
    Platform::Array<double>^ lastAxisReading;
    bool seen;
    Gamepad_devicePrivate() : rgc(nullptr), rgamepad(nullptr), lastSwitchReading(nullptr), lastButtonReading(nullptr), lastAxisReading(nullptr) {};
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
    struct Gamepad_device* deviceRecord;
    struct Gamepad_devicePrivate* deviceRecordPrivate;

    if (!inited) {
        return;
    }

    for (deviceIndex2 = 0; deviceIndex2 < numDevices; deviceIndex2++)
        ((struct Gamepad_devicePrivate*) devices[deviceIndex2]->privateData)->seen = false;

    for (Gamepad^ rgamepad : Gamepad::Gamepads)
    {
        RawGameController^ gamepad = RawGameController::FromGameController(rgamepad);
        duplicate = false;
        for (deviceIndex2 = 0; deviceIndex2 < numDevices; deviceIndex2++) {
            if (((struct Gamepad_devicePrivate*) devices[deviceIndex2]->privateData)->rgc == gamepad) {
                duplicate = true;
                ((struct Gamepad_devicePrivate*) devices[deviceIndex2]->privateData)->seen = true;
                break;
            }
        }
        if (duplicate) {
            continue;
        }

        deviceRecord = (struct Gamepad_device*) malloc(sizeof(struct Gamepad_device));
        deviceRecord->deviceID = nextDeviceID++;
        deviceRecord->description = getDeviceDescription(gamepad);
        deviceRecord->vendorID = 0;
        deviceRecord->productID = 0;
        deviceRecord->numAxes = 6;
        deviceRecord->numButtons = 17;
        deviceRecord->axisStates = (float*)calloc(sizeof(float), deviceRecord->numAxes);
        deviceRecord->buttonStates = (bool*)calloc(sizeof(bool), deviceRecord->numButtons);
        devices = (struct Gamepad_device**) realloc(devices, sizeof(struct Gamepad_device*) * (numDevices + 1));
        devices[numDevices++] = deviceRecord;

        deviceRecordPrivate = new Gamepad_devicePrivate();
        deviceRecordPrivate->rgamepad = rgamepad;
        deviceRecordPrivate->rgc = gamepad;
        deviceRecordPrivate->lastSwitchReading =
            ref new Platform::Array<GameControllerSwitchPosition>(gamepad->SwitchCount);
        deviceRecordPrivate->lastButtonReading =
            ref new Platform::Array<bool>(deviceRecord->numButtons);
        deviceRecordPrivate->lastAxisReading = ref new Platform::Array<double>(deviceRecord->numAxes);
        deviceRecordPrivate->seen = true;

        deviceRecord->privateData = deviceRecordPrivate;

        if (Gamepad_deviceAttachCallback != NULL) {
            Gamepad_deviceAttachCallback(deviceRecord, Gamepad_deviceAttachContext);
        }
    }

    for (RawGameController^ gamepad : RawGameController::RawGameControllers)
    {
        duplicate = false;
        for (deviceIndex2 = 0; deviceIndex2 < numDevices; deviceIndex2++) {
            if (((struct Gamepad_devicePrivate*) devices[deviceIndex2]->privateData)->rgc == gamepad) {
                duplicate = true;
                ((struct Gamepad_devicePrivate*) devices[deviceIndex2]->privateData)->seen = true;
                break;
            }
        }
        if (duplicate) {
            continue;
        }

        deviceRecord = (struct Gamepad_device*) malloc(sizeof(struct Gamepad_device));
        deviceRecord->deviceID = nextDeviceID++;
        deviceRecord->description = getDeviceDescription(gamepad);
        deviceRecord->vendorID = gamepad->HardwareVendorId;
        deviceRecord->productID = gamepad->HardwareProductId;
        deviceRecord->numAxes = gamepad->AxisCount;
        deviceRecord->numButtons = gamepad->ButtonCount;
        deviceRecord->axisStates = (float*)calloc(sizeof(float), deviceRecord->numAxes);
        deviceRecord->buttonStates = (bool*)calloc(sizeof(bool), deviceRecord->numButtons);
        devices = (struct Gamepad_device**) realloc(devices, sizeof(struct Gamepad_device*) * (numDevices + 1));
        devices[numDevices++] = deviceRecord;

        deviceRecordPrivate = new Gamepad_devicePrivate();
        deviceRecordPrivate->rgc = gamepad;
        deviceRecordPrivate->lastSwitchReading =
            ref new Platform::Array<GameControllerSwitchPosition>(gamepad->SwitchCount);
        deviceRecordPrivate->lastButtonReading =
            ref new Platform::Array<bool>(deviceRecord->numButtons);
        deviceRecordPrivate->lastAxisReading = ref new Platform::Array<double>(deviceRecord->numAxes);

        deviceRecordPrivate->seen = true;

        deviceRecord->privateData = deviceRecordPrivate;

        if (Gamepad_deviceAttachCallback != NULL) {
            Gamepad_deviceAttachCallback(deviceRecord, Gamepad_deviceAttachContext);
        }
    }
    for (deviceIndex2 = 0; deviceIndex2 < numDevices;)
        if (!((struct Gamepad_devicePrivate*) devices[deviceIndex2]->privateData)->seen) {
            if (Gamepad_deviceRemoveCallback != NULL) {
                Gamepad_deviceRemoveCallback(devices[deviceIndex2], Gamepad_deviceRemoveContext);
            }

            disposeDevice(devices[deviceIndex2]);
            deviceIndex = deviceIndex2;
            numDevices--;
            for (; deviceIndex < numDevices; deviceIndex++)
                devices[deviceIndex] = devices[deviceIndex + 1];
            devices[deviceIndex] = NULL;
        }
        else
            deviceIndex2++;
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

        if (devicePrivate->rgamepad != nullptr) {
            GamepadReading reading = devicePrivate->rgamepad->GetCurrentReading();
            handleAxisChange(device, 0, reading.LeftThumbstickX);
            handleAxisChange(device, 1, -reading.LeftThumbstickY);
            handleAxisChange(device, 2, reading.RightThumbstickX);
            handleAxisChange(device, 3, -reading.RightThumbstickY);
            handleAxisChange(device, 4, reading.LeftTrigger*2-1);
            handleAxisChange(device, 5, reading.RightTrigger*2-1);

            GamepadButtons b = reading.Buttons;
            handleButtonChange(device, 0, (b&GamepadButtons::A)!= GamepadButtons::None);
            handleButtonChange(device, 1, (b & GamepadButtons::B) != GamepadButtons::None);
            handleButtonChange(device, 2, (b & GamepadButtons::X) != GamepadButtons::None);
            handleButtonChange(device, 3, (b & GamepadButtons::Y) != GamepadButtons::None);
            handleButtonChange(device, 4, (b & GamepadButtons::LeftShoulder) != GamepadButtons::None);
            handleButtonChange(device, 5, (b & GamepadButtons::RightShoulder) != GamepadButtons::None);
            handleButtonChange(device, 6, reading.LeftTrigger>0);
            handleButtonChange(device, 7, reading.RightTrigger>0);
            handleButtonChange(device, 8, (b & GamepadButtons::View) != GamepadButtons::None);
            handleButtonChange(device, 9, (b & GamepadButtons::Menu) != GamepadButtons::None);
            handleButtonChange(device, 10, (b & GamepadButtons::LeftThumbstick) != GamepadButtons::None);
            handleButtonChange(device, 11, (b & GamepadButtons::RightThumbstick) != GamepadButtons::None);
            handleButtonChange(device, 12, (b & GamepadButtons::DPadUp) != GamepadButtons::None);
            handleButtonChange(device, 13, (b & GamepadButtons::DPadDown) != GamepadButtons::None);
            handleButtonChange(device, 14, (b & GamepadButtons::DPadLeft) != GamepadButtons::None);
            handleButtonChange(device, 15, (b & GamepadButtons::DPadRight) != GamepadButtons::None);
            handleButtonChange(device, 16, false);

        }
        else
        {
            unsigned long long ts = devicePrivate->rgc->GetCurrentReading(
                devicePrivate->lastButtonReading,
                devicePrivate->lastSwitchReading,
                devicePrivate->lastAxisReading);
            for (unsigned int i = 0; i < device->numAxes; i++) {
                float value = devicePrivate->lastAxisReading[i];
                if (i < 4) { //Assume these are left and right sticks: TODO embed a controller database into the plugin to figure out
                    value = (value - 0.5) * 2; //UWP report axis as 0->1 values
                }

                handleAxisChange(device, i, value);
            }
       for (unsigned int i = 0; i < device->numButtons; i++)
            handleButtonChange(device, i, devicePrivate->lastButtonReading[i]);

        }
    }
    inProcessEvents = false;
}
