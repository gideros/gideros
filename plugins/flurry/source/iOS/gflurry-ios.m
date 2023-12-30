#include "gflurry.h"

#import "Flurry.h"

static NSDictionary *getDictionary(const char **parameters)
{
    if (parameters == NULL)
        return nil;
    
    NSMutableDictionary *parameters2 = [NSMutableDictionary dictionaryWithCapacity:5];
    while (true)
    {
        if (*parameters == NULL)
            break;
        NSString *key = [NSString stringWithUTF8String:*parameters++];
        
        if (*parameters == NULL)
            break;
        NSString *value = [NSString stringWithUTF8String:*parameters++];
        
        [parameters2 setObject:value forKey:key];
    }

    return parameters2;
}

void gflurry_StartSession(const char *apiKey)
{
    [Flurry startSession:[NSString stringWithUTF8String:apiKey]];
}

void gflurry_LogEvent(const char *eventName, const char **parameters, int timed)
{
    NSDictionary *parameters2 = getDictionary(parameters);
    
    NSString *eventName2 = [NSString stringWithUTF8String:eventName];
    
    if (parameters2 && timed)
        [Flurry logEvent:eventName2 withParameters:parameters2 timed:YES];
    else if (!parameters2 && timed)
        [Flurry logEvent:eventName2 timed:YES];
    else if (parameters2 && !timed)
        [Flurry logEvent:eventName2 withParameters:parameters2];
    else if (!parameters2 && !timed)
        [Flurry logEvent:eventName2];
    
}

void gflurry_EndTimedEvent(const char *eventName, const char **parameters)
{
    NSDictionary *parameters2 = getDictionary(parameters);
    
    NSString *eventName2 = [NSString stringWithUTF8String:eventName];
    
    [Flurry endTimedEvent:eventName2 withParameters:parameters2];
}

void glurry_setSessionContinueSeconds(int seconds) {
    [Flurry setSessionContinueSeconds:seconds];
}

void gflurry_setGender(const char *gender) {
    NSString *gender2 = [NSString stringWithUTF8String:gender];
    [Flurry setGender:gender2];
}

void gflurry_setUserID(const char *userID) {
    NSString *userID2 = [NSString stringWithUTF8String:userID];
    [Flurry setUserID:userID2];
}

void gflurry_logError(const char *errorID, const char *message) {
    NSString *errorID2 = [NSString stringWithUTF8String:errorID];
    NSString *message2 = [NSString stringWithUTF8String:message];
    NSDictionary *userInfo =
             [NSDictionary dictionaryWithObjectsAndKeys:
              errorID2, NSLocalizedDescriptionKey,
                message2, NSLocalizedFailureReasonErrorKey,
                nil] ;
    
    NSError* error = [NSError errorWithDomain:@"com.tinymarble.Burds"
                code:123
            userInfo:userInfo];

    [Flurry logError:errorID2 message:message2 error:error];
}
