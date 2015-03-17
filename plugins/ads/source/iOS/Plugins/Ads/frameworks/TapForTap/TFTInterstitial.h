//
//  Copyright (c) 2013 Tap for Tap. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@class TFTInterstitial;

@protocol TFTInterstitialDelegate <NSObject>
@optional
- (void)tftInterstitialDidReceiveAd:(TFTInterstitial *)interstitial;
- (void)tftInterstitial:(TFTInterstitial *)interstitial didFail:(NSString *)reason;
- (void)tftInterstitialDidShow:(TFTInterstitial *)interstitial;
- (void)tftInterstitialWasTapped:(TFTInterstitial *)interstitial;
- (void)tftInterstitialWasDismissed:(TFTInterstitial *)interstitial;
@end

@interface TFTInterstitial : NSObject
+ (TFTInterstitial *)interstitial;
+ (TFTInterstitial *)interstitialWithDelegate:(id<TFTInterstitialDelegate>) delegate;

- (void)load;
- (void)showWithViewController:(UIViewController *)viewController;
- (void)showAndLoadWithViewController:(UIViewController *)viewController;
- (BOOL)readyToShow;
@end
