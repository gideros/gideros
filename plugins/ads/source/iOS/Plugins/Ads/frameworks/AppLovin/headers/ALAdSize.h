//
//  ALAdSize.h
//  sdk
//
//  Created by Basil on 2/27/12.
//  Copyright (c) 2013, AppLovin Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>

/*
 * @author Basil Shikin, Matt Szaro
 * @version 1.1
 */

/**
 * This class defines a size of an ad to be displayed. It is recommended to use default sizes that are
 * declared in this class (<code>BANNER</code>, <code>INTERSTITIAL</code>)
 */
@interface ALAdSize : NSObject<NSCopying>

/**
 * @name Ad Size Identification
 */

/**
 *  An <code>NSString</code> label which describes this ad size.
 */
@property (strong, nonatomic, readonly) NSString * label;

/**
 * @name Supported Ad Size Singletons
 */

/**
 *  Retrieve a singleton instance of the <code>BANNER</code> ad size object.
 *
 *  Banners are typically 320x50 ads added to the bottom of the view.
 *
 *  @return An instance of ALAdSize which represents the size <code>BANNER</code>.
 */
+(ALAdSize *) sizeBanner;

/**
 *  Retrieve a singleton instance of the <code>INTERSTITIAL</code> ad size object.
 *
 *  Interstitials are full-screen ads with high click-through rates which will fully cover the <code>UIViewController</code> below.
 *
 *  @return An instance of ALAdSize which represents the size <code>INTERSTITIAL</code>.
 */
+(ALAdSize *) sizeInterstitial;

/**
 *  Retrieve a singleton instance of the <code>MREC</code> ad size object.
 *
 *  MRECs are 320x250 (mostly square) advertisements.
 *
 *  @return An instance of ALAdSize which represents the size <code>MREC</code>.
 */
+(ALAdSize *) sizeMRec;

/**
 *  Retrieve a singleton instance of the <code>LEADER</code> ad size object.
 *
 *  Leaderboard ads are 728x90 ads intended for iPads.
 *
 *  @return An instance of ALAdSize which represents the size <code>LEADER</code>.
 */
+(ALAdSize *) sizeLeader;

/**
 *  Retrieve an <code>NSArray</code> of all available ad size singleton instances.
 *
 *  @return [NSArray arrayWithObjects: [ALAdSize sizeBanner], [ALAdSize sizeInterstitial], ..., nil];
 */
+(NSArray *) allSizes;

/**
 * @name Obtaining Instances from Strings
 */

/**
 *  Get a reference to the appropriate singleton based on a string. If no matching size exists, fall back to a default size.
 *
 *  @param label       A string representing the size you wish to retrieve. For example, <code>@"BANNER"</code>.
 *  @param defaultSize A singleton instance of ALAdSize to use in case the provided string does not correspond to a valid size. For example, [ALAdSize sizeBanner].
 *
 *  @return An instance of ALAdSize which matches the provided string, if available. If the provided string is not a valid ad size, then the <code>defaultSize</code> will be returned.
 */
+(ALAdSize*) sizeWithLabel: (NSString*) label orDefault: (ALAdSize*) defaultSize;

// ----------------------------------------------------

// These are no longer recommended or considered best practice.
// If possible, use a size like [ALAdSize sizeBanner] or [ALAdSize sizeInterstitial] instead.

@property (assign, nonatomic) NSUInteger width;
@property (assign, nonatomic) NSUInteger height;

-(instancetype) initWithLabel: (NSString *)label;
-(instancetype) initWithWidth: (NSUInteger)width height:(NSUInteger)height;

- (id)init __attribute__((unavailable("Use sizeWithLabel:orDefault: instead.")));
@end