#include <string>

std::string getLocale()
{
	NSString* locale = [[NSLocale currentLocale] localeIdentifier];
		
	return [locale UTF8String];
}

std::string getLanguage()
{
	NSString* language = [[NSLocale preferredLanguages] objectAtIndex:0];
	
	return [language UTF8String];
}