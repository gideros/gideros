/*
 *  sounddecoderavaudioplayer.mm
 *  AudioPlayer
 *
 *  Created by Atilim Cetin on 11/9/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "sounddecoderavaudioplayer.h"
#import <AVFoundation/AVFoundation.h>
#include <platformutil.h>
#include <gfile.h>
#include <gfile_p.h>

@interface MyAVAudioPlayerDelegate : NSObject <AVAudioPlayerDelegate>
{
	SoundDecoderAVAudioPlayer::SoundChannel_t* soundChannel_;
}
@end

@implementation MyAVAudioPlayerDelegate

-(id) init: (SoundDecoderAVAudioPlayer::SoundChannel_t*)soundChannel 
{
	if(self = [super init])
	{
		soundChannel_ = soundChannel;
	}
	
	return self;
}
		
- (void)audioPlayerDidFinishPlaying: (AVAudioPlayer *)audioPlayer successfully: (BOOL)flag
{
	//printf("audioPlayerDidFinishPlaying\n");
	
	soundChannel_->isPlaying = false;
	soundChannel_->lastPosition = soundChannel_->sound->length;
	
	if (soundChannel_->callback != NULL)
		soundChannel_->callback(soundChannel_->id, soundChannel_->data);
}


- (void)audioPlayerBeginInterruption:(AVAudioPlayer *)player
{
	//printf("audioPlayerBeginInterruption\n");
}

- (void)audioPlayerEndInterruption:(AVAudioPlayer *)player
{
	//printf("audioPlayerEndInterruption\n");
	[player play];
}

- (void)audioPlayerEndInterruption:(AVAudioPlayer *)player withFlags:(NSUInteger)flags
{
	//printf("audioPlayerEndInterruption withFlags:%d\n", flags);
	if (flags & AVAudioSessionInterruptionFlags_ShouldResume)
		[player play];
}

@end


unsigned int SoundDecoderAVAudioPlayer::createSound(const char* file)
{
	const char* fileName = g_pathForFile(file);
	
	FILE* fis = fopen(fileName, "rb");
	if (fis == NULL)
	{
		setSoundError(eSoundFileNotFound);
		return 0;
	}
	fclose(fis);
	
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
		
	NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:fileName]];
	
	NSError* error;
	AVAudioPlayer* audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
	
	[pool release];

	if (audioPlayer == NULL)
	{
		setSoundError(eSoundFormatNotRecognized);
		return 0;
	}

	[audioPlayer prepareToPlay];

	sounds_[nextid_] = new Sound_t(audioPlayer, fileName, audioPlayer.duration * 1000);
			
	return nextid_++;
}


void SoundDecoderAVAudioPlayer::destroySound(unsigned int sound)
{
	std::map<unsigned int, Sound_t*>::iterator iter = sounds_.find(sound);
	
	if (iter == sounds_.end())
		return;

	if (iter->second->audioPlayer != NULL)
		[iter->second->audioPlayer release];
		
	delete iter->second;
	
	sounds_.erase(iter);
}

double SoundDecoderAVAudioPlayer::getSoundLength(unsigned int sound)
{
	std::map<unsigned int, Sound_t*>::iterator iter = sounds_.find(sound);
	
	if (iter == sounds_.end())
		return 0;
	
	return iter->second->length;
}

unsigned int SoundDecoderAVAudioPlayer::playSound(unsigned int sound, double msec, int loops)
{
	std::map<unsigned int, Sound_t*>::iterator iter = sounds_.find(sound);
	
	if (iter == sounds_.end())
		return 0;
	
	loops--;
	if (loops < 0)
		loops = 0;
	
	const char* fileName = iter->second->fileName.c_str();

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:fileName]];
	
	AVAudioPlayer* audioPlayer;
	if (iter->second->audioPlayer != NULL)
	{
		audioPlayer = iter->second->audioPlayer;
		iter->second->audioPlayer = NULL;
	}
	else
	{	
		NSError* error;
		audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
	}
	
	[pool release];

	if (audioPlayer == NULL)
		return 0;

	SoundChannel_t* soundChannel = new SoundChannel_t(iter->second, audioPlayer, nextid_, true, msec);
		
	audioPlayer.delegate = [[MyAVAudioPlayerDelegate alloc] init:soundChannel];
	audioPlayer.numberOfLoops = loops;
	audioPlayer.currentTime = msec / 1000;
	[audioPlayer play];
	
	soundChannels_[nextid_] = soundChannel;
	
	return nextid_++;
}

void SoundDecoderAVAudioPlayer::destroySoundChannel(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);
	
	if (iter == soundChannels_.end())
		return;
	
	[iter->second->audioPlayer.delegate release];
	iter->second->audioPlayer.delegate = nil;

	[iter->second->audioPlayer stop];
		
	if (iter->second->sound->audioPlayer != NULL)
		[iter->second->sound->audioPlayer release];
			
	iter->second->sound->audioPlayer = iter->second->audioPlayer;		
		
	delete iter->second;
	
	soundChannels_.erase(iter);
}

double SoundDecoderAVAudioPlayer::getSoundChannelOffset(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);
	
	if (iter == soundChannels_.end())
		return 0;
	
	double lastPosition = iter->second->audioPlayer.currentTime * 1000;

	if (iter->second->isPlaying == true)
	{
		if (iter->second->audioPlayer.playing == YES)
		{
			iter->second->lastPosition = lastPosition;
		}
	}
			
	return iter->second->lastPosition;
}

void SoundDecoderAVAudioPlayer::setSoundChannelVolume(unsigned int soundChannel, float volume)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);
	
	if (iter == soundChannels_.end())
		return;
	
	iter->second->audioPlayer.volume = volume;
}

float SoundDecoderAVAudioPlayer::getSoundChannelVolume(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);
	
	if (iter == soundChannels_.end())
		return 0;

	return iter->second->audioPlayer.volume;
}


void SoundDecoderAVAudioPlayer::setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);
	
	if (iter == soundChannels_.end())
		return;
	
	iter->second->callback = callback;
	iter->second->data = data;
}


void SoundDecoderAVAudioPlayer::destroyAll()
{
	while (soundChannels_.empty() == false)
		destroySoundChannel(soundChannels_.begin()->first);
	
	while (sounds_.empty() == false)
		destroySound(sounds_.begin()->first);	
}
