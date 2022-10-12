#include <vector>
#include <string>

#include <stdlib.h>
#include <memory>
#include <concrt.h>
#include <ppltasks.h>
#include <giderosapi.h>
#include "gapplication.h"
#include "platform.h"

using namespace Concurrency;

using namespace Windows::System;
using namespace Platform;
using namespace Windows::Storage;

using namespace Windows::System::UserProfile;
using namespace Windows::Globalization;

using namespace Windows::ApplicationModel;
using namespace Windows::Security::ExchangeActiveSyncProvisioning;
using namespace Windows::System::Profile;

std::wstring utf8_ws(const char *str);
std::string utf8_us(const wchar_t *str);

std::vector<std::string> getDeviceInfo()
{
	std::vector<std::string> result;

	result.push_back("WinRT");

	std::wstring ws;
	AnalyticsVersionInfo ^ai = AnalyticsInfo::VersionInfo;
	ws = ai->DeviceFamily->Data();
	result.push_back(utf8_us(ws.c_str()));

	// get the system version number
	ws = ai->DeviceFamilyVersion->Data();
	long long v= strtoll(utf8_us(ws.c_str()).c_str(), NULL, 10);
	int v1 = (v & 0xFFFF000000000000L) >> 48;
	int v2 = (v & 0x0000FFFF00000000L) >> 32;
	int v3 = (v & 0x00000000FFFF0000L) >> 16;
	int v4 = (v & 0x000000000000FFFFL);
	char vs[120];
	sprintf_s(vs, 120, "%d.%d.%d.%d", v1, v2, v3, v4);
	result.push_back(std::string(vs));

	// get the device manufacturer and model name
	EasClientDeviceInformation ^eas = ref new EasClientDeviceInformation();
	ws = eas->SystemManufacturer->Data();
	result.push_back(std::string(ws.begin(), ws.end()));
	ws = eas->SystemProductName->Data();
	result.push_back(std::string(ws.begin(), ws.end()));

	// get the package architecure
	Package ^package = Package::Current;
	ws = package->Id->Architecture.ToString()->Data();
	result.push_back(std::string(ws.begin(), ws.end()));

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
	gdr_dispatchUi([&] {
		Launcher::LaunchUriAsync(uri);
	}, true);

	delete[] wcstring;
}

bool canOpenUrl(const char *url)
{
	return true;
}

std::string getLocale(){
	std::wstring data = GlobalizationPreferences::Languages->GetAt(0)->Data();
	return utf8_us(data.c_str());
}

std::string getLanguage()
{
	std::wstring data = GlobalizationPreferences::Languages->GetAt(0)->Data();
	std::string s=utf8_us(data.c_str());
	return s.substr(0,2);
}

std::string getAppId(){
	std::wstring data = Windows::ApplicationModel::Package::Current->Id->FamilyName->Data();
	return utf8_us(data.c_str());
}

void getSafeDisplayArea(int &x,int &y,int &w,int &h)
{
}

void setWindowSize(int width, int height){
#if WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
	gdr_dispatchUi([&] {
		Windows::UI::ViewManagement::ApplicationView^ view = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
		view->TryResizeView(Windows::Foundation::Size(width, height));
		}, true);
#endif
}

void setFullScreen(bool fullScreen){
#if WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
	gdr_dispatchUi([&] {
		Windows::UI::ViewManagement::ApplicationView^ view = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
		if (fullScreen)
			view->TryEnterFullScreenMode();
		else
			view->ExitFullScreenMode();
	}, true);
#endif
}

std::string getDeviceName(){
	std::wstring data = Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation().FriendlyName->Data();
	return utf8_us(data.c_str());
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
	gdr_dispatchUi([&] {
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
	}, true);
}

#if WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
bool setKeyboardVisibility(bool visible) {
	return false;
}
#else
bool setKeyboardVisibility(bool visible) {
	bool done = false;
	gdr_dispatchUi([&] {
		Windows::UI::ViewManagement::InputPane^ ip = Windows::UI::ViewManagement::InputPane::GetForCurrentView();
		if (visible)
			done = ip->TryShow();
		else
			done = ip->TryHide();
	}, true);
	return done;
}
#endif

bool setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText)
{
	return false;
}

int setClipboard(std::string data,std::string mimeType, int luaFunc) {
	Windows::ApplicationModel::DataTransfer::DataPackage^ dp = ref new Windows::ApplicationModel::DataTransfer::DataPackage;
	bool d = false;
	if (mimeType == "text/html") {
		dp->SetHtmlFormat(ref new String(utf8_ws(data.c_str()).c_str()));
		d = true;
	}
	if ((mimeType == "text/plain") || (mimeType == "")) {
		dp->SetText(ref new String(utf8_ws(data.c_str()).c_str()));
		d = true;
	}
	if (data == "")
		dp = nullptr;
	if (!d) return -1;

	gdr_dispatchUi([&] {
		Windows::ApplicationModel::DataTransfer::Clipboard::SetContent(dp);
	}, true);
	return 1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc) {
	if (luaFunc <0) return -1;
	Windows::ApplicationModel::DataTransfer::DataPackageView^ dv = nullptr;
	gdr_dispatchUi([&] {
		dv= Windows::ApplicationModel::DataTransfer::Clipboard::GetContent();
	}, true);
	if (dv != nullptr) {
		Platform::String^ type = Windows::ApplicationModel::DataTransfer::StandardDataFormats::Text;
		std::string res = "text/plain";
		if (mimeType=="text/html") {
			type = Windows::ApplicationModel::DataTransfer::StandardDataFormats::Html;
			res = mimeType;
		}
		if (dv->Contains(type)) {
			if (res=="text/html")
			create_task(dv->GetHtmlFormatAsync()).then([=](String^ cdata)
			{
				std::string data = utf8_us(cdata->Data());
				gapplication_clipboardCallback(luaFunc, 1,data.c_str() , res.c_str());
			});
			else //if (res=="text/plain")
				create_task(dv->GetTextAsync()).then([=](String^ cdata)
			{
				std::string data = utf8_us(cdata->Data());
				gapplication_clipboardCallback(luaFunc, 1, data.c_str(), res.c_str());
			});
			return 0;
		}
	}
	return -1;
}

int getKeyboardModifiers() {
	int m = 0;
	gdr_dispatchUi([&] {
		Windows::UI::Core::CoreWindow ^sender= Windows::UI::Core::CoreWindow::GetForCurrentThread();
		if (sender->GetKeyState(Windows::System::VirtualKey::Shift) != CoreVirtualKeyStates::None) m |= 1;
		if (sender->GetKeyState(Windows::System::VirtualKey::Menu) != CoreVirtualKeyStates::None) m |= 2;
		if (sender->GetKeyState(Windows::System::VirtualKey::Control) != CoreVirtualKeyStates::None) m |= 4;
	}, true);
	return m;
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

std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
	std::vector<gapplication_Variant> rets;
	gapplication_Variant r;
/*	if (!set) {
		if (!strcmp(what,"currentUrl"))
		{
			r.type=gapplication_Variant::STRING;
			r.s=currentUrl;
			rets.push_back(r);
		}
	}*/
	return rets;
}
