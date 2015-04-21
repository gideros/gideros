#include <vector>
#include <string>

#include <stdlib.h>
#include <memory>
#include <concrt.h>
#include <ppltasks.h>

using namespace Windows::System;
using namespace Platform;
using namespace Windows::Storage;

using namespace Windows::System::UserProfile;
using namespace Windows::Globalization;

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

void openUrl(const char* url)
{
	size_t newsize = strlen(url) + 1;
	wchar_t *wcstring = new wchar_t[newsize];

	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring, newsize, url, _TRUNCATE);

	Platform::String ^string = ref new String(wcstring);
	auto uri = ref new Windows::Foundation::Uri(string);
	Launcher::LaunchUriAsync(uri);

	delete[] wcstring;
}

bool canOpenUrl(const char *url)
{
	return true;
}

std::string getLocale(){
	std::wstring data = GlobalizationPreferences::Languages->GetAt(0)->Data();
	return std::string(data.begin(), data.end());
}

std::string getLanguage()
{
	std::wstring data = GlobalizationPreferences::Languages->GetAt(0)->Data();
	std::string s(data.begin(), data.end());
	return s.substr(0,2);
}

void setWindowSize(int width, int height){

}

void setFullScreen(bool fullScreen){

}

//using namespace Windows::Phone::Devices::Notification;
//VibrationDevice testVibrationDevice = VibrationDevice.GetDefault();
void vibrate()
{
	//testVibrationDevice.Vibrate(TimeSpan.FromSeconds(3));
}

void setKeepAwake(bool awake)
{

}

static int s_fps = 60;

extern "C" {

int g_getFps()
{
    return s_fps;
}

void g_setFps(int fps)
{
    s_fps = fps;
}

}

void g_exit()
{
	exit(0);
}