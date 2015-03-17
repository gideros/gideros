//
//  AdsClass.m
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "GameClass.h"
#import "GameProtocol.h"
#include "gamification.h"

@implementation GameClass

static NSMutableDictionary *games = [NSMutableDictionary dictionary];

+(void)init{
}

+(void)cleanup{
	for(NSString *key in games){
        id game = [games objectForKey:key];
        [game destroy];
        [game release];
        game = nil;
    }
    [games removeAllObjects];
}

+(void)initialize:(NSString*)provider{
	if(![games objectForKey:[provider lowercaseString]])
	{
		NSString *ProviderClass = @"Game";
		ProviderClass = [ProviderClass stringByAppendingString:[provider capitalizedString]];
		id game = [[NSClassFromString(ProviderClass) alloc] init];
		[games setObject:game forKey:[provider lowercaseString]];
	}
}

+(void)destroy:(NSString*)provider{
	id game = [games objectForKey:[provider lowercaseString]];
	if(game)
	{
        [game destroy];
        [game release];
        game = nil;
		[games removeObjectForKey:[provider lowercaseString]];
    }
}

+(void)login:(NSString*)provider with:(NSMutableArray*)parameters{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game login:parameters];
    }
}
+(void)logout:(NSString*)provider{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game logout];
    }
}
+(void)showLeaderboard:(NSString*)provider with:(NSString*)Id{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game showLeaderboard:Id];
    }
}
+(void)reportScore:(NSString*)provider with:(NSString*)Id andScore:(long)score with:(int)immediate{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game reportScore:Id andScore:score with:immediate];
    }
}
+(void)showAchievements:(NSString*)provider{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game showAchievements];
    }
}
+(void)reportAchievement:(NSString*)provider with:(NSString*)Id andSteps:(int)steps with:(int)immediate{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game reportAchievement:Id andSteps:steps with:immediate];
    }
}
+(void)loadAchievements:(NSString*)provider{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game loadAchievements];
    }
}
+(void)loadScores:(NSString*)provider with:(NSString*)Id andSpan:(int) span forCollection:(int) collection withResults:(int)maxResults{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game loadScores:Id andSpan:span forCollection:collection withResults:maxResults];
    }
}

+(void)loadState:(NSString*)provider with:(int)key{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game loadState:key];
    }
}
+(void)updateState:(NSString*)provider with:(NSData*)data forKey:(int)key withImmediate:(int)immediate{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game updateState:data forKey:key withImmediate:immediate];
    }
}
+(void)resolveState:(NSString*)provider with:(NSData*)data forKey:(int)key andVersion:(NSString*)ver{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game resolveState:data forKey:key andVersion:ver];
    }
}
+(void)deleteState:(NSString*)provider with:(int)key{
    id game = [games objectForKey:[provider lowercaseString]];
    if (game) {
        [game deleteState:key];
    }
}

+(void)loginComplete:(Class)provider{
    game_onLoginComplete([[GameClass modifyName:provider] UTF8String]);
}
+(void)loginError:(Class)provider with:(NSString*) error{
    game_onLoginError([[GameClass modifyName:provider] UTF8String], [error UTF8String]);
}
+(void)reportAchievementComplete:(Class)provider with:(NSString*)achId{
    game_onReportAchievementComplete([[GameClass modifyName:provider] UTF8String], [achId UTF8String]);
}
+(void)reportAchievementError:(Class)provider with:(NSString*)achId with:(NSString*) error{
    game_onReportAchievementError([[GameClass modifyName:provider] UTF8String], [achId UTF8String], [error UTF8String]);
}
+(void)reportScoreComplete:(Class)provider with:(NSString*)lId with:(long)score{
    game_onReportScoreComplete([[GameClass modifyName:provider] UTF8String], [lId UTF8String], score);
}
+(void)reportScoreError:(Class)provider with:(NSString*)lId with:(long)score with:(NSString*)error{
    game_onReportScoreError([[GameClass modifyName:provider] UTF8String], [lId UTF8String], [error UTF8String], score);
}
+(void)loadAchievementsComplete:(Class)provider with:(NSArray*) arr{
    std::vector<Achievement> achievements;
    for (NSMutableDictionary *achievement in arr) {
        Achievement gach = {[[achievement objectForKey:@"id"] UTF8String], [[achievement objectForKey:@"name"] UTF8String], [[achievement objectForKey:@"description"] UTF8String], [[achievement objectForKey:@"status"] intValue], [[achievement objectForKey:@"lastUpdate"] intValue], [[achievement objectForKey:@"currentSteps"] intValue], [[achievement objectForKey:@"totalSteps"] intValue]};
        
        achievements.push_back(gach);
    }
    Achievement gach = {"", "", "", 0, 0, 0, 0};
    achievements.push_back(gach);
    game_onLoadAchievementsComplete([[GameClass modifyName:provider] UTF8String], &achievements[0]);
}
+(void)loadAchievementsError:(Class)provider with:(NSString*)error{
    game_onLoadAchievementsError([[GameClass modifyName:provider] UTF8String], [error UTF8String]);
}
+(void)loadScoresComplete:(Class)provider with:(NSString*)lId with:(NSString*)name with:(NSArray*) arr{
    std::vector<Score> cscores;
    for (NSMutableDictionary *nextScore in arr) {
        Score gscores = {[[nextScore objectForKey:@"rank"] UTF8String], [[nextScore objectForKey:@"score"] UTF8String], [[nextScore objectForKey:@"name"] UTF8String], [[nextScore objectForKey:@"playerId"] UTF8String], [[nextScore objectForKey:@"timestamp"] intValue]};
        
        cscores.push_back(gscores);
    }
    Score gscores = {"", "", "", "", 0};
    cscores.push_back(gscores);
    game_onLoadScoresComplete([[GameClass modifyName:provider] UTF8String],[lId UTF8String], [name UTF8String], &cscores[0]);
}
+(void)loadScoresError:(Class)provider with:(NSString*)lId with:(NSString*)error{
    game_onLoadScoresError([[GameClass modifyName:provider] UTF8String], [lId UTF8String], [error UTF8String]);
}
+(void)stateLoaded:(Class)provider with:(int) key with:(NSData*)data with:(int) fresh{
    game_onStateLoaded([[GameClass modifyName:provider] UTF8String], key, [data bytes], (size_t)[data length], fresh);
}
+(void)stateError:(Class)provider with:(int) key with:(NSString*) error{
    game_onStateError([[GameClass modifyName:provider] UTF8String], key, [error UTF8String]);
}
+(void)stateConflict:(Class)provider with:(int)key with:(NSString*) ver with:(NSData*)localData with:(NSData*)serverData{
    game_onStateConflict([[GameClass modifyName:provider] UTF8String], key, [ver UTF8String], [localData bytes], (size_t)[localData length], [serverData bytes], (size_t)[serverData length]);
}
+(void)stateDeleted:(Class)provider with:(int) key{
    game_onStateDeleted([[GameClass modifyName:provider] UTF8String], key);
}


+(NSString*)modifyName:(Class)name{
    NSString *cname = NSStringFromClass(name);
    cname = [cname substringFromIndex:[@"Game" length]];
    return [cname lowercaseString];
}

@end
