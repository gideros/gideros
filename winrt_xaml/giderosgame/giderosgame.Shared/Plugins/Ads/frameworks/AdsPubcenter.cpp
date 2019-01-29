#include "pch.h"
#include "DirectXPage.xaml.h"
#include "giderosapi.h"
#include "../ads.h"
#include "AdsPubcenter.h"
using namespace Microsoft::Advertising::WinRT::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Core;

#define INTERSTITIAL "interstitial"

ref class AdsPubcenterListener sealed {
	bool toShow;
	Platform::String ^type;
	std::string stype;
	Platform::Object ^ad;
public:
	AdsPubcenterListener(Platform::String ^at, Platform::Object ^adc) {
		type = at;
		toShow = false;
		std::wstring fooW(type->Begin());
		std::string fooA(fooW.begin(), fooW.end());
		stype = fooA;
		ad = adc;
	}

	Platform::Object^ GetAd() { return ad; }
	void Show() {
		if (stype == INTERSTITIAL)
		{
			InterstitialAd ^iad = (InterstitialAd^)ad;
			gdr_dispatchUi([&]()
			{
				if (iad->State == InterstitialAdState::Ready)
				{
					iad->Show();
					gads_adDisplayed("pubcenter", stype.c_str());
				}
				else
					toShow = true;
			}, true);
		}
		else
		{
			gdr_dispatchUi([&]()
			{
				AdControl ^iad = (AdControl^)ad;
				if (!iad->Parent) {
					gdr_getRootView()->Children->Append(iad);
					gads_adDisplayed("pubcenter", stype.c_str());
				}
			}, true);
		}
	}

	void Hide() {
		if (stype == INTERSTITIAL)
		{
		}
		else
		{
			gdr_dispatchUi([&]()
			{
				AdControl ^iad = (AdControl^)ad;
				if (iad->Parent)
				{
					int cur = 0;
					for each (auto item in gdr_getRootView()->Children)
					{
						if (item == static_cast<UIElement^>(ad)) {
							break;
						}
						cur++;
					}
					gdr_getRootView()->Children->RemoveAt(cur);
					gads_adDismissed("pubcenter", stype.c_str());
				}
			}, false);
		}
	}

	void onAdRefresh(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args){
		gads_adReceived("pubcenter", stype.c_str());
	}

	void onAdError(Platform::Object^ sender, AdErrorEventArgs^ args){
		Platform::String^ errRT = args->ErrorMessage;
		std::wstring errW(errRT->Begin());
		std::string errStr(errW.begin(), errW.end());
		gads_adFailed("pubcenter", errStr.c_str(), stype.c_str());
	}

	void onAdClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args){
		gads_adActionBegin("pubcenter", stype.c_str());
	}

	void onAdReady(Object^ sender, Object^ e)
	{
		gads_adReceived("pubcenter", stype.c_str());
		if (toShow)
			Show();
		toShow = false;
	}

	// This is an event handler for the interstitial ad. It is triggered when the interstitial ad is cancelled.
	void onAdCancelled(Object^ sender, Object^ e)
	{
		gads_adDismissed("pubcenter", stype.c_str());
	}

	// This is an event handler for the interstitial ad. It is triggered when the interstitial ad has completed playback.
	void onAdCompleted(Object^ sender, Object^ e)
	{
		gads_adDismissed("pubcenter", stype.c_str());
		gads_adActionEnd("pubcenter", stype.c_str());
	}
};

static Platform::Collections::Map<Platform::String^, AdsPubcenterListener^> ^adMap = ref new Platform::Collections::Map<Platform::String^, AdsPubcenterListener^>();
static std::string curType;

Platform::String^ StringFromAscIIChars(const char* chars)
{
	size_t newsize = strlen(chars) + 1;
	wchar_t * wcstring = new wchar_t[newsize];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring, newsize, chars, _TRUNCATE);
	Platform::String^ str = ref new Platform::String(wcstring);
	delete[] wcstring;
	return str;
}

static AdControl^ getAdControl()
{
	if (curType== INTERSTITIAL)
		return nullptr;
	Platform::String ^stype = StringFromAscIIChars(curType.c_str());
	AdsPubcenterListener ^listener = nullptr;
	try { listener = adMap->Lookup(stype); }
	catch (Platform::OutOfBoundsException ^e) {};
	if (listener != nullptr)
		return (AdControl^) listener->GetAd();
	return nullptr;
}

AdsPubcenter::AdsPubcenter()
{
	adSizes["160x600"] = std::pair<int, int>(160, 600);
	adSizes["250x250"] = std::pair<int, int>(250, 250);
	adSizes["300x250"] = std::pair<int, int>(300, 250);
	adSizes["300x600"] = std::pair<int, int>(300, 600);
	adSizes["728x90"] = std::pair<int, int>(728, 90);
	adMap->Clear();
}

AdsPubcenter::~AdsPubcenter()
{
	hideAd("");
	adSizes.clear();
	adMap->Clear();
}

void AdsPubcenter::setKey(gads_Parameter *params){
	key = "d25517cb-12d4-4699-8bdc-52040c712cab";
	if (params->value)
		key = params->value;
}

void AdsPubcenter::enableTesting(){
	key = "d25517cb-12d4-4699-8bdc-52040c712cab";
	test = true;
}

