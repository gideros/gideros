//
//  FlurryOffer.h
//  Flurry iPhone Analytics Agent
//
//  Copyright 2010 Flurry, Inc. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface FlurryOffer : NSObject {
	NSString* appName;
	UIImage* appIcon;
	NSString* referralUrl;
	NSNumber* appPrice;
	NSString* appDescription;
}

@property (retain) NSString* appName;
@property (retain) UIImage* appIcon;
@property (retain) NSString* referralUrl;
@property (retain) NSNumber* appPrice;
@property (retain) NSString* appDescription;

@end

