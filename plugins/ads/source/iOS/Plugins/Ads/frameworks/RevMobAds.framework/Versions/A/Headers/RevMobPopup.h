#import <Foundation/Foundation.h>
#import "RevMobAdsDelegate.h"


@class RevMobPopup;
typedef void (^RevMobPopupSuccessfullHandler)(RevMobPopup *popup);
typedef void (^RevMobPopupFailureHandler)(RevMobPopup *popup, NSError *error);
typedef void (^RevMobPopupOnClickHandler)(RevMobPopup *popup);


/**
 Class responsable for handle the Popoup ad unit.
 */
@interface RevMobPopup : NSObject <UIAlertViewDelegate>

/**
 The delegate setted on this property is called when ad related events happend, see
 RevMobAdsDelegate for mode details.
 */
@property(nonatomic, assign) id<RevMobAdsDelegate> delegate;

/**
 Use this method to load the ad.
 
 @see loadWithSuccessHandler:andLoadFailHandler:onClickHandler:
 */
- (void)loadAd;

/**
 Use this method to load the ad with completion handlres.

 Example of usage:

    [popup loadWithSuccessHandler:^(RevMobPopup *popup) {
      [popup showAd];
      NSLog(@"Popup loaded");
    } andLoadFailHandler:^(RevMobPopup *popup, NSError *error) {
      NSLog(@"Popup error: %@",error);
    } onClickHandler:^(RevMobPopup *popup) {
      NSLog(@"Popup clicked");
    }];

 @param onAdLoadedHandler: A block that will be executed once the ad is loaded, can be nil.

 @param onAdFailedHandler: A block that will be executed once any error happen, can be nil.

 @param onClickHandler: A block that will be executed once the user click on the ad, can be nil.

 */
- (void)loadWithSuccessHandler:(RevMobPopupSuccessfullHandler)onAdLoadedHandler
            andLoadFailHandler:(RevMobPopupFailureHandler)onAdFailedHandler
                onClickHandler:(RevMobPopupOnClickHandler)onClickHandler;


/**
 Use this method to show the ad.
 */
- (void)showAd;


@end
