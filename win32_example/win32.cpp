#include <iostream>
#include <libnetwork.h>
#include <windows.h>
#include <windowsx.h>

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
#include <Shellscalingapi.h>

extern "C" {
  void g_setFps(int);
  int g_getFps();
  void setWin32Stuff(HINSTANCE hInst, HWND hwnd);
}

static std::wstring ws(const char *str)
{
    if (!str) return std::wstring();
    int sl=strlen(str);
    int sz = MultiByteToWideChar(CP_UTF8, 0, str, sl, 0, 0);
    std::wstring res(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, str, sl, &res[0], sz);
    return res;
}

static std::string us(const wchar_t *str)
{
    if (!str) return std::string();
    int sl=wcslen(str);
    int sz = WideCharToMultiByte(CP_UTF8, 0, str, sl, 0, 0,NULL,NULL);
    std::string res(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, str, sl, &res[0], sz,NULL,NULL);
    return res;
}

#define ID_TIMER   1

HWND hwndcopy;

std::string commandLine;
std::string PATH_Executable;
std::string PATH_Temp;
std::string PATH_Cache;
std::string PATH_AppName;

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
	std::wstring pluginDir=ws(PATH_Executable.c_str());
	pluginDir+=L"\\plugins\\";
	std::wstring pattern=pluginDir+L"*.dll";
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(pattern.c_str(), &fd);

	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
				std::wstring dll=pluginDir+fd.cFileName;
				wprintf(L"found DLL: %ls\n",dll.c_str());

				HMODULE hModule = LoadLibrary(dll.c_str());
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

  static wchar_t buf[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD dwCompNameLen = MAX_COMPUTERNAME_LENGTH;
  std::string name;

  if (GetComputerName(buf, &dwCompNameLen) != 0) {
    name=us(buf);
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
	bool closed_;
protected:
	virtual void setVisible(bool);
public:
	virtual void setSize(int w,int h);
	virtual void getSize(int &w,int &h);
	virtual void setPosition(int w,int h);
	virtual void getPosition(int &w,int &h);
	virtual void setState(int state);
	virtual int getState();
	virtual void setTitle(const char *title);
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
	if (!wnd) return CLOSED;
	int s=NORMAL;
	if (fs.isFullScreen) s=FULLSCREEN;
	else
	{
		if (IsIconic(wnd)) s=MINIMIZED;
		if (IsZoomed(wnd)) s=MAXIMIZED;
	}
	if (!IsWindowVisible(wnd)) s|=HIDDEN;
	if (closed_) s|=CLOSED;
	return s;
}

void W32Screen::setTitle(const char *title)
{
	SetWindowText(wnd,ws(title).c_str());
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
    rect.left=x;
    rect.top=y;
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
	if (visible) closed_=false;
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
	  wndclass.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(1));
	  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	  wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
	  wndclass.lpszMenuName  = MAKEINTRESOURCE(100);
	  wndclass.lpszClassName = L"GidW32Screen" ;
	  wndclass.hIconSm       = NULL ;

	  W32Class=RegisterClassEx (&wndclass) ;
	}

	  wnd = CreateWindow (L"GidW32Screen",         // window class name
			       L"",     // window caption
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

	  closed_=true;
	  SetPixelFormat( dc, format, &pfd );
	  screenMap[wnd]=this;
}

W32Screen::~W32Screen()
{
    DestroyWindow(wnd);
}

void W32Screen::closed()
{
	if (!wnd) return;
	closed_=true;
	setContent(NULL);
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
    if (s_applicationManager->drawFrame()) {
        SwapBuffers(hDC);
        wglMakeCurrent(NULL,NULL);
    }
    ScreenManager::manager->tick();
}

static bool mouseEntered=false;
static std::vector<int> touch_xs;
static std::vector<int> touch_ys;
static std::vector<int> touch_ids;
static std::vector<int> touch_types;
static std::vector<float> touch_pressures;

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
  else if (iMsg==WM_DPICHANGED)  {
        WORD g_dpi = HIWORD(wParam);
        //printf("DPI Changed:%d\n",g_dpi);
        RECT* const prcNewWindow = (RECT*)lParam;
        SetWindowPos(hwnd,
            NULL,
            prcNewWindow ->left,
            prcNewWindow ->top,
            prcNewWindow->right - prcNewWindow->left,
            prcNewWindow->bottom - prcNewWindow->top,
            SWP_NOZORDER | SWP_NOACTIVATE);
        return 0;
  }
  // allows large windows bigger than screen
