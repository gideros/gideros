//
//  GooglePlayService.m
//  GooglePlayTest
//
//  Created by Arturs Sosins on 9/11/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//
#include "gideros.h"
#import "GooglePlayService.h"

@implementation GooglePlayService

static NSString * const kClientID = @"561448401579-u43969iu9fqa43sff2di137h7nlbfnpt.apps.googleusercontent.com";
GPGPlayer *localPlayer;

-(id)init{
    return self;
}

+(void)initialize{
    
}

+(void)deinitialize{
    
}

+(BOOL)isAvailable{
    return YES;
}

+(void)handleOpenUrl:(NSURL*)url{
    [GPPURLHandler handleURL:url sourceApplication:@"com.apple.mobilesafari" annotation:nil];
}

+(void)login{
    GPPSignIn *signIn = [GPPSignIn sharedInstance];
    // You set kClientID in a previous step
    signIn.clientID = kClientID;
    signIn.scopes = [NSArray arrayWithObjects:
                     @"https://www.googleapis.com/auth/games",
                     @"https://www.googleapis.com/auth/appstate",
                     nil];
    signIn.language = [[NSLocale preferredLanguages] objectAtIndex:0];
    signIn.delegate = [[GooglePlayService alloc] init];
    signIn.shouldFetchGoogleUserID =YES;
    if(![signIn trySilentAuthentication])
    {
        [signIn authenticate];
    }
}

+(void)logout{
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        [[GPGManager sharedInstance] signout];
    }
}


+(void)showSettings{
    
}

+(void)showLeaderboard:(NSString*)Id{
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        GPGLeaderboardController *leadController = [[GPGLeaderboardController alloc] initWithLeaderboardId:Id];
        leadController.leaderboardDelegate = [[GooglePlayService alloc] init];
        [g_getRootViewController() presentViewController:leadController animated:YES completion:nil];
    }
}

+(void)reportScore:(NSString*)Id andScore:(int)score with:(int)immediate{
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        GPGScore *myScore = [[GPGScore alloc] initWithLeaderboardId:Id];
        myScore.value = score;
        [myScore submitScoreWithCompletionHandler: ^(GPGScoreReport *report, NSError *error) {
            if (error) {
                // Handle the error
            } else {
                gms_onScoreSubmitted();
            }
        }];
    }
}

+(void)showAchievements{
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        GPGAchievementController *achController = [[GPGAchievementController alloc] init];
        achController.achievementDelegate = [[GooglePlayService alloc] init];
        [g_getRootViewController() presentViewController:achController animated:YES completion:nil];
    }
}

+(void)reportAchievement:(NSString*)Id andSteps:(int)steps with:(int)immediate{
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        GPGAchievement *incrementMe = [GPGAchievement achievementWithId:Id];
        [incrementMe incrementAchievementNumSteps:steps completionHandler:^(BOOL newlyUnlocked, int currentSteps, NSError *error) {
                if (error) {
                    // Handle the error
                } else{
                    gms_onAchievementUpdated([Id UTF8String]);
                }
        }];
    }
}

+(void)reportAchievement:(NSString*)Id with:(int)immediate{
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        GPGAchievement *unlockMe = [GPGAchievement achievementWithId:Id];
        [unlockMe unlockAchievementWithCompletionHandler:^(BOOL newlyUnlocked, NSError *error) {
            if (error) {
                // Handle the error
            } else{
                gms_onAchievementUpdated([Id UTF8String]);
            }
        }];
    }
}

+(void)loadAchievements{
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        NSArray *allAchievements = [[GPGManager sharedInstance].applicationModel.achievement allMetadata];
        std::vector<Achievement> achievements;
        for (GPGAchievementMetadata *achievement in allAchievements) {
            Achievement gach = {[achievement.achievementId UTF8String], [achievement.name UTF8String], [achievement.achievementDescription UTF8String], achievement.state, achievement.lastUpdatedTimestamp, achievement.completedSteps, achievement.numberOfSteps};
			
			achievements.push_back(gach);
        }
        Achievement gach = {"", "", "", 0, 0, 0, 0};
        achievements.push_back(gach);
        gms_onAchievementsLoaded(&achievements[0]);
    }
}

