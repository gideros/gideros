#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stack>
#include <string>
#include <glog.h>

#include "giderosapi.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Platform;
using namespace Windows::Storage;

float screenw, screenh;

void getStdCoords(float xp, float yp, float &x, float &y)
{
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP

	DisplayInformation ^dinfo = DisplayInformation::GetForCurrentView();
	DisplayOrientations Orientation = dinfo->CurrentOrientation;

	if (Orientation == DisplayOrientations::Portrait){
		x = xp;
		y = yp;
	}
	else if (Orientation == DisplayOrientations::Landscape){
		x = screenw - yp;
		y = xp;
	}
	else if (Orientation == DisplayOrientations::LandscapeFlipped){
		x = yp;
		y = screenh - xp;
	}
	else {
		x = screenw - xp;
		y = screenh - yp;
	}
#else
	x = xp;
	y = yp;
#endif
}

/*
* Mutex Functions
*/

#include "pthread.h"
//#define PTW32_DLLPORT
//#define PTW32_CDECL

int PTW32_CDECL pthread_mutex_init(pthread_mutex_t * mutex,
	const pthread_mutexattr_t * attr)
{
	return 0;
}

int PTW32_CDECL pthread_mutex_destroy(pthread_mutex_t * mutex)
{
	return 0;
}


int PTW32_CDECL pthread_mutex_lock(pthread_mutex_t * mutex)
{
	return 0;
}


int PTW32_CDECL pthread_mutex_timedlock(pthread_mutex_t * mutex,
	const struct timespec *abstime)
{
	return 0;
}


int PTW32_CDECL pthread_mutex_trylock(pthread_mutex_t * mutex)
{
	return 0;
}

int PTW32_CDECL pthread_mutex_unlock(pthread_mutex_t * mutex)
{
	return 0;
}

int PTW32_CDECL pthread_mutex_consistent(pthread_mutex_t * mutex)
{
	return 0;
}

int PTW32_CDECL pthread_create(pthread_t * tid,
	const pthread_attr_t * attr,
	void *(PTW32_CDECL *start) (void *),
	void *arg)
{
	return 0;
}

int PTW32_CDECL pthread_join(pthread_t thread,
	void **value_ptr)
{
	return 0;
}

extern "C"
{
	wchar_t htonl(wchar_t w)
	{
		return w;
	}

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

	  screenw = Window->Bounds.Width;
	  screenh = Window->Bounds.Height;

	  std::wstring resourcePath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
	  std::wstring docsPath = ApplicationData::Current->LocalFolder->Path->Data();

	  gdr_initialize(Window, screenw, screenh, false, resourcePath.c_str(), docsPath.c_str());

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

      float xp = Args->CurrentPoint->Position.X;
      float yp = Args->CurrentPoint->Position.Y;
      
      float x, y;
      getStdCoords(xp, yp, x, y);
      
      //ginputp_mouseDown(x,y,0);
    }

    void PointerReleased(CoreWindow^ Window, PointerEventArgs^ Args)
    {
      
      float xp = Args->CurrentPoint->Position.X;
      float yp = Args->CurrentPoint->Position.Y;
      
      float x, y;
      getStdCoords(xp, yp, x, y);
      
      //ginputp_mouseUp(x,y,0);
    }

    void PointerMoved(CoreWindow^ Window, PointerEventArgs^ Args)
    {	
		if (Args->CurrentPoint->IsInContact){
			float xp = Args->CurrentPoint->Position.X;
			float yp = Args->CurrentPoint->Position.Y;

			float x, y;
			getStdCoords(xp, yp, x, y);

			//ginputp_mouseMove(x, y);
		}
    }

	void KeyDown(CoreWindow^ Window, KeyEventArgs^ Args)
	{
		//ginputp_keyDown((int)Args->VirtualKey);
	}

	void KeyUp(CoreWindow^ Window, KeyEventArgs^ Args)
	{
		//ginputp_keyUp((int)Args->VirtualKey);
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
