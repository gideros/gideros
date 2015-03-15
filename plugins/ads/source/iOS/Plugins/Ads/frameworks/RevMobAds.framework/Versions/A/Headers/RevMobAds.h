#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "RevMobAdsDelegate.h"

#import "RevMobAdLink.h"
#import "RevMobBannerView.h"
#import "RevMobBanner.h"
#import "RevMobButton.h"
#import "RevMobFullscreen.h"
#import "RevMobPopup.h"


typedef enum {
    RevMobAdsTestingModeOff = 0,
    RevMobAdsTestingModeWithAds,
    RevMobAdsTestingModeWithoutAds
} RevMobAdsTestingMode;

typedef enum {
    RevMobUserGenderUndefined = 0,
    RevMobUserGenderMale,
    RevMobUserGenderFemale
} RevMobUserGender;

typedef enum {
    RevMobParallaxModeOff = 0,
    RevMobParallaxModeDefault,
    RevMobParallaxModeWithBackground
} RevMobParallaxMode;

/**
 This is the main class to start using RevMob Ads.
 You can use the standard facade methods or the alternative object orientaded version.

 */
@interface RevMobAds : NSObject {
}

@property (nonatomic, assign) id<RevMobAdsDelegate> delegate __attribute__((deprecated));
@property (nonatomic, assign) NSUInteger connectionTimeout;


/**
 This property is used to put the SDK in testing mode, there are 3 possible states.
 
 - RevMobAdsTestingModeOff - Turn off the testing mode;
 
 - RevMobAdsTestingModeWithAds - Testing mode that always shows ads;
 
 - RevMobAdsTestingModeWithoutAds- Testing mode that never shows ads.

 
 **Important note**: You **can't** submmit your app to Apple with testing mode turned on, this will show test ads that don't monetize.

 */
@property (nonatomic, assign) RevMobAdsTestingMode testingMode;

/**
 This property is used to set the parallaxe effect on ad units.
 
 - RevMobParallaxModeOff - Turn off the parallax effect
 
 - RevMobParallaxModeDefault - Use the default parallax effect
 
 - RevMobParallaxModeWithBackground - Use the parallax with black background effect

 Default value is RevMobParallaxModeOff.
 */
@property (nonatomic, assign) RevMobParallaxMode parallaxMode;

/**
 This property is used to set the user gender in order to get targeted ads with higher eCPM.
 There are two options: RevMobUserGenderFemale and RevMobUserGenderMale.

 Example of usage:
 
     [RevMobAds session].userGender = RevMobUserGenderFemale;

 */
@property (nonatomic, assign) RevMobUserGender userGender;

/**
 This property is used to set the minumum of user age range in order to get targeted ads with higher eCPM.
 You should set also set userAgeRangeMax or alternatively set userBirthday.
 
 Example of usage:

     [RevMobAds session].userAgeRangeMin = 18;

 */
@property (nonatomic, assign) NSUInteger userAgeRangeMin;

/**
 This property is used to set the maximum of user age range in order to get targeted ads with higher eCPM.
 You should set also set userAgeRangeMin or alternatively set userBirthday.

 Example of usage:

     [RevMobAds session].userAgeRangeMax = 21;

 */
@property (nonatomic, assign) NSUInteger userAgeRangeMax;

/**
 This property is used to set user age in order to get targeted ads with higher eCPM.
 Alternatively you can set userAgeRangeMin and userAgeRangeMin.

 Example of usage:

     [RevMobAds session].userBirthday = userBirthday;

 */
@property (nonatomic, strong) NSDate *userBirthday;

/**
 This property is used to set user insterests in order to get targeted ads with higher eCPM.
 You should pass an NSArray with NSStrings.

 Example of usage:

     [RevMobAds session].userInterests = @[@"games", @"mobile", @"advertising"];

 */
@property (nonatomic, strong) NSArray *userInterests;

/**
 This property is used to set user page in order to get targeted ads with higher eCPM.
 You should pass an NSStrings for a user page.

 Example of usage:

     [RevMobAds session].userPage = @"http://www.facebook.com/revmob";

 */
@property (nonatomic, strong) NSString *userPage;


/**
 This method is used to set user location in order to get targeted ads with higher eCPM.
 You should pass double values for the user latitude, longitude and accuracy.
 
 Example of usage:
 
 RevMobAds *revmob = [RevMobAds session];
 
 CLLocation *location = self.locationManager.location;
 
 [self.locationManager setDistanceFilter: kCLDistanceFilterNone];
 [self.locationManager setDesiredAccuracy: kCLLocationAccuracyHundredMeters];
 [self.locationManager startUpdatingLocation];
 
 [revmob setUserLatitude: location.coordinate.latitude
 userLongitude: location.coordinate.longitude
 userAccuracy: location.horizontalAccuracy];

// */
- (void)setUserLatitude:(double)userLatitude userLongitude: (double)userLongitude userAccuracy: (double)userAccuracy;