+(void)loadScores:(NSString*)Id andSpan:(int) span forCollection:(int) collection withResults:(int)maxResults{
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        GPGLeaderboard *myLeaderboard = [GPGLeaderboard leaderboardWithId:Id];
        GPGLeaderboardMetadata *leaderboardData = [[GPGManager sharedInstance].applicationModel.leaderboard metadataForLeaderboardId:Id];
        myLeaderboard.social = YES;
        if(span == 0)
        {
            myLeaderboard.timeScope = GPGLeaderboardTimeScopeToday;
        }
        else if(span == 1)
        {
            myLeaderboard.timeScope = GPGLeaderboardTimeScopeThisWeek;
        }
        else if(span == 2)
        {
            myLeaderboard.timeScope = GPGLeaderboardTimeScopeAllTime;
        }
        [myLeaderboard loadScoresWithCompletionHandler:^(NSArray *scores, NSError *error) {
            int count = 1;
            std::vector<Score> cscores;
            for (GPGScore *nextScore in scores) {                
                Score gscores = {[nextScore.formattedRank UTF8String], [nextScore.formattedScore UTF8String], [nextScore.displayName UTF8String], [nextScore.playerId UTF8String], nextScore.writeTimestamp};
                
                cscores.push_back(gscores);
                
                if(count == maxResults)
                {
                    break;
                }
            }
            Score gscores = {"", "", "", "", 0};
            cscores.push_back(gscores);
            gms_onLeaderboardScoresLoaded([Id UTF8String], [leaderboardData.title UTF8String], &cscores[0]);
        }];
    }
}

+(void)loadPlayerScores:(NSString*)Id andSpan:(int) span forCollection:(int) collection withResults:(int)maxResults{
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        GPGLeaderboard *myLeaderboard = [GPGLeaderboard leaderboardWithId:Id];
        GPGLeaderboardMetadata *leaderboardData = [[GPGManager sharedInstance].applicationModel.leaderboard metadataForLeaderboardId:Id];
        myLeaderboard.social = YES;
        myLeaderboard.personalWindow = YES;
        if(span == 0)
        {
            myLeaderboard.timeScope = GPGLeaderboardTimeScopeToday;
        }
        else if(span == 1)
        {
            myLeaderboard.timeScope = GPGLeaderboardTimeScopeThisWeek;
        }
        else if(span == 2)
        {
            myLeaderboard.timeScope = GPGLeaderboardTimeScopeAllTime;
        }
        [myLeaderboard loadScoresWithCompletionHandler:^(NSArray *scores, NSError *error) {
            int count = 1;
            std::vector<Score> cscores;
            for (GPGScore *nextScore in scores) {
                Score gscores = {[nextScore.formattedRank UTF8String], [nextScore.formattedScore UTF8String], [nextScore.displayName UTF8String], [nextScore.playerId UTF8String], nextScore.writeTimestamp};
                
                cscores.push_back(gscores);
                
                if(count == maxResults)
                {
                    break;
                }
            }
            Score gscores = {"", "", "", "", 0};
            cscores.push_back(gscores);
            gms_onLeaderboardScoresLoaded([Id UTF8String], [leaderboardData.title UTF8String], &cscores[0]);
        }];
    }
}

+(void)loadState:(int)key{
	GPGAppStateModel *model = [GPGManager sharedInstance].applicationModel.appState;
	NSNumber *nKey = [NSNumber numberWithInt:key];

	[model loadForKey:nKey completionHandler:^(GPGAppStateLoadStatus status, NSError *error) {
		if (status == GPGAppStateLoadStatusSuccess) {
			NSData* data = [model stateDataForKey:nKey];
            gms_onStateLoaded(key, [data bytes], (size_t)[data length], 1);
		}
        else
        {
            NSString* error = @"unknown error";
            if (status == GPGAppStateLoadStatusNotFound) {
                error = @"no data";
            }
            gms_onStateError(key, [error UTF8String]);
		}
	} conflictHandler:^NSData *(NSNumber *key, NSData *localState, NSData *remoteState) {
		// This call tells the application that the version in the cloud has
		// changed since the last time it downloaded or saved state data.
		// Typically, the correct resolution to this data conflict is to throw
		// out the local state, and replace it with the state data from the cloud:
        gms_onStateConflict([key intValue], "version", [localState bytes], (size_t)[localState length], [remoteState bytes], (size_t)[remoteState length]);
		return remoteState;
	}];
}

+(void)updateState:(NSData*)data forKey:(int)key withImmediate:(int)immediate{
	GPGAppStateModel *model = [GPGManager sharedInstance].applicationModel.appState;
	NSNumber *nKey = [NSNumber numberWithInt:key];
	[model setStateData:data forKey:nKey];
    __block BOOL ignoreState = NO;

	[model updateForKey:nKey completionHandler:^(GPGAppStateWriteStatus status, NSError *error) {
		if (status == GPGAppStateWriteStatusSuccess) {
            if(!ignoreState)
                gms_onStateLoaded(key, [data bytes], [data length], 1);
            else
                ignoreState = NO;
		}
        else
        {
            NSString* error = @"unknown error";
            if (status == GPGAppStateWriteStatusBadKeyDataOrVersion) {
                error = @"incorrect key";
            }
            else if (status == GPGAppStateWriteStatusKeysQuotaExceeded) {
                error = @"key limit exceeded";
            }
            else if (status == GPGAppStateWriteStatusSizeExceeded) {
                error = @"too much data passed";
            }
            gms_onStateError(key, [error UTF8String]);
		}
		// Check for errors and other status flags here
	} conflictHandler:^NSData *(NSNumber *key, NSData *localState, NSData *remoteState) {
		// Uh oh. I need to resolve these two versions.
		// localState = State you are attempting to upload
		// remoteState = State currently saved in the cloud
        gms_onStateConflict([key intValue], "version", [localState bytes], (size_t)[localState length], [remoteState bytes], (size_t)[remoteState length]);
        ignoreState = YES;
		return data;
	}];
}

