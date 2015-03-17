//
//  GameGamekit.h
//  Test
//
//  Created by Arturs Sosins on 3/27/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GameProtocol.h"
#import <GameKit/GameKit.h> 

@interface GameGamekit : NSObject<GameProtocol, GKLeaderboardViewControllerDelegate, GKAchievementViewControllerDelegate>
@property(nonatomic, retain)NSMutableDictionary *achs;
@property(nonatomic, retain)NSURL *ubiq;
@end
