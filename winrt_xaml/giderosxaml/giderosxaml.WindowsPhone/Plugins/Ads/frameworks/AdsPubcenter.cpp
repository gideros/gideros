#include "pch.h"
#include "DirectXPage.xaml.h"
#include "giderosapi.h"
#include "../ads.h"
#include "AdsPubcenter.h"
using namespace Microsoft::Advertising::Mobile::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Core;

std::string curSize = "250x250";

ref class AdsPubcenterListener{
public:
	void onAdRefresh(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args){
		gads_adReceived("pubcenter", curSize.c_str());
	}

	void onAdError(Platform::Object^ sender, Microsoft::Advertising::Mobile::Common::AdErrorEventArgs^ args){
		Platform::String^ errRT = args->Error.ToString();;
		std::wstring errW(errRT->Begin());
		std::string errStr(errW.begin(), errW.end());
		gads_adFailed("pubcenter", errStr.c_str(), curSize.c_str());
	}

	void onAdClick(Platform::Object^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ args){
		gads_adActionBegin("pubcenter", curSize.c_str());
	}
};

AdControl^ ad;
AdsPubcenterListener^ listener;

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

AdsPubcenter::AdsPubcenter()
{
	adSizes["160x600"] = std::pair<int, int>(160, 600);
	adSizes["250x250"] = std::pair<int, int>(250, 250);
	adSizes["300x250"] = std::pair<int, int>(300, 250);
	adSizes["300x600"] = std::pair<int, int>(300, 600);
	adSizes["728x90"] = std::pair<int, int>(728, 90);

	testUnits["160x600"] = "10043136";
	testUnits["250x250"] = "10043107";
	testUnits["300x250"] = "10043055";
	testUnits["300x600"] = "10043030";
	testUnits["728x90"] = "10043000";

}

AdsPubcenter::~AdsPubcenter()
{
	adSizes.clear();
	testUnits.clear();
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
	std::string size = curSize;
	if (params->value){
		size = params->value;
		++params;
	}

	if (!ad || size != curSize){

		std::string unit = "10043107";
		if (params->value)
			unit = params->value;

		ad = ref new AdControl();
		if (test && testUnits.find(size) != testUnits.end())
			ad->AdUnitId = StringFromAscIIChars(testUnits[size].c_str());
		else
			ad->AdUnitId = StringFromAscIIChars(unit.c_str());

		ad->ApplicationId = StringFromAscIIChars(key.c_str());

		if (adSizes.find(size) != adSizes.end()){
			ad->Width = adSizes[size].first;
			ad->Height = adSizes[size].second;
		}
		else{
			ad->Height = 250;
			ad->Width = 250;
		}

		ad->IsAutoRefreshEnabled = true;
		ad->AutoRefreshIntervalInSeconds = 60;
		ad->Margin = Windows::UI::Xaml::Thickness(0, 0, 0, 0);
		ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Left;
		ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Top;

		listener = ref new AdsPubcenterListener;
		ad->AdRefreshed += ref new Windows::Foundation::EventHandler<Windows::UI::Xaml::RoutedEventArgs^>(listener, &AdsPubcenterListener::onAdRefresh);
		ad->ErrorOccurred += ref new Windows::Foundation::EventHandler<Microsoft::Advertising::Mobile::Common::AdErrorEventArgs ^>(listener, &AdsPubcenterListener::onAdError);
		ad->IsEnabledChanged += ref new Windows::UI::Xaml::DependencyPropertyChangedEventHandler(listener, &AdsPubcenterListener::onAdClick);
		//ad->PublisherMessageEvent += ref new Windows::Foundation::EventHandler<Microsoft::Advertising::Mobile::Common::PublisherMessageEventArgs ^>(listener, &AdsPubcenterListener::onAdAction);
	}
}

void AdsPubcenter::showAd(gads_Parameter *params){
	loadAd(params);
	if (!ad->Parent){
		CoreWindow::GetForCurrentThread()->Dispatcher->RunAsync(
			CoreDispatcherPriority::Normal,
			ref new DispatchedHandler([&]()
		{
			gdr_getRootView()->Children->Append(ad);
			gads_adDisplayed("pubcenter", curSize.c_str());
		}));
	}
}

void AdsPubcenter::hideAd(const char* type){
	if (ad->Parent){
		CoreWindow::GetForCurrentThread()->Dispatcher->RunAsync(
			CoreDispatcherPriority::Normal,
			ref new DispatchedHandler([&]()
		{
			int cur = 0;
			for each (auto item in gdr_getRootView()->Children)
			{
				if (item == static_cast<UIElement^>(ad)){
					break;
				}
				cur++;
			}
			gdr_getRootView()->Children->RemoveAt(cur);
			gads_adDismissed("pubcenter", curSize.c_str());
		}));
	}
}

void AdsPubcenter::setAlignment(const char *hor, const char *verr){
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
}

void AdsPubcenter::setX(int x){
	if (ad){
		ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Left;
		ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Top;
		ad->Margin = Windows::UI::Xaml::Thickness(x, ad->Margin.Top, 0, 0);
	}
}

void AdsPubcenter::setY(int y){
	if (ad){
		ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Left;
		ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Top;
		ad->Margin = Windows::UI::Xaml::Thickness(ad->Margin.Left, y, 0, 0);
	}
}

int AdsPubcenter::getX(){
	if (ad)
		return ad->Margin.Left;
	return 0;
}

int AdsPubcenter::getY(){
	if (ad)
		return ad->Margin.Top;
	return 0;
}

int AdsPubcenter::getWidth(){
	if (ad)
		return ad->Width;
	return 0;
}

int AdsPubcenter::getHeight(){
	if (ad)
		return ad->Height;
	return 0;
}