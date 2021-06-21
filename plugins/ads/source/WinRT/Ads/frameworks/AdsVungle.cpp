#include "pch.h"
#include "DirectXPage.xaml.h"
#include "giderosapi.h"
#include "../ads.h"
#include "AdsVungle.h"

using namespace VungleSDK;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::ApplicationModel::Core;

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

AdsVungle::AdsVungle()
{
	state = ref new Platform::Collections::UnorderedMap<Platform::String^, AdVungleState^>;
	banner = nullptr; //TODO Handle banners
	sdkInstance = nullptr;
}

AdsVungle::~AdsVungle()
{
	//TODO Cleanup states
}

void AdsVungle::setKey(gads_Parameter *params){
	const char* appId = params->value; params++;
	if (appId&&(sdkInstance==nullptr))
	{
		const char *userId = params->value; params++;
		if (!userId)
			userId = "Player";
		state->Clear();
		sdkInstance = AdFactory::GetInstance(StringFromAscIIChars(appId));
		listeners= ref new AdsVungleListener;
		listeners->adv = this;
		//Register event handlers
		sdkInstance->OnAdPlayableChanged += ref new EventHandler<VungleSDK::AdPlayableEventArgs^>(listeners, &AdsVungleListener::OnOnAdPlayableChanged);
		sdkInstance->OnAdStart += ref new EventHandler<VungleSDK::AdEventArgs^>(listeners, &AdsVungleListener::OnAdStart);
		sdkInstance->OnAdEnd += ref new EventHandler<VungleSDK::AdEndEventArgs^>(listeners, &AdsVungleListener::OnAdEnd);
		sdkInstance->Diagnostic += ref new EventHandler<VungleSDK::DiagnosticLogEvent ^>(listeners, &AdsVungleListener::Diagnostic);
		sdkInstance->OnInitCompleted += ref new EventHandler<VungleSDK::ConfigEventArgs^>(listeners, &AdsVungleListener::OnInitCompleted);
	}
}

void AdsVungle::enableTesting(){
}

void AdsVungle::loadAd(gads_Parameter *params){
	if (!params->value) return;
	Platform::String ^stype = StringFromAscIIChars(params->value);
	AdVungleState^ adstate = nullptr;
	
	try { adstate = state->Lookup(stype); }
	catch (Platform::OutOfBoundsException^ e) { return; };//No Such AD

	if (adstate->loaded) return; //Already loaded
	gdr_dispatchUi([&]() {
		if (banner)
			banner->LoadBannerAd(stype,VungleBannerSizes::Banner_320x50);
		else
			sdkInstance->LoadAd(stype);
	}, true);
}

void AdsVungle::showAd(gads_Parameter *params){
	if (!params->value) return;
	Platform::String^ stype = StringFromAscIIChars(params->value);

	AdVungleState^ adstate = nullptr;
	try { adstate = state->Lookup(stype); }
	catch (Platform::OutOfBoundsException^ e) { return;  };//No Such AD

	if (banner)
		gdr_dispatchUi([&]() {
			gdr_getRootView()->Children->Append(banner);
			gads_adDisplayed("vungle", adstate->type.c_str());
	}, true);

	if (!adstate->loaded) {
		if (!adstate->toshow)
			loadAd(params); 
		adstate->toshow = true;
		return; //Will be shown when loaded
	}
	PlayAd(stype, adstate);
}

void AdsVungle::PlayAd(Platform::String^ stype, AdVungleState^ adstate)
{
	adstate->toshow = false;
	gdr_dispatchUi([&]() {
		if (banner)
			banner->PlayBannerAd(stype, VungleSDK::VungleBannerSizes::Banner_320x50);
		else {
			AdConfig^ ac = ref new AdConfig;
			sdkInstance->PlayAdAsync(ac, stype);
		}
	}, true);
}

void AdsVungle::hideAd(const char* type){
	if (!type) return;
	Platform::String^ stype = StringFromAscIIChars(type);

	AdVungleState^ adstate = nullptr;
	try { adstate = state->Lookup(stype); }
	catch (Platform::OutOfBoundsException^ e) { return; };//No Such AD

	if (banner)
		gdr_dispatchUi([&]() {
		if (banner->Parent)
		{
			int cur = 0;
			for each (auto item in gdr_getRootView()->Children)
			{
				if (item == static_cast<UIElement^>(banner)) {
					break;
				}
				cur++;
			}
			gdr_getRootView()->Children->RemoveAt(cur);
			//gads_adDismissed("vungle", adstate->type.c_str());
			banner->StopBannerAd();
		}
	}, true);
}