#pragma mark - Alternative use

/**
 This method is necessary to get the ads objects.
 
 It must be the first method called on the application:didFinishLaunchingWithOptions: method of the application delegate.
 
 Example of Usage:
 
     - (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
 
         [RevMobAds startSessionWithAppID:@"your RevMob App ID"];
 
         self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
 
         // Override point for customization after application launch.
     }
 
 @param appID: You can collect your App ID at http://revmob.com by looking up your apps.
 */
+ (RevMobAds *)startSessionWithAppID:(NSString *)anAppId;

/**
 This method is necessary to get the ads objects with delegate.
 
 It must be the first method called on the application:didFinishLaunchingWithOptions: method of the application delegate.
 
 Example of Usage:
 
 - (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
 
 [RevMobAds startSessionWithAppID:@"your RevMob App ID" andDelegate:self];
 
 self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
 
 // Override point for customization after application launch.
 }

 @param appID: You can collect your App ID at http://revmob.com by looking up your apps.
 @param adelegate:  The delegate is called when ad related events happen, see
 RevMobAdsDelegate for mode details. Can be nil;
 
 */
+ (RevMobAds *)startSessionWithAppID:(NSString *)anAppId andDelegate:(id<RevMobAdsDelegate>)adelegate;

/**
 This method is necessary to get the ads objects with delegate.

 It must be the first method called on the application:didFinishLaunchingWithOptions: method of the application delegate.
 
 Example of Usage:
 
 - (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
 
    [RevMobAds startSessionWithAppID:@"your RevMob App ID"
                withSuccessHandler:^{
                    NSLog(@"Session started with block");
                } andFailHandler:^(NSError *error) {
                    NSLog(@"Session failed to start with block");
    }];

 
 self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
 
 // Override point for customization after application launch.
 }
 
 @param appID: You can collect your App ID at http://revmob.com by looking up your apps.
 @param onSessionStartedHandler: A block that will be executed once the session is started, can be nil.
 @param onSessionNotStartedHandler: A block that will be executed once the session failed to start, can be nil.
 
 */
+ (RevMobAds *)startSessionWithAppID:(NSString *)anAppId
                  withSuccessHandler:(void(^)())onSessionStartedHandler
                      andFailHandler:(void(^)(NSError *error))onSessionNotStartedHandler;


/**
 This method can be used to get the already initializaded sesseion of RevMobAds.
 
 @return If is called before the session initialization, this method will return nil.
 */
+ (RevMobAds *)session;

/**
 This method is useful to send us information about your environment, which facilitates we identifing what is going on.
 */
- (void) printEnvironmentInformation;

#pragma mark - Basic usage

/**
 Show a fullscreen ad.

 Example of usage:
     [[RevMobAds session] showFullscreen];
 */
- (void)showFullscreen;

/**
 Show a banner.

 Example of usage:
     [[RevMobAds session] showBanner];

 @see hideBanner
 */
- (void)showBanner;

/**
 Hide the banner that is displayed.

 Example of usage:
     [[RevMobAds session] hideBanner];

 @see showBanner
 */
- (void)hideBanner;

/**
 Show popup.

 Example of usage:
     [[RevMobAds session] showPopup];

 */
- (void)showPopup;

/**
 Open the iTunes with one advertised app.

 Example of usage:
     [[RevMobAds session] openAdLinkWithDelegate:self];

 @param adelegate:  The delegate is called when ad related events happend, see
 RevMobAdsDelegate for mode details. Can be nil;

 */
- (void)openAdLinkWithDelegate:(id<RevMobAdsDelegate>)adelegate;

#pragma mark - Advanced mode

/**
 This is the factory of RevMobFullscreen ad object

 Example of Usage:

     RevMobFullscreen *ad = [[RevMobAds session] fullscreen]; // you must retain this object
     [ad loadWithSuccessHandler:^(RevMobFullscreen *fs) {
       [fs showAd];
       NSLog(@"Ad loaded");
     } andLoadFailHandler:^(RevMobFullscreen *fs, NSError *error) {
       NSLog(@"Ad error: %@",error);
     } onClickHandler:^{
       NSLog(@"Ad clicked");
     } onCloseHandler:^{
       NSLog(@"Ad closed");
     }];

 @return RevMobFullscreen object.
*/
- (RevMobFullscreen *)fullscreen;

