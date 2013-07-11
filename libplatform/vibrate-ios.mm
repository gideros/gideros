#import <AudioToolbox/AudioToolbox.h>

void vibrate()
{
	AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}