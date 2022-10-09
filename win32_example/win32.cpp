#include <iostream>
#include <windows.h>

#include "gl/glew.h"
#include "gl/glext.h"
#include "wglext.h"

#include <pluginmanager.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stack>
#include <string>
#include <direct.h>
#include <binder.h>
#include <libnetwork.h>
#include "ginput-win32.h"
#include "luaapplication.h"
#include "platform.h"
#include "refptr.h"
#include <bytebuffer.h>
#include <event.h>
#include <application.h>
#include <gui.h>
#include <keycode.h>
#include <gevent.h>
#include <ginput.h>
#include <glog.h>
#include "gpath.h"
#include <gfile.h>
#include <gfile_p.h>
#include "gvfs-native.h"
#include "ggeolocation.h"
#include "gapplication.h"
#include "gapplication-win32.h"
#include "gaudio.h"
#include "ghttp.h"
#include "orientation.h"
#include "applicationmanager.h"
#include <map>
#include <screen.h>

extern "C" {
  void g_setFps(int);
  int g_getFps();
  void setWin32Stuff(HINSTANCE hInst, HWND hwnd);
}

#define ID_TIMER   1

HWND hwndcopy;

char commandLine[256];
// int dxChrome,dyChrome;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;

static bool drawok=false;
static int vsyncVal=0;
static HDC hDC;
static HGLRC hRC;

// ######################################################################

static void luaError(const char *error)
{
  glog_e("%s",error);
  exit(1);
}

// ######################################################################

static void loadPlugins()
{
  static char fullname[MAX_PATH];
  WIN32_FIND_DATA fd; 
  HANDLE hFind = FindFirstFile("plugins\\*.dll", &fd); 

  if(hFind != INVALID_HANDLE_VALUE) { 
    do { 
      // read all (real) files in current folder
      // , delete '!' read other 2 default folder . and ..
      if (! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

	strcpy(fullname,"plugins\\");
	strcat(fullname,fd.cFileName);
	printf("found DLL: %s\n",fullname);

	HMODULE hModule = LoadLibrary(fullname);
	void* plugin = (void*)GetProcAddress(hModule,"g_pluginMain");
	if (plugin)
	  PluginManager::instance().registerPlugin((void*(*)(lua_State*, int))plugin);
      }
    } while(FindNextFile(hFind, &fd)); 
    FindClose(hFind); 
  } 
}

// ######################################################################

static void printFunc(const char *str, int len, void *data)
{
  printf("%s",str);
}

// ######################################################################