void AdsVungle::setAlignment(const char *hor, const char *verr){
		gdr_dispatchUi([=]()
		{
			VungleSDK::UI::VungleAdControl^ ad = banner;
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

void AdsVungle::setX(int x){
		gdr_dispatchUi([=]()
		{
			VungleSDK::UI::VungleAdControl^ ad = banner;
			if (ad) {
				ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Left;
				ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Top;
				ad->Margin = Windows::UI::Xaml::Thickness(x, ad->Margin.Top, 0, 0);
			}
		}, true);
}

void AdsVungle::setY(int y){
		gdr_dispatchUi([=]()
		{
			VungleSDK::UI::VungleAdControl^ ad = banner;
			if (ad) {
			ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Left;
			ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Top;
			ad->Margin = Windows::UI::Xaml::Thickness(ad->Margin.Left, y, 0, 0);
		}
		}, true);
}

int AdsVungle::getX(){
	int v = 0;
	VungleSDK::UI::VungleAdControl^ ad = banner;
	if (ad)
		gdr_dispatchUi([&]() { v = ad->Margin.Left; }, true);
	return v;
}

int AdsVungle::getY(){
	int v = 0;
	VungleSDK::UI::VungleAdControl^ ad = banner;
	if (ad)
		gdr_dispatchUi([&]() { v = ad->Margin.Top; }, true);
	return v;
}

int AdsVungle::getWidth(){
	int v = 0;
	VungleSDK::UI::VungleAdControl^ ad = banner;
	if (ad)
		gdr_dispatchUi([&]() { v = ad->Width; }, true);
	return v;
}

int AdsVungle::getHeight(){
	int v = 0;
	VungleSDK::UI::VungleAdControl^ ad = banner;
	if (ad)
		gdr_dispatchUi([&]() { v = ad->Height; }, true);
	return v;
}

void AdsVungleListener::Diagnostic(Platform::Object^ sender, VungleSDK::DiagnosticLogEvent^ e)
{
/*	std::stringstream dmess;
	size_t converted;
	const wchar_t* wcLevel = e->Level.ToString()->Data();
	char cLevel[512];
	wcstombs_s(&converted, cLevel, 512, wcLevel, 512);

	const wchar_t* wcType = e->Type.Name->Data();
	char cType[512];
	wcstombs_s(&converted, cType, 512, wcType, 512);

	const wchar_t* wcException = e->Exception.ToString()->Data();
	char cException[512];
	wcstombs_s(&converted, cException, 512, wcException, 512);

	const wchar_t* wcMessage = e->Message->Data();
	char cMessage[512];
	if (wcMessage)
		wcstombs_s(&converted, cMessage, 512, wcMessage, 512);
	else cMessage[0] = 0;


	dmess << std::endl << "Diagnostics: " << cLevel << " " << cType << " " << cException << " " << cMessage;

	OutputDebugStringA(dmess.str().c_str());*/
}

void AdsVungleListener::OnInitCompleted(Platform::Object^ sender, VungleSDK::ConfigEventArgs^ args)
{
	if (!args->Initialized) {
		gads_adFailed("vungle", "Initialization error","");
	}
	else {
		int pcount = args->Placements->Length;
		for (int i = 0; i < pcount; i++) {
			AdVungleState^ empty = ref new AdVungleState;
			empty->loaded = false;
			empty->toshow = false;
			const wchar_t* wcType = args->Placements[i]->ReferenceId->Data();
			char cType[512];
			size_t converted;
			wcstombs_s(&converted, cType, 512, wcType, 512);
			empty->type = cType;
			adv->state->Insert(args->Placements[i]->ReferenceId, empty);
		}
		gads_adsReady("vungle", true);
	}
}

// Event handler called when e->AdPlayable is changed
void AdsVungleListener::OnOnAdPlayableChanged(Platform::Object^ sender, VungleSDK::AdPlayableEventArgs^ args)
{
	//Run asynchronously on the UI thread
	CoreApplication::MainView->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler(
			[this, args]
	{
		// args->AdPlayable - true if an ad is available to play, false otherwise
		// args->Placement  - placement ID in string

		//Change IsEnabled property for each button
		bool adPlayable = args->AdPlayable;
		AdVungleState^ adstate = nullptr;
		Platform::String^ stype = args->Placement;

		try { adstate = adv->state->Lookup(stype); }
		catch (Platform::OutOfBoundsException^ e) { return; };//No Such AD
		if (adPlayable) {
			//args->

			gads_adReceived("vungle", adstate->type.c_str());
			adstate->loaded = true;

			if (adstate->toshow) {
				adv->PlayAd(stype, adstate);
			}
		}
		else
			adstate->loaded = false;

	}));
}

// Event Handler called before playing an ad
void AdsVungleListener::OnAdStart(Platform::Object^ sender, VungleSDK::AdEventArgs^ e)
{
	// e.Id        - Vungle app ID in string
	// e.Placement - placement ID in string
	AdVungleState^ adstate = nullptr;
	Platform::String^ stype = e->Placement;

	try { adstate = adv->state->Lookup(stype); }
	catch (Platform::OutOfBoundsException^ e) { return; };//No Such AD
	adstate->toshow = false;

	gads_adDisplayed("vungle", adstate->type.c_str());
}

// DEPRECATED - use SdkInstance_OnAdEnd() instead
void AdsVungleListener::OnVideoView(Platform::Object^ sender, VungleSDK::AdViewEventArgs^ e) { }

// Event handler called when the user leaves ad and control is return to the hosting app
void AdsVungleListener::OnAdEnd(Platform::Object^ sender, VungleSDK::AdEndEventArgs^ e)
{
	// e->Id                  - Vungle app ID in string
	// e->Placement           - placement ID in string
	// e->IsCompletedView     - true when 80% or more of the video was watched
	// e->CallToActionClicked - true when the user has clicked download button on end card
	// e->WatchedDuration     - duration of video watched
	// e->VideoDuration       - DEPRECATED

	AdVungleState^ adstate = nullptr;
	Platform::String^ stype = e->Placement;

	try { adstate = adv->state->Lookup(stype); }
	catch (Platform::OutOfBoundsException^ e) { return; };//No Such AD
	adstate->loaded = false;

	if (e->CallToActionClicked)
		gads_adActionBegin("vungle", adstate->type.c_str());

	gads_adDismissed("vungle", adstate->type.c_str());
	gads_adActionEnd("vungle", adstate->type.c_str());

	if (e->IsCompletedView)
		gads_adRewarded("vungle", adstate->type.c_str(),0);
}
