#include <vector>
#include <string>

std::vector<std::string> getDeviceInfo()
{
	std::vector<std::string> result;

	result.push_back("WinRT");

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	result.push_back("Windows Phone");
#else
	result.push_back("Windows");
#endif

	return result;
}
