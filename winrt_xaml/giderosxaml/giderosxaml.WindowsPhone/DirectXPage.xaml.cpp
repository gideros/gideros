//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include "giderosapi.h"

using namespace giderosxaml;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::UI::Popups;

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
using namespace Windows::Phone::UI::Input;
#endif

extern "C"
{
#ifdef _M_IX86
	uint32_t htonl(uint32_t val)
	{
		const uint32_t x = 0x12345678;

		if (*(uint8_t*)&x == 0x12)    // big-endian machine
			return val;

		// little-endian machine
		val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
		return (val << 16) | (val >> 16);
	}
#endif

	void ExitProcess(int i)
	{
	}
}

// This function is only needed for the player. Should be empty for export projects as it contains
// APIs not permitted in Windows Store apps (FindNextFileA etc)

//#include <Windows.h>
void getDirectoryListing(const char* dir, std::vector<std::string>* files, std::vector<std::string>* directories)
{
	files->clear();
	directories->clear();

	WIN32_FIND_DATAA ffd;
	HANDLE hFind;

	std::string dirstar;

	int dirlen = strlen(dir);
	if (dirlen > 0 && (dir[dirlen - 1] == '/' || dir[dirlen - 1] == '\\'))
		dirstar = std::string(dir) + "*";
	else
		dirstar = std::string(dir) + "/*";

	std::wstring wsTmp(dirstar.begin(), dirstar.end());

	hFind = FindFirstFileEx(wsTmp.c_str(), FINDEX_INFO_LEVELS::FindExInfoBasic, &ffd, FINDEX_SEARCH_OPS::FindExSearchNameMatch, nullptr, 0);

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
				continue;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			directories->push_back(ffd.cFileName);
		else
			files->push_back(ffd.cFileName);
	} while (FindNextFileA(hFind, &ffd) != 0);

	FindClose(hFind);
}


DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr)
{
	InitializeComponent();

	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	// Disable all pointer visual feedback for better performance when touching.
//	auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
//	pointerVisualizationSettings->IsContactFeedbackEnabled = false; 
//	pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;

	// Register our SwapChainPanel to get independent input pointer events
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		// Register for pointer events, which will be raised on the background thread.
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

		// Begin processing input messages as they're delivered.
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// Run task on a dedicated high priority background thread.
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	std::wstring resourcePath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
	std::wstring docsPath = ApplicationData::Current->LocalFolder->Path->Data();
	std::wstring tempPath = ApplicationData::Current->TemporaryFolder->Path->Data();
	bool isPlayer = false;

	gdr_initialize(true, nullptr, swapChainPanel, 480, 800, isPlayer, resourcePath.c_str(), docsPath.c_str(), tempPath.c_str());
	gdr_drawFirstFrame();

	auto workItemHandler2 = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// Calculate the updated frame and render once per vertical blanking interval.
		while (action->Status == AsyncStatus::Started)
		{
//			Game.Update();
//			Game.Render();
			gdr_drawFrame(true);
		}
	});

	// Run task on a dedicated high priority background thread.

	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler2, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

DirectXPage::~DirectXPage()
{
	// Stop rendering and processing events on destruction.
	m_coreInput->Dispatcher->StopProcessEvents();
	gdr_exitGameLoop();
	gdr_deinitialize();
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
//		m_main->StartRenderLoop();
	}
	else
	{
//		m_main->StopRenderLoop();
	}
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
}


void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
}

// Called when the app bar button is clicked.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
}

void DirectXPage::OnPointerPressed(Object ^sender, PointerEventArgs^ Args)
{
	if (Args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		gdr_touchBegin(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, Args->CurrentPoint->PointerId);
	else if (Args->CurrentPoint->Properties->IsLeftButtonPressed)
		gdr_mouseDown(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 1);
	else if (Args->CurrentPoint->Properties->IsRightButtonPressed)
		gdr_mouseDown(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 2);
	else if (Args->CurrentPoint->Properties->IsBarrelButtonPressed || Args->CurrentPoint->Properties->IsHorizontalMouseWheel || Args->CurrentPoint->Properties->IsMiddleButtonPressed)
		gdr_mouseDown(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 4);
	else
		gdr_mouseDown(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 0);
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
}
