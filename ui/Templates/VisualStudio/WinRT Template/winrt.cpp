#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stack>
#include <string>

#include "giderosapi.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Platform;
using namespace Windows::Storage;

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

// ######################################################################
// the class definition for the core "framework" of our app
ref class App sealed : public IFrameworkView
{
    bool WindowClosed;
	ULONGLONG next_game_tick;

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
#endif
    }

    virtual void Load(String^ EntryPoint) {}

    virtual void Run()
    {

	  CoreWindow^ Window = CoreWindow::GetForCurrentThread();

	  std::wstring resourcePath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
	  std::wstring docsPath = ApplicationData::Current->LocalFolder->Path->Data();
	  bool isPlayer = false;

	  gdr_initialize(Window, Window->Bounds.Width, Window->Bounds.Height, isPlayer, resourcePath.c_str(), docsPath.c_str());

	  gdr_drawFirstFrame();
      
      const int TICK_PER_SECOND = 60;
      const int SKIP_TICKS = 1000 / TICK_PER_SECOND;
      const int MAX_FRAMESKIP = 10;
      
      next_game_tick = GetTickCount64();
      int loops;
      
      // repeat until window closes
      while(!WindowClosed){

		  loops = 0;
		  //	while (GetTickCount64() > next_game_tick && loops < MAX_FRAMESKIP) {
		  Window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

		  next_game_tick += SKIP_TICKS;
		  loops++;
		  //	} 

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
      next_game_tick = GetTickCount64();
	  gdr_resume();
    }

    void OnSizeChanged(CoreWindow ^sender, WindowSizeChangedEventArgs ^args)
    {
    }

    void PointerPressed(CoreWindow^ Window, PointerEventArgs^ Args)
    { 
	  if (Args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch) 
		  gdr_touchBegin(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, Args->CurrentPoint->PointerId);
	  else
		  gdr_mouseDown(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y);
    }

    void PointerReleased(CoreWindow^ Window, PointerEventArgs^ Args)
    {
		if (Args->CurrentPoint->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch)
			gdr_touchEnd(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y, Args->CurrentPoint->PointerId);
		else
			gdr_mouseUp(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y);
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
		else
			gdr_mouseUp(Args->CurrentPoint->Position.X, Args->CurrentPoint->Position.Y);
	}

	void KeyDown(CoreWindow^ Window, KeyEventArgs^ Args)
	{
		gdr_keyDown((int)Args->VirtualKey);
	}

	void KeyUp(CoreWindow^ Window, KeyEventArgs^ Args)
	{
		gdr_keyUp((int)Args->VirtualKey);
	}


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
