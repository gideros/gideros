//
//  GFBook.m
//  Test
//
//  Created by Arturs Sosins on 2/19/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import "GFBook.h"
#include "gfacebook.h"

static const NSArray *publishPermissions = @[@"publish_actions", @"publish_stream", @"ads_management", @"create_event", @"rsvp_event", @"manage_friendlists", @"manage_notifications", @"manage_pages"];

@implementation GFBook
-(id)init{
    self.readPermissions = [[NSMutableArray alloc] init];
    self.writePermissions = [[NSMutableArray alloc] init];
    self.loggedOut = NO;
    return self;
}
-(void)deinit{
    [self.readPermissions release];
    self.readPermissions = nil;
    
    [self.writePermissions release];
    self.writePermissions = nil;
}

-(void)login:(NSArray *)permissions{
    for(NSString *permission in permissions)
    {
        if([publishPermissions containsObject:permission])
        {
            [self.writePermissions addObject:permission];
        }
        else{
            [self.readPermissions addObject:permission];
        }
    }
    
    // Whenever a person opens the app, check for a cached session
    if (FBSession.activeSession.state == FBSessionStateCreatedTokenLoaded) {
        
        // If there's one, just open the session silently, without showing the user the login UI
        [FBSession openActiveSessionWithReadPermissions:self.readPermissions
                                           allowLoginUI:NO
                                      completionHandler:
         ^(FBSession *session, FBSessionState state, NSError *error) {
             [self sessionStateChanged:session state:state error:error];
         }];
    }
    else if (FBSession.activeSession.state != FBSessionStateOpen
        || FBSession.activeSession.state != FBSessionStateOpenTokenExtended) {
        [FBSession openActiveSessionWithReadPermissions:self.readPermissions
                                           allowLoginUI:YES
                                      completionHandler:
         ^(FBSession *session, FBSessionState state, NSError *error) {
             [self sessionStateChanged:session state:state error:error];
         }];
    }

}

-(void)sessionStateChanged:(FBSession*)session state:(FBSessionState)state error:(NSError*)error{
    // If the session was opened successfully
    if (!error && state == FBSessionStateOpen){
        [FBSession setActiveSession:session];
        [FBSession.activeSession refreshPermissionsWithCompletionHandler:nil];
        gfacebook_onLoginComplete();
        return;
    }
    if (state == FBSessionStateClosed && self.loggedOut){
        self.loggedOut = NO;
        // If the session is closed
        gfacebook_onLogoutComplete();
        return;
    }
    if (state == FBSessionStateClosedLoginFailed){
        // If the session is closed
        NSString *errorMsg = @"Login failed";
        if (error){
            if ([FBErrorUtility shouldNotifyUserForError:error] == YES){
                errorMsg = [FBErrorUtility userMessageForError:error];
            }
            else if ([FBErrorUtility errorCategoryForError:error] == FBErrorCategoryUserCancelled)
            {
                errorMsg = @"User cancelled login";
            } else if ([FBErrorUtility errorCategoryForError:error] == FBErrorCategoryAuthenticationReopenSession){
                [self login:self.readPermissions];
            }
        }
        gfacebook_onLoginError([errorMsg UTF8String]);
        return;
    }
    
    if (error){
        NSString *errorMsg = @"Login failed";
        if ([FBErrorUtility shouldNotifyUserForError:error] == YES){
            errorMsg = [FBErrorUtility userMessageForError:error];
        }
        else if ([FBErrorUtility errorCategoryForError:error] == FBErrorCategoryUserCancelled)
        {
            errorMsg = @"User cancelled login";
        } else if ([FBErrorUtility errorCategoryForError:error] == FBErrorCategoryAuthenticationReopenSession){
            [self login:self.readPermissions];
        }
        else {
            //Get more error information from the error
            NSDictionary *errorInformation = [[[error.userInfo objectForKey:@"com.facebook.sdk:ParsedJSONResponseKey"] objectForKey:@"body"] objectForKey:@"error"];
            
            errorMsg = [errorInformation objectForKey:@"message"];
        }
        gfacebook_onLoginError([errorMsg UTF8String]);
    }
}

