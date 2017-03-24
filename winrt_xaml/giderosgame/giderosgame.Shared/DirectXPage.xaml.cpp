//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include "giderosapi.h"

using namespace giderosgame;

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

void getDirectoryListing(const char* dir, std::vector<std::string>* files, std::vector<std::string>* directories)
{
	files->clear();
	directories->clear();

	std::string dirstar;
	int dirlen = strlen(dir);
	if (dirlen > 0 && (dir[dirlen - 1] == '/' || dir[dirlen - 1] == '\\'))
		dirstar = std::string(dir, dirlen - 1);
	else
		dirstar = std::string(dir);
	std::wstring wsTmp(dirstar.begin(), dirstar.end());
	for (auto it = wsTmp.begin(); it != wsTmp.end(); it++)
		if ((*it) == '/')
			*it = '\\';

	String^ dirName = ref new String(wsTmp.c_str());
	StorageFolder ^fld = nullptr;
	task<int> work = create_task(StorageFolder::GetFolderFromPathAsync(dirName)).then([&](StorageFolder^ folder)
	{
		fld = folder;
		return fld->GetFilesAsync();
	}).then([&](task<IVectorView<StorageFile^>^> task) {
		for (auto it = task.get()->First(); it->HasCurrent; it->MoveNext())
		{
			StorageFile^ file = it->Current;
			std::wstring ws = file->Name->Data();
			std::string fn(ws.begin(), ws.end());
			files->push_back(fn);
		}
		return fld->GetFoldersAsync();
	}).then([&](task<IVectorView<StorageFolder^>^> task) {
		for (auto it = task.get()->First(); it->HasCurrent; it->MoveNext())
		{
			StorageFolder^ file = it->Current;
			std::wstring ws = file->Name->Data();
			std::string fn(ws.begin(), ws.end());
			directories->push_back(fn);
		}
		return 0;
	});
	
	while (!work.is_done())
		Sleep(1);
}

static int canvasWidth=480, canvasHeight=320;
static DisplayOrientations canvasOrientation;
static bool canvasUpdated=false;
static CoreDispatcher^ uiDispatcher;

extern "C" void gdr_dispatchUi(std::function<void()> func,bool wait)
{
	try {
	if (uiDispatcher->HasThreadAccess)
		func();
	else
	{
		Windows::Foundation::IAsyncAction ^action=uiDispatcher->RunAsync(CoreDispatcherPriority::Normal,
			ref new Windows::UI::Core::DispatchedHandler(func));
		if (wait)
			while (action->Status == Windows::Foundation::AsyncStatus::Started)
				Sleep(1); //XXX There must be better to do
	}
	}
	catch (Exception ^e)
	{
		__debugbreak();
	}
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
		m_coreInput->PointerWheelChanged += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnWheelChanged);
		// Begin processing input messages as they're delivered.
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &DirectXPage::OnKeyUp);
	window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &DirectXPage::OnKeyDown);
	window->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &DirectXPage::OnKeyChar);
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs^>(this, &DirectXPage::OnBackButtonPressed);
#endif

	// Run task on a dedicated high priority background thread.
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	std::wstring resourcePath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
	std::wstring docsPath = ApplicationData::Current->LocalFolder->Path->Data();
	std::wstring tempPath = ApplicationData::Current->TemporaryFolder->Path->Data();

	uiDispatcher = CoreWindow::GetForCurrentThread()->Dispatcher;
	StorageFile^ file = nullptr;
	try {
		String^ fileName = ref new String(L"Assets\\properties.bin");
		IAsyncOperation<StorageFile^> ^gfa = Windows::ApplicationModel::Package::Current->InstalledLocation->GetFileAsync(fileName);
		while (gfa->Status == AsyncStatus::Started)
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		file = gfa->GetResults();
	}
	catch (Exception^ e)
	{
		file = nullptr;
	}

	bool isPlayer = (file == nullptr);

	int sw = swapChainPanel->ActualWidth;
	int sh = swapChainPanel->ActualHeight;
	if (sw&&sh)
	{
		canvasHeight = sh;
		canvasWidth = sw;
	}
	canvasOrientation = DisplayInformation::GetForCurrentView()->CurrentOrientation;
	canvasUpdated = true;

	gdr_initialize(true, nullptr, swapChainPanel, canvasWidth, canvasHeight, isPlayer, resourcePath.c_str(), docsPath.c_str(), tempPath.c_str());
	gdr_drawFirstFrame();

	auto workItemHandler2 = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// Calculate the updated frame and render once per vertical blanking interval.
		while (action->Status == AsyncStatus::Started)
		{
//			Game.Update();
//			Game.Render();

			if (canvasUpdated)
			{
				canvasUpdated = false;
				int orientation = 0;
				switch (canvasOrientation)
				{
				case DisplayOrientations::Portrait: orientation = 0; break;
				case DisplayOrientations::Landscape: orientation = 1; break;
				case DisplayOrientations::PortraitFlipped: orientation = 2; break;
				case DisplayOrientations::LandscapeFlipped: orientation = 3; break;
				}
				gdr_resize(canvasWidth, canvasHeight, orientation);
			}
			try {
				gdr_drawFrame(true);
			}
			catch (Exception^ e)
			{
				__debugbreak();
			}
		}
		canvasWidth = 0;
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
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
		Windows::UI::ViewManagement::StatusBar::GetForCurrentView()->HideAsync(); //Hidden by default
