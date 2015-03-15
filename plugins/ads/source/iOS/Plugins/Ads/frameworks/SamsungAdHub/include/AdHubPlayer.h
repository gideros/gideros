//
// AdHubPlayer.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AdHubUserProfile.h"
#import "AdHubAdSize.h"

@class AdHubParameter;
@class SADPlayerConnection;
@interface AdHubPlayer : NSObject
{
NSString * strInventoryID;
AdHubParameter *param;
SADPlayerConnection *movieConnecter;
id delegate;
NSString *profileStr;
}
@property (nonatomic, assign)id delegate;

/**
@brief Constructor
@param strId Inventory id
@return VAST player object
*/
- (id)initWithInventoryID:(NSString *)strId;

/**
@brief Start app launch roll ad
*/
- (void)startAdAppLaunchRoll;

/**
@brief Start pre roll ad
@param videoUrl Address of contents video
@param Title Title of contents video
*/
- (void)startAdPreRollWithContentURL:(NSString *)videoUrl title:(NSString *)Title;

/**
@brief Start overlay ad
@param videoUrl Address of contents video
@param Title Title of contents video
@param size Banner size
*/
-(void)startAdOverlayWithContentURL:(NSString *)videoUrl title:(NSString *)Title bannerSize:(AdHubAdSize)size;

/**
@brief Set user profile
@param p User profile object
@remark For Ad targetting, Informations of user like age, gender, religion, etc. can be set.
*/
- (void) setUserProfile:(AdHubUserProfile *)p;
@end

