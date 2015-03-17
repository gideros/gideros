//
//  InMobi.h
//  InMobi Monetization SDK
//
//  Copyright (c) 2013 InMobi. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

/**
 * Console log levels
 */
typedef enum {
    // No logs.
    IMLogLevelNone      = 0,

    // Minimal set of logs for debugging.
    IMLogLevelDebug     = 1,

    // Log everything
    // @note: Please turn off verbose mode before wide distribution like
    // AppStore. Keeping the verbose mode turned on might impact performance.
    IMLogLevelVerbose   = 2,
} IMLogLevel;

/**
 * Device Id collection masks
 */
typedef enum {
    // Use default ids for sdk device id collection. (default)
    IMDeviceIdMaskIncludeDefaultIds = 0,

    // Exclude odin1 identifier from sdk device id collection.
    IMDeviceIdMaskExcludeODIN1 = 1<<0,

    // Exclude advertiser identifier from sdk device id collection. (iOS 6+)
    IMDeviceIdMaskExcludeAdvertisingId = 1<<1,

    // Exclude vendor identifier from sdk device id collection. (iOS 6+)
    IMDeviceIdMaskExcludeVendorId = 1<<2,

    // @deprecated
    // @note: This flag is deprecated as sdk does not collect UDID any more.
    //        Exclude udid identifier from sdk device id collection.
    IMDeviceIdMaskExcludeUDID = 1<<3,

    // Exclude facebook's attribution id from sdk device id collection.
    IMDeviceIdMaskExcludeFacebookAttributionId = 1<<4,
} IMDeviceIdMask;

/**
 * User ids to help deliver more relevant ads.
 */
typedef enum {
    // User login id such as facebook, twitter, etc.
    kIMUserIdLogin,

    // For maintaining different sessions within the same login id.
    kIMUserIdSession,
} IMUserId;

/**
 * User Gender
 */
typedef enum {
    kIMGenderMale = 1,
    kIMGenderFemale,
    kIMGenderUnknown,
} IMGender;

/**
 * User Ethnicity
 */
typedef enum {
    kIMEthnicityHispanic = 1,
    kIMEthnicityCaucasian,
    kIMEthnicityAsian,
    kIMEthnicityAfricanAmerican,
    kIMEthnicityOther,
    kIMEthnicityUnknown,
} IMEthnicity;

/**
 * User Education
 */
typedef enum {
    kIMEducationHighSchoolOrLess = 1,
    kIMEducationCollegeOrGraduate,
    kIMEducationPostGraduateOrAbove,
    kIMEducationUnknown,
} IMEducation;

/**
 * Different Interstitial states
 */
typedef enum {
    // The state of interstitial cannot be determined.
    kIMInterstitialStateUnknown = 0,

    // The default state of an interstitial.
    // If an interstitial ad request fails, or if the user dismisses the
    // interstitial, the state will be changed back to init.
	kIMInterstitialStateInit,

    // Indicates that an interstitial ad request is in progress.
    kIMInterstitialStateLoading,

    // Indicates that an interstitial ad is ready to be displayed.
    // An interstitial ad can be displayed only if the state is ready.
    // You can call presentFromRootViewController: to display this ad.
    kIMInterstitialStateReady,

    // Indicates that an interstitial ad is displayed on the user's screen.
    kIMInterstitialStateActive

} IMInterstitialState;

/**
 * Interstitial ad mode
 */
typedef enum  {
    // Interstitial for AdNetwork.
    IMAdModeNetwork,

    // Interstitial for App Gallery.
    IMAdModeAppGallery
} IMAdMode;

/**
 * User HasChildren
 */
typedef enum {
    kIMHasChildrenTrue = 1,
    kIMHasChildrenFalse,
    kIMHasChildrenUnknown,
} IMHasChildren;

/**
 * User Marital Status
 */
typedef enum {
    kIMMaritalStatusSingle = 1,
    kIMMaritalStatusDivorced,
    kIMMaritalStatusEngaged,
    kIMMaritalStatusRelationship,
    kIMMaritalStatusUnknown,
} IMMaritalStatus;

/**
 * User Sexual Orientation
 */
typedef enum {
    kIMSexualOrientationStraight = 1,
    kIMSexualOrientationBisexual,
    kIMSexualOrientationGay,
    kIMSexualOrientationUnknown,
} IMSexualOrientation;

/**
 * General functions common to all InMobi SDKs
 */