-(void)logout{
    if (FBSession.activeSession.state == FBSessionStateOpen
        || FBSession.activeSession.state == FBSessionStateOpenTokenExtended) {
        self.loggedOut = YES;
        [FBSession.activeSession closeAndClearTokenInformation];
    }
}

-(NSString*)getAccessToken{
    return [FBSession activeSession].accessTokenData.accessToken;
}
-(NSTimeInterval)getExpirationDate{
    return [[FBSession activeSession].accessTokenData.expirationDate timeIntervalSince1970];
}

-(void)upload:(NSString*)path with:(NSString*)orig{
    if (FBSession.activeSession.state == FBSessionStateOpen
        || FBSession.activeSession.state == FBSessionStateOpenTokenExtended) {
        
        if ([FBSession.activeSession.permissions indexOfObject:@"publish_actions"] == NSNotFound) {
            // if we don't already have the permission, then we request it now
            [FBSession.activeSession requestNewPublishPermissions:self.writePermissions
                                                  defaultAudience:FBSessionDefaultAudienceFriends
                                                completionHandler:^(FBSession *session, NSError *error) {
                                                    if (!error) {
                                                        [self upload:path with:orig];
                                                    } else if (error.fberrorCategory != FBErrorCategoryUserCancelled) {
                                                        gfacebook_onRequestError([path UTF8String], "Permission denied");
                                                    }
                                                    else
                                                    {
                                                        gfacebook_onRequestError([path UTF8String], "No permission");
                                                    }
                                                }];
        } else {
            
            FBRequest* request = [FBRequest requestForUploadStagingResourceWithImage:[UIImage imageWithContentsOfFile:path]];
            
            FBRequestConnection *connection = [[FBRequestConnection alloc] init];
            
            connection.errorBehavior = FBRequestConnectionErrorBehaviorRetry;
            
            [connection addRequest:request
                 completionHandler:^(FBRequestConnection *connection, id result, NSError *error) {
                     if(error != NULL)
                     {
                         gfacebook_onRequestError([orig UTF8String], [[error localizedDescription] UTF8String]);
                     }
                     else{
                         gfacebook_onRequestComplete([orig UTF8String], [[self simpleJSONEncode:result] UTF8String]);
                     }
                 }];
            [connection start];
        }
    }

}

-(void)dialog:(NSString*)path withParams:(NSDictionary*)params{
    if (FBSession.activeSession.state == FBSessionStateOpen
        || FBSession.activeSession.state == FBSessionStateOpenTokenExtended) {
        [FBWebDialogs presentDialogModallyWithSession:[FBSession activeSession] dialog:path parameters:params handler:^(FBWebDialogResult result, NSURL *resultURL, NSError *error) {
            if(error != NULL)
            {
                gfacebook_onDialogError([path UTF8String], [[error localizedDescription] UTF8String]);
            }
            else{
                if(result == FBWebDialogResultDialogCompleted)
                {
                    if (resultURL.query)
                    {
                        NSMutableDictionary *res = [self decodeUrl:[resultURL query]];
                        if([res objectForKey:@"error_message"]){
                            gfacebook_onDialogError([path UTF8String], [[[[res objectForKey:@"error_message"]
                            stringByReplacingOccurrencesOfString:@"+" withString:@" "]
                            stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding] UTF8String]);
                        }
                        else if([res objectForKey:@"post_id"])
                        {
                            gfacebook_onDialogComplete([path UTF8String], [[res objectForKey:@"post_id"] UTF8String]);
                        }
                        else if([res objectForKey:@"request"])
                        {
                            gfacebook_onDialogComplete([path UTF8String], [[res objectForKey:@"request"] UTF8String]);
                        }
                        else{
                            gfacebook_onDialogError([path UTF8String], "Canceled");
                        }
                    }
                    else{
                        gfacebook_onDialogError([path UTF8String], "Canceled");
                    }
                }
                else{
                    gfacebook_onDialogError([path UTF8String], "Canceled");
                }
            }
        } delegate:self];
    }
}

