//
// AdHubInterstitial.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import "SADInterstitialAd.h"
#import "AdHubUserProfile.h"

@class AdHubParameter;
@interface AdHubInterstitial : SADInterstitialAd {
AdHubParameter *param;
id<SADInterstitialAdDelegate, NSObject> delegate;
NSString *profileStr;
}

/**
@brief Constructor
@param inventoryID Inventory ID
@return Interstitial Object
*/
-(id) initWithInventoryID:(NSString *)inventoryID;

/**
@brief Show interstitial ad
@param viewController viewcontroller that shows interstitial ad.
*/
-(void) presentFromViewController:(UIViewController*) viewController;

/**
@brief set user profile
@param p user profile object
@remark for Ad targetting, Informations of user like age, gender, religion, etc. can be set.
*/
-(void) setUserProfile:(AdHubUserProfile *)p;

/**
@brief Delegate object that handles events
*/
@property (nonatomic, assign) id delegate;
@end