+(void)resolveState:(NSData*)data forKey:(int)key andVersion:(NSString*)ver
{
	GPGAppStateModel *model = [GPGManager sharedInstance].applicationModel.appState;
	NSNumber *nKey = [NSNumber numberWithInt:key];
	[model setStateData:data forKey:nKey];
    
	[model updateForKey:nKey completionHandler:^(GPGAppStateWriteStatus status, NSError *error) {
		if (status == GPGAppStateWriteStatusSuccess) {
			gms_onStateLoaded(key, [data bytes], [data length], 1);
		}
        else
        {
            NSString* error = @"unknown error";
            if (status == GPGAppStateWriteStatusBadKeyDataOrVersion) {
                error = @"incorrect key";
            }
            else if (status == GPGAppStateWriteStatusKeysQuotaExceeded) {
                error = @"key limit exceeded";
            }
            else if (status == GPGAppStateWriteStatusSizeExceeded) {
                error = @"too much data passed";
            }
            gms_onStateError(key, [error UTF8String]);
		}
		// Check for errors and other status flags here
	} conflictHandler:^NSData *(NSNumber *key, NSData *localState, NSData *remoteState) {
		return data;
	}];

}

+(void)deleteState:(int)key{
	GPGAppStateModel *model = [GPGManager sharedInstance].applicationModel.appState;
	NSNumber *nKey = [NSNumber numberWithInt:key];
    [model deleteForKey:nKey completionHandler:^(GPGAppStateWriteStatus status, NSError *error) {
        if (status == GPGAppStateWriteStatusSuccess) {
			gms_onStateDeleted(key);
		}
        else
        {
            NSString* error = @"unknown error";
            if (status == GPGAppStateWriteStatusBadKeyDataOrVersion) {
                error = @"incorrect key";
            }
            else if (status == GPGAppStateWriteStatusKeysQuotaExceeded) {
                error = @"key limit exceeded";
            }
            else if (status == GPGAppStateWriteStatusSizeExceeded) {
                error = @"too much data passed";
            }
            gms_onStateError(key, [error UTF8String]);
		}
    }];
}

+(NSString*)getCurrentPlayer{
    // Are they signed in?
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        return localPlayer.name;
    }
    return @"";
}

+(NSString*)getCurrentPlayerId{
    // Are they signed in?
    if ([[GPGManager sharedInstance] hasAuthorizer]) {
        return localPlayer.playerId;
    }
    return @"";
}

-(void)finishedWithAuth:(GTMOAuth2Authentication *)auth error:(NSError *)error
{
    if (error == nil && auth) {
        [[GPGManager sharedInstance] signIn:[GPPSignIn sharedInstance]
            reauthorizeHandler:^(BOOL requiresKeychainWipe, NSError *error) {
            // If you hit this, auth has failed and you need to authenticate.
            // Most likely you can refresh behind the scenes
            if (requiresKeychainWipe) {
                [[GPPSignIn sharedInstance] signOut];
            }
            [[GPPSignIn sharedInstance] trySilentAuthentication];
        }];
        // Eventually, you'll want to do something here.
        // Request information about the local player
        [[[GPGManager sharedInstance] applicationModel] reloadDataForKey:GPGModelLocalPlayerKey
            completionHandler:^(NSError *error) {
            if (!error) {
                // Retrieve that information from the GPGApplicationModel
                GPGPlayerModel *playerModel = [[[GPGManager sharedInstance] applicationModel] player];
                localPlayer = playerModel.localPlayer;
            }
        }];
        gms_onSignInSucceeded();
    } else {
        gms_onSignInFailed();
    }
}

- (void)achievementViewControllerDidFinish: (GPGAchievementController *)viewController {
    [g_getRootViewController() dismissViewControllerAnimated:YES completion:nil];
}

- (void)leaderboardViewControllerDidFinish: (GPGLeaderboardController *)viewController {
    [g_getRootViewController() dismissViewControllerAnimated:YES completion:nil];
}

@end
