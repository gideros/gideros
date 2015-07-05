//
//  HZNativeAdImage.h
//  Heyzap
//
//  Created by Maximilian Tagher on 12/19/14.
//  Copyright (c) 2014 Heyzap. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

/**
 *  An object with the properties to construct an image.
 */
@interface HZNativeAdImage : NSObject

/**
 *  The URL of the image. You can use a library like SDWebImage or AFNetworking to asynchronously download and display the image.
 */
@property (nonatomic, readonly) NSURL *url;

/**
 *  The width of the image.
 */
@property (nonatomic, readonly) NSUInteger width;
/**
 *  The height of the image.
 */
@property (nonatomic, readonly) NSUInteger height;

/**
 *  The size of the image.
 */
@property (nonatomic, readonly) CGSize size;

@end
