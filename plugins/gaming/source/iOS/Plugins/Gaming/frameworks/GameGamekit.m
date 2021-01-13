//
//  GameGamekit.m
//  Test
//
//  Created by Arturs Sosins on 3/27/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import "GameGamekit.h"
#import "GameClass.h"

@implementation GameGamekit
-(id)init{
    self.achs = [NSMutableDictionary dictionary];
    self.ubiq = [[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:nil];
    if(self.ubiq) {
        // register to observe notifications from the store
        [[NSNotificationCenter defaultCenter]
         addObserver: self
         selector: @selector (storeDidChange:)
         name: NSUbiquitousKeyValueStoreDidChangeExternallyNotification
         object: [NSUbiquitousKeyValueStore defaultStore]];
    
        [[NSUbiquitousKeyValueStore defaultStore] synchronize];
    }
    return self;
}

-(void)destroy{
    [self.achs removeAllObjects];
}

-(void)login:(NSMutableArray*)parameters{
    GKLocalPlayer *p=[GKLocalPlayer localPlayer];
    p.authenticateHandler=^(UIViewController* viewController, NSError* error)
    {
        if (viewController!=nil)
            [self presentViewController:viewController];
        else if(p.isAuthenticated)
        {
            [GameClass loginComplete:[self class]];
            [GKAchievement loadAchievementsWithCompletionHandler:^(NSArray* achievements, NSError* error)
             {
                 if(!error){
                     for(int i = 0; i < [achievements count]; ++i)
                     {
                         GKAchievement* achievement = [achievements objectAtIndex:i];
                         if(achievement.completed)
                             [self.achs setObject:achievement.identifier forKey:achievement.identifier];
                     }
                 }
             }];
        }
        else{
            [GameClass loginError:[self class] with:[error localizedDescription]];
        }
    };
}
-(void)logout{
    
}
-(void)showLeaderboard:(NSString*)Id{
    GKGameCenterViewController* gameCenterController= [[GKGameCenterViewController alloc] init];
    if (gameCenterController != nil)
    {
        gameCenterController.gameCenterDelegate = self;
        gameCenterController.viewState = GKGameCenterViewControllerStateLeaderboards;
        gameCenterController.leaderboardTimeScope = GKLeaderboardTimeScopeToday;
        gameCenterController.leaderboardCategory = Id;
        [self presentViewController: gameCenterController];
    }
}
-(void)reportScore:(NSString*)Id andScore:(long)score with:(int)immediate{
    GKScore* scoreOb = [[GKScore alloc] initWithCategory:Id];
    scoreOb.value = score;
    [scoreOb reportScoreWithCompletionHandler:^(NSError* error)
     {
         if(!error){
             [GameClass reportScoreComplete:[self class] with:Id with:score];
         }
         else{
             [GameClass reportScoreError:[self class] with:Id with:score with:[error localizedDescription]];
         }
     }];
}
-(void)showAchievements{
    GKGameCenterViewController* gameCenterController= [[GKGameCenterViewController alloc] init];
    if (gameCenterController != nil)
    {
        gameCenterController.gameCenterDelegate = self;
        gameCenterController.viewState = GKGameCenterViewControllerStateAchievements;
        [self presentViewController: gameCenterController];
    }
}

-(void)getPlayerInfo{
   GKLocalPlayer *p=[GKLocalPlayer localPlayer];
   [GameClass playerInfoComplete:[self class] with:[p playerID] with:[p alias] with:@""];
}

-(void)reportAchievement:(NSString*)Id andSteps:(int)steps with:(int)immediate{
    
    [GKAchievement loadAchievementsWithCompletionHandler:^(NSArray *achievements, NSError *error) {
        
        if(!error)
        {
        
           for (GKAchievement *ach in achievements) {
                if([ach.identifier isEqualToString:Id] && ach.completed) {
                    return ;
                }
            }
        
            GKAchievement* achievement = [[GKAchievement alloc] initWithIdentifier:Id];
            if (steps<0)
	            achievement.percentComplete += (-steps);
            else if (steps>0)
	            achievement.percentComplete = steps;
	        else
	            achievement.percentComplete = 100;
            if([achievement respondsToSelector:@selector(setShowsCompletionBanner:)])
                achievement.showsCompletionBanner = true;
        
            [achievement reportAchievementWithCompletionHandler:^(NSError* error)
             {
                 if(!error){
                     [GameClass reportAchievementComplete:[self class] with:Id];
                     if(achievement.completed)
                         [self.achs setObject:achievement.identifier forKey:achievement.identifier];
                 }
                 else{
                     [GameClass reportAchievementError:[self class] with:Id with:[error localizedDescription]];
                 }
             }];
        }
        else{
            [GameClass reportAchievementError:[self class] with:Id with:[error localizedDescription]];
        }
        
    }];
}

-(void)revealAchievement:(NSString*)Id with:(int)immediate{
    
    [GKAchievement loadAchievementsWithCompletionHandler:^(NSArray *achievements, NSError *error) {
        
        if(!error)
        {
        
           for (GKAchievement *ach in achievements) {
                if([ach.identifier isEqualToString:Id] && ach.completed) {
                    return ;
                }
            }
        
            GKAchievement* achievement = [[GKAchievement alloc] initWithIdentifier:Id];
            achievement.percentComplete = 0;
            if([achievement respondsToSelector:@selector(setShowsCompletionBanner:)])
                achievement.showsCompletionBanner = true;
        
            [achievement reportAchievementWithCompletionHandler:^(NSError* error)
             {
                 if(!error){
                     [GameClass reportAchievementComplete:[self class] with:Id];
                     if(achievement.completed)
                         [self.achs setObject:achievement.identifier forKey:achievement.identifier];
                 }
                 else{
                     [GameClass reportAchievementError:[self class] with:Id with:[error localizedDescription]];
                 }
             }];
        }
        else{
            [GameClass reportAchievementError:[self class] with:Id with:[error localizedDescription]];
        }
        
    }];
}

-(void)loadAchievements{
    [GKAchievementDescription loadAchievementDescriptionsWithCompletionHandler:^(NSArray* achievementDescriptions, NSError* derror)
    {
        if(!derror){
            [GKAchievement loadAchievementsWithCompletionHandler:^(NSArray *achievements, NSError *error) {
                
                if(!error){
                    NSMutableDictionary *dic = [NSMutableDictionary dictionary];
                    for (GKAchievement *ach in achievements) {
                        [dic setObject:ach forKey:ach.identifier];
                    }
                    NSMutableArray *arr = [NSMutableArray array];
                    for(int i = 0; i < [achievementDescriptions count]; ++i)
                    {
                        GKAchievementDescription* ach = [achievementDescriptions objectAtIndex:i];
                        BOOL completed = false;
                        int lastUpdate = 0;
                        float percentComplete = 0;
                        GKAchievement *state = [dic objectForKey:ach.identifier];
                        if(state != NULL)
                        {
                            completed = state.completed;
                            lastUpdate = [state.lastReportedDate timeIntervalSince1970];
                            percentComplete = state.percentComplete;
                        }

                        NSMutableDictionary *achievement = [NSMutableDictionary dictionary];
                        [achievement setObject:ach.identifier forKey:@"id"];
                        [achievement setObject:ach.title forKey:@"name"];
                        [achievement setObject:ach.achievedDescription forKey:@"description"];
                        [achievement setObject:[NSString stringWithFormat:@"%d",((completed) ? 0 :(ach.hidden) ? 2 : 1)] forKey:@"status"];
                        [achievement setObject:[NSString stringWithFormat:@"%d",lastUpdate] forKey:@"lastUpdate"];
                        [achievement setObject:[NSString stringWithFormat:@"%f",percentComplete] forKey:@"currentSteps"];
                        [achievement setObject:@"100" forKey:@"totalSteps"];
                        [arr insertObject:achievement atIndex:i];
                    }
                    [GameClass loadAchievementsComplete:[self class] with:arr];
                  }
                  else{
                      [GameClass loadAchievementsError:[self class] with:[derror localizedDescription]];
                  }
            }];
        }
        else{
            [GameClass loadAchievementsError:[self class] with:[derror localizedDescription]];
        }
    }];
}
-(void)loadScores:(NSString*)Id andSpan:(int) span forCollection:(int) collection withResults:(int)maxResults{
    GKLeaderboard *lb = [[GKLeaderboard alloc] init];
    lb.category = Id;
    lb.timeScope = span;
    lb.playerScope = collection;
    
    NSRange range;
    range.location = 0;
    range.length = maxResults;
    lb.range = range;
    
    [lb loadScoresWithCompletionHandler:^(NSArray* scores, NSError* error)
     {
         if(!error){
             NSMutableArray *identifiers = [NSMutableArray array];
             for(int i = 0; i < [scores count]; i++)
             {
                 GKScore *score = [scores objectAtIndex:i];
                 [identifiers insertObject:score.playerID atIndex:i];
             }
             [GKPlayer loadPlayersForIdentifiers:identifiers withCompletionHandler:^(NSArray* players, NSError* plerror)
              {
                  if(!plerror){
                      NSMutableArray *arr = [NSMutableArray array];
                      for(int i = 0; i < [scores count]; i++)
                      {
                          GKScore *score = [scores objectAtIndex:i];
                          NSMutableDictionary *achievement = [NSMutableDictionary dictionary];
                          
                          [achievement setObject:[NSString stringWithFormat:@"%d", (int)(score.rank)] forKey:@"rank"];
                          [achievement setObject:[NSString stringWithFormat:@"%lld", score.value] forKey:@"score"];
                          if (score.player!=nil)
                          {
                              [achievement setObject:[score.player alias] forKey:@"name"];
                              [achievement setObject:@"" forKey:@"pic"];
                          }
                          else
                          {
                              [achievement setObject:@"" forKey:@"name"];
                              [achievement setObject:@"" forKey:@"pic"];
                          }
                          [achievement setObject:score.playerID forKey:@"playerId"];
                          [achievement setObject:[NSString stringWithFormat:@"%f", [score.date timeIntervalSince1970]] forKey:@"timestamp"];
                          [arr insertObject:achievement atIndex:i];
                      }
                      
                      [GameClass loadScoresComplete:[self class] with:Id with:@"lName" with:arr];
                  }
                  else{
                      [GameClass loadScoresError:[self class] with:Id with:[plerror localizedDescription]];
                  }
              }];
         }
         else{
             [GameClass loadScoresError:[self class] with:Id with:[error localizedDescription]];
         }
     }];
}
-(void)loadState:(int)key{
    if(self.ubiq) {
        [GameClass stateLoaded:[self class] with:key with:[[NSUbiquitousKeyValueStore defaultStore] objectForKey:[NSString stringWithFormat:@"%d", key]] with:1];
    }
}
-(void)updateState:(NSData*)data forKey:(int)key withImmediate:(int)immediate{
    if(self.ubiq) {
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults setObject:data forKey:[NSString stringWithFormat:@"%d", key]];
        [defaults synchronize];
    
        NSUbiquitousKeyValueStore *Cdefaults = [NSUbiquitousKeyValueStore defaultStore];
        [Cdefaults setObject:data forKey:[NSString stringWithFormat:@"%d", key]];
        if(immediate)
            [Cdefaults synchronize];
    }
}
-(void)resolveState:(NSData*)data forKey:(int)key andVersion:(NSString*)ver{
    if(self.ubiq) {
        [self updateState:data forKey:key withImmediate:false];
    }
}
-(void)deleteState:(int)key{
    if(self.ubiq) {
        [self updateState:nil forKey:key withImmediate:false];
        [GameClass stateDeleted:[self class] with:1];
    }
}

