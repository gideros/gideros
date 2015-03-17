//
//  GControllerManager.mm
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "GControllerManager.h"
#import "ControllerProtocol.h"

@implementation GControllerManager

static NSMutableDictionary *types = [NSMutableDictionary dictionary];
static NSMutableDictionary *allDevices = [NSMutableDictionary dictionary];
static NSMutableDictionary *devices = [NSMutableDictionary dictionary];
static NSMutableDictionary *players = [NSMutableDictionary dictionary];

+(void)init{
    [self initialize:@"Default"];
    [self initialize:@"Icade"];
}

+(void)cleanup{
	for(NSString *key in types){
        id c = [types objectForKey:key];
        [c destroy];
        [c release];
        c = nil;
    }
    [types removeAllObjects];
    [allDevices removeAllObjects];
    [devices removeAllObjects];
    [players removeAllObjects];
}

+(void)initialize:(NSString*)provider{
	if(![types objectForKey:[provider lowercaseString]])
	{
		NSString *ProviderClass = @"GController";
		ProviderClass = [ProviderClass stringByAppendingString:[provider capitalizedString]];
		id ad = [[NSClassFromString(ProviderClass) alloc] init];
		[types setObject:ad forKey:[provider lowercaseString]];
	}
}

+(NSInteger)isAnyAvailable{
    if([devices count] > 0)
        return 1;
    else
        return 0;
}

+(NSInteger)getPlayerCount{
    return [devices count];
}

+(int*)getPlayers{
    int* arr = new int[[GControllerManager getPlayerCount]];
    int i = 0;
	for(NSString* key in devices) {
        arr[i] = [[[devices objectForKey:key] getPlayerId] intValue];
        i++;
    }
    return arr;
}

+(NSString*)getControllerName:(NSNumber*) playerId{
    NSString* name = @"";
    NSString *type = [players objectForKey:playerId];
	if(type != nil)
    {
        id c = [types objectForKey:[type lowercaseString]];
        if(c)
        {
            NSArray* arr = [allDevices allKeysForObject:playerId];
            name = [c getControllerName:[arr firstObject]];
        }
    }
    return name;
}

+(void)addDevice:(NSString*)nid withType:(NSString*)type{
    if(![devices objectForKey:nid]){
        if(![allDevices objectForKey:nid]){
            NSNumber *playerId = [NSNumber numberWithInt:[allDevices count]+1];
            [allDevices setObject:playerId forKey:nid];
            [players setObject:playerId forKey:type];
        }
        [devices setObject:[[GController alloc] init:[allDevices objectForKey:nid]] forKey:nid];
    }
}

+(void)remDevice:(NSString*)nid{
    id c = [devices objectForKey:nid];
    if(c){
        [c destroy];
        [devices removeObjectForKey:nid];
    }
}

+(GController*)getController:(NSString*)nid withType:(NSString*)type{
    [GControllerManager addDevice:nid withType:type];
    return [devices objectForKey:nid];
}

@end