/**
 This is the factory of RevMobFullscreen ad object

 Example of Usage:

     RevMobFullscreen *ad = [[RevMobAds session] fullscreenWithPlacementId:@"your RevMob placementId"]; // you must retain this object
     [ad loadWithSuccessHandler:^(RevMobFullscreen *fs) {
       [fs showAd];
       NSLog(@"Ad loaded");
     } andLoadFailHandler:^(RevMobFullscreen *fs, NSError *error) {
       NSLog(@"Ad error: %@",error);
     } onClickHandler:^{
       NSLog(@"Ad clicked");
     } onCloseHandler:^{
       NSLog(@"Ad closed");
     }];


 @param placementId: Optional parameter that identify the placement of your ad,
 you can collect your ids at http://revmob.com by looking up your apps. Can be nil.
 @return RevMobFullscreen object.
 */
- (RevMobFullscreen *)fullscreenWithPlacementId:(NSString *)placementId;

/**
 This is the factory of RevMobBannerView ad object

 Example of Usage:
 
      RevMobBannerView *ad = [[RevMobAds session] bannerView]; // you must retain this object
      [ad loadWithSuccessHandler:^(RevMobBannerView *banner) {
        if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
          banner.frame = CGRectMake(0, 0, 768, 114);
        } else {
          banner.frame = CGRectMake(0, 0, 320, 50);
        }
        [self.view addSubview:banner];
        NSLog(@"Ad loaded");
      } andLoadFailHandler:^(RevMobBannerView *banner, NSError *error) {
        NSLog(@"Ad error: %@",error);
      } onClickHandler:^(RevMobBannerView *banner) {
        NSLog(@"Ad clicked");
      }];
 
  @return RevMobBannerView object. 
*/
- (RevMobBannerView *)bannerView;

/**
 This is the factory of RevMobBannerView ad object

 Example of Usage:

    RevMobBannerView *ad = [[RevMobAds session] bannerViewWithPlacementId:@"your RevMob placementId"]; // you must retain this object
    [ad loadWithSuccessHandler:^(RevMobBannerView *banner) {
      if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
      banner.frame = CGRectMake(0, 0, 768, 114);
    } else {
      banner.frame = CGRectMake(0, 0, 320, 50);
    }
      [self.view addSubview:banner];
      NSLog(@"Ad loaded");
    } andLoadFailHandler:^(RevMobBannerView *banner, NSError *error) {
      NSLog(@"Ad error: %@",error);
    } onClickHandler:^(RevMobBannerView *banner) {
      NSLog(@"Ad clicked");
    }];

 @param placementId: Optional parameter that identify the placement of your ad,
 you can collect your ids at http://revmob.com by looking up your apps. Can be nil.
 @return RevMobBannerView object.
 */
- (RevMobBannerView *)bannerViewWithPlacementId:(NSString *)placementId;


/**
 This is the factory of RevMobBanner ad object

 Example of Usage:
 
      RevMobBanner *ad = [[RevMobAds session] banner]; // you must retain this object
      [ad loadWithSuccessHandler:^(RevMobBanner *banner) {
        [banner showAd];
        NSLog(@"Ad loaded");
      } andLoadFailHandler:^(RevMobBanner *banner, NSError *error) {
        NSLog(@"Ad error: %@",error);
      } onClickHandler:^(RevMobBanner *banner) {
        NSLog(@"Ad clicked");
      }];

  @return RevMobBanner object.
*/
- (RevMobBanner *)banner;

/**
 This is the factory of RevMobBanner ad object

 Example of Usage:

      RevMobBanner *ad = [[RevMobAds session] bannerWithPlacementId:@"your RevMob placementId"]; // you must retain this object
      [ad loadWithSuccessHandler:^(RevMobBanner *banner) {
        [banner showAd];
        NSLog(@"Ad loaded");
      } andLoadFailHandler:^(RevMobBanner *banner, NSError *error) {
        NSLog(@"Ad error: %@",error);
      } onClickHandler:^(RevMobBanner *banner) {
        NSLog(@"Ad clicked");
      }];

 @param placementId: Optional parameter that identify the placement of your ad,
 you can collect your ids at http://revmob.com by looking up your apps. Can be nil.
 @return RevMobBanner object.
 */
- (RevMobBanner *)bannerWithPlacementId:(NSString *)placementId;


/**
 This is the factory of button ad object already loaded

 Example of Usage:

       UIButton *button = [[RevMobAds session] button];
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

  @return RevMobButton object.
*/
- (RevMobButton *)button;

