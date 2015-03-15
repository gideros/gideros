//
//  AdsClass.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "ads.h"

@interface AdsClass : NSObject

+(void)init;
+(void)cleanup;

+(UIViewController*)getRootViewController;

+(void)initialize:(NSString*)adprovider;
+(void)destroy:(NSString*)adprovider;
+(void)setKey:(NSString*)adprovider with:(NSMutableArray*)parameters;
+(void)loadAd:(NSString*)adprovider with:(NSMutableArray*)parameters;
+(void)showAd:(NSString*)adprovider with:(NSMutableArray*)parameters;
+(void)hideAd:(NSString*)adprovider with:(NSString*)type;
+(void)enableTesting:(NSString*)adprovider;
+(void)setAlignment:(NSString*)adprovider with:(NSString*)hor andWith:(NSString*)ver;
+(void)setX:(NSString*)adprovider with:(int)x;
+(void)setY:(NSString*)adprovider with:(int)y;
+(int)getX:(NSString*)adprovider;
+(int)getY:(NSString*)adprovider;
+(int)getWidth:(NSString*)adprovider;
+(int)getHeight:(NSString*)adprovider;
+(BOOL)hasConnection;
+(BOOL)isPortrait;
+(void)adReceived:(Class)adprovider forType:(NSString*)type;
+(void)adFailed:(Class)adprovider with:(NSString*)error forType:(NSString*)type;
+(void)adActionBegin:(Class)adprovider forType:(NSString*)type;
+(void)adActionEnd:(Class)adprovider forType:(NSString*)type;
+(void)adDismissed:(Class)adprovider forType:(NSString*)type;
+(void)adDisplayed:(Class)adprovider forType:(NSString*)type;
+(void)adError:(Class)adprovider with:(NSString*)error;
@end
