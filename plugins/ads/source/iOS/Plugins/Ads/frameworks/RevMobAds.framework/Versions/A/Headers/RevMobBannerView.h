#import <UIKit/UIKit.h>
#import "RevMobAdsDelegate.h"


@class RevMobBannerView;
typedef void (^RevMobBannerViewSuccessfullHandler)(RevMobBannerView *banner);
typedef void (^RevMobBannerViewFailureHandler)(RevMobBannerView *banner, NSError *error);
typedef void (^RevMobBannerViewOnclickHandler)(RevMobBannerView *banner);

/**
 Class responsable for handle the BannerView ad unit.
 
 You must integrate it in you app layout as a regualar UIView, if you don't want to handle
 the layout integration you should use RevMobBanner class.
 */
@interface RevMobBannerView : UIView <UIWebViewDelegate>

/**
 The delegate setted on this property is called when ad related events happend, see
 RevMobAdsDelegate for mode details.
 */
@property (nonatomic, assign) id<RevMobAdsDelegate> delegate;

/**
 Use this method to load the ad.
 
 @see loadWithSuccessHandler:andLoadFailHandler:onClickHandler:
 */
- (void)loadAd;

/**
 Use this method to load the ad.
 
 Example of usage:
 
     [banner loadWithSuccessHandler:^(RevMobBannerView *banner) {
       [banner setFrame:CGRectMake(10, 20, 200, 40)];
       [self.view addSubview:banner];
       NSLog(@"Ad loaded");
     } andLoadFailHandler:^(RevMobBannerView *banner, NSError *error) {
       NSLog(@"Ad error: %@",error);
     } onClickHandler:^(RevMobBannerView *banner) {
       NSLog(@"Ad clicked");
     }];

 @param onAdLoadedHandler: A block that will be executed once the ad is loaded, can be nil.

 @param onAdFailedHandler: A block that will be executed once any error happen, can be nil.

 @param onClickHandler: A block that will be executed once the user click on the ad, can be nil.
 */
- (void)loadWithSuccessHandler:(RevMobBannerViewSuccessfullHandler)onAdLoadedHandler
            andLoadFailHandler:(RevMobBannerViewFailureHandler)onAdFailedHandler
                onClickHandler:(RevMobBannerViewOnclickHandler)onClickHandler;

@end

