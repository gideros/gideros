#include "config.h"

#include <stdlib.h>
#include "alMain.h"
#include "AL/al.h"
#include "AL/alc.h"

volatile int g_sampleRate = 0;
volatile int g_bufferLength = 0;
volatile int g_bufferCount = 0;
volatile int g_mixerRunning = 0;
ALCdevice* g_device = NULL;

void g_processBuffer(void *buffer, size_t size)
{
	if (g_mixerRunning)
	{
		aluMixData(g_device, buffer, size);
	}
}

static const ALCchar android_device[] = "Android Default";

typedef struct
{
} AndroidData;

static ALCboolean android_open_playback(ALCdevice *device, const ALCchar *deviceName)
{
	g_device = device;

	AndroidData* data;

    if (!deviceName)
    {
        deviceName = android_device;
    }
    else if (strcmp(deviceName, android_device) != 0)
    {
        return ALC_FALSE;
    }

    data = (AndroidData*)calloc(1, sizeof(*data));
    device->szDeviceName = strdup(deviceName);
    device->ExtraData = data;
    return ALC_TRUE;
}

static void android_close_playback(ALCdevice *device)
{
	g_device = device;

	AndroidData* data = (AndroidData*)device->ExtraData;
    if (data != NULL)
    {
        free(data);
        device->ExtraData = NULL;
    }
}

static ALCboolean android_reset_playback(ALCdevice *device)
{
	g_device = device;

	AndroidData* data = (AndroidData*)device->ExtraData;

	device->FmtType = DevFmtShort;
	device->FmtChans = DevFmtStereo;

    SetDefaultChannelOrder(device);

    g_bufferLength = device->UpdateSize;
    g_bufferCount = device->NumUpdates;
    g_sampleRate = device->Frequency;
    g_mixerRunning = 1;

    return ALC_TRUE;
}

static void android_stop_playback(ALCdevice *device)
{
	g_device = device;

	AndroidData* data = (AndroidData*)device->ExtraData;
    g_mixerRunning = 0;
}

static ALCboolean android_open_capture(ALCdevice *pDevice, const ALCchar *deviceName)
{
    (void)pDevice;
    (void)deviceName;
    return ALC_FALSE;
}

static const BackendFuncs android_funcs = {
    android_open_playback,
    android_close_playback,
    android_reset_playback,
    android_stop_playback,
    android_open_capture,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

void alc_android_init(BackendFuncs *func_list)
{
    *func_list = android_funcs;
}

void alc_android_deinit(void)
{
}

void alc_android_probe(int type)
{
    if (type == DEVICE_PROBE)
    {
        AppendDeviceList(android_device);
    }
    else if (type == ALL_DEVICE_PROBE)
    {
        AppendAllDeviceList(android_device);
    }
}
