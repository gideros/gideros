#include "gmicrophone.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#endif

#include <stdlib.h>

#if defined(OPENAL_SUBDIR_OPENAL)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#elif defined(OPENAL_SUBDIR_AL)
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif

#include <map>

#include <pthread.h>

#include <math.h>

namespace {

static void calculateAmplitudeData(int numChannels, int bitsPerSample, gmicrophone_DataAvailableEvent *event)
{
    int n = event->sampleCount * numChannels;

    if (bitsPerSample == 8)
    {
        const unsigned char *data = (unsigned char*)event->data;

        float sum = 0;
        float peak = 0;

        for (int i = 0; i < n; ++i)
        {
            int d1 = abs((int)data[i] - 128);
            float d2 = d1 / 128.f;
            peak = std::max(peak, d2);
            sum += d2 * d2;
        }

        event->peakAmplitude = peak;
        event->averageAmplitude = sqrt(sum / n);
    }
    else if (bitsPerSample == 16)
    {
        const short *data = (short*)event->data;

        float sum = 0;
        float peak = 0;

        for (int i = 0; i < n; ++i)
        {
            int d1 = abs(data[i]);
            float d2 = d1 / 32768.f;
            peak = std::max(peak, d2);
            sum += d2 * d2;
        }

        event->peakAmplitude = peak;
        event->averageAmplitude = sqrt(sum / n);
    }
}

class GMicrophoneManager
{
public:
    GMicrophoneManager()
    {

    }

    ~GMicrophoneManager()
    {
        while (!microphones_.empty())
            Delete(microphones_.begin()->first);
    }

    g_id Create(const char *deviceName, int numChannels, int sampleRate, int bitsPerSample, gmicrophone_Error *error)
    {
        if (numChannels != 1 && numChannels != 2)
        {
            if (error)
                *error = GMICROPHONE_UNSUPPORTED_FORMAT;
            return 0;
        }

        if (bitsPerSample != 8 && bitsPerSample != 16)
        {
            if (error)
                *error = GMICROPHONE_UNSUPPORTED_FORMAT;
            return 0;
        }

        if (sampleRate < 4000 || sampleRate > 44100)
        {
            if (error)
                *error = GMICROPHONE_UNSUPPORTED_FORMAT;
            return 0;
        }

        ALenum format = 0;
        if (bitsPerSample == 8)
        {
            if (numChannels == 1)
                format = AL_FORMAT_MONO8;
            else if (numChannels == 2)
                format = AL_FORMAT_STEREO8;
        }
        else if (bitsPerSample == 16)
        {
            if (numChannels == 1)
                format = AL_FORMAT_MONO16;
            else if (numChannels == 2)
                format = AL_FORMAT_STEREO16;
        }

        ALCdevice *device = alcCaptureOpenDevice(NULL, sampleRate, format, sampleRate / 5);
        if (device == NULL)
        {
            if (error)
                *error = GMICROPHONE_CANNOT_OPEN_DEVICE;
            return 0;
        }

        g_id gid = g_NextId();

        microphones_[gid] = new Microphone(gid, device, numChannels, bitsPerSample);

        if (error)
            *error = GMICROPHONE_NO_ERROR;

        return gid;
    }

    void Delete(g_id microphone)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);

        if (iter == microphones_.end())
            return;

        Microphone *microphone2 = iter->second;

        Stop(microphone);

        alcCaptureCloseDevice(microphone2->device);

        delete microphone2;

        microphones_.erase(microphone);
    }

    void Start(g_id microphone)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);

        if (iter == microphones_.end())
            return;

        Microphone *microphone2 = iter->second;

        if (microphone2->isStarted)
            return;

        microphone2->exit = false;
        pthread_create(&microphone2->thread, NULL, run_s, microphone2);

        microphone2->isStarted = true;
    }

    void Stop(g_id microphone)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);

        if (iter == microphones_.end())
            return;

        Microphone *microphone2 = iter->second;

        if (!microphone2->isStarted)
            return;

        microphone2->exit = true;
        pthread_join(microphone2->thread, NULL);

        microphone2->isStarted = false;

        gevent_RemoveEventsWithGid(microphone2->gid);
    }

    g_id AddCallback(g_id microphone, gevent_Callback callback, void *udata)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);

        if (iter == microphones_.end())
            return 0;

        Microphone *microphone2 = iter->second;

        return microphone2->addCallback(callback, udata);
    }

    void RemoveCallback(g_id microphone, gevent_Callback callback, void *udata)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);

        if (iter == microphones_.end())
            return;

        Microphone *microphone2 = iter->second;

        microphone2->removeCallback(callback, udata);
    }

    void RemoveCallbackWithId(g_id microphone, g_id callback)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);

        if (iter == microphones_.end())
            return;

        Microphone *microphone2 = iter->second;

        microphone2->removeCallbackWithId(callback);
    }

