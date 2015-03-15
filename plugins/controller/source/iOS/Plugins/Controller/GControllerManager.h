//
//  GControllerManager.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GController.h"

@interface GControllerManager : NSObject

+(void)init;
+(void)cleanup;
+(NSInteger)isAnyAvailable;
+(NSInteger)getPlayerCount;
+(NSString*)getControllerName:(NSNumber*) nid;
+(int*)getPlayers;
+(void)addDevice:(NSString*)nid withType:(NSString*)type;
+(void)remDevice:(NSString*)nid;
+(GController*)getController:(NSString*)nid withType:(NSString*)type;
@end
