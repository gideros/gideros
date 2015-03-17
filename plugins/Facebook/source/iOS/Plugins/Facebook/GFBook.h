//
//  GFBook.h
//  Test
//
//  Created by Arturs Sosins on 2/19/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <FacebookSDK/FacebookSDK.h>

@interface GFBook : NSObject<FBWebDialogsDelegate>
@property(nonatomic, retain)NSMutableArray* readPermissions;
@property(nonatomic, retain)NSMutableArray* writePermissions;
@property(nonatomic)BOOL loggedOut;
-(id)init;
-(void)deinit;
-(void)login:(NSArray *)permissions;
-(void)logout;
-(NSString*)getAccessToken;
-(NSTimeInterval)getExpirationDate;
-(void)upload:(NSString*)path with:(NSString*)orig;
-(void)dialog:(NSString*)path withParams:(NSDictionary*)params;
-(void)request:(NSString*)path forMethod:(int)method withParams:(NSDictionary*)params;
-(void)sessionStateChanged:(FBSession*)session state:(FBSessionState)state error:(NSError*)error;
-(void)handleOpenUrl:(NSURL*)url;
- (void)applicationDidBecomeActive;
@end
