//
//  AdsProtocol.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol GameProtocol <NSObject>

-(id)init;
-(void)destroy;
-(void)login:(NSMutableArray*)parameters;
-(void)logout;
-(void)showLeaderboard:(NSString*)Id;
-(void)reportScore:(NSString*)Id andScore:(long)score with:(int)immediate;
-(void)showAchievements;
-(void)reportAchievement:(NSString*)Id andSteps:(int)steps with:(int)immediate;
-(void)loadAchievements;
-(void)loadScores:(NSString*)Id andSpan:(int) span forCollection:(int) collection withResults:(int)maxResults;
-(void)loadState:(int)key;
-(void)updateState:(NSData*)data forKey:(int)key withImmediate:(int)immediate;
-(void)resolveState:(NSData*)data forKey:(int)key andVersion:(NSString*)ver;
-(void)deleteState:(int)key;
@end