@interface InMobi : NSObject

/**
 * Initialize InMobi SDKs with the Publisher App Id obtained from InMobi portal.
 * @param publisherAppId publisher's app id
 */
+ (void)initialize:(NSString *)publisherAppId;

#pragma mark Console Log Levels

/**
 * Set the console logging level.
 * @param logLevel Log level to be set.
 */
+ (void)setLogLevel:(IMLogLevel)logLevel;

#pragma mark Device ID Mask

/**
 * This sets the Device Id Mask to restrict the Device Tracking not to be
 * based on certain Device Ids.
 * @param deviceIdMask Device id mask to be set.
 */
+ (void)setDeviceIdMask:(IMDeviceIdMask)deviceIdMask;

#pragma mark SDK Information

/**
 * Returns the sdk version.
 * @return the sdk version
 */
+ (NSString *)getVersion;

#pragma mark User Information

/**
 * Set user's gender.
 * @param gender Gender of the user
 */
+ (void)setGender:(IMGender)gender;

/**
 * Set user's educational qualification.
 * @param education Educational qualification of the user
 */
+ (void)setEducation:(IMEducation)education;

/**
 * Set user's ethnicity.
 * @param ethnicity Ethnicity of the user
 */
+ (void)setEthnicity:(IMEthnicity)ethnicity;

/**
 * Set user's date of birth.
 * @param dateOfBirth Date of Birth of the user as NSDate object
 */
+ (void)setDateOfBirth:(NSDate *)dateOfBirth;

/**
 * Set user's date of birth
 * @param month Birth month of the user
 * @param day Birth day of the user
 * @param year Birth year of the user
 */
+ (void)setDateOfBirthWithMonth:(NSUInteger)month
                            day:(NSUInteger)day
                           year:(NSUInteger)year;

/**
 * Set user's annual income.
 * @param income Annual income of the user in USD
 */
+ (void)setIncome:(NSInteger)income;

/**
 * Set user's age.
 * @param age Age of the user
 */
+ (void)setAge:(NSInteger)age;

/**
 * Set user's marital status.
 * @param status Marital status of the user
 */
+ (void)setMaritalStatus:(IMMaritalStatus)status;

/**
 * Set whether the user has any children.
 * @param children Boolean for whether the user has any children
 */
+ (void)setHasChildren:(IMHasChildren)children;

/**
 * Set user's sexual orientation.
 * @param sexualOrientation Sexual orientation of the user
 */
+ (void)setSexualOrientation:(IMSexualOrientation)sexualOrientation;

/**
 * Set user's language preference.
 * @param langugage Preferred language of the user
 */
+ (void)setLanguage:(NSString *)langugage;

/**
 * Set user's postal code.
 * @param postalCode Postal code of the user
 */
+ (void)setPostalCode:(NSString *)postalCode;

/**
 * Set user's area code.
 * @param areaCode Area code of the user
 */
+ (void)setAreaCode:(NSString *)areaCode;

/**
 * Set user's interests (contextually relevant strings comma separated).
 * @param interests Interests of the user. E.g: @"cars,bikes,racing"
 */
+ (void)setInterests:(NSString *)interests;

#pragma mark User Location

/**
 * Use this to set the user's current location to deliver more relevant ads.
 * However do not use Core Location just for advertising, make sure it is used
 * for more beneficial reasons as well.  It is both a good idea and part of
 * Apple's guidelines.
 * @param latitude Location latitude
 * @param longitude Location longitude
 * @param accuracyInMeters Location accuracy in meters
 */
+ (void)setLocationWithLatitude:(CGFloat)latitude
                      longitude:(CGFloat)longitude
                       accuracy:(CGFloat)accuracyInMeters;

/**
 * Provide user location for city level targeting.
 * @param city User's city
 * @param state User's state
 * @param country User's country
 */
+ (void)setLocationWithCity:(NSString *)city
                      state:(NSString *)state
                    country:(NSString *)country;

#pragma mark User IDs

/**
 * Set user ids such as facebook, twitter etc to deliver more relevant ads.
 * @param userId User id type
 * @param idValue User id value
 */
+ (void)addUserID:(IMUserId)userId withValue:(NSString *)idValue;

/**
 * Remove the user ids which was set before. This fails silently if the id type
 * was not set before.
 * @param userId User id type to remove
 */
+ (void)removeUserID:(IMUserId)userId;

@end
