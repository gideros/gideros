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
    [[GKLocalPlayer localPlayer] authenticateWithCompletionHandler:^(NSError* error)
    {
        if(!error)
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
    }];
}
-(void)logout{
    
}
-(void)showLeaderboard:(NSString*)Id{
    GKLeaderboardViewController* leaderboardVC = [[[GKLeaderboardViewController alloc] init] autorelease];
    if (leaderboardVC != nil)
    {
        leaderboardVC.timeScope = GKLeaderboardTimeScopeAllTime;
        leaderboardVC.leaderboardDelegate = self;
        [self presentViewController:leaderboardVC];
    }
}
-(void)reportScore:(NSString*)Id andScore:(long)score with:(int)immediate{
    GKScore* scoreOb = [[[GKScore alloc] initWithCategory:Id] autorelease];
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
    GKAchievementViewController* achievementsVC = [[[GKAchievementViewController alloc] init] autorelease];
    if (achievementsVC != nil)
    {
        achievementsVC.achievementDelegate = self;
        [self presentViewController:achievementsVC];
    }
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
        
            GKAchievement* achievement = [[[GKAchievement alloc] initWithIdentifier:Id] autorelease];
            achievement.percentComplete = steps;
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
    GKLeaderboard *lb = [[[GKLeaderboard alloc] init] autorelease];
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
                          
                          [achievement setObject:[NSString stringWithFormat:@"%d", score.rank] forKey:@"rank"];
                          [achievement setObject:[NSString stringWithFormat:@"%lld", score.value] forKey:@"score"];
                          [achievement setObject:@"name" forKey:@"name"];
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
	[rootVC presentModalViewController:vc animated:YES];
}

-(void) dismissModalViewController
{
	UIViewController* rootVC = g_getRootViewController();
	[rootVC dismissModalViewControllerAnimated:YES];
}

- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController
{
	[self dismissModalViewController];
}

-(void) achievementViewControllerDidFinish:(GKAchievementViewController*)viewController
{
	[self dismissModalViewController];
}

@end
