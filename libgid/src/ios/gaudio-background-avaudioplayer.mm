#include "../ggaudiomanager.h"

#import <AVFoundation/AVFoundation.h>

#include <gpath.h>

@class GGAVAudioPlayerDelegate;

class GGBackgroundAVAudioPlayerManager : public GGBackgroundMusicInterface
{
public:
    GGBackgroundAVAudioPlayerManager()
    {

    }
    
    ~GGBackgroundAVAudioPlayerManager()
    {
        while (!sounds_.empty())
        {
            Sound *sound = sounds_.begin()->second;
            BackgroundMusicDelete(sound->gid);
        }
    }
    
    virtual g_id BackgroundMusicCreateFromFile(const char *fileName, gaudio_Error *error)
    {
        const char *fileName2 = gpath_transform(fileName);

        FILE *fis = fopen(fileName2, "rb");
        if (fis == NULL)
        {
            if (error)
                *error = GAUDIO_CANNOT_OPEN_FILE;
            return 0;
        }
        fclose(fis);

        NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:fileName2]];
        
        NSError *error2;
        AVAudioPlayer *player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error2];

        if (player == nil)
        {
            if (error)
                *error = GAUDIO_UNRECOGNIZED_FORMAT;
            return 0;
        }
        
        unsigned int length = player.duration * 1000;

        [player release];

        g_id gid = g_NextId();
        sounds_[gid] = new Sound(gid, fileName2, length);

        if (error)
            *error = GAUDIO_NO_ERROR;
        
        return gid;
    }

    virtual void BackgroundMusicDelete(g_id backgroundMusic)
    {
        std::map<g_id, Sound*>::iterator iter = sounds_.find(backgroundMusic);
        if (iter == sounds_.end())
            return;
        
        Sound *sound2 = iter->second;
        
        std::set<Channel*>::iterator iter2, e = sound2->channels.end();
        for (iter2 = sound2->channels.begin(); iter2 != e; ++iter2)
        {
            Channel *channel = *iter2;
            
            if (channel->player)
            {
                id<AVAudioPlayerDelegate> delegate = channel->player.delegate;
                channel->player.delegate = nil;
                [delegate release];                
                [channel->player release];
            }
            
            channels_.erase(channel->gid);
            
            gevent_RemoveEventsWithGid(channel->gid);
            
            delete channel;
        }

        delete sound2;
        
        sounds_.erase(iter);
    }

    virtual unsigned int BackgroundMusicGetLength(g_id backgroundMusic)
    {
        std::map<g_id, Sound*>::iterator iter = sounds_.find(backgroundMusic);
        if (iter == sounds_.end())
            return 0;

        Sound *sound2 = iter->second;

        return sound2->length;
    }
    
    virtual g_id BackgroundMusicPlay(g_id backgroudMusic, bool paused);
    
    virtual void BackgroundChannelStop(g_id backgroundChannel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel2 = iter->second;
        
        if (channel2->player)
        {
            id<AVAudioPlayerDelegate> delegate = channel2->player.delegate;
            channel2->player.delegate = nil;
            [delegate release];
            [channel2->player release];
            channel2->player = nil;
        }
        
        channel2->sound->channels.erase(channel2);
        
        gevent_RemoveEventsWithGid(channel2->gid);
        
        delete channel2;
        
        channels_.erase(iter);
    }

    virtual void BackgroundChannelSetPosition(g_id backgroundChannel, unsigned int position)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel2 = iter->second;
                
        if (channel2->player == nil)
            return;

        channel2->player.currentTime = position / 1000.0;
        
        if (channel2->paused)
            [channel2->player prepareToPlay];
    }

    virtual unsigned int BackgroundChannelGetPosition(g_id backgroundChannel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return 0;
        
        Channel *channel2 = iter->second;
                
        if (channel2->player == nil)
            return channel2->lastPosition;
        
        return channel2->player.currentTime * 1000.0;
    }
    
    virtual void BackgroundChannelSetPaused(g_id backgroundChannel, bool paused)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel2 = iter->second;

        channel2->paused = paused;
        
        if (channel2->player)
        {
            if (paused)
                [channel2->player pause];
            else
                [channel2->player play];
        }
    }

    virtual bool BackgroundChannelIsPaused(g_id backgroundChannel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return false;
        
        Channel *channel2 = iter->second;
        
        return channel2->paused;
    }

    virtual bool BackgroundChannelIsPlaying(g_id backgroundChannel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return false;
        
        Channel *channel2 = iter->second;

        if (channel2->player == nil)
            return false;
        
        return channel2->player.isPlaying;
    }

    virtual void BackgroundChannelSetVolume(g_id backgroundChannel, float volume)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel2 = iter->second;
        
        channel2->volume = volume;
        
        if (channel2->player)
            channel2->player.volume = volume;
    }

    virtual float BackgroundChannelGetVolume(g_id backgroundChannel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return 0.f;
        
        Channel *channel2 = iter->second;
        
        return channel2->volume;
    }

    virtual void BackgroundChannelSetLooping(g_id backgroundChannel, bool looping)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel2 = iter->second;

        channel2->looping = looping;
        
        if (channel2->player)
            channel2->player.numberOfLoops = looping ? -1 : 0;
    }

    virtual bool BackgroundChannelIsLooping(g_id backgroundChannel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return false;
        
        Channel *channel2 = iter->second;
        
        return channel2->looping;
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
    
    g_bool BackgroundChannelIsValid(g_id backgroundChannel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return g_false;
        
        Channel *channel2 = iter->second;
        
        return channel2->player != nil;
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
            
            if (channel2->player == nil)
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
    
    void audioPlayerDidFinishPlaying(g_id backgroundChannel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel = iter->second;
        
        if (channel->player)
        {
            id<AVAudioPlayerDelegate> delegate = channel->player.delegate;
            channel->player.delegate = nil;
            [delegate release];
            [channel->player release];
            channel->player = nil;
        }

        channel->lastPosition = channel->sound->length;
        
        gaudio_ChannelCompleteEvent *event = (gaudio_ChannelCompleteEvent*)malloc(sizeof(gaudio_ChannelCompleteEvent));
        event->channel = channel->gid;
        
        gevent_EnqueueEvent(channel->gid, callback_s, GAUDIO_CHANNEL_COMPLETE_EVENT, event, 1, channel);
    }
    
    void audioPlayerBeginInterruption(g_id backgroundChannel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel2 = iter->second;

        channel2->interrupted = true;
    }
    
    void audioPlayerEndInterruption(g_id backgroundChannel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel2 = iter->second;
        
        if (channel2->interrupted == true)
        {
            if (channel2->player != nil)
                [channel2->player play];
            
            channel2->interrupted = false;
        }        
    }
    
    void audioPlayerEndInterruption_withFlags(g_id backgroundChannel, NSUInteger flags)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(backgroundChannel);
        if (iter == channels_.end())
            return;
        
        Channel *channel2 = iter->second;
        
        if (channel2->interrupted == true)
        {
            if (channel2->player != nil)
#if TARGET_OS_TV == 0
                if (flags & AVAudioSessionInterruptionFlags_ShouldResume)
                    [channel2->player play];
#endif
            
            channel2->interrupted = false;
        }
    }

private:
    struct Channel;
    
    struct Sound
    {
        Sound(g_id gid, const char *fileName, unsigned int length) :
            gid(gid),
            fileName(fileName),
            length(length)
        {
            
        }
        
        g_id gid;
        std::string fileName;        
        unsigned int length;
        std::set<Channel*> channels;
    };
    
    struct Channel
    {
        Channel(g_id gid, AVAudioPlayer *player, Sound *sound) :
            gid(gid),
            player(player),
            sound(sound),
            paused(true),
            volume(1.f),
            looping(false),
            interrupted(false),
            lastPosition(0)
        {
            
        }
        
        g_id gid;
        Sound *sound;
        AVAudioPlayer *player;
        bool paused;
        float volume;
        bool looping;
        bool interrupted;
        unsigned int lastPosition;
        gevent_CallbackList callbackList;
    };

    std::map<g_id, Sound*> sounds_;
    std::map<g_id, Channel*> channels_;

    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<Channel*>(udata)->callbackList.dispatchEvent(type, event);
    }
};

