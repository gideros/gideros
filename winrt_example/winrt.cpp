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

#include "giderosapi.h"

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

// ######################################################################
// the class definition for the core "framework" of our app
ref class App sealed : public IFrameworkView
{
    bool WindowClosed;

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
#else
		HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs^>(this, &App::OnBackButtonPressed);   
#endif
    }

    virtual void Load(String^ EntryPoint) {}

    virtual void Run()
    {

	  CoreWindow^ Window = CoreWindow::GetForCurrentThread();

	  std::wstring resourcePath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
	  std::wstring docsPath = ApplicationData::Current->LocalFolder->Path->Data();
	  bool isPlayer = true;

	  gdr_initialize(Window, Window->Bounds.Width, Window->Bounds.Height, isPlayer, resourcePath.c_str(), docsPath.c_str());

	  gdr_drawFirstFrame();
            
      // repeat until window closes
      while(!WindowClosed){
		  gdr_drawFrame();
      }

	  gdr_exitGameLoop();

	  gdr_deinitialize();
    }
    
    virtual void Uninitialize() {}
            
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
