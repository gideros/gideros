#import <Foundation/Foundation.h>
#import "RevMobAdsDelegate.h"


typedef enum {
    RevMobButtonStatusUndefined = -1,
    RevMobButtonStatusNew = 0,
    RevMobButtonStatusLoading,
    RevMobButtonStatusLoaded,
    RevMobButtonStatusLoadError
} RevMobButtonStatus;

@class RevMobButton;
typedef void (^RevMobButtonSuccessfullHandler)(RevMobButton *button);
typedef void (^RevMobButtonFailureHandler)(RevMobButton *button, NSError *error);
typedef void (^RevMobButtonOnclickHandler)(RevMobButton *button);


/**
 Subclass of UIButton, you can use in your app just as a regular UIButton.

 You should alter just the appearance of it.

 When the button is clicked a RevMobAdLink is used, so the behaviour is the same
 of the adLink. The intention is to facilitate the implementation of a "More games"
 button.
 */
@interface RevMobButton : UIButton

/**
 The delegate setted on this property is called when ad related events happend, see
 RevMobAdsDelegate for mode details.
 */
@property (nonatomic, assign) id<RevMobAdsDelegate> delegate;


/**
 This property can be use to check the status of the button.
 
 The status can be:
 
 *RevMobButtonStatusNew* - New button, ad not loaded yet;
 
 *RevMobButtonStatusLoading* - Ad is loading;
 
 *RevMobButtonStatusLoaded* - Ad loaded;
 
 *RevMobButtonStatusLoadError* - Error while loading the ad, this button
 can't be reused to show the store anymore, it's necessary to create a new
 button. This can happen when there is no internet connection.
 
 *RevMobButtonStatusUndefined* - There is an unknow error or an unexpected
 behaviour.
 */
@property (atomic, readonly) RevMobButtonStatus status;

/**
 Use this method to load the ad.
 
 @see loadWithSuccessHandler:andLoadFailHandler:onClickHandler:
 */
- (void)loadAd;

/**
 Use this method to load the ad with completion handlres.
 
 Example of usage:

    [button loadWithSuccessHandler:^(RevMobButton *button) {
      [button setFrame:CGRectMake(10, yCoordinateControl, 300, 40)];
      [button setTitle:@"Free Games" forState:UIControlStateNormal];
      [self.view addSubview:button];
      NSLog(@"Ad received");
    } andLoadFailHandler:^(RevMobButton *button, NSError *error) {
      NSLog(@"Burron error: %@",error);
    } onClickHandler:^(RevMobButton *button) {
      NSLog(@"Button clicked!");
    }];
 
 @param onAdLoadedHandler: A block that will be executed once the ad is loaded, can be nil.

 @param onAdFailedHandler: A block that will be executed once any error happen, can be nil.

 */
- (void)loadWithSuccessHandler:(RevMobButtonSuccessfullHandler)onAdLoadedHandler
            andLoadFailHandler:(RevMobButtonFailureHandler)onAdFailedHandler
                onClickHandler:(RevMobButtonOnclickHandler)onClickHandler;


@end
