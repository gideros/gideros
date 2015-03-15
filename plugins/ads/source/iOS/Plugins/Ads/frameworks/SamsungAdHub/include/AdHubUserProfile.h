//
// AdHubUserProfile.h
// SamsungAdHub
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface AdHubUserProfile : NSObject {
NSString *gender;
int age;
NSString *interest;
NSString *maritalstatus;
NSString *religion;
NSMutableString *rtv;
NSMutableArray *dInterest;
}

/**
@brief Female constant
@return @"f"
*/
+(NSString*) FEMALE;

/**
@brief Male constant
@return @"m"
*/
+(NSString*) MALE;

/**
@brief Single constant
@return @"single"
*/
+(NSString*) SINGLE;

/**
@brief Married constant
@return @"married"
*/
+(NSString*) MARRIED;

/**
@brief Buddhism constant
@return @"Buddhism"
*/
+(NSString*) BUDDHISM;

/**
@brief Christianity constant
@return @"Christianity"
*/
+(NSString*) CHRISTIANITY;

/**
@brief Islam constant
@return @"Islam"
*/
+(NSString*) ISLAM;

/**
@brief Judaism constant
@return @"Judaism"
*/
+(NSString*) JUDAISM;

/**
@brief Hinduism constant
@return @"Hinduism"
*/
+(NSString*) HINDUISM;

/**
@brief Unaffiliated constant
@return @"Unaffiliated"
*/
+(NSString*) UNAFFILIATED;

/**
@brief Others constant
@return @"Others"
*/
+(NSString*) OTHERS;

/**
@brief Set user's gender
@parameter g Gender, @"m" or @"f"
@remark If other value is used, it is set as unknown
*/
-(void) setGender:(NSString*)g;

/**
@brief Set user's marital status
@parameter m Marital status, @"single" or @"married"
@remark If other value is used, it is set as unknown
*/
-(void) setMaritalstatus:(NSString*)m;

/**
@brief Set user's religion
@parameter r Religion, @"Buddhism", @"Christianity", @"Islam", @"Judaism", @"Hinduism" or @"Unaffiliated
@remark If other value is used, it is set as @"Others"
*/
-(void) setReligion:(NSString*)r;

/**
@brief Set user's age
@parameter a Age
@remark If a is negative number a is set as 0. If a is bigger than 99, a is set as 99
*/
-(void) setAge:(int)a;

/**
@brief Set user's interests.
@parameter i Interests
@remark It can be called several times
*/
-(void)setInterests:(NSString*)i;
@end

