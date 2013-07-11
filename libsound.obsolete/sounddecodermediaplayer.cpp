#include "sounddecodermediaplayer.h"
#include <platformutil.h>
#include <gfile.h>
#include <gfile_p.h>
#include <gvfs-android.h>

extern "C"
{

void Java_com_giderosmobile_android_player_MediaPlayerManager_nativeSoundCompleteCallback(JNIEnv* env, jclass cls, jint soundChannel, jlong data)
{
	SoundDecoderMediaPlayer* that = (SoundDecoderMediaPlayer*)data;
	that->soundCompleteCallback(soundChannel);
}

void Java_com_giderosmobile_android_player_MediaPlayerManager_nativeSoundChannelPaused(JNIEnv* env, jclass cls, jint soundChannel, jlong data)
{
	SoundDecoderMediaPlayer* that = (SoundDecoderMediaPlayer*)data;
	that->soundChannelPaused(soundChannel);
}

void Java_com_giderosmobile_android_player_MediaPlayerManager_nativeSoundChannelResumed(JNIEnv* env, jclass cls, jint soundChannel, jlong data)
{
	SoundDecoderMediaPlayer* that = (SoundDecoderMediaPlayer*)data;
	that->soundChannelResumed(soundChannel);
}

}

void SoundDecoderMediaPlayer::soundCompleteCallback(unsigned int soundChannel)
{
	std::map<unsigned int, SoundCompleteCallback_t>::iterator iter = soundCompleteCallbacks_.find(soundChannel + nextid_);

	if (iter == soundCompleteCallbacks_.end())
		return;

	if (iter->second.callback)
		soundCompleteCallbacks2_.push_back(iter->second);
}

SoundDecoderMediaPlayer::SoundDecoderMediaPlayer(unsigned int nextid, JavaVM* vm) : nextid_(nextid), vm_(vm)
{
	JNIEnv* env = getEnv();

	jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
	javaNativeBridge_ = (jclass)env->NewGlobalRef(localRefCls);
	env->DeleteLocalRef(localRefCls);

	createSoundID_ = env->GetStaticMethodID(javaNativeBridge_, "createSound", "(Ljava/lang/String;Z)I");
	destroySoundID_ = env->GetStaticMethodID(javaNativeBridge_, "destroySound", "(I)V");
	getSoundLengthID_ = env->GetStaticMethodID(javaNativeBridge_, "getSoundLength", "(I)D");
	playSoundID_ = env->GetStaticMethodID(javaNativeBridge_, "playSound", "(IDZJ)I");
	destroySoundChannelID_ = env->GetStaticMethodID(javaNativeBridge_, "destroySoundChannel", "(I)V");
	getSoundChannelOffsetID_ = env->GetStaticMethodID(javaNativeBridge_, "getSoundChannelOffset", "(I)D");
	setSoundChannelLoopingID_ = env->GetStaticMethodID(javaNativeBridge_, "setSoundChannelLooping", "(IZ)V");
	setSoundChannelVolumeID_ = env->GetStaticMethodID(javaNativeBridge_, "setSoundChannelVolume", "(IF)V");
	getSoundChannelVolumeID_ = env->GetStaticMethodID(javaNativeBridge_, "getSoundChannelVolume", "(I)F");
	destroyAllID_ = env->GetStaticMethodID(javaNativeBridge_, "destroyAll", "()V");
	getSoundErrorID_ = env->GetStaticMethodID(javaNativeBridge_, "getSoundError", "()I");

	soundError_ = eNoSoundError;
}

SoundDecoderMediaPlayer::~SoundDecoderMediaPlayer()
{
	JNIEnv* env = getEnv();

	env->DeleteGlobalRef(javaNativeBridge_);
}

JNIEnv* SoundDecoderMediaPlayer::getEnv()
{
	JNIEnv* env = NULL;
	vm_->GetEnv((void**)&env, JNI_VERSION_1_4);
	return env;
}

unsigned int SoundDecoderMediaPlayer::createSound(const char* file)
{
	const char* fileName = NULL;

	if (gvfs_isPlayerModeEnabled())
		fileName = g_pathForFile(file);		// player
	else
		fileName = file;					// not player

	JNIEnv* env = getEnv();

	jstring jfileName = env->NewStringUTF(fileName);

	int sound = env->CallStaticIntMethod(javaNativeBridge_, createSoundID_, jfileName, (jboolean)(getFileType(fileName) == eResourceFile));

	env->DeleteLocalRef(jfileName);

	if (sound == 0)
	{
		int error = env->CallStaticIntMethod(javaNativeBridge_, getSoundErrorID_);
		setSoundError((SoundError)error);

		return 0;
	}

	return sound + nextid_;
}

