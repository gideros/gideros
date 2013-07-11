#include <jni.h>
#include <stdlib.h>

const int INFO_SAMPLERATE = 0;
const int INFO_DSPBUFFERLENGTH = 1;
const int INFO_DSPNUMBUFFERS = 2;
const int INFO_MIXERRUNNING = 3;


extern "C"
{
extern int g_sampleRate;
extern int g_bufferLength;
extern int g_bufferCount;
extern int g_mixerRunning;
void g_processBuffer(void *buffer, size_t size);
}

extern "C"
{
jint Java_com_giderosmobile_android_player_AudioDevice_getInfo(JNIEnv* env, jobject obj, int paramInt)
{
	switch (paramInt)
	{
		case INFO_SAMPLERATE:
			return g_sampleRate;
		case INFO_DSPBUFFERLENGTH:
			return g_bufferLength;
		case INFO_DSPNUMBUFFERS:
			return g_bufferCount;
		case INFO_MIXERRUNNING:
			return g_mixerRunning;
	}

	return 0;
}

jint Java_com_giderosmobile_android_player_AudioDevice_process(JNIEnv* env, jobject obj, jobject paramByteBuffer)
{
	void* buffer = env->GetDirectBufferAddress(paramByteBuffer);
	jlong size = env->GetDirectBufferCapacity(paramByteBuffer);

	g_processBuffer(buffer, size/(2*2));	// 2 for short, 2 for stereo

	return 0;
}
}
