//
//  HZNativeAdCollection.h
//  Heyzap
//
//  Created by Maximilian Tagher on 9/8/14.
//  Copyright (c) 2014 Heyzap. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 *  This object is a thin wrapper around an `NSArray` of `HZNativeAd`s. From it, you can access the individual ads and report an impression on all the ads at once.
 */
@interface HZNativeAdCollection : NSObject

/**
 *  An array of `HZNativeAd`s. This array is guaranteed to have alteast one object.
 */
@property (nonatomic, readonly) NSArray *ads;

/**
 *  If you request multiple ads and want to show them all at once (say, in a `UITableView`), call this method to report an impression for all of them.
 *
 *  @warning Do not reuse an `HZNativeAd` after showing it to the user. This will cause your impressions to be underreported. Instead fetch new ads using `HZNativeAdController.`
 */
- (void)reportImpressionOnAllAds;


@end
