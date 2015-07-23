#include "gmicrophone.h"

#include <jni.h>

#include <map>

#include <math.h>
#include <stdlib.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

namespace {

static void downsample16(int numChannels, int sampleCount, int newSampleCount, const void *in, void *out)
{
    const short *cin = (const short*)in;
    short *cout = (short*)out;

    for (int i = 0; i < newSampleCount; ++i)
    {
        int j = (i * sampleCount + (sampleCount / 2)) / newSampleCount;
        for (int k = 0; k < numChannels; ++k)
        {
            cout[i * numChannels + k] = cin[j * numChannels + k];
        }
    }
}

static void downsample8(int numChannels, int sampleCount, int newSampleCount, const void *in, void *out)
{
    const short *cin = (const short*)in;
    unsigned char *cout = (unsigned char*)out;

    for (int i = 0; i < newSampleCount; ++i)
    {
        int j = (i * sampleCount + (sampleCount / 2)) / newSampleCount;
        for (int k = 0; k < numChannels; ++k)
        {
            cout[i * numChannels + k] = (cin[j * numChannels + k] >> 8) + 128;
        }
    }
}

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
		JNIEnv *env = g_getJNIEnv();
		jclass localClass = env->FindClass("com/giderosmobile/android/plugins/microphone/GMicrophone");
		cls_ = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Init", "(J)V"), (jlong)this);
	}

	~GMicrophoneManager()
	{
        while (!microphones_.empty())
            Delete(microphones_.begin()->first);

		JNIEnv *env = g_getJNIEnv();

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Cleanup", "()V"));

		env->DeleteGlobalRef(cls_);
	}

    g_id Create(const char *deviceName, int numChannels, int sampleRate, int bitsPerSample, gmicrophone_Error *error)
	{
		JNIEnv *env = g_getJNIEnv();

        g_id gid = g_NextId();	

		jint error2 = env->CallStaticIntMethod(cls_, env->GetStaticMethodID(cls_, "Create", "(JIII)I"), (jlong)gid, (jint)numChannels, (jint)44100, (jint)16);

		if (error2 != GMICROPHONE_NO_ERROR)
		{
			if (error)
				*error = (gmicrophone_Error)error2;
			return 0;
		}
		
		microphones_[gid] = new Microphone(gid, numChannels, sampleRate, bitsPerSample);
		
		if (error)
			*error = GMICROPHONE_NO_ERROR;
			
		return gid;
	}
	
	void Delete(g_id microphone)
    {
		JNIEnv *env = g_getJNIEnv();

        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);

        if (iter == microphones_.end())
            return;
			
        Microphone *microphone2 = iter->second;

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Delete", "(J)V"), (jlong)microphone);
		
		delete microphone2;

        microphones_.erase(microphone);

        gevent_RemoveEventsWithGid(microphone);
	}

    void Start(g_id microphone)
    {
		JNIEnv *env = g_getJNIEnv();

        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);

        if (iter == microphones_.end())
            return;

        Microphone *microphone2 = iter->second;

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Start", "(J)V"), (jlong)microphone);
    }

    void Stop(g_id microphone)
    {
		JNIEnv *env = g_getJNIEnv();

        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);

        if (iter == microphones_.end())
            return;

        Microphone *microphone2 = iter->second;

		env->CallStaticVoidMethod(cls_, env->GetStaticMethodID(cls_, "Stop", "(J)V"), (jlong)microphone);

        gevent_RemoveEventsWithGid(microphone);
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
	
	void onDataAvailable(g_id microphone, void *data, int size)
	{
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);

        if (iter == microphones_.end())
            return;

        Microphone *microphone2 = iter->second;
		
		microphone2->onDataAvailable(data, size);
	}

private:
    class Microphone
    {
    public:
        Microphone(g_id gid, int numChannels, int sampleRate, int bitsPerSample) :
			gid(gid),
			numChannels(numChannels),
			sampleRate(sampleRate),			
            bitsPerSample(bitsPerSample)
        {
            bytesPerSample = ((bitsPerSample + 7) / 8) * numChannels;
        }

        g_id gid;
		int numChannels;
		int sampleRate;
        int bitsPerSample;
		int bytesPerSample;

	public:
		void onDataAvailable(void *data, int size)
		{
			int sampleCount = size / numChannels;
			int newSampleCount = (sampleCount * sampleRate) / 44100;
			size_t structSize = sizeof(gmicrophone_DataAvailableEvent);
			size_t dataSize = newSampleCount * bytesPerSample;

			gmicrophone_DataAvailableEvent *event = (gmicrophone_DataAvailableEvent*)malloc(structSize + dataSize);

			event->microphone = gid;
			event->data = (char*)event + structSize;
			event->sampleCount = newSampleCount;
			
			if (bitsPerSample == 8)
				downsample8(numChannels, sampleCount, newSampleCount, data, event->data);
			else
				downsample16(numChannels, sampleCount, newSampleCount, data, event->data);

			calculateAmplitudeData(numChannels, bitsPerSample, event);

			gevent_EnqueueEvent(gid, callback_s, GMICROPHONE_DATA_AVAILABLE_EVENT, event, 1, this);
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
	
	jclass cls_;
};

static GMicrophoneManager *s_manager = NULL;

}

extern "C" {

JNIEXPORT void JNICALL Java_com_giderosmobile_android_plugins_microphone_GMicrophone_onDataAvailableByte(JNIEnv *env, jclass clz, jlong id, jbyteArray jaudioData, jint size, jlong data)
{
	jbyte *audioData = env->GetByteArrayElements(jaudioData, 0);
	unsigned char *audioData2 = (unsigned char*)audioData;
	for (int i = 0; i < size; ++i)
		audioData2[i] = (int)audioData[i] + 128;
	((GMicrophoneManager*)data)->onDataAvailable(id, audioData, size);
	env->ReleaseByteArrayElements(jaudioData, audioData, 0);	
}

JNIEXPORT void JNICALL Java_com_giderosmobile_android_plugins_microphone_GMicrophone_onDataAvailableShort(JNIEnv *env, jclass clz, jlong id, jshortArray jaudioData, jint size, jlong data)
{
	jshort *audioData = env->GetShortArrayElements(jaudioData, 0);
	((GMicrophoneManager*)data)->onDataAvailable(id, audioData, size);
	env->ReleaseShortArrayElements(jaudioData, audioData, 0);	
}

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