std::string getDeviceName()
{

  static char buf[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD dwCompNameLen = MAX_COMPUTERNAME_LENGTH;
  std::string name;

  if (GetComputerName(buf, &dwCompNameLen) != 0) {
    name=buf;
  }

  return name;
}

// ######################################################################

// ### SCREENS
class W32Screen;
LRESULT CALLBACK W32Proc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static std::map<HWND,W32Screen *> screenMap;

class W32ScreenManager : public ScreenManager {
public:
	HGLRC master;
	HDC defaultDC;
	HINSTANCE hInstance;
	W32ScreenManager(HDC dc,HGLRC gl,HINSTANCE hInstance);
	virtual Screen *openScreen(Application *application,int id);
	virtual void screenDestroyed();
};

class W32Screen : public Screen {
	virtual void tick();
	HDC dc;
	HWND wnd;
	W32FullScreen fs;
protected:
	virtual void setVisible(bool);
public:
	virtual void setSize(int w,int h);
	virtual void getSize(int &w,int &h);
	virtual void setPosition(int w,int h);
	virtual void getPosition(int &w,int &h);
	virtual void setState(int state);
	virtual int getState();
	virtual void getMaxSize(int &w,int &h);
	virtual int getId();
	virtual void closed();
	W32Screen(Application *application,HINSTANCE hInstance);
	~W32Screen();
};

void W32Screen::tick()
{
	if (!wnd) return;
	if (IsWindowVisible(wnd))
	{
		Matrix4 m;
		W32ScreenManager *sm=((W32ScreenManager *)(ScreenManager::manager));
		wglMakeCurrent(dc,sm->master);
		glBindFramebuffer(GL_FRAMEBUFFER, 0/*defaultFramebufferObject()*/);
		draw(m);
	    SwapBuffers(dc);
		wglMakeCurrent(NULL,NULL);
	}
}

void W32Screen::setSize(int w,int h)
{
	if (!wnd) return;
    RECT rect;
    rect.top=0;
    rect.left=0;
    rect.right=w;
    rect.bottom=h;

    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
    SetWindowPos(wnd,HWND_TOP,0,0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
}

void W32Screen::getSize(int &w,int &h)
{
	if (!wnd) return;
 	RECT rect;
	GetClientRect(wnd,&rect);
	w=rect.right-rect.left;
	h=rect.bottom-rect.top;
}

void W32Screen::setState(int state)
{
	if (!wnd) return;
	W32SetFullScreen((state&FULLSCREEN),wnd,&fs);
	if (state&MINIMIZED) ShowWindow(wnd,SW_SHOWMINIMIZED);
	if (state&MAXIMIZED) ShowWindow(wnd,SW_SHOWMAXIMIZED);
}

int W32Screen::getState()
{
	if (!wnd) return 0;
	int s=NORMAL;
	if (fs.isFullScreen) s=FULLSCREEN;
	else
	{
		if (IsIconic(wnd)) s=MINIMIZED;
		if (IsZoomed(wnd)) s=MAXIMIZED;
	}
	if (!IsWindowVisible(wnd)) s|=HIDDEN;
	if (wnd==0) s|=CLOSED;
	return s;
}

void W32Screen::getMaxSize(int &w,int &h)
{
    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(monitor_info);
    GetMonitorInfo(MonitorFromWindow(hwndcopy, MONITOR_DEFAULTTONEAREST),
                    &monitor_info);
	w=monitor_info.rcMonitor.right-monitor_info.rcMonitor.left;
	h=monitor_info.rcMonitor.bottom-monitor_info.rcMonitor.top;
}

void W32Screen::setPosition(int x,int y)
{
	if (!wnd) return;
    RECT rect;
    rect.top=x;
    rect.left=y;
    rect.right=x;
    rect.bottom=y;

    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
    SetWindowPos(wnd,HWND_TOP,rect.left,rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOSIZE);
}

void W32Screen::getPosition(int &x,int &y)
{
	if (!wnd) return;
	RECT rect;
	GetClientRect(wnd,&rect);
	x=rect.left;
	y=rect.top;
}

int W32Screen::getId()
{
	return 0;
}

void W32Screen::setVisible(bool visible)
{
	if (!wnd) return;
	ShowWindow(wnd,visible?SW_SHOW:SW_HIDE);
}

static ATOM W32Class=0;

W32Screen::W32Screen(Application *application,HINSTANCE hInstance) : Screen(application)
{
	if (!W32Class)
	{
	  WNDCLASSEX  wndclass;

	  wndclass.cbSize        = sizeof (wndclass) ;
	  wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	  wndclass.lpfnWndProc   = W32Proc ;
	  wndclass.cbClsExtra    = 0 ;
	  wndclass.cbWndExtra    = 0 ;
	  wndclass.hInstance     = hInstance ;
	  wndclass.hIcon         = NULL;
	  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	  wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
	  wndclass.lpszMenuName  = MAKEINTRESOURCE(100);
	  wndclass.lpszClassName = "GidW32Screen" ;
	  wndclass.hIconSm       = NULL ;

	  W32Class=RegisterClassEx (&wndclass) ;
	}

	  wnd = CreateWindow ("GidW32Screen",         // window class name
			       "",     // window caption
			       WS_OVERLAPPEDWINDOW,     // window style
			       0,           // initial x position
			       0,           // initial y position
			       100,           // initial x size
			       100,           // initial y size
			       NULL,                    // parent window handle
			       NULL,                    // window menu handle
			       hInstance,               // program instance handle
			       NULL) ;		             // creation parameters
	  dc=GetDC(wnd);
	  //printf("ScreenCreated:%d DC:%d\n",wnd,dc);
	  // set the pixel format for the DC
	  PIXELFORMATDESCRIPTOR pfd;
	  int format,i;
	  ZeroMemory( &pfd, sizeof( pfd ) );
	  pfd.nSize = sizeof( pfd );
	  pfd.nVersion = 1;
	  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	  pfd.iPixelType = PFD_TYPE_RGBA;
	  pfd.cColorBits = 24;
	  pfd.cDepthBits = 24;
	  pfd.cStencilBits = 8;
	  pfd.iLayerType = PFD_MAIN_PLANE;

	  format = ChoosePixelFormat( dc, &pfd );

	  SetPixelFormat( dc, format, &pfd );
	  screenMap[wnd]=this;
}

W32Screen::~W32Screen()
{
}

void W32Screen::closed()
{
	wnd=0;
}

LRESULT CALLBACK W32Proc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps ;
  if (iMsg==WM_CREATE){
	  return 0;
  }
  else if (iMsg==WM_PAINT){

    BeginPaint (hwnd, &ps) ;
    EndPaint (hwnd, &ps) ;
    return 0 ;
  }
  else if (iMsg==WM_CLOSE){
    DestroyWindow(hwnd);
    screenMap[hwnd]->closed();
    return 0;
  }
  else if (iMsg==WM_DESTROY){
    return 0 ;
  }

  return DefWindowProc(hwnd, iMsg, wParam, lParam) ;
}


W32ScreenManager::W32ScreenManager(HDC dc,HGLRC gl,HINSTANCE hInstance)
{
	defaultDC=dc;
	master=gl;
	this->hInstance=hInstance;
}

void W32ScreenManager::screenDestroyed()
{
}


Screen *W32ScreenManager::openScreen(Application *application,int id)
{
	return (Screen *)SendMessage(hwndcopy,WM_USER+0x10,0,(LPARAM)application);
}

// ######################################################################

void EnableOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC)
{
  PIXELFORMATDESCRIPTOR pfd;
  int format,i;
	
  // get the device context (DC)
  *hDC = GetDC( hWnd );
	
  // set the pixel format for the DC
  ZeroMemory( &pfd, sizeof( pfd ) );
  pfd.nSize = sizeof( pfd );
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  format = ChoosePixelFormat( *hDC, &pfd );

  SetPixelFormat( *hDC, format, &pfd );
	
  // create and enable the render context (RC)
  *hRC = wglCreateContext( *hDC );
  wglMakeCurrent( *hDC, *hRC );

  vsyncVal=0;
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");
    printf("wgl extensions=%s\n",_wglGetExtensionsStringEXT());
    
    if (strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") == NULL)
    {
      printf("Extension not found WGL_EXT_swap_control\n");
    }

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT"); 
    
    if (wglSwapIntervalEXT == NULL){
      printf("No wglSwapIntervalEXT, reverting to timer events\n");
      return;
    }

    if (strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control_tear") == NULL)
    	vsyncVal=1;
    else
    	vsyncVal=-1;
    vsyncVal=0;
    printf("VSYNC VAL:%d\n",vsyncVal);
    wglSwapIntervalEXT(vsyncVal);

}

// ######################################################################

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( hRC );
  ReleaseDC( hWnd, hDC );
}

// ######################################################################


// ######################################################################

static HINSTANCE hInst;
static ApplicationManager *s_applicationManager;
LuaApplication *application_;
static bool glChanged;
static int glWidth,glHeight;

void render()
{
	wglMakeCurrent(hDC,hRC);
	if (glChanged)
	{
		s_applicationManager->surfaceChanged(glWidth,glHeight,glWidth>glHeight?90:0);
		glChanged=false;
	}
    s_applicationManager->drawFrame();
    SwapBuffers(hDC);
    wglMakeCurrent(NULL,NULL);
    ScreenManager::manager->tick();
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
  static RECT clientRect,winRect;

  PAINTSTRUCT ps ;

  if (iMsg==WM_CREATE){

    EnableOpenGL(hwnd, &hDC, &hRC);

    if (glewInit()){
      printf("glewInit failed to initialise!\n");
      exit(1);
    }

    hwndcopy=hwnd;                  // for platform-win32

    loadPlugins();

    s_applicationManager=new ApplicationManager();
    application_=s_applicationManager->getApplication();

    s_applicationManager->surfaceCreated();
	ScreenManager::manager=new W32ScreenManager(hDC,hRC,hInst);
	wglMakeCurrent(NULL,NULL);
	drawok=true;

   return 0;
  }
  else if (iMsg==WM_SIZE){

    glWidth=LOWORD(lParam);
    glHeight=HIWORD(lParam);
    glChanged=true;
    //printf("WM_SIZE: %d x %d\n",glWidth,glHeight);
    return 0;
  }
  // allows large windows bigger than screen
  else if (iMsg==WM_GETMINMAXINFO) {
    DefWindowProc(hwnd, iMsg, wParam, lParam);
    MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
    pmmi->ptMaxTrackSize.x = 2000;
    pmmi->ptMaxTrackSize.y = 2000;
    return 0;
  }
  else if (iMsg==WM_LBUTTONDOWN){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseDown(LOWORD(lParam), HIWORD(lParam), 1,m);
    return 0;
  }
  else if (iMsg==WM_LBUTTONUP){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseUp(LOWORD(lParam), HIWORD(lParam), 1,m);
    return 0;
  }
  else if (iMsg==WM_RBUTTONDOWN){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseDown(LOWORD(lParam), HIWORD(lParam), 2,m);
    return 0;
  }
  else if (iMsg==WM_RBUTTONUP){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseUp(LOWORD(lParam), HIWORD(lParam), 2,m);
    return 0;
  }
  else if (iMsg==WM_MBUTTONDOWN){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseDown(LOWORD(lParam), HIWORD(lParam), 4,m);
    return 0;
  }
  else if (iMsg==WM_MBUTTONUP){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseUp(LOWORD(lParam), HIWORD(lParam), 4,m);
    return 0;
  }
  else if (iMsg==WM_MOUSEMOVE){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
	  int b=0;
	  if (wParam&MK_LBUTTON) b|=1;
	  if (wParam&MK_RBUTTON) b|=2;
	  if (wParam&MK_MBUTTON) b|=4;
	  if (b)
		  ginputp_mouseMove(LOWORD(lParam), HIWORD(lParam),b,m);
	  else
		  ginputp_mouseHover(LOWORD(lParam), HIWORD(lParam),b,m);
    return 0;
  }
  else if (iMsg==WM_MOUSEWHEEL){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseWheel(LOWORD(lParam), HIWORD(lParam), 0,GET_WHEEL_DELTA_WPARAM(wParam),m);
    return 0;
  }
  else if (iMsg==WM_KEYDOWN){
	  int m=0;
	  if (GetKeyState(VK_CONTROL)) m|=GINPUT_CTRL_MODIFIER;
	  if (GetKeyState(VK_SHIFT)) m|=GINPUT_SHIFT_MODIFIER;
	  if (GetKeyState(VK_MENU)) m|=GINPUT_ALT_MODIFIER;
    ginputp_keyDown(wParam,m);
    return 0;
  }
  else if (iMsg==WM_KEYUP){
	  int m=0;
	  if (GetKeyState(VK_CONTROL)) m|=GINPUT_CTRL_MODIFIER;
	  if (GetKeyState(VK_SHIFT)) m|=GINPUT_SHIFT_MODIFIER;
	  if (GetKeyState(VK_MENU)) m|=GINPUT_ALT_MODIFIER;
    ginputp_keyUp(wParam,m);
    return 0;
  }
  else if (iMsg==WM_CHAR){
	char sc[4];
	char *obuf=sc;
	int uni=wParam;
	 if (uni<0x80)
	  *(obuf++)=uni;
	 else
	 {
	  if (uni<0x800)
	  {
	   *(obuf++)=0xC0|(uni>>6);
	   *(obuf++)=0x80|(uni&0x3F);
	  }
	  else
	  {
	    *(obuf++)=0xE0|(uni>>12);
	    *(obuf++)=0x80|((uni>>6)&0x3F);
	    *(obuf++)=0x80|(uni&0x3F);
	  }
	 }
	 *(obuf++)=0;
    ginputp_keyChar(sc);
    return 0;
  }
  else if (iMsg==WM_PAINT){
    BeginPaint (hwnd, &ps) ;
    EndPaint (hwnd, &ps) ;
    return 0 ;
  }
  else if (iMsg==(WM_USER+0x10)) {
		return (LRESULT) new W32Screen(application_->getApplication(),hInst);
  }
  else if (iMsg==WM_CLOSE){
	    drawok=false;
	    Sleep(30);
		wglMakeCurrent(hDC,hRC);
    printf("WM_CLOSE Called\n");

      PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT"); 
    
      if (wglSwapIntervalEXT == NULL){
	printf("Error, no wglSwapIntervalEXT\n");
	exit(1);
      }
      wglSwapIntervalEXT(0);
    
    // application
    delete s_applicationManager;
    DisableOpenGL(hwnd, hDC, hRC);

    DestroyWindow(hwnd);
    return 0;
  }
  else if (iMsg==WM_DESTROY){
    printf("WM_DESTROY Called\n");
    PostQuitMessage (0) ;

    return 0 ;
  }

  return DefWindowProc(hwnd, iMsg, wParam, lParam) ;
}

// ######################################################################
static bool runRender=true;
DWORD WINAPI RenderMain(LPVOID lpParam)
{
	double ck=iclock();
	double ifrm=0;
	int fps=0;
	timeBeginPeriod(1);
	while (runRender)
	{
		if (g_getFps() != fps){
			fps=g_getFps();
			ifrm=(g_getFps()>0)?1.0/g_getFps():0;
		}
		double ct=iclock();
		if (drawok&&((ct-ck)>=ifrm))
		{
			render();
			ck=ct;
		}
		double mt=iclock();
		double wt=ifrm-(mt-ck);
		int wti=(wt*1000);
		if (wti>1)
		{
			Sleep(wti-1);
		}
	}
	timeEndPeriod(1);
	return 0;
}

// ######################################################################

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
  static char szAppName[] = "giderosGame" ;
  HWND        hwnd ;
  MSG         msg ;
  WNDCLASSEX  wndclass ;
  int ret;

  printf("szCmdLine=%s\n",szCmdLine);

  wndclass.cbSize        = sizeof (wndclass) ;
  wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
  wndclass.lpfnWndProc   = WndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = 0 ;
  wndclass.hInstance     = hInstance ;
  wndclass.hIcon         = NULL;
  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
  wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
  wndclass.lpszMenuName  = MAKEINTRESOURCE(100);
  wndclass.lpszClassName = szAppName ;
  wndclass.hIconSm       = NULL ;
  
  RegisterClassEx (&wndclass) ;

  hInst=hInstance;
  hwnd = CreateWindow (szAppName,         // window class name
		       "Gideros Win32",     // window caption
		       WS_OVERLAPPEDWINDOW,     // window style
		       0,           // initial x position
		       0,           // initial y position
		       320,           // initial x size
		       480,           // initial y size
		       NULL,                    // parent window handle
		       NULL,                    // window menu handle
		       hInstance,               // program instance handle
		       NULL) ;		             // creation parameters

  setWin32Stuff(hInstance,hwnd);

  // ----------------------------------------------------------------------
  // Create window
  // ----------------------------------------------------------------------

  ShowWindow (hwnd, iCmdShow) ;
  //  UpdateWindow (hwnd) ;

  HANDLE rThread=CreateThread(NULL,0,&RenderMain,NULL,0,NULL);
  while (GetMessage (&msg, NULL, 0, 0)) {
    TranslateMessage (&msg) ;
    DispatchMessage (&msg) ;
  }
  runRender=false;
  WaitForSingleObject(rThread, INFINITE);

  printf("program ends\n");
  return msg.wParam ;
}

