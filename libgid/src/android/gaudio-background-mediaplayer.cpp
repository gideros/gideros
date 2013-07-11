#include "../ggaudiomanager.h"
#include <jni.h>
#include <gpath.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class GGBackgroundMediaPlayerManager : public GGBackgroundMusicInterface
{
public:
	GGBackgroundMediaPlayerManager()
	{
		JNIEnv *env = g_getJNIEnv();
		
		jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		cls_ = (jclass)env->NewGlobalRef(localRefCls);
		env->DeleteLocalRef(localRefCls);

		BackgroundMusicCreateFromFileID_ = env->GetStaticMethodID(cls_, "BackgroundMusicCreateFromFile", "(Ljava/lang/String;[I)J");
		BackgroundMusicDeleteID_ = env->GetStaticMethodID(cls_, "BackgroundMusicDelete", "(J)V");
		BackgroundMusicGetLengthID_ = env->GetStaticMethodID(cls_, "BackgroundMusicGetLength", "(J)I");
		BackgroundMusicPlayID_ = env->GetStaticMethodID(cls_, "BackgroundMusicPlay", "(JZJ)J");
		BackgroundChannelStopID_ = env->GetStaticMethodID(cls_, "BackgroundChannelStop", "(J)V");
		BackgroundChannelSetPositionID_ = env->GetStaticMethodID(cls_, "BackgroundChannelSetPosition", "(JI)V");
		BackgroundChannelGetPositionID_ = env->GetStaticMethodID(cls_, "BackgroundChannelGetPosition", "(J)I");
		BackgroundChannelSetPausedID_ = env->GetStaticMethodID(cls_, "BackgroundChannelSetPaused", "(JZ)V");
		BackgroundChannelIsPausedID_ = env->GetStaticMethodID(cls_, "BackgroundChannelIsPaused", "(J)Z");
		BackgroundChannelIsPlayingID_ = env->GetStaticMethodID(cls_, "BackgroundChannelIsPlaying", "(J)Z");
		BackgroundChannelSetVolumeID_ = env->GetStaticMethodID(cls_, "BackgroundChannelSetVolume", "(JF)V");
		BackgroundChannelGetVolumeID_ = env->GetStaticMethodID(cls_, "BackgroundChannelGetVolume", "(J)F");
		BackgroundChannelSetLoopingID_ = env->GetStaticMethodID(cls_, "BackgroundChannelSetLooping", "(JZ)V");
		BackgroundChannelIsLoopingID_ = env->GetStaticMethodID(cls_, "BackgroundChannelIsLooping", "(J)Z");
	}

	virtual ~GGBackgroundMediaPlayerManager()
	{
		JNIEnv *env = g_getJNIEnv();

        while (!sounds_.empty())
        {
            Sound *sound = sounds_.begin()->second;
            BackgroundMusicDelete(sound->gid);
        }

		env->DeleteGlobalRef(cls_);
	}

    virtual g_id BackgroundMusicCreateFromFile(const char *fileName, gaudio_Error *error)
	{
		JNIEnv *env = g_getJNIEnv();

		jstring jfileName = env->NewStringUTF(gpath_transform(fileName));
		
		jintArray jerrorArray = env->NewIntArray(1);
		
		g_id sound = env->CallStaticLongMethod(cls_, BackgroundMusicCreateFromFileID_, jfileName, jerrorArray);
	
		jint jerror;
		env->GetIntArrayRegion(jerrorArray, 0, 1, &jerror);
	
		env->DeleteLocalRef(jerrorArray);
		env->DeleteLocalRef(jfileName);
		
		if (error)
			*error = (gaudio_Error)jerror;
			
		if (sound == 0)
			return 0;

		sounds_[sound] = new Sound(sound);
		
		return sound;
	}

    virtual void BackgroundMusicDelete(g_id backgroundMusic)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Sound*>::iterator iter = sounds_.find(backgroundMusic);
		if (iter == sounds_.end())
			return;

		Sound *sound2 = iter->second;

		std::set<Channel*>::iterator iter2, e = sound2->channels.end();
		for (iter2 = sound2->channels.begin(); iter2 != e; ++iter2)
		{
			Channel *channel = *iter2;

			env->CallStaticVoidMethod(cls_, BackgroundChannelStopID_, (jlong)channel->gid);

			channels_.erase(channel->gid);

			gevent_RemoveEventsWithGid(channel->gid);

			delete channel;
		}

		env->CallStaticVoidMethod(cls_, BackgroundMusicDeleteID_, (jlong)backgroundMusic);

		delete sound2;

		sounds_.erase(iter);
	}
    virtual unsigned int BackgroundMusicGetLength(g_id backgroundMusic)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Sound*>::iterator iter = sounds_.find(backgroundMusic);
		if (iter == sounds_.end())
			return 0;

		Sound *sound2 = iter->second;

		return env->CallStaticIntMethod(cls_, BackgroundMusicGetLengthID_, (jlong)backgroundMusic);
	}

    virtual g_id BackgroundMusicPlay(g_id backgroudMusic, bool paused)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Sound*>::iterator iter = sounds_.find(backgroudMusic);
		if (iter == sounds_.end())
			return 0;

		Sound *sound2 = iter->second;

		g_id channel = env->CallStaticLongMethod(cls_, BackgroundMusicPlayID_, (jlong)backgroudMusic, (jboolean)paused, (jlong)this);

		Channel *channel2 = new Channel(channel, sound2);

		sound2->channels.insert(channel2);

		channels_[channel] = channel2;

		return channel;	
	}

	virtual void BackgroundChannelStop(g_id backgroundChannel)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
		if (iter == channels_.end())
			return;

		Channel *channel2 = iter->second;

		env->CallStaticVoidMethod(cls_, BackgroundChannelStopID_, (jlong)backgroundChannel);

		channel2->sound->channels.erase(channel2);

		gevent_RemoveEventsWithGid(channel2->gid);

		delete channel2;

		channels_.erase(iter);
	}

	virtual void BackgroundChannelSetPosition(g_id backgroundChannel, unsigned int position)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
		if (iter == channels_.end())
			return;

		Channel *channel2 = iter->second;

		env->CallStaticVoidMethod(cls_, BackgroundChannelSetPositionID_, (jlong)backgroundChannel, (jint)position);	
	}


    virtual unsigned int BackgroundChannelGetPosition(g_id backgroundChannel)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
		if (iter == channels_.end())
			return 0;

		Channel *channel2 = iter->second;

		return env->CallStaticIntMethod(cls_, BackgroundChannelGetPositionID_, (jlong)backgroundChannel);
	}
    
	virtual void BackgroundChannelSetPaused(g_id backgroundChannel, bool paused)
	{
		JNIEnv *env = g_getJNIEnv();
		
		std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
		if (iter == channels_.end())
			return;

		Channel *channel2 = iter->second;

		env->CallStaticVoidMethod(cls_, BackgroundChannelSetPausedID_, (jlong)backgroundChannel, (jboolean)paused);		
	}

    virtual bool BackgroundChannelIsPaused(g_id backgroundChannel)
	{
		JNIEnv *env = g_getJNIEnv();
		
		std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
		if (iter == channels_.end())
			return false;

		Channel *channel2 = iter->second;

		return env->CallStaticBooleanMethod(cls_, BackgroundChannelIsPausedID_, (jlong)backgroundChannel);
	}

    virtual bool BackgroundChannelIsPlaying(g_id backgroundChannel)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
		if (iter == channels_.end())
			return false;

		Channel *channel2 = iter->second;

		return env->CallStaticBooleanMethod(cls_, BackgroundChannelIsPlayingID_, (jlong)backgroundChannel);
	}

    virtual void BackgroundChannelSetVolume(g_id backgroundChannel, float volume)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
		if (iter == channels_.end())
			return;

		Channel *channel2 = iter->second;
		
		env->CallStaticVoidMethod(cls_, BackgroundChannelSetVolumeID_, (jlong)backgroundChannel, (jfloat)volume);
	}

    virtual float BackgroundChannelGetVolume(g_id backgroundChannel)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
		if (iter == channels_.end())
			return 0.f;

		Channel *channel2 = iter->second;

		return env->CallStaticFloatMethod(cls_, BackgroundChannelSetVolumeID_, (jlong)backgroundChannel);
	}

    virtual void BackgroundChannelSetLooping(g_id backgroundChannel, bool looping)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
		if (iter == channels_.end())
			return;

		Channel *channel2 = iter->second;
		
		env->CallStaticVoidMethod(cls_, BackgroundChannelSetLoopingID_, (jlong)backgroundChannel, (jboolean)looping);
	}

    virtual bool BackgroundChannelIsLooping(g_id backgroundChannel)
	{
		JNIEnv *env = g_getJNIEnv();

		std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
		if (iter == channels_.end())
			return false;

		Channel *channel2 = iter->second;

		return env->CallStaticBooleanMethod(cls_, BackgroundChannelIsLoopingID_, (jlong)backgroundChannel);
	}

    g_id BackgroundChannelAddCallback(g_id backgroundChannel, gevent_Callback callback, void *udata)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return 0;
        
        Channel *channel2 = iter->second;
        
        return channel2->callbackList.addCallback(callback, udata);
    }
    
    void BackgroundChannelRemoveCallback(g_id backgroundChannel, gevent_Callback callback, void *udata)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel2 = iter->second;
        
        channel2->callbackList.removeCallback(callback, udata);
    }
    
    void BackgroundChannelRemoveCallbackWithGid(g_id backgroundChannel, g_id gid)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel2 = iter->second;
        
        channel2->callbackList.removeCallbackWithGid(gid);   
    }

    void preTick()
    {
        
    }

    void postTick()
    {
        std::map<g_id, Channel*>::iterator iter = channels_.begin(), end = channels_.end();
        while (iter != end)
        {
            Channel *channel2 = iter->second;
            
            if (channel2->completed)
            {
                channel2->sound->channels.erase(channel2);
                delete channel2;
                channels_.erase(iter++);
            }
            else
            {
                ++iter;
            }
        }   
    }
	
