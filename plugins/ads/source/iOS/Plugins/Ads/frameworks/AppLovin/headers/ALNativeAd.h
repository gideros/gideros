//
//  ALNativeAd.h
//  sdk
//
//  Created by Matt Szaro on 5/21/15.
//
//

#import <Foundation/Foundation.h>
#import "ALNullabilityAnnotations.h"

@interface ALNativeAd : NSObject

/**
 *  A unique ID which identifies this advertisement.
 *
 *  Should you need to report a broken ad to AppLovin support, please include this number's longValue.
 */
@property (strong, nonatomic, readonly) NSNumber* __alnonnull adIdNumber;

/**
 *  The title of the native ad.
 */
@property (copy, nonatomic, readonly)   NSString* __alnullable title;

/**
 *  The description of the native ad.
 */
@property (copy, nonatomic, readonly)   NSString* __alnullable descriptionText;

/**
 *  The caption text of the native ad.
 */
@property (copy, nonatomic, readonly)   NSString* __alnullable captionText;

/**
 *  The CTA text of the native ad.
 */
@property (copy, nonatomic, readonly)   NSString* __alnullable ctaText;

/**
 *  The app icon URL of the native ad.
 */
@property (copy, nonatomic, readonly)   NSURL* __alnullable iconURL;

/**
 *  The ad image URL for a non-video native ad.
 */
@property (copy, nonatomic, readonly)   NSURL* __alnullable imageURL;

/**
 *  The star rating image URL of the native ad. Please use floatValue when extracting value from the NSNumber
 */
@property (strong, nonatomic, readonly) NSNumber* __alnullable starRating;

/**
 *  The video URL for a video native ad.
 *
 *  Note that if this native ad does not contain a video, this property will be nil.
 */
@property (copy, nonatomic, readonly)   NSURL* __alnullable videoURL;

/**
 *  The impression tracking URL of the native ad.
 */
@property (copy, nonatomic, readonly)   NSURL* __alnonnull impressionTrackingURL;

/**
 *  The click URL the native ad redirects to.
 */
@property (copy, nonatomic, readonly)   NSURL* __alnonnull clickURL;

/**
 *  The video begin tracking URL of the native ad.
 *
 *  Note that if this native ad does not contain a video, this property will be nil.
 */
@property (copy, nonatomic, readonly)   NSURL* __alnullable videoStartTrackingURL;

/**
 * Retrieve the URL which should be fired upon video completion.
 *
 * @param percentViewed The percentage of the video (0 - 100) that was viewed by the user.
 * @param firstPlay Whether or not this postback represents initial playback of the video. The first time you begin playback, you should pass true. If the video is paused for any reason and then later resumed mid-playback, you should fire this postback a second time, passing false to firstPlay.
 */
- (alnullable NSURL*) videoEndTrackingURL: (NSUInteger) percentViewed firstPlay: (BOOL) firstPlay;

/**
 *  Represents the precaching states of the slot's images.
 */
@property (assign, atomic, readonly, getter=isImagePrecached) BOOL imagePrecached;

/**
 *  Represents the precaching state of the slot's video.
 *
 *  Note that if this native ad does not contain a video, this property will always be NO.
 */
@property (assign, atomic, readonly, getter=isVideoPrecached) BOOL videoPrecached;

@end
