#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stack>
#include <string>

#include <xaudio2.h>

#include "ginput-winrt.h"
#include "luaapplication.h"
#include "platform.h"
#include "refptr.h"
#include <bytebuffer.h>
#include <event.h>
#include <application.h>
#include <keycode.h>
#include <gevent.h>
#include <ginput.h>
#include <glog.h>
#include "gpath.h"
#include "gvfs-native.h"
#include "ggeolocation.h"
#include "gapplication.h"
#include "gaudio.h"
#include "ghttp.h"

#include "giderosapi.h"

#include "dxcompat.hpp"
#include "dxglobals.h"
//#include "gstdio.h"

using namespace Microsoft::WRL;
using namespace DirectX;

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Popups;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Platform;
using namespace Windows::Storage;

extern bool dxcompat_force_lines;

IXAudio2 *g_audioengine;
IXAudio2MasteringVoice *g_masteringvoice;
IXAudio2SourceVoice* g_source;

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

// ######################################################################
// this function loads a file into an Array^

Array<byte>^ LoadShaderFile(std::string File){
  Array<byte>^ FileData = nullptr;

  // open the file    
  std::ifstream VertexFile(File, std::ios::in | std::ios::binary | std::ios::ate);

  // if open was successful
  if(VertexFile.is_open())    {
    // find the length of the file
    int Length = (int)VertexFile.tellg();

    // collect the file data
    FileData = ref new Array<byte>(Length);
    VertexFile.seekg(0, std::ios::beg);
    VertexFile.read(reinterpret_cast<char*>(FileData->Data), Length);
    VertexFile.close();
  }    
  return FileData;
}

