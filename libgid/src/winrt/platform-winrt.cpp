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

std::string getDeviceName(){
	std::wstring data = Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation().FriendlyName->Data();
	return std::string(data.begin(), data.end());
}

#if WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
void vibrate(int ms)
{
}
#else
using namespace Windows::Phone::Devices::Notification;
using namespace Windows::Foundation;
VibrationDevice^ vibrationDevice = VibrationDevice::GetDefault();
void vibrate(int ms)
{
	if (ms > 5000)
		ms = 5000;
	else if (ms < 0)
		ms = 0;
	if (vibrationDevice){
		struct TimeSpan ts;
		ts.Duration = ms * 10000;
		vibrationDevice->Vibrate(ts);
	}
}
#endif
using namespace Windows::System::Display;

DisplayRequest^ dispRequest;
bool requested = false;

void setKeepAwake(bool awake)
{
	if (!dispRequest){
		dispRequest = ref new DisplayRequest();
	}
	if (awake != requested){
		requested = !requested;
		if (awake)
			dispRequest->RequestActive();
		else
			dispRequest->RequestRelease();
	}
}

bool setKeyboardVisibility(bool visible){
	return false;
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
	Windows::ApplicationModel::Core::CoreApplication::Exit();
}

bool g_checkStringProperty(bool isSet, const char* what){
    return false;
}

void g_setProperty(const char* what, const char* arg){

}

const char* g_getProperty(const char* what, const char* arg){
	return "";
}
