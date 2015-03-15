#import <Foundation/Foundation.h>
#import "RevMobAdsDelegate.h"


@class RevMobBanner;
typedef void (^RevMobBannerSuccessfullHandler)(RevMobBanner *banner);
typedef void (^RevMobBannerFailureHandler)(RevMobBanner *banner, NSError *erro);
typedef void (^RevMobBannerOnClickHandler)(RevMobBanner *banner);

/**
 Class responsable for handle the Banner ad unit.

 With this class the banner is showed in your app without the necessity to 
 handle the layout and inside your app. The banner is showed over your app.
 If you want to integrate the banner inside your app layout you should use
 RevMobBannerView class.
 */
@interface RevMobBanner : NSObject

/**
 The delegate setted on this property is called when ad related events happend, see
 RevMobAdsDelegate for mode details.
 */
@property (nonatomic, assign) id<RevMobAdsDelegate> delegate;

/**
 You can use this property to define the position of the banner in the screen.
 The default is a banner on the botton of the screen.
 */
@property (nonatomic, assign) CGRect frame;

/**
 The default behaviour of the banner is rotate for all the orientations.
 If you want to change this you should set an array in this property with the
 desired orientations.
 
 Example of use:

     banner.supportedInterfaceOrientations = @[ @(UIInterfaceOrientationPortrait) ];
 */
@property (nonatomic, strong) NSArray *supportedInterfaceOrientations;

/**
 Use this method to load the ad.

 @see loadWithSuccessHandler:andLoadFailHandler:onClickHandler:
 */
- (void)loadAd;

/**
 Use this method to load the ad.

 Example of usage:

     [banner loadWithSuccessHandler:^(RevMobBanner *banner) {
       [banner showAd];
       NSLog(@"Ad loaded");
     } andLoadFailHandler:^(RevMobBanner *banner, NSError *error) {
       NSLog(@"Ad error: %@",error);
     } onClickHandler:^(RevMobBanner *banner) {
       NSLog(@"Ad clicked");
     }];

 @param onAdLoadedHandler: A block that will be executed once the ad is loaded, can be nil.

 @param onAdFailedHandler: A block that will be executed once any error happen, can be nil.

 @param onClickHandler: A block that will be executed once the user click on the ad, can be nil.
 */
- (void)loadWithSuccessHandler:(RevMobBannerSuccessfullHandler)onAdLoadedHandler
            andLoadFailHandler:(RevMobBannerFailureHandler)onAdFailedHandler
                onClickHandler:(RevMobBannerOnClickHandler)onClickHandler;

/**
 Use this method to show the ad.
 */
- (void)showAd;

/**
 Use this method to hide the ad.
 */
- (void)hideAd;

@end