void AdsPubcenter::loadAd(gads_Parameter *params){
	std::string size = INTERSTITIAL;
	if (params->value){
		size = params->value;
		++params;
	}

	curType	= size;

	std::string unit = "11389925"; //Test Unit
	if ((!test) && params->value)
		unit = params->value;

	Platform::String ^stype = StringFromAscIIChars(size.c_str());
	AdsPubcenterListener ^listener = nullptr;
	try { listener = adMap->Lookup(stype); } catch (Platform::OutOfBoundsException ^e) {};

	
		gdr_dispatchUi([&]() {

			if (size == INTERSTITIAL) {
				if (!listener) {
					interstitialAd = ref new InterstitialAd();
					AdsPubcenterListener^ listener = ref new AdsPubcenterListener(StringFromAscIIChars(size.c_str()), interstitialAd);
					interstitialAd->ErrorOccurred += ref new Windows::Foundation::EventHandler<AdErrorEventArgs^>(listener, &AdsPubcenterListener::onAdError);
					interstitialAd->AdReady += ref new Windows::Foundation::EventHandler<Platform::Object^>(listener, &AdsPubcenterListener::onAdReady);
					interstitialAd->Cancelled += ref new Windows::Foundation::EventHandler<Platform::Object^>(listener, &AdsPubcenterListener::onAdCancelled);
					interstitialAd->Completed += ref new Windows::Foundation::EventHandler<Platform::Object^>(listener, &AdsPubcenterListener::onAdCompleted);
					adMap->Insert(stype, listener);
				}
				interstitialAd->RequestAd(AdType::Video, StringFromAscIIChars(key.c_str()), StringFromAscIIChars(unit.c_str()));
				return;
			}
			if (!listener) {
				AdControl^ ad = ref new AdControl();
				ad->AdUnitId = StringFromAscIIChars(unit.c_str());

				ad->ApplicationId = StringFromAscIIChars(key.c_str());

				if (adSizes.find(size) != adSizes.end()) {
					ad->Width = adSizes[size].first;
					ad->Height = adSizes[size].second;
				}
				else {
					ad->Height = 250;
					ad->Width = 250;
				}

				ad->IsAutoRefreshEnabled = true;
				ad->AutoRefreshIntervalInSeconds = 60;
				ad->Margin = Windows::UI::Xaml::Thickness(0, 0, 0, 0);
				ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Left;
				ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Top;

				AdsPubcenterListener^ listener = ref new AdsPubcenterListener(StringFromAscIIChars(size.c_str()), ad);
				ad->AdRefreshed += ref new Windows::Foundation::EventHandler<Windows::UI::Xaml::RoutedEventArgs^>(listener, &AdsPubcenterListener::onAdRefresh);
				ad->ErrorOccurred += ref new Windows::Foundation::EventHandler<AdErrorEventArgs ^>(listener, &AdsPubcenterListener::onAdError);
				ad->IsEngagedChanged += ref new Windows::Foundation::EventHandler<Windows::UI::Xaml::RoutedEventArgs^>(listener, &AdsPubcenterListener::onAdClick);
				//ad->PublisherMessageEvent += ref new Windows::Foundation::EventHandler<Microsoft::Advertising::Mobile::Common::PublisherMessageEventArgs ^>(listener, &AdsPubcenterListener::onAdAction);
				adMap->Insert(stype, listener);
			}
		}, true);
}

void AdsPubcenter::showAd(gads_Parameter *params){
	loadAd(params);
	std::string size = "interstitial";
	if (params->value) {
		size = params->value;
		++params;
	}
	curType = size;
	Platform::String ^stype = StringFromAscIIChars(size.c_str());
	AdsPubcenterListener ^listener = nullptr;
	try { listener = adMap->Lookup(stype); }
	catch (Platform::OutOfBoundsException ^e) {};
	if (listener != nullptr)
		listener->Show();
}

void AdsPubcenter::hideAd(const char* type){
	curType = type;
	Platform::String ^stype = StringFromAscIIChars(type);
	AdsPubcenterListener ^listener = adMap->Lookup(stype);
	if (listener != nullptr)
		listener->Hide();
}

void AdsPubcenter::setAlignment(const char *hor, const char *verr){
		gdr_dispatchUi([=]()
		{
			AdControl^ ad = getAdControl();
			if (ad){
		ad->Margin = Windows::UI::Xaml::Thickness(0, 0, 0, 0);

		if (std::strcmp(hor, "left") == 0)
			ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Left;
		else if (std::strcmp(hor, "right") == 0)
			ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Right;
		else
			ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Center;

		if (std::strcmp(verr, "top") == 0)
			ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Top;
		else if (std::strcmp(verr, "bottom") == 0)
			ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Bottom;
		else
			ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Center;
	}
		}, true);
}

void AdsPubcenter::setX(int x){
		gdr_dispatchUi([=]()
		{
			AdControl^ ad = getAdControl();
			if (ad) {
				ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Left;
				ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Top;
				ad->Margin = Windows::UI::Xaml::Thickness(x, ad->Margin.Top, 0, 0);
			}
		}, true);
}

void AdsPubcenter::setY(int y){
		gdr_dispatchUi([=]()
		{
		AdControl^ ad = getAdControl();
		if (ad) {
			ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Left;
			ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Top;
			ad->Margin = Windows::UI::Xaml::Thickness(ad->Margin.Left, y, 0, 0);
		}
		}, true);
}

int AdsPubcenter::getX(){
	int v = 0;
	AdControl^ ad = getAdControl();
	if (ad)
		gdr_dispatchUi([&]() { v = ad->Margin.Left; }, true);
	return v;
}

int AdsPubcenter::getY(){
	int v = 0;
	AdControl^ ad = getAdControl();
	if (ad)
		gdr_dispatchUi([&]() { v = ad->Margin.Top; }, true);
	return v;
}

int AdsPubcenter::getWidth(){
	int v = 0;
	AdControl^ ad = getAdControl();
	if (ad)
		gdr_dispatchUi([&]() { v = ad->Width; }, true);
	return v;
}

int AdsPubcenter::getHeight(){
	int v = 0;
	AdControl^ ad = getAdControl();
	if (ad)
		gdr_dispatchUi([&]() { v = ad->Height; }, true);
	return v;
}
