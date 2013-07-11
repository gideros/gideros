#import <UIKit/UIKit.h>

#include <vector>
#include <string>

#include <sys/socket.h> // Per msqr
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>

// typeSpecifier -> "hw.machine" or "hw.model"
NSString* getSysInfoByName(const char* typeSpecifier)
{
    size_t size;
    sysctlbyname(typeSpecifier, NULL, &size, NULL, 0);
    
    char* answer = (char*)malloc(size);
    sysctlbyname(typeSpecifier, answer, &size, NULL, 0);
    
    NSString* results = [NSString stringWithCString:answer encoding: NSUTF8StringEncoding];
	
    free(answer);
    return results;
}


std::vector<std::string> getDeviceInfo()
{
	std::vector<std::string> result;

	UIDevice* device = [UIDevice currentDevice];
		
	result.push_back("iOS");
	result.push_back([[device systemVersion] UTF8String]);
	result.push_back([[device model] UTF8String]);
	switch (UI_USER_INTERFACE_IDIOM())
	{
		case UIUserInterfaceIdiomPhone:
			result.push_back("iPhone");
			break;
		case UIUserInterfaceIdiomPad:
			result.push_back("iPad");
			break;
		default:
			result.push_back("");
	}	
	result.push_back([getSysInfoByName("hw.machine") UTF8String]);
	
	return result;
}
