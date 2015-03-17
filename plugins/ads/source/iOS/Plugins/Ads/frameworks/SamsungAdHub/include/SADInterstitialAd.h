//
// SADInterstitialAd.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol SADInterstitialAdDelegate;
@interface SADInterstitialAd : NSObject
@property (nonatomic, assign) NSObject<SADInterstitialAdDelegate> *delegate;
@property (nonatomic, retain) NSString *inventoryID;
- (void)presentFromViewController:(UIViewController *)viewController url:(NSString *)url;
@end