//######################################################################
// this function initializes and prepares Direct3D for use
void InitD3D()
{ 

  // ----------------------------------------------------------------------
  // Create swap chain, get g_dev, g_devcon
  // ----------------------------------------------------------------------

  ComPtr<ID3D11Device> dev11;
  ComPtr<ID3D11DeviceContext> devcon11;

  D3D11CreateDevice(
		    nullptr,
		    D3D_DRIVER_TYPE_HARDWARE,
		    nullptr,
			D3D11_CREATE_DEVICE_DEBUG,
		    nullptr,
		    0,
		    D3D11_SDK_VERSION,
		    &dev11,
		    nullptr,
		    &devcon11);

  // Convert the pointers from Direct3D 11 to Direct3D 11.1
  dev11.As(&g_dev);
  devcon11.As(&g_devcon);

  // Obtain the DXGI factory
  // ComPtr<IDXGIDevice1> dxgiDevice;
  g_dev.As(&dxgiDevice);
  ComPtr<IDXGIAdapter> dxgiAdapter;
  dxgiDevice->GetAdapter(&dxgiAdapter);
  ComPtr<IDXGIFactory2> dxgiFactory;
  dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);
   
  // create a struct to hold information about the swap chain    
  DXGI_SWAP_CHAIN_DESC1 scd;    
  // clear out the struct for use    
  ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC1));    	

  // fill the swap chain description struct    
  scd.Width = 0;
  scd.Height = 0;
  scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  scd.Stereo = false;
  scd.SampleDesc.Count = 1;                               	// DISABLE ANTI-ALIASING
  scd.SampleDesc.Quality = 0;
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      	// how swap chain is to be used    
  scd.BufferCount = 2;                                    	// one back buffer for WP8 (Windows 8: 2)
  //scd.Scaling = DXGI_SCALING_STRETCH;                       // WP8 (Windows 8: not set)
  scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;                // WP8 (Windows 8: DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL)
  scd.Flags = 0;

  CoreWindow^ Window=CoreWindow::GetForCurrentThread();

  dxgiFactory->CreateSwapChainForCoreWindow(
					    g_dev.Get(),
					    reinterpret_cast<IUnknown*>(Window),
					    &scd,
					    nullptr,
					    &g_swapchain);
  
  // ----------------------------------------------------------------------
  // Setup back buffer, get g_backbuffer
  // ----------------------------------------------------------------------
  
  ID3D11Texture2D *pBackBuffer;
  g_swapchain->GetBuffer(0,__uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
  
  g_dev->CreateRenderTargetView(pBackBuffer,NULL,&g_backbuffer);
  pBackBuffer->Release();
  
  g_devcon->OMSetRenderTargets(1,&g_backbuffer,NULL);  // could call this "rendertarget"

  // ----------------------------------------------------------------------
  // Setup viewport, will also be reset using glViewport commands
  // ----------------------------------------------------------------------
  
  D3D11_VIEWPORT viewport;
  ZeroMemory(&viewport,sizeof(D3D11_VIEWPORT));

  float scaley;

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
  DisplayInformation ^dinfo = DisplayInformation::GetForCurrentView();
  scaley = dinfo->RawPixelsPerViewPixel; // Windows phone
#else
  scaley = 1.0f;   // Windows 8 PC
#endif

  float basex = 0;
  float basey = 0;
  float windoww = Window->Bounds.Width;  // default values means stretch to fit full screen
  float windowh = Window->Bounds.Height;  // Lua can change later. Note that screenw/h are in scaled coords

  viewport.TopLeftX = basex*scaley;
  viewport.TopLeftY = basey*scaley;
  viewport.Width = windoww*scaley;  // Direct3D needs actual pixels
  viewport.Height = windowh*scaley;
  
  g_devcon->RSSetViewports(1,&viewport);

  // ----------------------------------------------------------------------
  // load and compile the two shaders    
  // Write erros to VC++ output
  // create g_pVS, g_pPS and initialise
  // ----------------------------------------------------------------------

  Array<byte>^ VSFile=LoadShaderFile("assets\\VertexShader.cso");
  Array<byte>^ PSFile=LoadShaderFile("assets\\PixelShader.cso");

  g_dev->CreateVertexShader(VSFile->Data, VSFile->Length, NULL, &g_pVS);    
  g_dev->CreatePixelShader (PSFile->Data, PSFile->Length, NULL, &g_pPS);

  g_devcon->VSSetShader(g_pVS, 0, 0);    
  g_devcon->PSSetShader(g_pPS, 0, 0);    

  // ----------------------------------------------------------------------
  // create the input layout object. Set g_pLayout. There are 3 entries
  // for position, colour and texture coord
  // ----------------------------------------------------------------------

  D3D11_INPUT_ELEMENT_DESC ied[] =    {        
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},        
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}, 
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}
  };    

  g_dev->CreateInputLayout(ied, 3, VSFile->Data, VSFile->Length, &g_pLayout);    
  g_devcon->IASetInputLayout(g_pLayout);

  // ----------------------------------------------------------------------
  // Create vertex buffer object g_pVBuffer. Data from this will be passed
  // through to vertex shader, vertex by vertex. We will Map/Unmap this
  // to tell D3D what to draw
  // ----------------------------------------------------------------------

  D3D11_BUFFER_DESC bd;    
  ZeroMemory(&bd, sizeof(bd));

  bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU    
  bd.ByteWidth = sizeof(VERTEX) * 256;             // size is the VERTEX struct * 256
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer    
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer    

  g_dev->CreateBuffer(&bd, NULL, &g_pVBuffer);       // create the buffer    

  // ----------------------------------------------------------------------
  // Create a constant buffer (NB must be multiple of 16 bytes)
  // Important field is use_tex. If 1, pixel shader will use current texture
  // UpdateSubresource fills with data and SetConstantBuffer xfers to shader
  // ----------------------------------------------------------------------

  D3D11_BUFFER_DESC bd2;
  ZeroMemory(&bd2, sizeof(bd2));

  bd2.Usage=D3D11_USAGE_DEFAULT;
  bd2.ByteWidth=sizeof(const_buffer);
  bd2.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
  bd2.CPUAccessFlags=0;

  struct const_buffer mycb;
  mycb.use_tex=0;

  HRESULT hr;
  hr=g_dev->CreateBuffer(&bd2,NULL,&g_CB);

  g_devcon->UpdateSubresource(g_CB,0,nullptr,&mycb,0,0);
  g_devcon->PSSetConstantBuffers(0, 1, &g_CB);

  // ----------------------------------------------------------------------
  // Blend state
  // ----------------------------------------------------------------------

  D3D11_BLEND_DESC blendStateDesc;
  ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

  blendStateDesc.AlphaToCoverageEnable = FALSE;
  blendStateDesc.IndependentBlendEnable = FALSE;
  blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
  blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
  blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
  blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  g_dev->CreateBlendState(&blendStateDesc, &g_pBlendState);
  g_devcon->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFF);

  // ----------------------------------------------------------------------
  // Create a sampler, this interpolates textures in the Pixel Shader
  // This is just boiler plate
  // ----------------------------------------------------------------------

  D3D11_SAMPLER_DESC sampDesc;
  ZeroMemory( &sampDesc, sizeof(sampDesc) );
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

  g_dev->CreateSamplerState( &sampDesc, &g_samplerLinear );
  g_devcon->PSSetSamplers(0,1,&g_samplerLinear);  // only do this once

  // Set rasterizer state to switch off backface culling

  D3D11_RASTERIZER_DESC rasterDesc;

  rasterDesc.AntialiasedLineEnable = false;
  rasterDesc.CullMode = D3D11_CULL_NONE;
  rasterDesc.DepthBias = 0;
  rasterDesc.DepthBiasClamp = 0;
  rasterDesc.DepthClipEnable = true;
  rasterDesc.FillMode = D3D11_FILL_SOLID;
  rasterDesc.FrontCounterClockwise = false;
  rasterDesc.MultisampleEnable = false;
  rasterDesc.ScissorEnable = false;
  rasterDesc.SlopeScaledDepthBias = 0.0f;

  ID3D11RasterizerState *m_rasterState;

  HRESULT result = g_dev->CreateRasterizerState(&rasterDesc, &m_rasterState);
  g_devcon->RSSetState(m_rasterState);

  // ----------------------------------------------------------------------
  // Finally do some "openGL" manipulations so that vertex coords are in
  // pixels in a 2D surface with origin at top-left 
  // ----------------------------------------------------------------------

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  float orient = 0, logicalw=320, logicalh=480;
  glRotatef(orient, 0.0,0.0,1.0);
  glOrtho(0.0, logicalw, logicalh, 0.0, -1.0, 1.0);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

