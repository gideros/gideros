//
//  InMobiAnalytics.h
//  InMobi Monetization SDK
//
//  Copyright (c) 2013 InMobi. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 * This class collects analytics for a publisher's application.
 */
@interface InMobiAnalytics : NSObject

/**
 * Use this method if and only if you want to manually track sessions
 * This method is idempotent, meaning, if automatic track sessions is enabled
 * or startSession was called before, this method does nothing.
 */
+ (void)startSessionManually;

/**
 * Use this method if and only if you want to manually end sessions
 * This method will also terminate automatically tracked sessions if enabled
 */
+ (void)endSessionManually;

/**
 * Call this method when the user has started or re-started a section.
 * @param sectionName A name string for the beginning section.
 * @param additionalParams Optional NSDictionary object. This dictionary can
 * have value type as (NSNumber,NSString) only.
 */
+ (void)beginSection:(NSString *)sectionName withParams:(NSDictionary *)additionalParams;

/**
 * Call this method when the user has started or re-started a section.
 * @param sectionName A name string for the beginning section.
 */
+ (void)beginSection:(NSString *)sectionName;

/**
 * Call this method when the user has completed,failed or finished a section.
 * @param sectionName A name string for the ending session.
 * @param additionalParams Optional NSDictionary object. This dictionary can
 * have value type as (NSNumber,NSString) only.
 */
+ (void)endSection:(NSString *)sectionName withParams:(NSDictionary *)additionalParams;

/**
 * Call this method when the user has completed,failed or finished a section.
 * @param sectionName A name string for the ending session.
 */
+ (void)endSection:(NSString *)sectionName;

/**
 * Call this method if you want to track custom events as happening during a
 * section.
 * @param eventName The custom event name.
 * @param additionalParams Optional NSDictionary to be provided by you.
 */
+ (void)tagEvent:(NSString *)eventName withParams:(NSDictionary *)additionalParams;

/**
 * Call this method if you want to track custom events as happening during a
 * section.
 * @param eventName The custom event name.
 */
+ (void)tagEvent:(NSString *)eventName;

/**
 * Call this method with the SKPaymentTransaction object, for manually tagging
 * the transaction.
 * @param skTransaction The SKPaymentTransaction object.
 */
+ (void)tagTransactionManually:(id)skTransaction;

/**
 * Reports the Application Custom Goals to InMobi Ad Tracker.
 * This is a non blocking API (returns immediately), and performs the
 * reporting job in the background.
 * When a goal is reported, it is made sure that it reaches InMobi Ad Tracker
 * server whenever the device is connected to internet. Different goals can be
 * reported without worrying about the internet connection.
 *
 * @deprecated Please start using tagEvent to manage goals
 * @param goalName Your custom goal name.
 */
+ (void)reportCustomGoal:(NSString *)goalName;

@end
