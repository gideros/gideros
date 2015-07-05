//
//  ALAdSize.h
//  sdk
//
//  Created by Basil on 2/27/12.
//  Copyright (c) 2013, AppLovin Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ALNullabilityAnnotations.h"

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
@property (copy, nonatomic, readonly) NSString* __alnonnull label;

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
+(alnonnull ALAdSize *) sizeBanner;

/**
 *  Retrieve a singleton instance of the <code>INTERSTITIAL</code> ad size object.
 *
 *  Interstitials are full-screen ads with high click-through rates which will fully cover the <code>UIViewController</code> below.
 *
 *  @return An instance of ALAdSize which represents the size <code>INTERSTITIAL</code>.
 */
+(alnonnull ALAdSize *) sizeInterstitial;

/**
 *  Retrieve a singleton instance of the <code>MREC</code> ad size object.
 *
 *  MRECs are 320x250 (mostly square) advertisements.
 *
 *  @return An instance of ALAdSize which represents the size <code>MREC</code>.
 */
+(alnonnull ALAdSize *) sizeMRec;

/**
 *  Retrieve a singleton instance of the <code>LEADER</code> ad size object.
 *
 *  Leaderboard ads are 728x90 ads intended for iPads.
 *
 *  @return An instance of ALAdSize which represents the size <code>LEADER</code>.
 */
+(alnonnull ALAdSize *) sizeLeader;

/**
 *  Retrieve an <code>NSArray</code> of all available ad size singleton instances.
 *
 *  @return [NSArray arrayWithObjects: [ALAdSize sizeBanner], [ALAdSize sizeInterstitial], ..., nil];
 */
+(alnonnull NSArray *) allSizes;

// ----------------------------------------------------

// These are no longer recommended or considered best practice.
// If possible, use a size like [ALAdSize sizeBanner] or [ALAdSize sizeInterstitial] instead.

@property (assign, nonatomic) NSUInteger width;
@property (assign, nonatomic) NSUInteger height;

-(alnullable instancetype) initWithLabel: (alnonnull NSString *)label __deprecated_msg("Do not alloc-init your own instances; use an existing singleton size like [ALAdSize sizeBanner]");
-(alnullable instancetype) initWithWidth: (NSUInteger)width height:(NSUInteger)height __deprecated_msg("Do not alloc-init your own instances; use an existing singleton size like [ALAdSize sizeBanner]");
+(alnonnull ALAdSize*) sizeWithLabel: (alnonnull NSString*) label orDefault: (alnonnull ALAdSize*) defaultSize __deprecated_msg("Custom ad sizes are no longer supported; use an existing singleton size like [ALAdSize sizeBanner]");
- (alnullable id)init __attribute__((unavailable("Do not alloc-init your own instances; use an existing singleton size like [ALAdSize sizeBanner]")));
@end