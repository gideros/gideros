/*
 * Copyright (c) 2014, Smart Balloon, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of 'Smart Balloon, Inc.' nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Foundation/Foundation.h>

/** HZVideoAd is responsible for fetching and showing video ads. */
@interface HZVideoAd : NSObject

#pragma mark - Showing Ads

/** Shows a video ad if one is available */
+ (void) show;

/**
 *  Shows a video ad for a given tag, if available.
 *
 *  @param tag An identifier for the location of the ad which you can use to disable the ad from your dashboard.
 */
+ (void) showForTag: (NSString *) tag;

/**
 *  Shows a video for a given tag, if available.
 *
 *  @param tag        An identifier for the location of the ad which you can use to disable the ad from your dashboard.
 *  @param completion A block called when the video is shown or fails to show. `result` states whether the show was sucessful; the error object describes the issue, if there was one.
 */
+ (void) showForTag:(NSString *)tag completion:(void (^)(BOOL result, NSError *error))completion;


/**
 *  Fetches a video ad from Heyzap.
 */
+ (void) fetch;


/**
 *  Fetches a video ad from Heyzap.
 *
 *  @param completion A block called when the video is fetched or fails to fetch. `result` states whether the fetch was sucessful; the error object describes the issue, if there was one.
 */
+ (void) fetchWithCompletion: (void (^)(BOOL result, NSError *error))completion;

/** Fetches a new ad for a tag from Heyzap.
 @param tag An identifier for the location of the ad which you can use to disable the ad from your dashboard.
 */
+ (void) fetchForTag: (NSString *) tag;


/**
 *  Fetches a new ad for the given tag.
 *
 *  @param tag        An identifier for the location of the ad which you can use to disable the ad from your dashboard.
 *  @param completion A block called when the video is fetched or fails to fetch. `result` states whether the fetch was sucessful; the error object describes the issue, if there was one.
 *
 */
+ (void) fetchForTag:(NSString *)tag withCompletion: (void (^)(BOOL result, NSError *error))completion;

/**
 *  Whether or not a video ad is ready to show
 *
 *  @return If the video is ready to show
 */
+ (BOOL) isAvailable;

/**
 *  Whether or not the video is ready to show for the given tag
 *
 *  @param tag An identifier for the location of the ad which you can use to disable the ad from your dashboard.
 *
 *  @return Whether or not the video is ready to show for the given tag
 */
+ (BOOL) isAvailableForTag: (NSString *) tag;

/**
 *  Dismisses the current ad, if visible.
 */
+ (void) hide;

# pragma mark - Testing;
+ (void) setCreativeID:(int)creativeID;

@end
