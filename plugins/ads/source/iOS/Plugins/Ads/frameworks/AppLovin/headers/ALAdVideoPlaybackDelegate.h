//
//  ALAdVideoPlaybackDelegate.h
//  sdk
//
//  Created by Matt Szaro on 5/23/13.
//  Copyright (c) 2013, AppLovin Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ALAd.h"
/**
 * This protocol defines a listener for ad video playback events.
 * For ads which do not contain videos, no callbacks will be triggered.
 */
@class ALAdService;

@protocol ALAdVideoPlaybackDelegate <NSObject>

/**
 * This method is invoked when a video starts playing in an ad.
 *
 * This method is invoked on the main UI thread.
 *
 * @param ad Ad in which video playback began.
 */
-(void) videoPlaybackBeganInAd: (ALAd*) ad;

/**
 * This method is invoked when a video stops playing in an ad.
 *
 * This method is invoked on the main UI thread.
 *
 * @param ad                Ad in which video playback ended.
 * @param percentPlayed     How much of the video was watched, as a percent.
 * @param wasFullyWatched   Whether or not the video was watched to, or very near to, completion.
 */
-(void) videoPlaybackEndedInAd: (ALAd*) ad atPlaybackPercent:(NSNumber*) percentPlayed fullyWatched: (BOOL) wasFullyWatched;

@end
