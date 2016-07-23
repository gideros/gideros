#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stack>
#include <string>
#include <vector>
#include <ppltasks.h>

#include "giderosapi.h"

using namespace concurrency;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
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
}

// ######################################################################
// the class definition for the core "framework" of our app
ref class App sealed : public IFrameworkView
{
    bool WindowClosed;
	std::wstring resourcePath;
	std::wstring docsPath;
	std::wstring tempPath;

public:
    virtual void Initialize(CoreApplicationView^ AppView)
    {
		AppView->Activated += ref new TypedEventHandler
			<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);
        CoreApplication::Suspending +=
            ref new EventHandler<SuspendingEventArgs^>(this, &App::Suspending);
        CoreApplication::Resuming +=
            ref new EventHandler<Object^>(this, &App::Resuming);
        WindowClosed = false;    // initialize to false
    }

    virtual void SetWindow(CoreWindow^ Window)
    {
		resourcePath= Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
		docsPath = ApplicationData::Current->LocalFolder->Path->Data();
		tempPath = ApplicationData::Current->TemporaryFolder->Path->Data();

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

		// false means "don't use XAML"
		gdr_initialize(false, Window, nullptr, Window->Bounds.Width, Window->Bounds.Height, isPlayer, resourcePath.c_str(), docsPath.c_str(), tempPath.c_str());

		Window->Closed += ref new TypedEventHandler
			<CoreWindow^, CoreWindowEventArgs^>(this, &App::Closed);
		Window->PointerPressed += ref new TypedEventHandler
			<CoreWindow^, PointerEventArgs^>(this, &App::PointerPressed);
		Window->PointerReleased += ref new TypedEventHandler
			<CoreWindow^, PointerEventArgs^>(this, &App::PointerReleased);
		Window->PointerMoved += ref new TypedEventHandler
			<CoreWindow^, PointerEventArgs^>(this, &App::PointerMoved);
		Window->PointerCaptureLost += ref new TypedEventHandler
			<CoreWindow^, PointerEventArgs^>(this, &App::PointerLost);

      
#if WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
		Window->SizeChanged += ref new TypedEventHandler
			<CoreWindow ^, WindowSizeChangedEventArgs ^>(this, &App::OnSizeChanged);

		Window->KeyDown += ref new TypedEventHandler
			<CoreWindow^, KeyEventArgs^>(this, &App::KeyDown);
		Window->KeyUp += ref new TypedEventHandler
			<CoreWindow^, KeyEventArgs^>(this, &App::KeyUp);
		Window->PointerWheelChanged += ref new TypedEventHandler
			<CoreWindow^, PointerEventArgs^>(this, &App::WheelChanged);
		Window->CharacterReceived += ref new TypedEventHandler
				<CoreWindow^, CharacterReceivedEventArgs^>(this, &App::KeyChar);
#else
		HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs^>(this, &App::OnBackButtonPressed);   
#endif
    }

    virtual void Load(String^ EntryPoint) {}

    virtual void Run()
    {
	  gdr_drawFirstFrame();
            
      // repeat until window closes
      while(!WindowClosed){
		  gdr_drawFrame(false);
      }

	  gdr_exitGameLoop();
    }
    
    virtual void Uninitialize()
	{
		gdr_deinitialize();
	}
            
    void OnActivated(CoreApplicationView^ CoreAppView, IActivatedEventArgs^ Args)
    {
      CoreWindow^ Window = CoreWindow::GetForCurrentThread();
      Window->Activate();
    } 

    void Closed(CoreWindow^ sender, CoreWindowEventArgs^ args)
    {
      WindowClosed = true;    // time to end the endless loop
    }

    void Suspending(Object^ Sender, SuspendingEventArgs^ Args) 
    {
      Windows::ApplicationModel::SuspendingDeferral^ deferral = Args->SuspendingOperation->GetDeferral();
	
      // Save application data
	  gdr_suspend();

	  deferral->Complete();
    }

    void Resuming(Object^ Sender, Object^ Args) 
    {
	  gdr_resume();
    }

    void OnSizeChanged(CoreWindow ^sender, WindowSizeChangedEventArgs ^args)
    {
		gdr_resize(args->Size.Width, args->Size.Height);
    }

    void PointerPressed(CoreWindow^ Window, PointerEventArgs^ Args)
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

    void PointerReleased(CoreWindow^ Window, PointerEventArgs^ Args)
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

    void PointerMoved(CoreWindow^ Window, PointerEventArgs^ Args)
    {	
		if (Args->CurrentPoint->IsInContact){
			if (Args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
				gdr_touchMove(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, Args->CurrentPoint->PointerId);
			else
				gdr_mouseMove(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y);
		}
		else{
			gdr_mouseHover(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y);
		}
    }

	void PointerLost(CoreWindow^ Window, PointerEventArgs^ Args)
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

	void KeyDown(CoreWindow^ Window, KeyEventArgs^ Args)
	{
		Args->Handled = true;
		gdr_keyDown((int)Args->VirtualKey);
	}

	void KeyUp(CoreWindow^ Window, KeyEventArgs^ Args)
	{
		Args->Handled = true;
		gdr_keyUp((int)Args->VirtualKey);
	}

	void KeyChar(CoreWindow^ Window, CharacterReceivedEventArgs^ Args)
	{
		Args->Handled = true;
		char buf[16];
		memset(buf,0,16);
		wchar_t wc=Args->KeyCode;
		WideCharToMultiByte(CP_UTF8,0,&wc,1,
				buf,15,	NULL,NULL);
		gdr_keyChar(buf);
	}

	void WheelChanged(CoreWindow^ Window, PointerEventArgs^ Args)
	{
		gdr_mouseWheel(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, Args->CurrentPoint->Properties->MouseWheelDelta);
	}
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	void OnBackButtonPressed(Object^ sender, BackPressedEventArgs^ args)
	{
		gdr_keyDown(301);
		gdr_keyUp(301);
		args->Handled = true;
	}
#endif

};

// ######################################################################

ref class AppSource sealed : IFrameworkViewSource
{
 public:
  virtual IFrameworkView^ CreateView()
  {
    return ref new App();
  }
};

[MTAThread] // define main as multi-threaded apartment function

int main(Array<String^>^ args)
{

  CoreApplication::Run(ref new AppSource());
  return 0;
}