public:
	void onChannelComplete(jlong backgroundChannel)
	{
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel = iter->second;
		
		channel->completed = true;
        
        gaudio_ChannelCompleteEvent *event = (gaudio_ChannelCompleteEvent*)malloc(sizeof(gaudio_ChannelCompleteEvent));
        event->channel = channel->gid;
        
        gevent_EnqueueEvent(channel->gid, callback_s, GAUDIO_CHANNEL_COMPLETE_EVENT, event, 1, channel);
	}

private:
	jclass cls_;
	jmethodID BackgroundMusicCreateFromFileID_;
	jmethodID BackgroundMusicDeleteID_;        
	jmethodID BackgroundMusicGetLengthID_;     
	jmethodID BackgroundMusicPlayID_;          
	jmethodID BackgroundChannelStopID_;        
	jmethodID BackgroundChannelSetPositionID_; 
	jmethodID BackgroundChannelGetPositionID_; 
	jmethodID BackgroundChannelSetPausedID_;   
	jmethodID BackgroundChannelIsPausedID_;    
	jmethodID BackgroundChannelIsPlayingID_;   
	jmethodID BackgroundChannelSetVolumeID_;   
	jmethodID BackgroundChannelGetVolumeID_;   
	jmethodID BackgroundChannelSetLoopingID_;  
	jmethodID BackgroundChannelIsLoopingID_;   

