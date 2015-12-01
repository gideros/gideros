#include "pch.h"
#include "DirectXPage.xaml.h"
#include "giderosapi.h"
#include "AdsPubcenter.h"
using namespace Microsoft::Advertising::Mobile::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Core;
using namespace trianglexaml;

AdsPubcenter::AdsPubcenter()
{
	CoreWindow::GetForCurrentThread()->Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([]()
	{
		// Do stuff on the UI Thread
		AdControl^ ad = ref new AdControl();
		ad->AdUnitId = "10043055";
		ad->ApplicationId = "d25517cb-12d4-4699-8bdc-52040c712cab";
		ad->Height = 250;
		ad->Width = 300;
		ad->IsAutoRefreshEnabled = true;
		ad->AutoRefreshIntervalInSeconds = 60;
		ad->Margin = Windows::UI::Xaml::Thickness(35, 0, 0, 0);
		ad->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Left;
		ad->VerticalAlignment = Windows::UI::Xaml::VerticalAlignment::Top;
		gdr_getRootView()->Children->Append(ad);
	}));

}

AdsPubcenter::~AdsPubcenter()
{

}