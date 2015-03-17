//
//  ALErrorCodes.h
//  sdk
//
//  Created by Matt Szaro on 4/8/14.
//
//

// Loading & Displaying Ads

// Indicates that no ads are currently eligible for your device & location.
#define kALErrorCodeNoFill 204

// Indicates that a fetch ad request timed out (usually due to poor connectivity).
#define kALErrorCodeAdRequestNetworkTimeout -1001

// Indicates that an unspecified network issue occured, for instance if the user is in Airplane Mode.
#define kALErrorCodeAdRequestUnspecifiedError -1

// Indicates that an attempt to cache a resource to the filesystem failed; the device may be out of space.
#define kALErrorCodeUnableToPrecacheResources -200



// Rewarded Videos

// Indicates that the developer called for a rewarded video before one was available.
#define kALErrorCodeIncentiviziedAdNotPreloaded -300

// Indicates that an unknown server-side error occurred.
#define kALErrorCodeIncentivizedUnknownServerError -400

// Indicates that a reward validation requested timed out (usually due to poor connectivity).
#define kALErrorCodeIncentivizedValidationNetworkTimeout -500

// Indicates that the user exited out of the video early.
// You may or may not wish to grant a reward depending on your preference.
#define kALErrorCodeIncentivizedUserClosedVideo -600