private:
    struct Channel;

    struct Sound
    {
        Sound(g_id gid) :
            gid(gid)
        {

        }

        g_id gid;
        std::set<Channel*> channels;
    };

    struct Channel
    {
        Channel(g_id gid, Sound *sound) :
            gid(gid),
            sound(sound),
			completed(false)
        {

        }

        g_id gid;
        Sound *sound;
		bool completed;
        gevent_CallbackList callbackList;
    };

    std::map<g_id, Sound*> sounds_;
    std::map<g_id, Channel*> channels_;

    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<Channel*>(udata)->callbackList.dispatchEvent(type, event);
    }
};

extern "C" {

GGBackgroundMusicInterface *GGBackgroundMediaPlayerManagerCreate()
{
	return new GGBackgroundMediaPlayerManager();
}

void GGBackgroundMediaPlayerManagerDelete(GGBackgroundMusicInterface *manager)
{
	delete manager;
}

jlong Java_com_giderosmobile_android_player_GGMediaPlayerManager_nextgid(JNIEnv* env, jclass cls)
{
	return g_NextId();
}

void Java_com_giderosmobile_android_player_GGMediaPlayerManager_onChannelComplete(JNIEnv* env, jclass cls, jlong backgroundChannel, jlong data)
{
	((GGBackgroundMediaPlayerManager*)data)->onChannelComplete(backgroundChannel);
}

}