-(void)storeDidChange:(NSNotification *)notification{
    NSDictionary* userInfo = [notification userInfo];
    NSNumber* reasonForChange = [userInfo objectForKey:NSUbiquitousKeyValueStoreChangeReasonKey];
    NSInteger reason = -1;
    
    // If a reason could not be determined, do not update anything.
    if (!reasonForChange)
        return;
    
    // Update only for changes from the server.
    reason = [reasonForChange integerValue];
    if ((reason == NSUbiquitousKeyValueStoreServerChange) ||
        (reason == NSUbiquitousKeyValueStoreInitialSyncChange)) {
        // If something is changing externally, get the changes
        // and update the corresponding keys locally.
        NSArray* changedKeys = [userInfo objectForKey:NSUbiquitousKeyValueStoreChangedKeysKey];
        NSUbiquitousKeyValueStore* store = [NSUbiquitousKeyValueStore defaultStore];
        NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
        
        // This loop assumes you are using the same key names in both
        // the user defaults database and the iCloud key-value store
        for (NSString* key in changedKeys) {
            [GameClass stateConflict:[self class] with:[key intValue] with:@"1" with:[userDefaults objectForKey:key] with:[store objectForKey:key]];
        }
    }
}

-(void) presentViewController:(UIViewController*)vc
{
	UIViewController* rootVC = g_getRootViewController();
    [rootVC presentViewController:vc animated:YES completion:nil];
}

-(void) dismissModalViewController
{
	UIViewController* rootVC = g_getRootViewController();
    [rootVC dismissViewControllerAnimated:YES completion:nil];
}

- (void)gameCenterViewControllerDidFinish:(GKGameCenterViewController *)viewController
{
	[self dismissModalViewController];
}

@end
