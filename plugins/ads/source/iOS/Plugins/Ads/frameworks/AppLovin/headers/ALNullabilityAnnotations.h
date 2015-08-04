//
//  ALNullabilityAnnotations.h
//  sdk
//
//  Created by Matt Szaro on 4/14/15.
//
//

/**
 This class exposes the new nullability annotation features of
 Xcode 6.3+ in a safe way, by providing AppLovin wrappers around
 them, and defining them to either the Clang defines or empty.
 
 This is done to allow you to take advantage of these new Obj-C/Swift
 features on Xcode 6.3+ while still maintaining build capability for
 older versions of Xcode.
*/

#ifndef sdk_ALNullabilityAnnotations_h
#define sdk_ALNullabilityAnnotations_h

#if  __has_feature(nullability)
    #define alnullable nullable
    #define alnonnull nonnull
    #define __alnullable __nullable
    #define __alnonnull __nonnull
#else
    #define alnullable
    #define alnonnull
    #define __alnullable
    #define __alnonnull
#endif

#endif