@interface GGAVAudioPlayerDelegate : NSObject <AVAudioPlayerDelegate>
{
    GGBackgroundAVAudioPlayerManager *manager_;
	g_id backgroundChannel_;
}

@property (nonatomic, assign) GGBackgroundAVAudioPlayerManager *manager;
@property (nonatomic, assign) g_id backgroundChannel;

@end

@implementation GGAVAudioPlayerDelegate

@synthesize manager = manager_;
@synthesize backgroundChannel = backgroundChannel_;

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
    manager_->audioPlayerDidFinishPlaying(backgroundChannel_);
}

- (void)audioPlayerBeginInterruption:(AVAudioPlayer *)player
{
    manager_->audioPlayerBeginInterruption(backgroundChannel_);
}

- (void)audioPlayerEndInterruption:(AVAudioPlayer *)player
{
    manager_->audioPlayerEndInterruption(backgroundChannel_);
}

- (void)audioPlayerEndInterruption:(AVAudioPlayer *)player withFlags:(NSUInteger)flags
{
    manager_->audioPlayerEndInterruption_withFlags(backgroundChannel_, flags);
}

@end

g_id GGBackgroundAVAudioPlayerManager::BackgroundMusicPlay(g_id backgroudMusic, bool paused)
{
    std::map<g_id, Sound*>::iterator iter = sounds_.find(backgroudMusic);
    if (iter == sounds_.end())
        return 0;
    
    Sound *sound2 = iter->second;
    
    NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:sound2->fileName.c_str()]];
    
    NSError *error2;
    AVAudioPlayer *player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error2];
    
    if (player == nil)
        return 0;
    
    g_id gid = g_NextId();

    GGAVAudioPlayerDelegate *delegate = [[GGAVAudioPlayerDelegate alloc] init];
    delegate.manager = this;
    delegate.backgroundChannel = gid;
    
    player.delegate = delegate;
    
    Channel *channel = new Channel(gid, player, sound2);
    
    sound2->channels.insert(channel);
    
    channels_[gid] = channel;
    
    channel->paused = paused;
    if (paused)
        [channel->player prepareToPlay];
    else
        [channel->player play];
    
    return gid;
}


extern "C"
{

GGBackgroundMusicInterface *GGBackgroundAVAudioPlayerManagerCreate()
{
    return new GGBackgroundAVAudioPlayerManager();
}

void GGBackgroundAVAudioPlayerManagerDelete(GGBackgroundMusicInterface *manager)
{
    delete manager;
}

}
