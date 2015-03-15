//
//  ALAdRewardDelegate.h
//  sdk
//
//  Created by Matt Szaro on 1/3/14.
//
//

#import <Foundation/Foundation.h>
#import "ALAd.h"


/**
 *  This protocol defines a listener for rewarded video events.
 */
@protocol ALAdRewardDelegate <NSObject>

@required

/**
 *  This method is invoked if a user viewed a rewarded video and their reward was approved by the AppLovin server.
 *
 * If you are using reward validation for incentivized videos, this method
 * will be invoked if we contacted AppLovin successfully. This means that we believe the
 * reward is legitimate and should be awarded. Please note that ideally you should refresh the
 * user's balance from your server at this point to prevent tampering with local data on jailbroken devices.
 *
 * The response NSDictionary will typically includes the keys "currency" and "amount", which point to NSStrings containing the name and amount of the virtual currency to be awarded.
 *
 *  @param ad       Ad which was viewed.
 *  @param response Dictionary containing response data, including "currency" and "amount".
 */
-(void) rewardValidationRequestForAd: (ALAd*) ad didSucceedWithResponse: (NSDictionary*) response;

/**
 * This method will be invoked if we were able to contact AppLovin, but the user has already received
 * the maximum number of coins you allowed per day in the web UI.
 *
 *  @param ad       Ad which was viewed.
 *  @param response Dictionary containing response data from the server.
 */
-(void) rewardValidationRequestForAd: (ALAd*) ad didExceedQuotaWithResponse: (NSDictionary*) response;

/**
 * This method will be invoked if the AppLovin server rejected the reward request.
 * This would usually happen if the user fails to pass an anti-fraud check.
 *
 *  @param ad       Ad which was viewed.
 *  @param response Dictionary containing response data from the server.
 */
-(void) rewardValidationRequestForAd: (ALAd *) ad wasRejectedWithResponse: (NSDictionary*) response;

/**
 * This method will be invoked if were unable to contact AppLovin, so no ping will be heading to your server.
 *
 *  @param ad           Ad which was viewed.
 *  @param responseCode A failure code corresponding to a constant defined in <code>ALErrorCodes.h</code>.
 */
-(void) rewardValidationRequestForAd: (ALAd*) ad didFailWithError: (NSInteger) responseCode;

@optional

/**
 * This method will be invoked if the user chooses 'no' when asked if they want to view a rewarded video.
 *
 * This is only possible if you have the pre-video modal enabled in the Manage Apps UI.
 *
 * @param ad       Ad which was offered to the user, but declined.
 */
-(void) userDeclinedToViewAd: (ALAd*) ad;

@end
