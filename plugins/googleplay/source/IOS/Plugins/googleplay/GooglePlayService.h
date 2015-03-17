//
//  GooglePlayService.h
//  GooglePlayTest
//
//  Created by Arturs Sosins on 9/11/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gms.h"
#import <Foundation/Foundation.h>
#import <GooglePlus/GooglePlus.h>
#import <PlayGameServices/PlayGameServices.h>

@interface GooglePlayService : NSObject <GPPSignInDelegate, GPGAchievementControllerDelegate, GPGLeaderboardControllerDelegate>
+(void)initialize;
+(void)deinitialize;
+(BOOL)isAvailable;
+(void)handleOpenUrl:(NSURL*)url;
+(void)login;
+(void)logout;
+(void)showSettings;
+(void)showLeaderboard:(NSString*)Id;
+(void)reportScore:(NSString*)Id andScore:(int)score with:(int)immediate;
+(void)showAchievements;
+(void)reportAchievement:(NSString*)Id andSteps:(int)steps with:(int)immediate;
+(void)reportAchievement:(NSString*)Id with:(int)immediate;
+(void)loadAchievements;
+(void)loadScores:(NSString*)Id andSpan:(int) span forCollection:(int) collection withResults:(int)maxResults;
+(void)loadPlayerScores:(NSString*)Id andSpan:(int) span forCollection:(int) collection withResults:(int)maxResults;
+(void)loadState:(int)key;
+(void)updateState:(NSData*)data forKey:(int)key withImmediate:(int)immediate;
+(void)resolveState:(NSData*)data forKey:(int)key andVersion:(NSString*)ver;
+(void)deleteState:(int)key;
+(NSString*)getCurrentPlayer;
+(NSString*)getCurrentPlayerId;
@end