#endif
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
	canvasOrientation = sender->CurrentOrientation;
	canvasUpdated = true;
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

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ Args)
{
	if (Args->CurrentPoint->IsInContact) {
		if (Args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
			gdr_touchMove(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, Args->CurrentPoint->PointerId);
		else
			gdr_mouseMove(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y);
	}
	else {
		gdr_mouseHover(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y);
	}
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ Args)
{
	if (Args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		gdr_touchEnd(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, Args->CurrentPoint->PointerId);
	else if (Args->CurrentPoint->Properties->IsLeftButtonPressed)
		gdr_mouseUp(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 1);
	else if (Args->CurrentPoint->Properties->IsRightButtonPressed)
		gdr_mouseUp(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 2);
	else if (Args->CurrentPoint->Properties->IsBarrelButtonPressed || Args->CurrentPoint->Properties->IsHorizontalMouseWheel || Args->CurrentPoint->Properties->IsMiddleButtonPressed)
		gdr_mouseUp(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 4);
	else
		gdr_mouseUp(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 0);
}

void DirectXPage::OnPointerLost(Object^ sender, PointerEventArgs^ Args)
{
	if (Args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
		gdr_touchCancel(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, Args->CurrentPoint->PointerId);
	else if (Args->CurrentPoint->Properties->IsLeftButtonPressed)
		gdr_mouseUp(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 1);
	else if (Args->CurrentPoint->Properties->IsRightButtonPressed)
		gdr_mouseUp(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 2);
	else if (Args->CurrentPoint->Properties->IsBarrelButtonPressed || Args->CurrentPoint->Properties->IsHorizontalMouseWheel || Args->CurrentPoint->Properties->IsMiddleButtonPressed)
		gdr_mouseUp(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 4);
	else
		gdr_mouseUp(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, 0);
}

void DirectXPage::OnKeyDown(CoreWindow^ sender, KeyEventArgs^ Args)
{
	Args->Handled = true;
	gdr_keyDown((int)Args->VirtualKey);
}

void DirectXPage::OnKeyUp(CoreWindow^ sender, KeyEventArgs^ Args)
{
	Args->Handled = true;
	gdr_keyUp((int)Args->VirtualKey);
}

void DirectXPage::OnKeyChar(CoreWindow^ sender, CharacterReceivedEventArgs^ Args)
{
	Args->Handled = true;
	char buf[16];
	memset(buf, 0, 16);
	wchar_t wc = Args->KeyCode;
	WideCharToMultiByte(CP_UTF8, 0, &wc, 1,
		buf, 15, NULL, NULL);
	gdr_keyChar(buf);
}

void DirectXPage::OnWheelChanged(Object^ sender, PointerEventArgs^ Args)
{
	gdr_mouseWheel(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, Args->CurrentPoint->Properties->MouseWheelDelta);
}

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
void DirectXPage::OnBackButtonPressed(Object^ sender, BackPressedEventArgs^ args)
{
	gdr_keyDown(301);
	gdr_keyUp(301);
	args->Handled = true;
}
#endif

using namespace Microsoft::WRL; 
#ifdef WINSTORE
extern ComPtr<IDXGISwapChain1> g_swapchain;             // the pointer to the swap chain interface (11.1)
#else
extern IDXGISwapChain *g_swapchain;             // the pointer to the swap chain interface
#endif

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
#ifdef WINSTORE
	if (g_swapchain)
	{
		DXGI_MATRIX_3X2_F inverseScale = { 0 };
		inverseScale._11 = 1.0f / sender->CompositionScaleX;
		inverseScale._22 = 1.0f / sender->CompositionScaleY;
		Microsoft::WRL::ComPtr<IDXGISwapChain2>  m_swapChain;
		g_swapchain.As(&m_swapChain);
		m_swapChain->SetMatrixTransform(&inverseScale);
	}
#endif
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	canvasWidth = e->NewSize.Width;
	canvasHeight = e->NewSize.Height;
	canvasUpdated = true;
}
