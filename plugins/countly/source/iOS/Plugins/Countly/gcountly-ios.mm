#include "gcountly.h"

#import "Countly.h"

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

void gcountly_StartSession(const char *apiKey)
{
    [[Countly sharedInstance] startOnCloudWithAppKey:[NSString stringWithUTF8String:apiKey]];
}

void gcountly_LogEvent(const char *eventName, int count, double sum, const char **parameters)
{
    NSDictionary *parameters2 = getDictionary(parameters);
    
    NSString *eventName2 = [NSString stringWithUTF8String:eventName];
    
    if (parameters2 && sum != 0)
        [[Countly sharedInstance] recordEvent:eventName2 segmentation:parameters2 count:count sum:sum];
    else if (parameters2)
        [[Countly sharedInstance] recordEvent:eventName2 segmentation:parameters2 count:count];
    else if (sum != 0)
        [[Countly sharedInstance] recordEvent:eventName2 count:count sum:sum];
    else
        [[Countly sharedInstance] recordEvent:eventName2 count:count];

    
}