/*  else if (iMsg==WM_GETMINMAXINFO) {
    DefWindowProc(hwnd, iMsg, wParam, lParam);
    MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
    pmmi->ptMaxTrackSize.x = 20000;
    pmmi->ptMaxTrackSize.y = 20000;
    return 0;
  }*/
  else if (iMsg==WM_LBUTTONDOWN){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
	  if (wParam&VK_MENU) m|=GINPUT_ALT_MODIFIER; // new 20221114 XXX
    ginputp_mouseDown(LOWORD(lParam), HIWORD(lParam), 1,m);
    return 0;
  }
  else if (iMsg==WM_LBUTTONUP){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
	  if (wParam&VK_MENU) m|=GINPUT_ALT_MODIFIER; // new 20221114 XXX
    ginputp_mouseUp(LOWORD(lParam), HIWORD(lParam), 1,m);
    return 0;
  }
  else if (iMsg==WM_RBUTTONDOWN){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
	  if (wParam&VK_MENU) m|=GINPUT_ALT_MODIFIER; // new 20221114 XXX
    ginputp_mouseDown(LOWORD(lParam), HIWORD(lParam), 2,m);
    return 0;
  }
  else if (iMsg==WM_RBUTTONUP){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
	  if (wParam&VK_MENU) m|=GINPUT_ALT_MODIFIER; // new 20221114 XXX
    ginputp_mouseUp(LOWORD(lParam), HIWORD(lParam), 2,m);
    return 0;
  }
  else if (iMsg==WM_MBUTTONDOWN){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
	  if (wParam&VK_MENU) m|=GINPUT_ALT_MODIFIER; // new 20221114 XXX
    ginputp_mouseDown(LOWORD(lParam), HIWORD(lParam), 4,m);
    return 0;
  }
  else if (iMsg==WM_MBUTTONUP){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
	  if (wParam&VK_MENU) m|=GINPUT_ALT_MODIFIER; // new 20221114 XXX
    ginputp_mouseUp(LOWORD(lParam), HIWORD(lParam), 4,m);
    return 0;
  }
  else if (iMsg==WM_MOUSEMOVE){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
	  if (wParam&VK_MENU) m|=GINPUT_ALT_MODIFIER; // new 20221114 XXX
	  int b=0;
	  if (wParam&MK_LBUTTON) b|=1;
	  if (wParam&MK_RBUTTON) b|=2;
	  if (wParam&MK_MBUTTON) b|=4;
	  if (!mouseEntered)
	  {
		  TRACKMOUSEEVENT lpEventTrack;

		  lpEventTrack.cbSize = sizeof( TRACKMOUSEEVENT );
		  lpEventTrack.dwFlags = TME_LEAVE;
		  lpEventTrack.dwHoverTime = 1;
		  lpEventTrack.hwndTrack = hwnd;

		  mouseEntered=true;
		  TrackMouseEvent(&lpEventTrack);
		  ginputp_mouseEnter(LOWORD(lParam), HIWORD(lParam),b,m);
	  }
	  if (b)
		  ginputp_mouseMove(LOWORD(lParam), HIWORD(lParam),b,m);
	  else
		  ginputp_mouseHover(LOWORD(lParam), HIWORD(lParam),b,m);
    return 0;
  }
  else if (iMsg==WM_MOUSELEAVE){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
	  if (wParam&VK_MENU) m|=GINPUT_ALT_MODIFIER;
      ginputp_mouseLeave(0,0,m);
      mouseEntered=false;
    return 0;
  }
  else if (iMsg==WM_MOUSEWHEEL){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
	  if (wParam&VK_MENU) m|=GINPUT_ALT_MODIFIER;
	  int b=0;
	  if (wParam&MK_LBUTTON) b|=1;
	  if (wParam&MK_RBUTTON) b|=2;
	  if (wParam&MK_MBUTTON) b|=4;
	  POINT pt;
	  pt.x = GET_X_LPARAM(lParam);
	  pt.y = GET_Y_LPARAM(lParam);
	  ScreenToClient(hwnd, &pt);
	  ginputp_mouseWheel(pt.x, pt.y, b, GET_WHEEL_DELTA_WPARAM(wParam),m); // new 20221114 XXX
    return 0;
  }
  else if ((iMsg==WM_POINTERUPDATE)||(iMsg==WM_POINTERENTER)||(iMsg==WM_POINTERLEAVE)||
		  (iMsg==WM_POINTERUP)||(iMsg==WM_POINTERDOWN)||(iMsg==WM_POINTERWHEEL)||
		  (iMsg==WM_POINTERCAPTURECHANGED)) {
	  int m=0;
	  int b=0;
	  if ((iMsg!=WM_POINTERWHEEL)&&(iMsg!=WM_POINTERCAPTURECHANGED)) {
		  if (IS_POINTER_FIRSTBUTTON_WPARAM(wParam)) b|=1;
		  if (IS_POINTER_SECONDBUTTON_WPARAM(wParam)) b|=2;
		  if (IS_POINTER_THIRDBUTTON_WPARAM(wParam)) b|=4;
	  }

	  POINT pt;
	  pt.x = GET_X_LPARAM(lParam);
	  pt.y = GET_Y_LPARAM(lParam);
	  ScreenToClient(hwnd, &pt);
	  int tid=GET_POINTERID_WPARAM(wParam);
	  bool contact=(iMsg==WM_POINTERCAPTURECHANGED)?false:IS_POINTER_INCONTACT_WPARAM(wParam);
	  float pressure=0;
	  int touchType=2;
	  POINTER_INPUT_TYPE ptype;
	  if (GetPointerType(tid,&ptype)) {
		  POINTER_INFO *pi=NULL;
		  POINTER_TOUCH_INFO pti;
		  POINTER_PEN_INFO ppi;
		  switch (ptype) {
		  case PT_TOUCH: {
			  if (GetPointerTouchInfo(tid,&pti)) {
				  pi=&pti.pointerInfo;
				  if (pti.touchMask&TOUCH_MASK_PRESSURE)
					  pressure=(1.f/512)*pti.pressure;
			  }
			  touchType=0; break;
		  }
		  case PT_PEN: {
			  if (GetPointerPenInfo(tid,&ppi)) {
				  pi=&ppi.pointerInfo;
				  if (ppi.penMask&PEN_MASK_PRESSURE)
					  pressure=(1.f/512)*ppi.pressure;
			  }
			  touchType=1; break;
		  }
		  }
		  if (pi) {
			  if (pi->dwKeyStates&POINTER_MOD_SHIFT) m|=1;
			  if (pi->dwKeyStates&POINTER_MOD_CTRL) m|=4;
		  }
	  }

	  int tloc=-1;
	  for (size_t i=0;i<touch_xs.size(); i++)
		  if (touch_ids[i]==tid) tloc=i;
	  bool pcontact=(tid>=0);
	  if (contact)
	  {
		  if (tloc<0) {
			  tloc=touch_xs.size();
			  touch_xs.push_back(pt.x);
			  touch_ys.push_back(pt.y);
			  touch_ids.push_back(tid);
			  touch_pressures.push_back(pressure);
			  touch_types.push_back(touchType);
		  }
		  else {
			  touch_xs[tloc]=pt.x;
			  touch_ys[tloc]=pt.y;
			  touch_pressures[tloc]=pressure;
		  }
	  }
	  else {
		  if (tloc>=0) {
			  touch_xs.erase(touch_xs.begin()+tloc);
			  touch_ys.erase(touch_ys.begin()+tloc);
			  touch_ids.erase(touch_ids.begin()+tloc);
			  touch_pressures.erase(touch_pressures.begin()+tloc);
			  touch_types.erase(touch_types.begin()+tloc);
			  tloc=-1;
		  }
	  }

	  if (iMsg==WM_POINTERCAPTURECHANGED)
	  {
		  if (pcontact)
			  ginputp_touchesCancel(pt.x,pt.y, tid, pressure, touchType, touch_xs.size(), touch_xs.data(), touch_ys.data(), touch_ids.data(), touch_pressures.data(), touch_types.data(),m,b);
	  }
	  else if (iMsg==WM_POINTERWHEEL)
		  ginputp_mouseWheel(pt.x,pt.y,b,GET_WHEEL_DELTA_WPARAM(wParam),m);
	  else if (iMsg==WM_POINTERUP)
		  ginputp_touchesEnd(pt.x,pt.y, tid, pressure, touchType, touch_xs.size(), touch_xs.data(), touch_ys.data(), touch_ids.data(), touch_pressures.data(), touch_types.data(),m,b);
	  else if (iMsg==WM_POINTERDOWN)
		  ginputp_touchesBegin(pt.x,pt.y, tid, pressure, touchType, touch_xs.size(), touch_xs.data(), touch_ys.data(), touch_ids.data(), touch_pressures.data(), touch_types.data(),m,b);
	  else if (iMsg==WM_POINTERENTER)
	      ginputp_mouseEnter(pt.x,pt.y,b,m);
	  else if (iMsg==WM_POINTERLEAVE)
	      ginputp_mouseLeave(pt.x,pt.y,m);
	  else if (iMsg==WM_POINTERUPDATE) {
		  if (contact)
			  ginputp_touchesMove(pt.x,pt.y, tid, pressure, touchType, touch_xs.size(), touch_xs.data(), touch_ys.data(), touch_ids.data(), touch_pressures.data(), touch_types.data(),m,b);
		  else
			  ginputp_mouseHover(pt.x,pt.y,b,m);
	  }
    return 0;
  }
  else if ((iMsg==WM_KEYDOWN)||(iMsg==WM_SYSKEYDOWN)) {
	  int m=0;
	  if (GetKeyState(VK_CONTROL)) m|=GINPUT_CTRL_MODIFIER;
	  if (GetKeyState(VK_SHIFT)) m|=GINPUT_SHIFT_MODIFIER;
	  if (GetKeyState(VK_MENU)) m|=GINPUT_ALT_MODIFIER;
    ginputp_keyDown(wParam,m);
    if ((iMsg==WM_KEYDOWN)||(wParam==VK_F10))
    	return 0;
  }
  else if ((iMsg==WM_KEYUP)||(iMsg==WM_SYSKEYUP)) {
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
	    printf("WM_CLOSE Called\n");
	    drawok=false;
	    Sleep(30);


		wglMakeCurrent(hDC,hRC);
      PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
      if (wglSwapIntervalEXT == NULL){
		printf("Error, no wglSwapIntervalEXT\n");
		exit(1);
      }
      wglSwapIntervalEXT(0);

	    s_applicationManager->stop();

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

static const char szAppName[256] = "WindowsDesktopTemplate" ;
static const char szAppTitle[256] = "Win32 Template App Name" ;
int WINAPI wWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR szCmdLine, int iCmdShow)
{
  commandLine=us(szCmdLine);
  PATH_AppName=szAppName;
  std::wstring wAppName=ws(szAppName);
  //Get standard paths
  {
	  wchar_t szDir[MAX_PATH]={0, };
	  GetModuleFileName(NULL, szDir, sizeof(szDir));
	  wchar_t * pEnd = wcsrchr(szDir, L'\\');
	  if (pEnd)
		*pEnd = L'\0';
	  PATH_Executable=us(szDir);
  }
  //Get standard paths
  {
	  PATH_Temp=us(_wgetenv(L"TEMP"));
	  PATH_Cache=PATH_Temp+"\\"+PATH_AppName;
	  CreateDirectory(ws(PATH_Cache.c_str()).c_str(),NULL);
  }

#if NTDDI_VERSION >= NTDDI_WINBLUE
  SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#else
  SetProcessDPIAware();
#endif

  HWND        hwnd ;
  MSG         msg ;
  WNDCLASSEX  wndclass ;
  int ret;

  wprintf(L"szCmdLine=%ls\n",szCmdLine);

  wndclass.cbSize        = sizeof (wndclass) ;
  wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
  wndclass.lpfnWndProc   = WndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = 0 ;
  wndclass.hInstance     = hInstance ;
  wndclass.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(1));
  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
  wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
  wndclass.lpszMenuName  = MAKEINTRESOURCE(100);
  wndclass.lpszClassName = wAppName.c_str();
  wndclass.hIconSm       = NULL ;
  
  RegisterClassEx (&wndclass) ;

  hInst=hInstance;
  hwnd = CreateWindow (wAppName.c_str(),         // window class name
		       ws(szAppTitle).c_str(),     // window caption
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
