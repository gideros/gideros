//
//  AdsClass.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "gideros.h"

@interface GameClass : NSObject

+(void)init;
+(void)cleanup;

+(void)initialize:(NSString*)provider;
+(void)destroy:(NSString*)provider;
+(void)login:(NSString*)provider with:(NSMutableArray*)parameters;
+(void)logout:(NSString*)provider;
+(void)showLeaderboard:(NSString*)provider with:(NSString*)Id;
+(void)reportScore:(NSString*)provider with:(NSString*)Id andScore:(long)score with:(int)immediate;
+(void)showAchievements:(NSString*)provider;
+(void)reportAchievement:(NSString*)provider with:(NSString*)Id andSteps:(int)steps with:(int)immediate;
+(void)loadAchievements:(NSString*)provider;
+(void)loadScores:(NSString*)provider with:(NSString*)Id andSpan:(int) span forCollection:(int) collection withResults:(int)maxResults;
+(void)loadState:(NSString*)provider with:(int)key;
+(void)updateState:(NSString*)provider with:(NSData*)data forKey:(int)key withImmediate:(int)immediate;
+(void)resolveState:(NSString*)provider with:(NSData*)data forKey:(int)key andVersion:(NSString*)ver;
+(void)deleteState:(NSString*)provider with:(int)key;

+(void)loginComplete:(Class)provider;
+(void)loginError:(Class)provider with:(NSString*) error;
+(void)reportAchievementComplete:(Class)provider with:(NSString*)achId;
+(void)reportAchievementError:(Class)provider with:(NSString*)achId with:(NSString*) error;
+(void)reportScoreComplete:(Class)provider with:(NSString*)lId with:(long)score;
+(void)reportScoreError:(Class)provider with:(NSString*)lId with:(long)score with:(NSString*)error;
+(void)loadAchievementsComplete:(Class)provider with:(NSArray*) arr;
+(void)loadAchievementsError:(Class)provider with:(NSString*)error;
+(void)loadScoresComplete:(Class)provider with:(NSString*)lId with:(NSString*)name with:(NSArray*) arr;
+(void)loadScoresError:(Class)provider with:(NSString*)lId with:(NSString*)error;
+(void)stateLoaded:(Class)provider with:(int) key with:(NSData*)data with:(int) fresh;
+(void)stateError:(Class)provider with:(int) key with:(NSString*) error;
+(void)stateConflict:(Class)provider with:(int)key with:(NSString*) ver with:(NSData*)localData with:(NSData*)serverData;
+(void)stateDeleted:(Class)provider with:(int) key;
@end
