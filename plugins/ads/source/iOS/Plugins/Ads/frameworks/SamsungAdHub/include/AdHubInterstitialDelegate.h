//
// AdHubInterstitialDelegate.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@class AdHubInterstitial, AdHubRequestError;
@protocol AdHubInterstitialDelegate <NSObject>
@optional

/**
@brief interstitial It is called, when interstitial ad is unloaded.
*/
- (void)interstitialAdDidUnload:(AdHubInterstitial *)interstitialAd;

/**
@brief It is called, when interstitial ad has error.
*/
- (void)interstitialAd:(AdHubInterstitial *)interstitialAd didFailWithError:(AdHubRequestError *)error;

/**
@brief interstitial It is called, before interstitial ad is loaded.
*/
- (void)interstitialAdWillLoad:(AdHubInterstitial *)interstitialAd;

/**
@brief interstitial It is called, after interstitial ad is loaded.
*/
- (void)interstitialAdDidLoad:(AdHubInterstitial *)interstitialAd;
@end

