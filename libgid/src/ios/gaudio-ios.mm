#include <gaudio.h>
#include "../ggaudiomanager.h"

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

#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>

#if TARGET_OS_TV == 0
@interface GGAVAudioSessionDelegate : NSObject<AVAudioSessionDelegate>
#else
@interface GGAVAudioSessionDelegate : NSObject<NSObject>
#endif
{
    GGAudioManager *audioManager_;
}

@property (nonatomic, assign) GGAudioManager *audioManager;

@end

@implementation GGAVAudioSessionDelegate

@synthesize audioManager = audioManager_;

- (void)beginInterruption
{
    audioManager_->beginInterruption();
}

- (void)endInterruptionWithFlags:(NSUInteger)flags
{
#if TARGET_OS_TV == 0
    if (flags & AVAudioSessionInterruptionFlags_ShouldResume)
        audioManager_->endInterruption();
#endif
}

- (void)endInterruption
{
    audioManager_->endInterruption();
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    audioManager_->endInterruption();
}

@end

extern "C" {
GGSampleInterface *GGSampleOpenALManagerCreate();
void GGSampleOpenALManagerDelete(GGSampleInterface *manager);

GGStreamInterface *GGStreamOpenALManagerCreate();
void GGStreamOpenALManagerDelete(GGStreamInterface *manager);

GGBackgroundMusicInterface *GGBackgroundAVAudioPlayerManagerCreate();
void GGBackgroundAVAudioPlayerManagerDelete(GGBackgroundMusicInterface *manager);
}

struct GGAudioSystemData
{
    ALCdevice *device;
    ALCcontext *context;
    GGAVAudioSessionDelegate *delegate;
};

void GGAudioManager::systemInit()
{
    systemData_ = (GGAudioSystemData*)malloc(sizeof(GGAudioSystemData));

    [[AVAudioSession sharedInstance] setActive:YES error:nil];
    
    systemData_->delegate = [[GGAVAudioSessionDelegate alloc] init];
    systemData_->delegate.audioManager = this;
#if TARGET_OS_TV == 0
    [[AVAudioSession sharedInstance] setDelegate:systemData_->delegate];
#endif
    [[NSNotificationCenter defaultCenter]
     addObserver:systemData_->delegate
     selector:@selector(applicationDidBecomeActive:)
     name:UIApplicationDidBecomeActiveNotification
     object:nil];
    
    systemData_->device = alcOpenDevice(NULL);

    systemData_->context = alcCreateContext(systemData_->device, NULL);

    alcMakeContextCurrent(systemData_->context);
}

void GGAudioManager::systemCleanup()
{
    alcMakeContextCurrent(NULL);
    alcDestroyContext(systemData_->context);
    alcCloseDevice(systemData_->device);
#if TARGET_OS_TV == 0
    [[AVAudioSession sharedInstance] setDelegate:nil];
#endif
    [[NSNotificationCenter defaultCenter] removeObserver:systemData_->delegate];
    [systemData_->delegate release];

    [[AVAudioSession sharedInstance] setActive:NO error:nil];

    free(systemData_);
}

void GGAudioManager::createBackgroundMusicInterface()
{
    backgroundMusicInterface_ = GGBackgroundAVAudioPlayerManagerCreate();
}

void GGAudioManager::deleteBackgroundMusicInterface()
{
    GGBackgroundAVAudioPlayerManagerDelete(backgroundMusicInterface_);
}

void GGAudioManager::beginInterruption()
{
    if (interrupted_ == true)
        return;

    alcMakeContextCurrent(NULL);
    
    interrupted_ = true;
}

void GGAudioManager::endInterruption()
{
    if (interrupted_ == false)
        return;
    
    [NSThread sleepForTimeInterval:0.1];
    alcMakeContextCurrent(systemData_->context);

    interrupted_ = false;
}

void GGSoundManager::interfacesInit()
{
    loaders_["wav"] = GGAudioLoader(gaudio_WavOpen, gaudio_WavClose, gaudio_WavRead, gaudio_WavSeek, gaudio_WavTell);
    loaders_["mod"] = GGAudioLoader(gaudio_XmpOpen, gaudio_XmpClose, gaudio_XmpRead, gaudio_XmpSeek, gaudio_XmpTell);
    loaders_["xm"] = GGAudioLoader(gaudio_XmpOpen, gaudio_XmpClose, gaudio_XmpRead, gaudio_XmpSeek, gaudio_XmpTell);
    loaders_["it"] = GGAudioLoader(gaudio_XmpOpen, gaudio_XmpClose, gaudio_XmpRead, gaudio_XmpSeek, gaudio_XmpTell);
    loaders_["s3m"] = GGAudioLoader(gaudio_XmpOpen, gaudio_XmpClose, gaudio_XmpRead, gaudio_XmpSeek, gaudio_XmpTell);


    sampleInterface_ = GGSampleOpenALManagerCreate();
    streamInterface_ = GGStreamOpenALManagerCreate();
}

void GGSoundManager::interfacesCleanup()
{
    GGSampleOpenALManagerDelete(sampleInterface_);
    GGStreamOpenALManagerDelete(streamInterface_);
}

void GGAudioManager::AdvanceStreamBuffers()
{

}

void GGSoundManager::AdvanceStreamBuffers()
{

}