-(void)request:(NSString*)path forMethod:(int)method withParams:(NSMutableDictionary*)params{
    if (FBSession.activeSession.state == FBSessionStateOpen
        || FBSession.activeSession.state == FBSessionStateOpenTokenExtended) {
        
        if (method == 1 && [FBSession.activeSession.permissions indexOfObject:@"publish_actions"] == NSNotFound) {
                // if we don't already have the permission, then we request it now
            NSLog(@"Write: %@", self.writePermissions);
                [FBSession.activeSession requestNewPublishPermissions:self.writePermissions
                    defaultAudience:FBSessionDefaultAudienceFriends
                    completionHandler:^(FBSession *session, NSError *error) {
                        if (!error) {
                            [self request:path forMethod:method withParams:params];
                        } else if (error.fberrorCategory != FBErrorCategoryUserCancelled) {
                            gfacebook_onRequestError([path UTF8String], "Permission denied");
                        }
                        else
                        {
                            [FBSession.activeSession refreshPermissionsWithCompletionHandler:nil];
                            gfacebook_onRequestError([path UTF8String], "No permission");
                        }
                }];
        } else {

            if(params != nil && [params objectForKey:@"path"] != nil){
                [params setValue:[UIImage imageWithContentsOfFile:[params objectForKey:@"path"]] forKey:@"picture"];
                [params removeObjectForKey:@"path"];
                if([params objectForKey:@"album"] != nil){
                    path = [[params objectForKey:@"album"] stringByAppendingString:@"/photos"];
                }
            }
        
            FBRequest* request = [FBRequest requestWithGraphPath:path parameters:params HTTPMethod:[self convertMethod:method]];
            
            FBRequestConnection *connection = [[FBRequestConnection alloc] init];
            
            connection.errorBehavior = FBRequestConnectionErrorBehaviorRetry;
            
            [connection addRequest:request
                 completionHandler:^(FBRequestConnection *connection, id result, NSError *error) {
                     if(error != NULL)
                     {
                         gfacebook_onRequestError([path UTF8String], [[error localizedDescription] UTF8String]);
                     }
                     else{
                         gfacebook_onRequestComplete([path UTF8String], [[self simpleJSONEncode:result] UTF8String]);
                     }
                 }];
            [connection start];
        }
    }
}

- (void)webDialogsWillPresentDialog:(NSString *)dialog
                         parameters:(NSMutableDictionary *)parameters
                            session:(FBSession *)session
{
    [[[UIApplication sharedApplication].windows objectAtIndex:0] makeKeyAndVisible];
}

-(NSString *)simpleJSONEncode:(id)data{
    if (data) {
        NSData *json = [NSJSONSerialization dataWithJSONObject:data
                                                       options:0
                                                         error:nil];
        return [[[NSString alloc] initWithData:json
                                      encoding:NSUTF8StringEncoding]
                autorelease];
    } else {
        return nil;
    }
}

-(NSMutableDictionary*)decodeUrl:(NSString*)url{
    NSMutableDictionary *params = [[NSMutableDictionary alloc] init];
    for (NSString *param in [url componentsSeparatedByString:@"&"]) {
        NSArray *elts = [param componentsSeparatedByString:@"="];
        if([elts count] < 2) continue;
        [params setObject:[elts objectAtIndex:1] forKey:[elts objectAtIndex:0]];
    }
    return params;
}

-(NSString*)convertMethod:(int) method{
    if(method == 1)
        return @"POST";
    else if(method == 2)
        return @"DELETE";
    else
        return @"GET";
}

-(void)handleOpenUrl:(NSURL*)url{
    //cold start
    [FBSession.activeSession setStateChangeHandler:
     ^(FBSession *session, FBSessionState state, NSError *error) {
        [self sessionStateChanged:session state:state error:error];
     }];
    [FBAppCall handleOpenURL:url sourceApplication:@"com.apple.mobilesafari"];
}

- (void)applicationDidBecomeActive{
    [FBAppCall handleDidBecomeActive];
}


@end