private:
    static void *run_s(void *arg)
    {
        static_cast<Microphone*>(arg)->run();
        return NULL;
    }

private:
    class Microphone
    {
    public:
        Microphone(g_id gid, ALCdevice *device, int numChannels, int bitsPerSample) :
            gid(gid),
            device(device),
            numChannels(numChannels),
            bitsPerSample(bitsPerSample),
            isStarted(false)
        {
            bytesPerSample = ((bitsPerSample + 7) / 8) * numChannels;
        }

        void run()
        {
            alcCaptureStart(device);

            size_t structSize = sizeof(gmicrophone_DataAvailableEvent);

            while (!exit)
            {
                for (int i = 0; i < 10 && !exit; ++i)
                {
#ifdef _WIN32
                    Sleep(10);
#else
                    usleep(10 * 1000);
#endif
                }

                if (exit)
                    break;

                ALint sampleCount;
                alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, sizeof(sampleCount), &sampleCount);

                if (sampleCount == 0)
                    continue;

                size_t dataSize = sampleCount * bytesPerSample;

                gmicrophone_DataAvailableEvent *event = (gmicrophone_DataAvailableEvent*)malloc(structSize + dataSize);

                event->microphone = gid;
                event->data = (char*)event + structSize;
                event->sampleCount = sampleCount;

                alcCaptureSamples(device, event->data, sampleCount);

                calculateAmplitudeData(numChannels, bitsPerSample, event);

                gevent_EnqueueEvent(gid, callback_s, GMICROPHONE_DATA_AVAILABLE_EVENT, event, 1, this);
            }

            alcCaptureStop(device);
        }

        g_id addCallback(gevent_Callback callback, void *udata)
        {
            return callbackList_.addCallback(callback, udata);
        }

        void removeCallback(gevent_Callback callback, void *udata)
        {
            callbackList_.removeCallback(callback, udata);
        }

        void removeCallbackWithId(g_id callback)
        {
            callbackList_.removeCallbackWithGid(callback);
        }

        g_id gid;
        ALCdevice *device;
        int numChannels;
        int bitsPerSample;
        int bytesPerSample;
        bool isStarted;
        pthread_t thread;
        volatile bool exit;

    private:
        static void callback_s(int type, void *event, void *udata)
        {
            static_cast<Microphone*>(udata)->callback(type, event);
        }

        void callback(int type, void *event)
        {
            callbackList_.dispatchEvent(type, event);
        }

    private:
        gevent_CallbackList callbackList_;
    };

    std::map<g_id, Microphone*> microphones_;
};

static GMicrophoneManager *s_manager = NULL;

}

extern "C" {

void gmicrophone_Init()
{
    s_manager = new GMicrophoneManager;
}

void gmicrophone_Cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

g_id gmicrophone_Create(const char *deviceName, int numChannels, int sampleRate, int bitsPerSample, gmicrophone_Error *error)
{
    return s_manager->Create(deviceName, numChannels, sampleRate, bitsPerSample, error);
}

void gmicrophone_Delete(g_id microphone)
{
    s_manager->Delete(microphone);
}

void gmicrophone_Start(g_id microphone)
{
    s_manager->Start(microphone);
}

void gmicrophone_Stop(g_id microphone)
{
    s_manager->Stop(microphone);
}

g_id gmicrophone_AddCallback(g_id microphone, gevent_Callback callback, void *udata)
{
    return s_manager->AddCallback(microphone, callback, udata);
}

void gmicrophone_RemoveCallback(g_id microphone, gevent_Callback callback, void *udata)
{
    s_manager->RemoveCallback(microphone, callback, udata);
}

void gmicrophone_RemoveCallbackWithId(g_id microphone, g_id callback)
{
    s_manager->RemoveCallbackWithId(microphone, callback);
}

}
