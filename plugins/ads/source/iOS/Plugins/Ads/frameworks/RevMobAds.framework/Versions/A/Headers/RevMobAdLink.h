#import <Foundation/Foundation.h>
#import "RevMobAdsDelegate.h"


@class RevMobAdLink;
typedef void (^RevMobAdLinkSuccessfullHandler)(RevMobAdLink *adLink);
typedef void (^RevMobAdLinkFailureHandler)(RevMobAdLink *adLink, NSError *erro);


/**
 Class responsable for handle the AdLink ad unit.
 
 If you want a button alread configured with an adLink use the RevMobButton.
 */
@interface RevMobAdLink : NSObject {
}

/**
 The delegate setted on this property is called when ad related events happend, see
 RevMobAdsDelegate for mode details.
 */
@property(nonatomic, assign) id<RevMobAdsDelegate> delegate;

/**
 Use this method to load the ad.
 
 @see loadWithSuccessHandler:andLoadFailHandler:
 */
- (void)loadAd;

/**
 Use this method to load the ad.


 Example of usage:

     [link loadWithSuccessHandler:^(RevMobAdLink *link) {
       [link openLink];
       NSLog(@"Ad loaded");
     } andLoadFailHandler:^(RevMobAdLink *link, NSError *error) {
       NSLog(@"Ad error: %@",error);
     }];

 @param onAdLoadedHandler: A block that will be executed once the ad is loaded, can be nil.

 @param onAdFailedHandler: A block that will be executed once any error happen, can be nil.
*/
- (void)loadWithSuccessHandler:(RevMobAdLinkSuccessfullHandler)onAdLoadedHandler
            andLoadFailHandler:(RevMobAdLinkFailureHandler)onAdFailedHandler;

/**
 Open the ad link.

 Use this method to open the iTunes with the advertised app.

 */
- (void)openLink;


@end