void SoundDecoderMediaPlayer::destroySound(unsigned int sound)
{
	JNIEnv* env = getEnv();
	env->CallStaticVoidMethod(javaNativeBridge_, destroySoundID_, (jint)(sound - nextid_));
}

double SoundDecoderMediaPlayer::getSoundLength(unsigned int sound)
{
	JNIEnv* env = getEnv();
	return env->CallStaticDoubleMethod(javaNativeBridge_, getSoundLengthID_, (jint)(sound - nextid_));
}

unsigned int SoundDecoderMediaPlayer::playSound(unsigned int sound, double msec, int loops)
{
	bool looping = loops >= 2;

	JNIEnv* env = getEnv();
	int soundChannel = env->CallStaticIntMethod(javaNativeBridge_, playSoundID_, (jint)(sound - nextid_), (jdouble)msec, (jboolean)looping, (jlong)this);

	if (soundChannel == 0)
		return 0;

	if (looping)
	{
		double length = getSoundLength(sound);
		SoundChannelClock_t clock;
		clock.endTime = (loops - 0.5) * length - msec;
		soundChannelClocks_[soundChannel] = clock;
	}

	return soundChannel + nextid_;
}

void SoundDecoderMediaPlayer::destroySoundChannel(unsigned int soundChannel)
{
	JNIEnv* env = getEnv();
	env->CallStaticVoidMethod(javaNativeBridge_, destroySoundChannelID_, (jint)(soundChannel - nextid_));

	soundCompleteCallbacks_.erase(soundChannel);
	soundChannelClocks_.erase(soundChannel);
}

double SoundDecoderMediaPlayer::getSoundChannelOffset(unsigned int soundChannel)
{
	JNIEnv* env = getEnv();
	return env->CallStaticDoubleMethod(javaNativeBridge_, getSoundChannelOffsetID_, (jint)(soundChannel - nextid_));
}

void SoundDecoderMediaPlayer::setSoundChannelVolume(unsigned int soundChannel, float volume)
{
	JNIEnv* env = getEnv();
	env->CallStaticVoidMethod(javaNativeBridge_, setSoundChannelVolumeID_, (jint)(soundChannel - nextid_), (jfloat)volume);
}

float SoundDecoderMediaPlayer::getSoundChannelVolume(unsigned int soundChannel)
{
	JNIEnv* env = getEnv();
	return env->CallStaticFloatMethod(javaNativeBridge_, getSoundChannelVolumeID_, (jint)(soundChannel - nextid_));
}

void SoundDecoderMediaPlayer::setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data)
{
	soundCompleteCallbacks_[soundChannel] = SoundCompleteCallback_t(soundChannel, callback, data);
}

void SoundDecoderMediaPlayer::tickSound()
{
	for (size_t i = 0; i < soundCompleteCallbacks2_.size(); ++i)
	{
		int id = soundCompleteCallbacks2_[i].id;

		if (soundCompleteCallbacks_.find(id) != soundCompleteCallbacks_.end())
		{
			void(*callback)(unsigned int, void*) = soundCompleteCallbacks2_[i].callback;
			void* data = soundCompleteCallbacks2_[i].data;

			if (callback)
				callback(id, data);
		}
	}

	soundCompleteCallbacks2_.clear();

	std::map<unsigned int, SoundChannelClock_t>::iterator
			iter = soundChannelClocks_.begin(),
			e = soundChannelClocks_.end();

	while (iter != e)
	{
		if (iter->second.stopWatch.clock() * 1000 > iter->second.endTime)
		{
			setSoundChannelLooping(iter->first, false);
			soundChannelClocks_.erase(iter++);
		}
		else
			++iter;
	}
}

void SoundDecoderMediaPlayer::destroyAll()
{
	soundCompleteCallbacks_.clear();
	soundCompleteCallbacks2_.clear();

	JNIEnv* env = getEnv();
	env->CallStaticVoidMethod(javaNativeBridge_, destroyAllID_);
}

void SoundDecoderMediaPlayer::soundChannelPaused(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannelClock_t>::iterator iter = soundChannelClocks_.find(soundChannel);
	if (iter == soundChannelClocks_.end())
		return;

	iter->second.stopWatch.pause();
}

void SoundDecoderMediaPlayer::soundChannelResumed(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannelClock_t>::iterator iter = soundChannelClocks_.find(soundChannel);
	if (iter == soundChannelClocks_.end())
		return;

	iter->second.stopWatch.resume();
}

void SoundDecoderMediaPlayer::setSoundChannelLooping(unsigned int soundChannel, bool looping)
{
	JNIEnv* env = getEnv();
	env->CallStaticVoidMethod(javaNativeBridge_, setSoundChannelLoopingID_, soundChannel, (jboolean)looping);
}