// ######################################################################
// this is the function that cleans up Direct3D and COM
void CleanD3D()
{    
  // close and release all existing COM objects    
  g_pLayout->Release();
  g_pVS->Release();
  g_pPS->Release();
  g_pVBuffer->Release();
//  g_swapchain->Release();    
  g_backbuffer->Release();
//  g_dev->Release();    
//  g_devcon->Release();
  g_CB->Release();
  g_samplerLinear->Release();
}

// ######################################################################
void InitXAudio2()
{
	XAudio2Create(&g_audioengine);
	HRESULT hr = g_audioengine->CreateMasteringVoice(&g_masteringvoice);
}

// ######################################################################
void CleanXAudio2()
{
	g_audioengine->Release();
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

		const char* resourcePath = (const char*)Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
		const char* docsPath = (const char*)ApplicationData::Current->LocalFolder->Path->Data();;

      GStatus status;

	  dxcompat_force_lines = false;

	  InitD3D();
	  InitXAudio2();

	  GLuint zero;
	  glGenTextures(1, &zero);
	  glBindTexture(GL_TEXTURE_2D,zero);
	  assert(zero == 0);

	  glGenFramebuffers(1, &zero);
	  glBindFramebuffer(GL_FRAMEBUFFER, zero);
	  assert(zero == 0);

	  gdr_initialize(screenw, screenh, true, resourcePath, docsPath);

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

		  //g_application->enterFrame(&status);
		  gaudio_AdvanceStreamBuffers();

		  next_game_tick += SKIP_TICKS;
		  loops++;
		  //	} 

		  g_devcon->OMSetRenderTargets(1, &g_backbuffer, nullptr);
		  g_devcon->ClearRenderTargetView(g_backbuffer, backcol);

		  //	application_->clearBuffers();
		  //g_application->renderScene();   // optional argument deltaFrameCount
		  gdr_drawFrame();

		  g_swapchain->Present(1, 0);
      }

	  gdr_exitGameLoop();

      CleanD3D();
      CleanXAudio2();

      gaudio_Cleanup();
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
      
      dxgiDevice->Trim();
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
      
      ginputp_mouseDown(x,y,0);
    }

    void PointerReleased(CoreWindow^ Window, PointerEventArgs^ Args)
    {
      
      float xp = Args->CurrentPoint->Position.X;
      float yp = Args->CurrentPoint->Position.Y;
      
      float x, y;
      getStdCoords(xp, yp, x, y);
      
      ginputp_mouseUp(x,y,0);
    }

    void PointerMoved(CoreWindow^ Window, PointerEventArgs^ Args)
    {	
		if (Args->CurrentPoint->IsInContact){
			float xp = Args->CurrentPoint->Position.X;
			float yp = Args->CurrentPoint->Position.Y;

			float x, y;
			getStdCoords(xp, yp, x, y);

			ginputp_mouseMove(x, y);
		}
    }

	void KeyDown(CoreWindow^ Window, KeyEventArgs^ Args)
	{
		ginputp_keyDown((int)Args->VirtualKey);
	}

	void KeyUp(CoreWindow^ Window, KeyEventArgs^ Args)
	{
		ginputp_keyUp((int)Args->VirtualKey);
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
