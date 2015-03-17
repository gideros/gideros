//
//  ALAdLoadDelegate.h
//  sdk
//
//  Created by Basil on 3/23/12.
//  Copyright (c) 2013, AppLovin Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ALAd.h"

/**
 * This protocol defines a listener for ad load events.
 */
@class ALAdService;

@protocol ALAdLoadDelegate <NSObject>

/**
 * This method is invoked when an ad is loaded by the AdService.
 *
 * This method is invoked on the main UI thread.
 *
 * @param adService AdService which loaded the ad. Will not return nil.
 * @param ad        Ad that was loaded. Will not return nil.
 */
-(void)adService:(ALAdService *)adService didLoadAd:(ALAd *)ad;

/**
 * This method is invoked when an ad load fails.
 *
 * This method is invoked on the main UI thread.
 *
 * @param adService AdService which failed to load an ad. Will not return nil.
 * @param code      An error code corresponding with a constant defined in <code>ALErrorCodes.h</code>.
 */
-(void)adService:(ALAdService *)adService didFailToLoadAdWithError:(int)code;

@end
