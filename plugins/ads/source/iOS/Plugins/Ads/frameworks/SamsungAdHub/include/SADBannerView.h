//
// SADBannerView.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AdHubAdSize.h"

@protocol SADBannerViewDelegate;
@class AdHubAdORMMAView;
@interface SADBannerView : UIView {
NSString *adUrl;
NSTimer *refreshTimer;
BOOL isRefreshing;
BOOL isFillingSuperViewNeeded;
}
@property BOOL isFillingSuperViewNeeded;
@property (nonatomic, retain) NSString *inventoryID;
@property (nonatomic, assign) AdHubAdSize adSize;
@property (nonatomic, assign) NSObject<SADBannerViewDelegate> *delegate;
@property (nonatomic, assign) BOOL isRefreshing;
@property (nonatomic, retain) UIViewController *rootViewController;
- (id)initWithAdSize:(AdHubAdSize)adSize origin:(CGPoint)origin;
- (void)startAd:(NSString *)url;
- (AdHubAdORMMAView *)adORMMAView;
- (void)ormarViewVideoType:(int)vt;
- (void)deRegisterObserver;
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context;
-(void) killCarouselTimer ;
-(void) setPortrait;
-(void) setLandscape;
@end

