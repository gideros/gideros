#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@protocol RevMobAdsDelegate <NSObject>

/**
 Fired by Fullscreen, banner and popup. Called when the communication with the server is finished with error.

 @param error: contains error information.
 */
- (void)revmobAdDidFailWithError:(NSError *)error;

@optional

# pragma mark Ad Callbacks (Fullscreen, Banner and Popup)

/**
 Fired when session is started.
 */
- (void)revmobSessionIsStarted;

/**
 Fired when session fails to start.
 */
- (void)revmobSessionNotStartedWithError:(NSError *)error;

/**
 Fired by Fullscreen, banner and popup. Called when the communication with the server is finished with success.
 */
- (void)revmobAdDidReceive;

/**
 Fired by Fullscreen, banner and popup. Called when the Ad is displayed in the screen.
 */
- (void)revmobAdDisplayed;

/**
 Fired by Fullscreen, banner, button and popup.
 */
- (void)revmobUserClickedInTheAd;

/**
 Fired by Fullscreen and popup.
 */
- (void)revmobUserClosedTheAd;

# pragma mark Advertiser Callbacks

/**
 Called if install is successfully registered
 */
- (void)installDidReceive;

/**
 Called if install couldn't be registered
 */
- (void)installDidFail;

@end