/**
 This is the factory of button ad object already loaded

 Example of Usage:

     UIButton *button = [[RevMobAds session] buttonWithPlacementId:@"your RevMob placementId"];
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

 @param placementId: Optional parameter that identify the placement of your ad,
 you can collect your ids at http://revmob.com by looking up your apps. Can be nil.
 @return RevMobButton object.
 */
- (RevMobButton *)buttonWithPlacementId:(NSString *)placementId;


/**
 This is the factory of button ad object not loaded

 Example of Usage:

     UIButton *button = [[RevMobAds session] buttonUnloaded];
     [button setFrame:CGRectMake(0, 0, 200, 50)];
     [button setTitle:@"Free Games" forState:UIControlStateNormal];

     [button loadWithSuccessHandler:^(RevMobButton *button) {
       [self.view addSubview:button];
     } andLoadFailHandler:^(RevMobButton *button, NSError *error) {
       NSLog(@"Button error: %@",erro);
     } onClickHandler:^(RevMobButton *button) {
       NSLog(@"Button clicked!");
     }];
 
 @return RevMobButton object.
 */
- (RevMobButton *)buttonUnloaded;

/**
 This is the factory of button ad object not loaded

 Example of Usage:

     UIButton *button = [[RevMobAds session] buttonUnloadedWithPlacementId:@"your RevMob placementId"];
     [button setFrame:CGRectMake(0, 0, 200, 50)];
     [button setTitle:@"Free Games" forState:UIControlStateNormal];

     [button loadWithSuccessHandler:^(RevMobButton *button) {
       [self.view addSubview:button];
     } andLoadFailHandler:^(RevMobButton *button, NSError *error) {
       NSLog(@"Button error: %@",erro);
     } onClickHandler:^(RevMobButton *button) {
       NSLog(@"Button clicked!");
     }];

 @param placementId: Optional parameter that identify the placement of your ad,
 you can collect your ids at http://revmob.com by looking up your apps. Can be nil.
 @return RevMobButton object.
 */
- (RevMobButton *)buttonUnloadedWithPlacementId:(NSString *)placementId;

/**
 This is the factory of adLink object

 Example of Usage:
 
     RevMobAdLink *ad = [[RevMobAds session] adLink]; // you must retain this object
     [link loadWithSuccessHandler:^(RevMobAdLink *link) {
       [link openLink];
       NSLog(@"Ad loaded");
     } andLoadFailHandler:^(RevMobAdLink *link, NSError *error) {
       NSLog(@"Ad error: %@",error);
     }];

  @return RevMobAdLink object.
*/
- (RevMobAdLink *)adLink;

/**
 This is the factory of adLink object

 Example of Usage:

      RevMobAdLink *ad = [[RevMobAds session] adLinkWithPlacementId:@"your RevMob placementId"]; // you must retain this object
      [link loadWithSuccessHandler:^(RevMobAdLink *link) {
        [link openLink];
        NSLog(@"Ad loaded");
      } andLoadFailHandler:^(RevMobAdLink *link, NSError *error) {
        NSLog(@"Ad error: %@",error);
      }];

 @param placementId: Optional parameter that identify the placement of your ad,
 you can collect your ids at http://revmob.com by looking up your apps. Can be nil.
 @return RevMobAdLink object.
 */
- (RevMobAdLink *)adLinkWithPlacementId:(NSString *)placementId;

/**
 This is the factory of popup ad object

 Example of Usage:
 
      RevMobPopup *ad = [[RevMobAds session] popup]; // you must retain this object
      [ad loadWithSuccessHandler:^(RevMobPopup *popup) {
        [popup showAd];
        NSLog(@"Popup loaded");
      } andLoadFailHandler:^(RevMobPopup *popup, NSError *error) {
        NSLog(@"Popup error: %@",error);
      } onClickHandler:^(RevMobPopup *popup) {
        NSLog(@"Popup clicked");
      }];

  @return RevMobPopup object.
*/
- (RevMobPopup *)popup;

/**
 This is the factory of popup ad object

 Example of Usage:

      RevMobPopup *ad = [[RevMobAds session] popupWithPlacementId:@"your RevMob placementId"]; // you must retain this object
      [ad loadWithSuccessHandler:^(RevMobPopup *popup) {
        [popup showAd];
        NSLog(@"Popup loaded");
      } andLoadFailHandler:^(RevMobPopup *popup, NSError *error) {
        NSLog(@"Popup error: %@",error);
      } onClickHandler:^(RevMobPopup *popup) {
        NSLog(@"Popup clicked");
      }];

 @param placementId: Optional parameter that identify the placement of your ad,
 you can collect your ids at http://revmob.com by looking up your apps. Can be nil.
 @return RevMobPopup object.
 */
- (RevMobPopup *)popupWithPlacementId:(NSString *)placementId;


@end
